/**
 * @file      parametermodbusbool.cpp
 * @brief     Определение функций класса логического Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include <modbus.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "globals.h"
#include "parametermodbusbool.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "server.h"

V7ParameterModbusBool::V7ParameterModbusBool() {
	mParamType = paramType_type::p_boolean;

}

V7ParameterModbusBool::~V7ParameterModbusBool() {

}

bool V7ParameterModbusBool::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7ParameterModbus::initParam(pXMLNode, pDevice))
		return false;
	mSize = 2;
	return true;
}

void V7ParameterModbusBool::setModbusValueToCurrentDataPipe(struct timeval *tp,
		uint16_t* data, const validState_type& validState) {
	string strData;

	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}
		stringstream streamTmp;
		streamTmp << (*data == 0 ? "0" : "1");
		strData = streamTmp.str();
	}
	setValueToCurrentDataPipe(tp, strData, validState);
}

void V7ParameterModbusBool::setModbusValueToCurrentDataPipe(struct timeval* tp,
		uint8_t* data, const validState_type& validState) {
	string strData;

	if (validState == validState_type::valid) {
		if (!data)
			return;

		stringstream streamTmp;

		if (*data == 0)
			streamTmp << "0";
		else
			streamTmp << "1";

		strData = streamTmp.str();
	}
	setValueToCurrentDataPipe(tp, strData, validState);
}

void V7ParameterModbusBool::writeParameterModbus() {

	std::string strNewValue;

	if (!mNewValueFlag || !getStrNewValueFromPipe(strNewValue)
			|| !mpDeviceModbus || !mpPort) {
		return;
	}

	confirmData_type* confirmData = new confirmData_type;
	confirmData->globalID = mServerID;
	confirmData->dateTime = bigbrother::timeStamp(); // текущее время

	if (mType == CURRENT_PARAM) {
		confirmData->state = inDataState_type::readonly;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	int16_t iValue = 0;

	if (strNewValue == "0") {
		iValue = 0;
	} else if (strNewValue == "1") {
		iValue = 1;
	} else {
		confirmData->state = inDataState_type::invalid_data;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	int mbRes;
	//!Какой функцией писать

	if (isFuncSupported(0x05)) {
		QueryMsgWriteSingleCoil * msg = new QueryMsgWriteSingleCoil;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x05;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress);
		msg->mbValueHi = (iValue > 0 ? 1 : 0);
		msg->mbValueLo = 0;
		//! Запрос

		waitForFreePort();
		mbRes = mpPort->request(msg);
		delete msg;

	} else if (isFuncSupported(0x06)) {
		QueryMsgWriteSingle *msg = new QueryMsgWriteSingle;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x06;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress);
		msg->mbValueHi = (iValue > 0 ? 1 : 0);
		msg->mbValueLo = 0;
		//! Запрос
		waitForFreePort();
		mbRes = mpPort->request(msg);
		delete msg;

	} else if (isFuncSupported(0x10)) {
		QueryMsgWriteMulti *msg = new QueryMsgWriteMulti;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x10;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress);
		msg->mbQuantityRegHi = 0x00;
		msg->mbQuantityRegLo = 0x01;
		msg->mbByteCnt = msg->mbQuantityRegLo * 2;
		msg->mbValues = reinterpret_cast<uint8_t*>(&iValue);
		//! Запрос
		waitForFreePort();
		mbRes = mpPort->request(msg);
		delete msg;

	}

	else {
		confirmData->state = inDataState_type::invalid_function;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	if (mbRes != 0) {
		//!Анализируем ошибку
		confirmData->state = cvrtErrToInputDataSTate(mbRes); //!!!!!!!
		setDataToSetpointConfirmBuffer(confirmData);

	} else {

		//!Ставим задание на вычитку
		mLastReading.tv_sec = 0;
		mLastReading.tv_usec = 0;
		PRINTDEBUG("====> Read after write (bool) <<<<<<<<<<<");
		//!Заносим результат в базу
		confirmData->state = inDataState_type::success;
		setDataToSetpointConfirmBuffer(confirmData);
	}

}

