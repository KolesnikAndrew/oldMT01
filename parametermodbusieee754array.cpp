/*
 * parametermodbusIEEE754.cpp
 *
 *  Created on: 11 трав. 2018 р.
 *      Author: v7r
 */

#include "parametermodbusieee754.h"

#include "QueryMsg.h"
#include "devicemodbus.h"

ParameterModbusIEEE754SingleArray::ParameterModbusIEEE754SingleArray() :
		mArraySize(0), mState(validState_type::invalid_unknown), mData("") {
	mParamType = paramType_type::p_byte_array_single;
}

ParameterModbusIEEE754SingleArray::~ParameterModbusIEEE754SingleArray() {

}

void ParameterModbusIEEE754SingleArray::setModbusValueToCurrentDataPipe(
		struct timeval* tp, uint8_t* data, const validState_type& validState) {

	setVlaues(tp, data, validState);
	setValueToCurrentDataPipe(&mTimeStamp, mData, mState);

}

bool ParameterModbusIEEE754SingleArray::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {

	using namespace bigbrother;
	using xmlpp::Element;
	using xmlpp::Attribute;
	using Glib::ustring;

	if (!V7ParameterModbus::initParam(pXMLNode, pDevice)) {
		return false;
	}

	const Element* paramElement = dynamic_cast<const Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusSingleArray\" not found" << endl;
		return false;
	}
	int line = paramElement->get_line();
	//!Забираем атрибуты
	const uint8_t cntAttr = 3;
	const Attribute* attributes[cntAttr] = { 0 };
	const ustring attrNames[cntAttr] = { "unit", "arraySize", "scaleFactor" };
	ustring attrValues[cntAttr];
	for (int i = 0; i < cntAttr; ++i) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
		}
	}
	//unit --------------------------------------------------------------------------------------
	mUnit = attrValues[0];
	// arraySize --------------------------------------------------------------------------------
	int tmpArrSize(0);
	std::stringstream(attrValues[1]) >> tmpArrSize;
	if (tmpArrSize <= 0) {
		tmpArrSize = 128;
	}
	mArraySize = tmpArrSize;
	//scaleFactor ---------------------------------------------------------------------------------
	if (attributes[2]) {
		string tmpScale = replaceCommaWithDot(attrValues[2]);
		mScaleFactor = cvrtStrToNumber<double>(tmpScale);
		if (mScaleFactor < 0) {
			cerr << "Error in attribute values (\"scaleFactor\") (line=" << line
					<< ") " << endl;
			mDelta = 0;
		}
	}
	return true;
}

void ParameterModbusIEEE754SingleArray::writeParameterModbus() {
	clog << "not implemented" << endl;

}

int ParameterModbusIEEE754SingleArray::getArraySize() const {
	return mArraySize;
}

std::string ParameterModbusIEEE754SingleArray::getArrayData() const {
	return mData;

}

validState_type ParameterModbusIEEE754SingleArray::getValidState() const{
	return mState;
}


void ParameterModbusIEEE754SingleArray::setVlaues(struct timeval *tp,
		uint8_t* data, const validState_type& validState) {

	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}
		if (mOrder == byteOrder_type::motorola || mOrder == byteOrder_type::high_byte_low_word) {
			for (int i = 0; i < mArraySize * 4 ; i += 4) {
				bigbrother::swapBytesInIeeeSingleRead(data + i);
			}
		} else if (mOrder == byteOrder_type::intel || mOrder == byteOrder_type::low_byte_low_word) {
		}
	}
	mData.assign(reinterpret_cast<char*>(data), mArraySize * 4);
	mState = validState;
	mTimeStamp = *tp;

}

