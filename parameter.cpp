/**
 * @file      parameter.cpp
 * @brief     Определение функций класса параметра устройства
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include <string>
#include <string.h>
#include <stdio.h>
#include "globals.h"
#include "modem.h"
#include "port.h"
#include "parameter.h"
#include "device.h"
#include "devicemodbus.h"
#include "server.h"

V7Parameter::V7Parameter() :
		mServerID(0), mNumber(0), mPollingPeriod(0), mType(CURRENT_PARAM), mpDevice(
		NULL), mNewValueFlag(false)/*, mOldValueAvailable(false)*/, mOldValidState(
				validState_type::invalid_unknown), mUmkaFileStatus(
				fileStatus_type::start_process), mLastReading( { 0, 0 }), mIsReadingUmkaFile(
				false), mParamType(paramType_type::p_undefined), mpPort(0), mpModem(
				0), mpServer(0), mpDeviceModbus(0), mParameterSmartZip(true) {
}

V7Parameter::~V7Parameter() {

}

bool V7Parameter::initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice) {
	mpDevice = pDevice;
	if (!mpDevice)
		return false;
	mpDeviceModbus = dynamic_cast<V7DeviceModbus*>(mpDevice);
	mpPort = mpDevice->getPort();
	if (!mpPort) {
		return false;
	}
	mpModem = mpPort->GetModem();
	if (!mpModem)
		return false;
	mpServer = mpModem->getServerPtr();

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterXxx\" not found" << endl;
		return false;
	}

	int line = paramElement->get_line();

	//!Забираем атрибуты
	const xmlpp::Attribute* attributes[5];
	const Glib::ustring attrNames[5] = { "serverID", "name", "number",
			"pollingPeriod", "type" };
	Glib::ustring attrValues[5];
	for (int i = 0; i < 5; i++) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		if (!attributes[i] && i < 3) {
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i])
			attrValues[i] = attributes[i]->get_value();
	}

	//!Преобразуем атрибуты
	//serverID -----------------------------------------------------------------------------------
	istringstream(attrValues[0]) >> mServerID;
	//name ---------------------------------------------------------------------------------------
	mName = attrValues[1];
	if (mName.empty()) {
		cerr << "Error in the value of the attribute \"name\" (line=" << line
				<< ") " << endl;
		return false;
	}
	//number -------------------------------------------------------------------------------------
	istringstream(attrValues[2]) >> mNumber;
	if (mNumber == 0) {
		cerr << "Error in the value of the attribute \"number\" (line=" << line
				<< ") " << endl;
		return false;
	}
	//pollingPeriod ------------------------------------------------------------------------------
	istringstream(attrValues[3]) >> mPollingPeriod;
	if (mpModem->getPoolingPeriod() > mPollingPeriod)
		mpModem->setPoolingPeriod(mPollingPeriod);
	//type ---------------------------------------------------------------------------------------
	if (attrValues[4] == "setpoint")
		mType = SETPOINT_PARAM;
	else if (attrValues[4] == "current")
		mType = CURRENT_PARAM;
	else {
		cerr << "Error in the value of the attribute \"type\" (line=" << line
				<< ") " << endl;
		return false;
	}

	return true;
}

bool V7Parameter::needToGetValue(const struct timeval* time) const {

	return (mPollingPeriod
			<= (time->tv_usec - mLastReading.tv_usec) / 1000
					+ (time->tv_sec - mLastReading.tv_sec) * 1000);
}

void V7Parameter::setLastReading(const struct timeval* time) {
	if (!time) {
		return;
	}
	mLastReading = *time;
}

void V7Parameter::setValueToCurrentDataPipe(struct timeval *tp,
		const string& data, const validState_type& validState) {

	if (!tp) {
		return;
	}
	//!Отсеиваем значения, которые не изменились
	if (mParameterSmartZip && validState == validState_type::valid
			&& mStrOldValue == data) {
		return;
	}

	mStrOldValue = validState == validState_type::valid ? data : "";
	mOldValidState = validState;

	if (!mpDevice || !mpPort || !mpModem || !mpServer) {
		return;
	}

	outputData_type* outputData = new outputData_type;
	outputData->globalID = mServerID;
	outputData->validState = validState;
	outputData->value = mStrOldValue;

	outputData->dateTime = bigbrother::timeStamp();
	outputData->ms = tp->tv_usec / 1000;
	outputData->paramType = mParamType;
	mpServer->SetDataToCurrentDataBuffer(outputData);
}

