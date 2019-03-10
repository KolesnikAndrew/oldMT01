/**
 * @file      Port.cpp
 * @brief     Определение функций класса порта модема
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "globals.h"
#include "devicemodbus.h"
#include "deviceShunt.h"
#include "devicemodemstat.h"
#include "Din16Dout8.h"
#include "device.h"
#include "port.h"
#include "scada.h"
#include "utilites.h"

using namespace std;

V7Port::V7Port() :
		mThread(0), mpModem(0), mpScada(0), mNecessaryToFree(false), mpPortSettings(
				0), mBackendType(RTU), mpModbusEngine(0), flagBusy(false), mpShunt(
				0), mpPortToScada(0), mTriesNumber(2) {
	pthread_mutex_init(&mMutexInputData, NULL);

	PRINTDEBUG("V7Port created");

}

V7Port::~V7Port() {
	while (!mvpDevices.empty()) {
		delete mvpDevices.back();
		mvpDevices.pop_back();
	}

	delete mpScada;
	delete mpShunt;
	delete mpPortSettings;
	delete mpModbusEngine;
	pthread_mutex_destroy(&mMutexInputData);

}

bool V7Port::Start() {

	PRINTDEBUG("V7Port::Start()");

	if (pthread_create(&mThread, NULL, V7Port::ThreadFunc, (void*) this) == 0) {
		if (mpScada) {
			mpScada->Start();
		}
		if (mpShunt) {
			mpShunt->Start();
		}
		return true;
	} else {
		return false;
	}
}

bool V7Port::Wait() {
	if (mpScada) {
		mpScada->Wait();
	}
	if (mpShunt) {
		mpShunt->Wait();
	}
	if (pthread_join(mThread, NULL) == 0) {
		return true;
	} else {
		return false;
	}
}

engine_backend_type V7Port::getEngineTypeConfig(
		const xmlpp::Element* portElement) {
	const Glib::ustring attrNames = "modbusBackend";
	const xmlpp::Attribute* mode = portElement->get_attribute(attrNames);
	if (!mode) {
		PRINTDEBUG(
				"[V7Port::getEngineTypeConfig] Set backend to RTU as default")
		return RTU;
	}
	if (mode->get_value().uppercase() == "RTU") {
		return RTU;
	} else if (mode->get_value().uppercase() == "TCP") {
		return TCP;
	}
//	else if (mode->get_value().uppercase() == "ASCII") {
//		//TODO not realized
//		return ASCII;
//	}
	else {
		//RTU is default backend
		return RTU;
	}

}

void V7Port::setPortSettings(PortSettings* ptr) {
	mpPortSettings = ptr;
}

PortSettings* V7Port::getPortSettings() const {
	return mpPortSettings;
}

bool V7Port::initRtuBackendConfig(const xmlpp::Element* portElement) {

	int line = portElement->get_line();
	const xmlpp::Attribute* attribute;
	const Glib::ustring attrName = "name";
	attribute = portElement->get_attribute(attrName);
	if (!attribute) {
		cerr << "Attribute \"" << attrName << "\" not found (line=" << line
				<< ") " << endl;
		return false;
	}
	mName = attribute->get_value();
	mpPortSettings->modbusBackend = RTU;
	dynamic_cast<RtuPortSettings*>(mpPortSettings)->portName = mName;
	PRINTDEBUG2("RTU: ", mName);
	return true;
}

bool V7Port::initTcpBackendConfig(const xmlpp::Element* portElement) {

	using namespace bigbrother;
	PRINTDEBUG("Init tcp backend");
	int line = portElement->get_line();
	//!Забираем атрибуты
	const short numParams = 6;
	const xmlpp::Attribute* attributes[numParams];
	const Glib::ustring attrNames[numParams] = { "name", "address", "port",
			"mode", "netmask", "gateway" };

	Glib::ustring attrValues[numParams];
	for (short i = 0; i < numParams; i++) {
		attributes[i] = portElement->get_attribute(attrNames[i]);
		if ((attributes[i] == 0) && i != 5) {
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
			//PRINTDEBUG(attrValues[i]);
		}
	}

	TcpPortSettings* pSet = dynamic_cast<TcpPortSettings*>(mpPortSettings);
	// name
	mName = attrValues[0];
	cout << ">>>>>>>>>>>>>>>>>>>>>>" << mName << endl;
	// ip-address
	std::string tmp_addr = attrValues[1];
	if (ipValidate(tmp_addr)) {

		pSet->netAddress = tmp_addr;
	} else {
		PRINTDEBUG("[TCP port] IP-address set to default!");
		pSet->netAddress = "10.0.0.253";
	}
	// ip-port
	int tmp;
	std::istringstream(attrValues[2]) >> tmp;
	if (tmp > 501 && tmp <= 65535) {
		pSet->servicePort = tmp;
	} else {
		pSet->servicePort = 502;
	}
	// mode
	if (toLowerCase(attrValues[3]) == "master"
			|| toLowerCase(attrValues[3]) == "slave") {
		pSet->modbusMode = attrValues[3];
	} else {
		pSet->modbusMode = "master";
	}
	// netmask
	tmp_addr = attrValues[4];
	if (ipValidate(tmp_addr)) {
		pSet->netMask = tmp_addr;
	} else {
		PRINTDEBUG("[TCP port] Netmask set to default!");
		pSet->netMask = "255.0.0.0";
	}
	// gateway
	tmp_addr = attrValues[5];
	if (ipValidate(tmp_addr)) {
		pSet->netGateway = tmp_addr;
	} else {
		PRINTDEBUG("[TCP port] Gateway address set to default!");
		pSet->netGateway = "10.0.0.252";
	}
	// backend
	mpPortSettings->modbusBackend = TCP;
	return true;
}

bool V7Port::Init(const xmlpp::Node* pXMLNode, V7Modem* pModem) {
	PRINTDEBUG("[V7Port::Init()]: init port")
	if (!pModem)
		return false;
	mpModem = pModem;

	const xmlpp::Element* portElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!portElement) {
		cerr << "Element \"Port\" not found" << endl;
		return false;
	}
	mBackendType = getEngineTypeConfig(portElement);
	// выделяем память под хранение структуры
	if (mBackendType == RTU) {
		mpPortSettings = new RtuPortSettings;
		mpPortSettings->portName = mName;
		initRtuBackendConfig(portElement);
		mpModbusEngine = new EngineModbusRTU;

	} else if (mBackendType == TCP) {
		mpPortSettings = new TcpPortSettings;
		mpPortSettings->portName = mName;
		initTcpBackendConfig(portElement);
		mpModbusEngine = new EngineModbusTCP;
	}

	int line = portElement->get_line();
	cout << "Port settings: " << endl << *getPortSettings() << flush << endl;
	//!Modbus устройства

	xmlpp::Node::NodeList deviceList = portElement->get_children(
			"DeviceModbus");

	if (!deviceList.empty()) {
		for (xmlpp::Node::NodeList::iterator iter = deviceList.begin();
				iter != deviceList.end(); ++iter) {

			const xmlpp::Element* deviceElement =
					dynamic_cast<const xmlpp::Element*>(*iter);
			//TODO переделать на работу со словарем устройств modbusDeviceTypes
			// см. globals.h
			int devTypeId;
			std::stringstream(
					deviceElement->get_attribute("TypeId")->get_value())
					>> devTypeId;
			if (devTypeId < 1001 || devTypeId > 1003) {
				V7DeviceModbus * mbDev = new V7DeviceModbus;
				if (!mbDev->Init(*iter, this)) {
					delete mbDev;
					return false;
				}
				mbDev->setTypeID(devTypeId);
				mvpDevices.push_back(mbDev);
			}
			if (devTypeId == 1001) {
				PRINTDEBUG2("Din16Dout8v1 found in config => ", devTypeId);
				Din16Dout8v1* mbDev = new Din16Dout8v1;
				if (!mbDev->Init(*iter, this)) {
					delete mbDev;
					return false;
				}
				mbDev->setTypeID(devTypeId);
				mvpDevices.push_back(mbDev);
			} else if (devTypeId == 1002) {
				PRINTDEBUG2("Din16Dout8v2 found in config => ", devTypeId);
				Din16Dout8* mbDev = new Din16Dout8;
				if (!mbDev->Init(*iter, this)) {
					delete mbDev;
					return false;
				}
				mbDev->setTypeID(devTypeId);
				mvpDevices.push_back(mbDev);
			} else if (devTypeId == 1003) {
				PRINTDEBUG2("StatMT02 found in config => ", devTypeId);
				DeviceModemStat* mbDev = new DeviceModemStat;
				if (!mbDev->Init(*iter, this)) {
					delete mbDev;
					return false;
				}
				mbDev->setTypeID(devTypeId);
				mvpDevices.push_back(mbDev);
			}
		}
	} else {
		return false;
	}
	// Shunt
	deviceList = portElement->get_children("DeviceShunt");
	if (!deviceList.empty()) {
		if (deviceList.size() > 1) {
			PRINTDEBUG2("Modem has more than one Shunt (Din16Dout8), line: ",
					line)
			return false;
		}
		xmlpp::Node::NodeList::iterator iter = deviceList.begin();
		mpShunt = new DeviceShunt;
		if (!mpShunt->Init(*iter, this)) {
			return false;
		}
	}
	//!Внешнюю скаду, если она есть
	xmlpp::Node::NodeList scadaList = portElement->get_children("ScadaPort");
	if (!scadaList.empty()) {
		if (scadaList.size() > 1) {
			PRINTDEBUG2("Port has more than one SCADA, line:", line)
			return false;
		}
		PRINTDEBUG("New scada");
		mpScada = new V7Scada;

		if (!mpScada->Init(*scadaList.begin(), this, mvpDevices)) {
			PRINTDEBUG("Error init SCADA");
			return false;
		}
	}

	if (mvpDevices.empty()) {
		PRINTDEBUG2("Devices not found, line: ", line);
		return false;
	}

	//!< Инициализируем движок
	mpModbusEngine->init(mpPortSettings);
	return true;
}

V7Modem * V7Port::GetModem() {
	return mpModem;
}

bool V7Port::setDataToDevice(inputData_type* inputData) {
	if (!inputData)
		return false;

	for (int i = 0; i < mvpDevices.size(); i++) {
		if (mvpDevices[i]->setDataToDevice(inputData))
			return true;
	}
	return false;
}

bool V7Port::setDataToDevice(umkaFile_type* inputData) {
	//PRINTDEBUG2("V7Port::SetDataToFile::: inputData SET DATA TO FILE  ",(int) inputData->umkaFileStatus);
	if (!inputData)
		return false;

	for (int i = 0; i < mvpDevices.size(); ++i) {
		if (mvpDevices[i]->setDataToDevice(inputData)) {
			return true;
		}
	}
	return false;
}

void V7Port::setModbusEngine(EngineModbus* eng) {
	mpModbusEngine = eng;
}

EngineModbus * V7Port::getModbusEngine() const {
	return mpModbusEngine;
}

void V7Port::Run() {
	const int sz = mvpDevices.size();
	while (!endWorkFlag) {
		for (int i = 0; i < sz /*mvpDevices.size()*/; i++) {
			// для офлайн режима не опрашиваем устройства
			if (offlineMode) {
				sched_yield();
				sleep(600); //спим в оффлайне
				continue;
			}
			if (!mvpDevices[i]) {
				sched_yield();
				continue;
			}
			//!сеанс связи с устройством
			mvpDevices[i]->Session();
		}
		sched_yield();
	}
}

