/*
 * parametermodbusIEEE754.cpp
 *
 *  Created on: 11 трав. 2018 р.
 *      Author: v7r
 */

#include "parametermodbusieee754.h"

#include "QueryMsg.h"
#include "devicemodbus.h"

ParameterModbusIEEE754SingleMatrix::ParameterModbusIEEE754SingleMatrix() {
	mParamType = paramType_type::p_single_matrix;
}

ParameterModbusIEEE754SingleMatrix::~ParameterModbusIEEE754SingleMatrix() {

}

bool ParameterModbusIEEE754SingleMatrix::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7ParameterModbus::initParam(pXMLNode, pDevice))
		return false;

	mSize = 1;

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusSingleMatrix\" not found" << endl;
		return false;
	}

	int line = paramElement->get_line();

	//!Берем список допустимых значений
	auto arrayList = paramElement->get_children("ParameterModbusSingleArray");
	if (arrayList.empty()) {
		cerr << "ParameterModbusSingleArray not found (line=" << line << ") "
				<< endl;
		return false;
	}

	for (auto it : arrayList) {
		/** флаг готовности **/
		auto * mbS = new ParameterModbusIEEE754SingleArray;
		mbS->setParameterSmartZip(false);
		if (!mbS->initParam(it, mpDevice)) {
			return false;
		}
		mArrays.push_back(mbS);
	}
	return true;
}

void ParameterModbusIEEE754SingleMatrix::setModbusValueToCurrentDataPipe(
		struct timeval* tp, uint8_t* data, const validState_type& validState) {

	string strData;
	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}

		stringstream streamTmp;
		streamTmp << *data;
		strData = streamTmp.str();
	}
	if (strData == mOldData) {
		PRINTDEBUG("equal");
		return;
	}
	setValueToCurrentDataPipe(tp, strData, validState);
	mOldData = strData;

}

void ParameterModbusIEEE754SingleMatrix::setModbusValueToCurrentDataPipe(
		struct timeval *tp, const std::string& data,
		const validState_type& validState) {

	if (data == mOldData) {
		PRINTDEBUG("equal");
		return;
	}

	setValueToCurrentDataPipe(tp, data, validState);
	mOldData = data;

}

void ParameterModbusIEEE754SingleMatrix::writeParameterModbus() {
	cerr << "Matrix: Write is not permitted" << endl;
}

std::vector<ParameterModbusIEEE754SingleArray*> ParameterModbusIEEE754SingleMatrix::getArray() const {
	return mArrays;

}

void ParameterModbusIEEE754SingleMatrix::addData(const std::string& data) {
	mData += data;
}

size_t ParameterModbusIEEE754SingleMatrix::getDataSize() const {
	return mData.size();
}

std::string ParameterModbusIEEE754SingleMatrix::getData() const {
	return mData;
}

void ParameterModbusIEEE754SingleMatrix::resetData() {
	mData.resize(0);
}

bool ParameterModbusIEEE754SingleMatrix::isDataReady() {
	return mState.getStatus() > 0 ? true : false;
}

void ParameterModbusIEEE754SingleMatrix::setDataReadyState(
		const ParameterModbusWordState& state) {
	mState = state;
}

ParameterModbusWordState ParameterModbusIEEE754SingleMatrix::getDataReadyState() const {
	return mState;
}
