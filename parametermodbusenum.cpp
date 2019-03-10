/**
 * @file      parametermodbusenum.cpp
 * @brief     Определение функций класса перечисляемого Modbus-параметра
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
#include <math.h>
#include "globals.h"
#include "parameter.h"
#include "parametermodbusenum.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "server.h"

V7ParameterModbusEnum::V7ParameterModbusEnum() {
	mParamType = paramType_type::p_enum;
}

V7ParameterModbusEnum::~V7ParameterModbusEnum() {

}

bool V7ParameterModbusEnum::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7ParameterModbus::initParam(pXMLNode, pDevice))
		return false;

	mSize = 1;

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusEnum\" not found" << endl;
		return false;
	}

	int line = paramElement->get_line();

	//!Берем список допустимых значений
	xmlpp::Node::NodeList allowedValuesList = paramElement->get_children(
			"PossibleValue");
	if (allowedValuesList.empty()) {
		cerr << "PossibleValue not found (line=" << line << ") " << endl;
		return false;
	}

	for (xmlpp::Node::NodeList::iterator iter = allowedValuesList.begin();
			iter != allowedValuesList.end(); ++iter) {
		const xmlpp::Node* node = *iter;
		const xmlpp::Element* nodeElement =
				dynamic_cast<const xmlpp::Element*>(node);

		if (!nodeElement) {
			cerr << "Element \"ParameterModbusEnum\" contains an error (line="
					<< line << ") " << endl;
			return false;
		}
		line = nodeElement->get_line();
		string strTmp = nodeElement->get_child_text()->get_content();
		if (strTmp.empty()) {
			cerr << "Element \"PossibleValue\" is empty (line=" << line << ") "
					<< endl;
			return false;
		}
		mvAllowedValues.push_back(strTmp);
	}

	return true;
}

void V7ParameterModbusEnum::setModbusValueToCurrentDataPipe(struct timeval *tp,
		uint16_t* data, const validState_type& validState) {

	string strData;
	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}

		stringstream streamTmp;
		streamTmp << *data;
		strData = streamTmp.str();
	} else {
	}
	setValueToCurrentDataPipe(tp, strData, validState);

}

void V7ParameterModbusEnum::setModbusValueToCurrentDataPipe(struct timeval* tp,
		uint8_t* data, const validState_type& validState) {

	string strData = "";

	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}

		stringstream streamTmp;
		streamTmp
				<< static_cast<uint16_t>(data[0] << 8)
						+ static_cast<uint16_t>(data[1]);
		strData = streamTmp.str();

	} else {
		strData = "";
	}
	setValueToCurrentDataPipe(tp, strData, validState);
}

void V7ParameterModbusEnum::writeParameterModbus() {

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

	uint16_t iValue = (uint16_t) atoi(strNewValue.c_str());

	if (iValue >= mvAllowedValues.size()) {
		confirmData->state = inDataState_type::invalid_data;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	int mbRes;
	//!Какой функцией писать
	if (isFuncSupported(0x06)) {
		QueryMsgWriteSingle *msg = new QueryMsgWriteSingle;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x06;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress);
		msg->mbValueHi = iValue >> 8;
		msg->mbValueLo = iValue & 0xff;

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
		//! Запрос
		msg->mbValues = reinterpret_cast<uint8_t*>(&iValue);
		waitForFreePort();
		mbRes = mpPort->request(msg);
		delete msg;
	} else {
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
		PRINTDEBUG("====> Read after write (enum) <<<<<<<<<<<");
		//!Заносим результат в базу
		confirmData->state = inDataState_type::success;
		setDataToSetpointConfirmBuffer(confirmData);
	}
}