engine_backend_type V7Port::getBackendType() const {
	return mBackendType;
}

bool V7Port::isBusy() const {
	return flagBusy;
}

int V7Port::request(QueryMsg * msg) {

	flagBusy = true;
	const useconds_t pauseInUs = 300;
	int exitCode(0); //нет ошибки
	// подключение к движку
	if (!mpModbusEngine->getModbusContext()) {
		mpModbusEngine->open();
	}
	// если подключились
	if (mpModbusEngine->connect()) {
//        PRINTDEBUG2("\n[Port::request()] Modbus ctx is: ",
//                mpModbusEngine->getModbusContext());
		msg->makeRawRequest();
		int retCode = mpModbusEngine->sendRawRequest(msg->getRequestBuffer(),
				msg->getRequestSize());
		if (retCode != 0) {
			// error & exit
			PRINTDEBUG("[Port::request()] error & exit");
			usleep(pauseInUs);
			msg->setErrorCode(retCode);
		} else {
			uint8_t reqFunc = msg->getRequestFunction();
			uint8_t tmp[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
			uint8_t* pTmp = tmp;
			int tmpSize(0);
			for (int i = 0; i < mTriesNumber; ++i) {
				retCode = mpModbusEngine->receiveRawConfirmation(&pTmp,
						tmpSize);
//				cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
//				cout << *msg << endl;
//				cout << "==========================" << endl;
//				cout << "ret_code=" << retCode << endl;
//				bigbrother::print_buf(pTmp, tmpSize);
//				cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
				if (retCode == 0) {
					break;
				}
				msg->makeRawRequest();
				mpModbusEngine->sendRawRequest(msg->getRequestBuffer(),
						msg->getRequestSize());
				//reinit
				tmpSize = 0;
				memset(tmp, 0, MODBUS_TCP_MAX_ADU_LENGTH);

			}
			if (retCode != 0 && tmpSize < 3) {
				msg->setErrorCode(retCode); // exit
			} else {
				if (mBackendType == RTU) {
					if (reqFunc != tmp[1]) {
						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[2]));
					} else {
						msg->setErrorCode(retCode);
						msg->setResponseBuffer(tmp, tmpSize);
					}
				} else if (mBackendType == TCP) {
					if (reqFunc != tmp[7]) {
						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[8]));
					} else {
						msg->setErrorCode(retCode);
						msg->setResponseBuffer(tmp + 6, tmpSize); ///
					}
				}

			}

		}
