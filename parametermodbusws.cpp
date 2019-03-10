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
#include "parametermodbusws.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "server.h"
#include <bitset>

ParameterModbusWordState::ParameterModbusWordState() :
		mLastBit(0), mFirstBit(0), mMask(0), mStatus(
				std::numeric_limits<uint16_t>::max()) {
	mParamType = paramType_type::p_word_state;
}

ParameterModbusWordState::~ParameterModbusWordState() {
}

bool ParameterModbusWordState::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7ParameterModbus::initParam(pXMLNode, pDevice))
		return false;

	//mSize = 1;

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusWordStateEnum\" not found" << endl;
		return false;
	}

	int line = paramElement->get_line();

	const uint8_t numParams = 2;
	const xmlpp::Attribute* attributes[numParams];
	const Glib::ustring attrNames[numParams] = { "firstBit", "lastBit" };

	Glib::ustring attrValues[numParams];
	for (int i = 0; i < numParams; ++i) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		// all paraams is required
		if (attributes[i] == NULL) {
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
		}
	}
	//
	int16_t tmpFirst(0), tmpLast(0);
	istringstream(attrValues[0]) >> tmpFirst;
	istringstream(attrValues[1]) >> tmpLast;
	if (abs(tmpFirst) > abs(tmpLast)) {
		std::swap(tmpFirst, tmpLast);
	}
	mLastBit = abs(tmpLast);
	mFirstBit = abs(tmpFirst);
	makeMask();
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
			cerr
					<< "Element \"ParameterModbusWordStateEnum\" contains an error (line="
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

void ParameterModbusWordState::setModbusValueToCurrentDataPipe(
		struct timeval* tp, uint16_t* data, const validState_type& validState) {
	cerr << "[debug] ParameterWordState. Write operation is not permitted."
			<< endl;

}

void ParameterModbusWordState::setModbusValueToCurrentDataPipe(
		struct timeval* tp, uint8_t* data, const validState_type& validState) {
	string strData;

	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}
		mStatus = static_cast<uint16_t>(data[0] << 8)
				+ static_cast<uint16_t>(data[1]);
		PRINTDEBUG("ParameterWordState");
		PRINTDEBUG2("status before:  ", std::bitset<16>(mStatus));
		mStatus &= mMask;
		mStatus >>= mFirstBit;
		PRINTDEBUG2("mask:    ", std::bitset<16>(mMask));
		PRINTDEBUG2("status af:  ", std::bitset<16>(mStatus));
		PRINTDEBUG("ParameterWordState");

	}
	setValueToCurrentDataPipe(tp, std::to_string(mStatus), validState);

}

void ParameterModbusWordState::writeParameterModbus() {

	std::string strNewValue;

	if (!mNewValueFlag || !getStrNewValueFromPipe(strNewValue)
			|| !mpDeviceModbus || !mpPort) {
		return;
	}

	confirmData_type* confirmData = new confirmData_type;
	confirmData->globalID = mServerID;
	confirmData->dateTime = bigbrother::timeStamp(); // текущее время
	confirmData->state = inDataState_type::readonly;
	setDataToSetpointConfirmBuffer(confirmData);
	return;
}

uint16_t ParameterModbusWordState::getMask() const {
	return mMask;
}

void ParameterModbusWordState::setStatus(uint16_t val) {
	mStatus = val;
}

void ParameterModbusWordState::makeMask() {
	uint8_t tmp(mFirstBit);

	while (tmp <= mLastBit) {
		mMask += (1 << tmp);
		++tmp;
	}
}

uint16_t ParameterModbusWordState::getStatus() const {
	return mStatus;
}
