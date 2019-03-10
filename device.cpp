/**
 * @file      device.cpp
 * @brief     Определение функций класса устройства на порту
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include "portsettings.h"
#include "globals.h"
#include "port.h"
#include "parameter.h"
#include "device.h"

V7Device::V7Device() :
		mNumber(0), mBaud(9600), mParity('N'), mDataBit(8), mStopBit(1), mpPort(
				nullptr), mSessionNumber(0), mConfigAT27Buffer(""), mTypeID(0) {

}

V7Device::~V7Device() {
	while (!mvpParameters.empty()) {
		delete mvpParameters.back();
		mvpParameters.pop_back();
	}
	while (!mvpStatusWords.empty()) {
		delete mvpStatusWords.back();
		mvpStatusWords.pop_back();
	}
	while (!mvpUmkaFiles.empty()) {
		delete mvpUmkaFiles.back();
		mvpUmkaFiles.pop_back();
	}
	while (!mvpByteSingleArrays.empty()) {
		delete mvpByteSingleArrays.back();
		mvpByteSingleArrays.pop_back();
	}
	while (!mvpSingleMatrix.empty()) {
		delete mvpSingleMatrix.back();
		mvpSingleMatrix.pop_back();
	}
}

bool V7Device::Init(const xmlpp::Node* pXMLNode, V7Port* pPort) {
	mpPort = dynamic_cast<V7Port*>(pPort);
	if (!mpPort)
		return false;

	const xmlpp::Element* deviceElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!deviceElement) {
		cerr << "Element \"DeviceModbus\" not found" << endl;
		return false;
	}

	const int line = deviceElement->get_line();

	//!Забираем атрибуты
	const short numOfParams = 8;
	const xmlpp::Attribute* attributes[numOfParams];
	const Glib::ustring attrNames[numOfParams] = { "number", "baud", "parity",
			"dataBit", "stopBit", "serialNumber", "name", "TypeId" };

	Glib::ustring attrValues[numOfParams];
	for (int i = 0; i < numOfParams; i++) {
		attributes[i] = deviceElement->get_attribute(attrNames[i]);
		if (!attributes[i]) {
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		attrValues[i] = attributes[i]->get_value();
	}

	//!Преобразуем атрибуты
	//number----------------------------------------------------------------------------------------
	istringstream(attrValues[0]) >> mNumber;
	if (mNumber == 0) {
		cerr << "Error in the value of the attribute \"number\" (line=" << line
				<< ") " << endl;
		return false;
	}
	//baud----------------------------------------------------------------------------------------------------
	istringstream(attrValues[1]) >> mBaud;
	if (mBaud <= 0 || mBaud > 1000000) {
		mBaud = 9600;
		cerr << "Error in the value of the attribute \"baud\" (line=" << line
				<< ") " << endl;
		cerr << "Default value was setupped" << endl;
	}
	//parity--------------------------------------------------------------------------------------------------
	if (!attrValues[2].empty()
			&& ((attrValues[2][0] == 'N') || (attrValues[2][0] == 'E')
					|| (attrValues[2][0] == 'O')))
		mParity = attrValues[2][0];
	else {
		mParity = attrValues[2][0];
		cerr << "Error in the value of the attribute \"mParity\" (line=" << line
				<< ") " << endl;
		cerr << "Default value was setupped" << endl;
	}
	//dataBit-------------------------------------------------------------------------------------------------
	istringstream(attrValues[3]) >> mDataBit;
	if ((mDataBit != 5) && (mDataBit != 6) && (mDataBit != 7)
			&& (mDataBit != 8)) {
		mDataBit = 8;
		cerr << "Error in the value of the attribute \"dataBit\" (line=" << line
				<< ") " << endl;
		cerr << "Default value was setupped" << endl;
	}
	//stopBit-------------------------------------------------------------------------------------------------
	istringstream(attrValues[4]) >> mStopBit;
	if ((mStopBit != 1) && (mStopBit != 2)) {
		cerr << "Error in the value of the attribute \"stopBit\" (line=" << line
				<< ") " << endl;
		cerr << "Default value was setupped" << endl;
		mStopBit = 1;
	}
	//serialNumber----------------------------------------------------------------------------------
	mSerialNumber = attrValues[5];
	//name------------------------------------------------------------------------------------------
	mName = attrValues[6];
	//typeID
	istringstream(attrValues[7]) >> mTypeID;
	if (mTypeID < 1001 || mTypeID > 1003) {
		mTypeID = 3;
	}

	//!< Настройка параметров порта для RTU
	if (pPort->getPortSettings()->modbusBackend == RTU) {
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->baud = mBaud;
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->dataBit =
				mDataBit;
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->parity =
				mParity;
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->stopBit =
				mStopBit;
	}

	return true;
}

V7Port* V7Device::getPort() {
	return mpPort;
}

bool V7Device::setDataToDevice(inputData_type* inputData) {
	if (!inputData)
		return false;

	for (int i = 0; i < mvpParameters.size(); i++) {
		if (mvpParameters[i]->setDataToDevice(inputData))
			return true;
	}
	return false;
}

bool V7Device::setDataToDevice(umkaFile_type* inputData) {
	//PRINTDEBUG2("Device::SetDataToFile::: inputData SET DATA TO FILE  ", (int)inputData->umkaFileStatus);
	if (!inputData) {
		return false;
	}
	mConfigAT27Buffer = inputData->config_buffer;
	if (inputData->isWriteMode) {
		inputData->globalID = getUmkaConfigFileIdAndSetConfigBuffer(); //! get and set id to config
	}

	for (int i = 0; i < mvpUmkaFiles.size(); i++) {
		//PRINTDEBUG2("V7Device  ", inputData->umkaFileStatus)
		if (mvpUmkaFiles[i]->setDataToDevice(inputData)) {
			return true;
		}
	}
	return false;

}


validState_type V7Device::cvrtErrToValidState(int errCode) const {
	switch (errCode) {
	case 0x00:
		return validState_type::valid;
		break;
	case 0x01:
		return validState_type::invalid_function; //Неверная функция запроса
		break;
	case 0x02:
		return validState_type::invalid_address; //Неверное значение адреса
		break;
	case 0x03:
		return validState_type::invalid_unknown;
		break;
	case 0x04:
		return validState_type::invalid_answer; //Ошибочный ответ устройства
		break;
	case 0x05:
		return validState_type::invalid_unknown;
		break;
	case 0x06:
		return validState_type::device_busy;
		break;
	case 0x08:
		return validState_type::invalid_unknown;
		break;
	case 0x0a:
	case 0x0b:
		return validState_type::not_responding;
		break;
	default:
		return validState_type::invalid_unknown; //Неизвестная ошибка
		break;
	}
}

//int V7Device::getTypeId() const {
//	return mTypeID;
//}

inDataState_type V7Device::cvrtErrToInputDataSTate(int16_t err) {
	switch (err) {
	case 0x00:
		return inDataState_type::success;
	case 0x01:
		return inDataState_type::invalid_function; //Неверная функция запроса
		break;
	case 0x02:
		return inDataState_type::invalid_address; //Неверное значение адреса
		break;
	case 0x03:
		return inDataState_type::invalid_data;
		break;
	case 0x04:
		return inDataState_type::invalid_answer; //Ошибочный ответ устройства
		break;
	case 0x05:
		return inDataState_type::unknown;
		break;
	case 0x06:
		return inDataState_type::device_busy;
		break;
	case 0x08:
		return inDataState_type::unknown;
		break;
	case 0x0a:
	case 0x0b:
		return inDataState_type::not_responding;
		break;
	default:
		return inDataState_type::unknown; //Неизвестная ошибка
		break;
	}

}

unsigned int V7Device::getUmkaConfigFileIdAndSetConfigBuffer() const {
	unsigned int id = 0;
	for (int i = 0; i < mvpUmkaFiles.size(); ++i) {
		ParameterModbusFile* tmp =
				dynamic_cast<ParameterModbusFile*>(mvpUmkaFiles[i]);
		if (tmp->isConfigFile()) {
			id = tmp->getGlobalServerId();
			tmp->setConfigFileAT27Buffer(mConfigAT27Buffer);
			tmp->setDeviceConfigNumber(mNumber);
		}
	}
	return id;

}