//        PRINTDEBUG2("\n[Port::request()] Request result:\n", *msg);
		exitCode = msg->getErrorCode();
		mpModbusEngine->close();
	} else {
		exitCode = 0x04; // slave device failure
	}
	flagBusy = false;
	return exitCode;
}

bool ScadaPort::initRtuBackendConfig(const xmlpp::Element* scadaPortElement) {

	int line = scadaPortElement->get_line();

	//!Забираем атрибуты
	const uint8_t numOfParams = 9;
	const xmlpp::Attribute* attributes[numOfParams];
	const Glib::ustring attrNames[numOfParams] = { "name", "baud", "parity",
			"dataBit", "stopBit", "byteTimeout", "responseTimeout", "timeout",
			"portName" };
	Glib::ustring attrValues[numOfParams];
	for (int i = 0; i < numOfParams; ++i) {
		attributes[i] = scadaPortElement->get_attribute(attrNames[i]);
		if ((i == 0 && !attributes[i])
				&& (i == numOfParams - 1 && !attributes[numOfParams - 1])) {
			cerr << "[ScadaPort::InitRtuBackend]: Attribute \"" << attrNames[i]
					<< "\" not found (line=" << line << ") " << endl;
			return false;
		}
		if ((i > 0 && i < 5) && !attributes[i]) {
			cerr << "[ScadaPort::InitRtuBackend]: Attribute \"" << attrNames[i]
					<< "\" not found (line=" << line << ") " << endl;
			return false;
		} else if ((i == 5 || i == 6) && !attributes[i]) {
			attrValues[i] = static_cast<Glib::ustring>("0");
		} else if (i == 7 && attributes[i]) {
			attrValues[6] = attributes[i]->get_value();
			attrValues[5] = static_cast<Glib::ustring>("0");
		} else if (i == 7 && !attributes[i]) {
			continue;
		} else {
			if (i != 0 && i != numOfParams - 1)
				attrValues[i] = attributes[i]->get_value();
			if ((i == 0 || i == numOfParams - 1) && attributes[i]) {
				attrValues[i] = attributes[i]->get_value();
			}
		}
	}
	RtuPortSettings *pSet = dynamic_cast<RtuPortSettings *>(mpPortSettings);

	string portName =
			attrValues[0].empty() ? attrValues[numOfParams - 1] : attrValues[0]; /**< Имя порта в файловой системе, к которому подключена внешняя SCADA */
	int baud; /**< Скорость обмена */
	istringstream(attrValues[1]) >> baud;
	if (baud <= 0 || baud > 115200) {
		cerr
				<< "[ScadaPort::Init]: Error in the value of the attribute \"baud\" (line="
				<< line << ") " << endl;
		clog << "[ScadaPort]: baud rate set to default (9600)" << endl;
		baud = 9600;
	}
	char parity; /**< Четность ('N' -none, 'E' - четный, 'O' - нечетный) */
	if (!attrValues[2].empty()
			&& ((attrValues[2][0] == 'N') || (attrValues[2][0] == 'E')
					|| (attrValues[2][0] == 'O')))
		parity = attrValues[2][0];
	else {
		cerr
				<< "[ScadaPort::Init]: Error in the value of the attribute \"mParity\" (line="
				<< line << ") " << endl;
		clog << "[ScadaPort]: parity set to default (N)" << endl;
		parity = 'N';
	}
	int dataBit; /**< Количество бит данных 5, 6, 7 или 8 */
	istringstream(attrValues[3]) >> dataBit;
	if ((dataBit != 5) && (dataBit != 6) && (dataBit != 7) && (dataBit != 8)) {
		cerr
				<< "[ScadaPort::Init]: Error in the value of the attribute \"dataBit\" (line="
				<< line << ") " << endl;
		clog << "[ScadaPort]: dataBit set to default (8)" << endl;
		dataBit = 8;
	}
	int stopBit; /**< Количество стоп-бит 1 или 2 */
	istringstream(attrValues[4]) >> stopBit;
	if ((stopBit != 1) && (stopBit != 2)) {
		cerr
				<< "[ScadaPort::Init]: Error in the value of the attribute \"stopBit\" (line="
				<< line << ") " << endl;
		clog << "[ScadaPort]: stopBit set to default (1)" << endl;
		stopBit = 1;
	}
	unsigned int byteTimeout;
	struct timeval stByteTimeout;
	istringstream(attrValues[5]) >> byteTimeout;
	if ((byteTimeout > 0) && (byteTimeout < 5000)) {
		stByteTimeout.tv_sec = byteTimeout / 1000;
		stByteTimeout.tv_usec = (byteTimeout % 1000) * 1000;
	} else {
		stByteTimeout.tv_sec = 0;
		stByteTimeout.tv_usec = 500000;     //!По умолчанию 0.5с
		clog << "[ScadaPort]: byte timeout set to default." << endl;
	}
	unsigned int responseTimeout;
	struct timeval stResponseTimeout;
	istringstream(attrValues[6]) >> responseTimeout;
	if ((responseTimeout > 0) && (responseTimeout < 5000)) {
		stResponseTimeout.tv_sec = responseTimeout / 1000;
		stResponseTimeout.tv_usec = (responseTimeout % 1000) * 1000;

	} else {
		responseTimeout = 500; // 0.5 sec
		stResponseTimeout.tv_sec = responseTimeout / 1000;
		stResponseTimeout.tv_usec = (responseTimeout % 1000) * 1000;
		clog << "[ScadaPort]: response timeout set to default." << endl;
	}
	pSet->portName = portName;
	pSet->baud = baud;
	pSet->parity = parity;
	pSet->dataBit = dataBit;
	pSet->stopBit = stopBit;
	pSet->byteTimeout = stByteTimeout;
	pSet->responseTimeout = stResponseTimeout;
	*dynamic_cast<RtuPortSettings*>(mpPortSettings) = *pSet;
	mpPortSettings->modbusBackend = RTU;
	return true;

}

