/*
 * parametermodbusIEEE754.cpp
 *
 *  Created on: 11 трав. 2018 р.
 *      Author: v7r
 */

#include "parametermodbusieee754.h"

#include "QueryMsg.h"
#include "devicemodbus.h"

ParameterModbusIEEE754::ParameterModbusIEEE754() :
		mOldValue(0.0), mMaxValue(0.0), mMinValue(0.0), mUnit(""), mDelta(0.0), mScaleFactor(
				1.0) {
	mParamType = paramType_type::p_single;
}

ParameterModbusIEEE754::~ParameterModbusIEEE754() {

}

bool ParameterModbusIEEE754::initParam(const xmlpp::Node* pXMLNode,
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
		cerr << "Element \"ParameterModbusSingle\" not found" << endl;
		return false;
	}
	int line = paramElement->get_line();
	//!Забираем атрибуты
	const uint8_t cntAttr = 5;
	const Attribute* attributes[cntAttr] = { 0 };
	const ustring attrNames[cntAttr] = { "unit", "minValue", "maxValue",
			"delta", "scaleFactor" };
	ustring attrValues[cntAttr];
	for (int i = 0; i < cntAttr; i++) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
		}
	}
	//unit --------------------------------------------------------------------------------------
	mUnit = attrValues[0];
	//minValue maxValue -------------------------------------------------------------------------
	if (attributes[1] && attributes[2]) {
		mMinValue = cvrtStrToNumber<double>(replaceCommaWithDot(attrValues[1]));
		mMaxValue = cvrtStrToNumber<double>(replaceCommaWithDot(attrValues[2]));
		if (mMinValue > mMaxValue) {
			cerr
					<< "Error in attribute values (\"minValue\", \"maxValue\") (line="
					<< line << ") " << endl;
			//return false
			mMaxValue = mMinValue;
		}
	}
	//delta --------------------------------------------------------------------------------------
	if (attributes[3]) {
		string tmpDelta = replaceCommaWithDot(attrValues[3]);
		mDelta = cvrtStrToNumber<double>(tmpDelta);
		//PRINTDEBUG2("mDelta: ", mDelta);
		if (mDelta < 0) {
			cerr << "Error in attribute values (\"delta\") (line=" << line
					<< ") " << endl;
			//return false;
			mDelta = 0;
		}
	}
	//scaleFactor --------------------------------------------------------------------------------------
	if (attributes[4]) {
		string tmpScale = replaceCommaWithDot(attrValues[4]);
		mScaleFactor = cvrtStrToNumber<double>(tmpScale);
		if (mScaleFactor < 0) {
			cerr << "Error in attribute values (\"scaleFactor\") (line=" << line
					<< ") " << endl;
			mDelta = 0;
		}
	}
	return true;
}

void ParameterModbusIEEE754::setModbusValueToCurrentDataPipe(struct timeval *tp,
		uint16_t* data, const validState_type& validState) {
	cerr << "Not implemented\n";

}

void ParameterModbusIEEE754Single::writeParameterModbus() {

	std::string strNewValue;

	if (!mNewValueFlag || !getStrNewValueFromPipe(strNewValue) || !mpDeviceModbus || !mpPort) {
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

	if (!bigbrother::isStrIsNumber<float>(strNewValue)) {

		confirmData->state = inDataState_type::invalid_data;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	if (std::stod(strNewValue) > mMaxValue
			|| std::stod(strNewValue) < mMinValue) {
		PRINTDEBUG("PMBSingle MinMaxValue Error");
		confirmData->state = inDataState_type::invalid_data;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	union {
		unsigned char c[sizeof(float)];
		float f;
	} setData;

	//char* pEnd;
	setData.f = std::strtof(strNewValue.c_str(), nullptr);
	int mbRes;
#ifdef DEBUG
	bigbrother::printFloatAsChar(setData.f);
#endif
	bigbrother::swapBytesInIeeeSingleWrite(setData.c);
	uint16_t writeAddress = mAddress - 1; //!!!! SLB specific
	if (isFuncSupported(0x10)) {
		QueryMsgWriteMulti *msg = new QueryMsgWriteMulti;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x10;
		msg->mbAddressHi = static_cast<uint8_t>((writeAddress) >> 8);
		msg->mbAddressLo = static_cast<uint8_t>((writeAddress));
		msg->mbQuantityRegHi = 0x00;
		msg->mbQuantityRegLo = 0x02;
		msg->mbByteCnt = msg->mbQuantityRegLo * 2;
		msg->mbValues = setData.c;
//! Запрос
		waitForFreePort();
		mbRes = mpPort->request(msg);
		delete msg;

	} else if (isFuncSupported(0x17)) {
//TODO реализовать класс
		confirmData->state = inDataState_type::invalid_function;
		setDataToSetpointConfirmBuffer(confirmData);
		return;

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
//!Заносим результат в базу
		confirmData->state = inDataState_type::success;
		setDataToSetpointConfirmBuffer(confirmData);
	}
}

void ParameterModbusIEEE754Single::setModbusValueToCurrentDataPipe(
		struct timeval* tp, uint8_t* data, const validState_type& validState) {

	const uint8_t szFloat = sizeof(float);
	union my_float_t {
		unsigned char c[szFloat];
		float f;
	} tmp;

	if (validState == validState_type::valid) {
		if (!data) {
			return;
		}
		if (mOrder == byteOrder_type::motorola || mOrder == byteOrder_type::high_byte_low_word) {
			bigbrother::swapBytesInIeeeSingleRead(data);
			memcpy(tmp.c, data, 4);
		} else if (mOrder == byteOrder_type::intel || mOrder == byteOrder_type::low_byte_low_word) {
			memcpy(tmp.c, data, szFloat);
		}
	}
#ifdef DEBUG
	bigbrother::printFloatAsChar(tmp.f);
#endif

	const bool enableDelta =
			(mDelta > 3 * std::numeric_limits<float>::epsilon());
	if (enableDelta /*&& mOldValueAvailable*/) {
		if ((validState == validState_type::valid) && (mOldValidState == validState_type::valid)
				&& (tmp.f >= mOldValue - mDelta)
				&& (tmp.f <= mOldValue + mDelta)) {
			PRINTDEBUG("delta => return, no value was changed.");
			return; //!Значение не вышло за пределы дельты
		}
	}

	mOldValue = tmp.f;
	//mOldValueAvailable = validState == validState_type::valid ? true : false;

	setValueToCurrentDataPipe(tp, std::to_string(tmp.f), validState);
}