void V7Parameter::setDataToSetpointConfirmBuffer(
		confirmData_type* confirmData) {
	mpServer->SetDataToSetpointConfirmBuffer(confirmData);
}

bool V7Parameter::setDataToDevice(inputData_type* inputData) {
	if (!inputData)
		return false;

	if (inputData->globalID == mServerID) {
		pthread_mutex_lock(&mpPort->mMutexInputData);
		mNewValue = inputData->value;
		mNewValueFlag = true;
		pthread_mutex_unlock(&mpPort->mMutexInputData);
		return true;
	}
	return false;
}

bool V7Parameter::setDataToDevice(umkaFile_type* inputData) {
	PRINTDEBUG2("Parameter:: inputData SET DATA TO FILE  ",
			(int )inputData->umkaFileStatus);
	PRINTDEBUG2("Parameter:: global ID  ", mServerID);
	PRINTDEBUG2("Parameter:: global ID  ", inputData->globalID);
	if (!inputData) {
		return false;
	}
	if (inputData->globalID == mServerID) {
		if (!mpDevice || !mpDevice->getPort()) {
			return false;
		}
		PRINTDEBUG2("(1) mUmkaFileStatus >>>>  ", (int )mUmkaFileStatus);

		if (mUmkaFileStatus != fileStatus_type::ready_file) {
			mUmkaFileStatus = inputData->umkaFileStatus;
			switch (inputData->umkaFileStatus) {
			case fileStatus_type::start_process:
			case fileStatus_type::in_process:
			case fileStatus_type::need_restart:
				dynamic_cast<ParameterModbusFile*>(this)->setReadFlag();
				break;
			case fileStatus_type::need_delete:
				dynamic_cast<ParameterModbusFile*>(this)->setDeleteFlag();
				break;
			case fileStatus_type::need_stop:
				dynamic_cast<ParameterModbusFile*>(this)->setStopFlag();
				break;
			case fileStatus_type::need_write:
				dynamic_cast<ParameterModbusFile*>(this)->setWriteFlag();
				break;
			default:
				break;
			}
		}
		if (inputData->umkaFileStatus == fileStatus_type::send_success) {
			mUmkaFileStatus = fileStatus_type::unknown;
			dynamic_cast<ParameterModbusFile*>(this)->resetAllStateFlags();
			PRINTDEBUG2("(2) mUmkaFileStatus >>>>  ", (int )mUmkaFileStatus);
		}
		return true;
	} else {
		PRINTDEBUG2("BAD ID PARAM:  ", inputData->globalID)
		dynamic_cast<ParameterModbusFile*>(this)->badIdAnswer(
				inputData->globalID);

	}
	return false;
}

fileStatus_type V7Parameter::getFileStatus() const {
	return mUmkaFileStatus;
}

bool V7Parameter::isReadFileInProcess() {
	return mIsReadingUmkaFile;
}

void V7Parameter::setFileStatus(const fileStatus_type& status) {
	mUmkaFileStatus = status;
}

bool V7Parameter::isNewValue() {
	return mNewValueFlag;
}

unsigned int V7Parameter::getGlobalServerId() const {
	return mServerID;
}

void V7Parameter::waitForFreePort() {
	while (mpDevice->getPort()->isBusy()) {
		sched_yield();		//sleep(0);
	}
}

string V7Parameter::getNewValue() const {
	return mNewValue;
}

void V7Parameter::resetNewValueFlag() {
	if (mNewValueFlag) {
		mNewValueFlag = false;
	}
}

paramType_type V7Parameter::getParamType() const {
	return mParamType;
}
void V7Parameter::setParamType(const paramType_type &pt) {
	mParamType = pt;
}

bool V7Parameter::getStrNewValueFromPipe(std::string& strNewValue) {

	if (!mNewValueFlag) {
		return false;
	}

	pthread_mutex_lock(&mpPort->mMutexInputData);
	strNewValue = mNewValue;
	mNewValueFlag = false;
	pthread_mutex_unlock(&mpPort->mMutexInputData);

	clog << "[debug]: New value was received. id=" << mServerID << " , value="
			<< mNewValue << " , name=" << mName << endl;
	return true;
}

void V7Parameter::setParameterSmartZip(const bool smartZip) {
	mParameterSmartZip = smartZip;
}

bool V7Parameter::getParameterSmartZip() const {
	return mParameterSmartZip;
}

void V7Parameter::resetLastReadingTime() {
	mLastReading = {0,0};
}