bool ScadaPort::Init(const xmlpp::Node* pXMLNode, V7Modem* pModem) {

	PRINTDEBUG("[ScadaPort::Init()]: init port")
	if (!pModem)
		return false;
	mpModem = pModem;
	const xmlpp::Element* portElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!portElement) {
		cerr << "Element \"ScadaPort\" not found" << endl;
		return false;
	}
	mBackendType = getEngineTypeConfig(portElement);
	const bool masterMode = isTcpMasterMode(portElement);
	// выделяем память под хранение структуры
	if (mBackendType == RTU) {
		mpPortSettings = new RtuPortSettings;
		mpPortSettings->portName = mName;
		initRtuBackendConfig(portElement);
		mpModbusEngine = new EngineModbusRTU;
	} else if (mBackendType == TCP) {
		mpPortSettings = new TcpPortSettings;
		initTcpBackendConfig(portElement);
		if (masterMode) {
			clog << "TCP SCADA is master" << endl;
			mpModbusEngine = new EngineModbusTCP;
		} else {
			cout << "TCP SCADA is slave" << endl << flush;
			mpModbusEngine = new EngineModbusTCPServer;
		}
	}
	//!< Инициализируем движок
	mpModbusEngine->init(mpPortSettings);
	return true;
}

bool ScadaPort::isTcpMasterMode(const xmlpp::Element* portElement) {
	const Glib::ustring attrNames = "mode";
	const xmlpp::Attribute* mode = portElement->get_attribute(attrNames);
	if (!mode) {
		return true;
	}
	if (mode->get_value().uppercase() == "MASTER") {
		return true;
	} else if (mode->get_value().uppercase() == "SLAVE") {
		return false;
	}
	return true;
}

void V7Port::setTriesNumber(const int val) {
	if (val > 0 && val < 4) {
		mTriesNumber = val;
	}
}
