/**
 * @file      scada.cpp
 * @brief     Определение функций класса внешней SCADA
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @author    Инженер-программист Савченко Владимир
 * @copyright © TRIOLCORP, 2015-2017
 *
 */

#include <iostream>
#include <iomanip>
#include <string.h>
#include <errno.h>
#include <fstream>
#include <modbus.h>
#include "globals.h"
#include "port.h"
#include "port.h"
#include "scada.h"

#ifdef NEED_SCADA_SAVELOG_IN_MODEM
#define NEED_SCADA_SAVELOG_IN_MODEM
#endif

#undef NEED_SCADA_SAVELOG_IN_MODEM

#ifndef SCADA_DEBUG_MODBUS
#define SCADA_DEBUG_MODBUS
#endif

#undef SCADA_DEBUG_MODBUS

uint8_t V7Scada::mKnownModbusFubctions[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x0F, 0x10, 0x11, 0x17, 0x2b, 0x68, 0x69, 0x14 };

V7Scada::V7Scada() :
		mThread(0), mpPort(0), mpScadaPort(0), /*mpPortToScada(0), mpPortToDevice(NULL),*/numOfDevices(
				0), mCntReceived(1), mCntReceivedFromDevice(0), mCntTransmittedToScada(
				0), mCntTransmittedToDevice(0) {
	PRINTDEBUG("[Scada]: Scada created")
}

V7Scada::~V7Scada()

{
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
	cntLog();
#endif
	delete mpScadaPort;

}

bool V7Scada::Init(const xmlpp::Node* pXMLNode, V7Port* pPort,
		std::vector<V7Device*> pDevices) {

	mpPort = dynamic_cast<V7Port*>(pPort);
	if (!mpPort) {
		return false;
	}

	const xmlpp::Element* scadaElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!scadaElement) {
		cerr << "[Scada::Init]: Element \"ScadaPort\" not found" << endl;
		return false;
	}

	mpScadaPort = new ScadaPort;
	mpScadaPort->Init(pXMLNode, mpPort->GetModem());

	numOfDevices = pDevices.size();

	for (std::vector<V7Device*>::const_iterator it = pDevices.begin();
			it != pDevices.end(); ++it) {
		if (!(*it)) {
			PRINTDEBUG(
					"[Scada::Init]: Unable to create the libmodbus context to Device!");
			return false;
		} else {
			V7DeviceModbus *tmpDev = dynamic_cast<V7DeviceModbus*>(*it);
			mpDevicePool.insert(
					std::pair<uint8_t, modbus_t*>(tmpDev->getDeviceAddress(),
							tmpDev->GetModbusContext()));
			clog << "\nPool get:   " << tmpDev->getDeviceAddress() << ",  "
					<< tmpDev->GetModbusContext() << endl;
		}
	}
	clog << "Scada port settings: "<< endl << *(mpScadaPort->getPortSettings()) << flush << endl;
	return true;
}

bool V7Scada::Start() {
#ifdef DEBUG
	clog << "[Scada]: Scada started" << endl;
#endif
	if (pthread_create(&mThread, NULL, V7Scada::ThreadFunc, (void*) this) == 0)
		return true;
	else
		return false;
}

bool V7Scada::Wait() {
	if (pthread_join(mThread, NULL) == 0)
		return true;
	else
		return false;
}


void V7Scada::Run() {
	uint8_t bufferSCADA[BUFFER_LENGTH] = { 0 };
	uint8_t *pScadaBuf = bufferSCADA;
	uint8_t bufferDevice[BUFFER_LENGTH] = { 0 };
	int length = 0;
	uint8_t func = 0;
	uint8_t addr = 0;
	const useconds_t waitPeriod = 300;
	EngineModbus *pScadaEngineModbus = mpScadaPort->getModbusEngine();
	pScadaEngineModbus->open();
	pScadaEngineModbus->connect();
	engine_backend_type backend = pScadaEngineModbus->getBackendMode();
	while (!endWorkFlag) {
		if (backend == TCP) {
			pSemaphoreTCP->Wait();
		}
		int r = pScadaEngineModbus->receiveDataFromPort(&pScadaBuf, length);
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
		++mCntReceived;
		if (mCntReceived % 1000 == 0) {
			cntLog();
		}
#endif
		if (r != 0) {
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			usleep(waitPeriod);
			continue; // данные не получали
		}
		func = pScadaEngineModbus->getModbusFunction();
		addr = pScadaEngineModbus->getModbusAddress();
		//отсеиваем 00 2b
		if (addr == 0 && func == 0x2b) {
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			usleep(waitPeriod);
			continue;
		}
		//групповая запись
		if (addr == 0 && (func == 0x06 || func == 0x10) /*func != 0x2b*/) {
			writePool(mpDevicePool, pScadaBuf, length);
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			usleep(waitPeriod);
			continue;
		}
		// отсеиваем не свои посылки
		if (!isDevIDinPool(addr)) {
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			usleep(waitPeriod);
			continue; // данные не получали
		}

		if (!checkModbusFunction(func, mKnownModbusFubctions,
				sizeof(mKnownModbusFubctions)
						/ sizeof(mKnownModbusFubctions[0]))) {
			cerr << "[Scada::Run] Uknown function: " << hex << (int) func << dec
					<< endl;
			pScadaEngineModbus->close();
			usleep(waitPeriod);
			if (!pScadaEngineModbus->connect()) {
				cerr << "[Scada::Run]: Unable to connect " << strerror(errno)
						<< endl;
			}
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			memset(bufferDevice, 0, BUFFER_LENGTH);
			continue;
		}
		/**
		 * Вписываем данные в порт оборудования,length-2 - срезаем crc, т.к.
		 * libmodbus сам подставит crc.
		 * Отлавливаем броадкаст и 0x2b, перенаправляем на первое устройство
		 * (для корректной работы УМКА3  СПО Триол)
		 */
		//! Запрос
		QueryMsgRaw *msg = new QueryMsgRaw(length, pScadaBuf);
		while (mpPort->isBusy()) {
			usleep(300);
		}
		// запрос к устройству
		int mbRes = mpPort->request(msg);
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
		++mCntTransmittedToDevice;
#endif
		if (mbRes != 0) {
			/* Connection closed by the client or error */
			PRINTDEBUG2(
					"[Scada::Run] Connection closed by the client or error on device. ",
					mbRes);
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
			stringstream tmp;
			tmp << mbRes;
			string res =
					"[Scada::Run] modbus_send_raw_request to device error: mbRes="
							+ tmp.str();
			errStamp(res.c_str());
#endif
			//tcp
			if (backend == TCP) {
				pScadaEngineModbus->sendRawRequest(0, mbRes);
				pSemaphoreScada->Post();
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
				++mCntTransmittedToScada;
#endif
			} else {
				pScadaEngineModbus->replyExeption(pScadaBuf, mbRes);
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
				++mCntTransmittedToScada;
#endif
			}
			memset(bufferSCADA, 0, BUFFER_LENGTH);
			memset(bufferDevice, 0, BUFFER_LENGTH);
			usleep(waitPeriod);
			continue;
		} else {
			length = msg->getResponseSize();
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
			++mCntReceivedFromDevice;
#endif
			if (addr != 0) {
				int r = pScadaEngineModbus->sendRawRequest(
						msg->getResponseBuffer(), length);
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
				++mCntTransmittedToScada;
#endif
				if (backend == TCP) {
					pSemaphoreScada->Post();
				}
				if (r != 0) {
					cerr << "Connection closed by the client or error SCADA"
							<< strerror(errno) << endl;
#ifdef NEED_SCADA_SAVELOG_IN_MODEM
					stringstream tmp;
					tmp << r;
					string res =
							"[Scada::Run] modbus_send_raw_request to SCADA error: mbRes="
									+ tmp.str();
					errStamp(res.c_str());
#endif
					memset(bufferSCADA, 0, BUFFER_LENGTH);
					memset(bufferDevice, 0, BUFFER_LENGTH);
					continue;
				}
			}
		}

		memset(bufferSCADA, 0, BUFFER_LENGTH);
		memset(bufferDevice, 0, BUFFER_LENGTH);
		delete msg;
		usleep(waitPeriod);
	}
}

void V7Scada::errStamp(const char * source) const {
	time_t now = time(NULL);
	std::ofstream err_stamp;
	if (bigbrother::getFileSize(MODBUS_ERR_FILE_PATH) <= 1000000) {
		err_stamp.open(MODBUS_ERR_FILE_PATH, std::ios_base::app);
	} else {
		err_stamp.open(MODBUS_ERR_FILE_PATH);
	}

	if (err_stamp.is_open()) {
		err_stamp << ctime(&now) << " : " << source << endl;
		err_stamp.close();
	}
}

void V7Scada::cntLog() {
	time_t now = time(NULL);

	if (bigbrother::getFileSize(CNT_PACKETS__FILE_PATH) <= 1000000) {
		mCntPacketsFile.open(CNT_PACKETS__FILE_PATH, std::ios_base::app);
	} else {
		mCntPacketsFile.open(CNT_PACKETS__FILE_PATH);
	}
	if (mCntPacketsFile.is_open()) {
		mCntPacketsFile << ctime(&now) << endl;
		mCntPacketsFile << "[Received from SCADA: " << mCntReceived - 1
				<< " Transmitted to device " << mCntTransmittedToDevice
				<< " Received from device: " << mCntReceivedFromDevice
				<< "  Transmitted to SCADA: " << mCntTransmittedToScada << " ]"
				<< endl;
		mCntPacketsFile.close();
	}

}

bool V7Scada::checkModbusFunction(uint8_t func, uint8_t *knownFunc,
		uint16_t sz) {
	for (uint16_t i = 0; i < sz; ++i) {
		if (func == knownFunc[i]) {
			return true;
		}
	}
	return false;
}

bool V7Scada::matchSlaveId(const uint8_t* buf, const uint id) {
	if (buf && (buf[0] == id)) {
		return true;
	}
	return false;
}

bool V7Scada::isDevIDinPool(const uint id) {
	if (mpDevicePool.find(id) != mpDevicePool.end()) {
		return true;
	} else {
		return false;
	}

}

void V7Scada::writePool(std::map<uint8_t, modbus_t*> mpDevicePool,
		uint8_t *cmdBuffer, uint8_t cmdBufferLen) {
	typedef std::map<uint8_t, modbus_t*>::iterator it_t;
	for (it_t it = mpDevicePool.begin(); it != mpDevicePool.end(); ++it) {
		cmdBuffer[0] = it->first;
		QueryMsgRaw *msg = new QueryMsgRaw(cmdBufferLen - 2, cmdBuffer);
		while (mpPort->isBusy()) {
			sleep(0);
		}
		int mbRes = mpPort->request(msg);
		delete msg;
	}

}
