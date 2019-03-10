/**
 * @file      parametermodbusint.cpp
 * @brief     Определение функций класса целочисленного Modbus-параметра
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
#include "parametermodbusint.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "server.h"
#include <algorithm>

V7ParameterModbusInt::V7ParameterModbusInt() :
		mSign(UNSIGNED_SIGN), mDecimalPlaces(0), mMinValue(0), mMaxValue(0), mDelta(
				0), mOldValue(0) {
	mParamType = paramType_type::p_int;
}

V7ParameterModbusInt::~V7ParameterModbusInt() {

}

bool V7ParameterModbusInt::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7ParameterModbus::initParam(pXMLNode, pDevice))
		return false;

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusInteger\" not found" << endl;
		return false;
	}

	int line = paramElement->get_line();

	//!Забираем атрибуты
	const xmlpp::Attribute* attributes[6] = { 0 };
	const Glib::ustring attrNames[6] = { "sign", "unit", "decimalPlaces",
			"minValue", "maxValue", "delta" };
	Glib::ustring attrValues[6];
	for (int i = 0; i < 6; i++) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		if ((!attributes[i]) && ((i == 0) || (i == 1))) { //decimalPlaces, minValue и maxValue необязательные
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i])
			attrValues[i] = attributes[i]->get_value();
	}

	//!Преобразуем атрибуты
	//sign --------------------------------------------------------------------
	if (attrValues[0] == "signed")
		mSign = SIGNED_SIGN;
	else if (attrValues[0] == "unsigned")
		mSign = UNSIGNED_SIGN;
	else {
		cerr << "Error in the value of the attribute \"sign\" (line=" << line
				<< ") " << endl;
		return false;
	}
	//unit --------------------------------------------------------------------------------------
	mUnit = attrValues[1];
	//decimalPlaces -----------------------------------------------------------------------------
	if (attributes[2])
		istringstream(attrValues[2]) >> mDecimalPlaces;
	//minValue maxValue -------------------------------------------------------------------------
	if (attributes[3] && attributes[4]) {
		//!
		//! Заменяем запятую на точку
		//! Берем пределы в целочисленном виде
		//! Внимание, это влияет на способ трансляции пределов в файле
		//! конфигурации модема. Такой вариант соответсвует записи параметров
		//! в экселевском файле, который передают разработчики УМКА
		//! Корректное отображение пределов сейчас реализовано в клиенте
		//! @date 01/11/2017
		//! @author vl
		mMinValue = static_cast<long>(atof(
				bigbrother::replaceCommaWithDot(attrValues[3]).c_str()));
		mMaxValue = static_cast<long>(atof(
				bigbrother::replaceCommaWithDot(attrValues[4]).c_str()));

		if (mMinValue > mMaxValue) {
			cerr
					<< "Error in attribute values (\"minValue\", \"maxValue\") (line="
					<< line << "). The value will be swapped. " << endl;
			std::swap(mMinValue, mMaxValue);
		}

	}
	//delta --------------------------------------------------------------------------------------
	if (attributes[5]) {
		//!Заменяем запятую на точку
		mDelta = atof(bigbrother::replaceCommaWithDot(attrValues[5]).c_str());
		if (mDelta < 0) {
			cerr << "Error in attribute values (\"delta\") (line=" << line
					<< "). Set delta to \"0\" as default. " << endl;
			mDelta = 0;
		}
	}

	return true;
}

void V7ParameterModbusInt::setModbusValueToCurrentDataPipe(struct timeval *tp,
		uint16_t* data, const validState_type& validState) {
	string strData;
	double dData = 0;

	if (validState == validState_type::valid) {
		if (!data)
			return;

		union {
			uint16_t uiData;
			int16_t siData;
		} i16Data;

		union {
			uint32_t uiData;
			int32_t siData;
		} i32Data;

		stringstream streamTmp;

		if (getSize() == 1) {
			i16Data.uiData = *data;
			if (mSign == UNSIGNED_SIGN)
				dData = i16Data.uiData;    //streamTmp << i16Data.uiData;
			else
				//!mSign == SIGNED_SIGN
				dData = i16Data.siData; //streamTmp << i16Data.siData;
		} else { //etSize() == 2
			if (mOrder == byteOrder_type::motorola) {
				i32Data.uiData = data[0];
				i32Data.uiData <<= 16;
				i32Data.uiData += data[1];
			} else { //mOrder == INTEL_ORDER
				i32Data.uiData = data[1];
				i32Data.uiData <<= 16;
				i32Data.uiData += data[0];
			}
			if (mSign == UNSIGNED_SIGN)
				dData = i32Data.uiData;   //streamTmp << i32Data.uiData;
			else
				//!mSign == SIGNED_SIGN
				dData = i32Data.siData; //streamTmp << i32Data.siData;
		}
		dData /= pow(10, mDecimalPlaces);
		streamTmp << dData;
		strData = streamTmp.str();
	}

	//!Отсеиваем значения, которые не изменились
	const bool enableDelta = (mDelta > 0);
	if (enableDelta /*&& mOldValueAvailable*/) { //!есть старые значения
		if ((validState == validState_type::valid)
				&& (mOldValidState == validState_type::valid)
				&& (dData <= (mOldValue + mDelta))
				&& (dData >= (mOldValue - mDelta))) //!Сравниваем значения
			return; //!Значение не вышло за пределы дельты
	}
	mOldValue = dData;
	//mOldValueAvailable = validState == validState_type::valid ? true : false;

	setValueToCurrentDataPipe(tp, strData, validState);
}

void V7ParameterModbusInt::setModbusValueToCurrentDataPipe(struct timeval* tp,
		uint8_t* pdata, const validState_type& validState) {
	string strData;
	double dData = 0;

	if (validState == validState_type::valid) {
		if (!pdata)
			return;

		union {
			uint16_t uiData;
			int16_t siData;
		} i16Data;

		union {
			uint32_t uiData;
			int32_t siData;
		} i32Data;

		stringstream streamTmp;
		if (getSize() == 1) {
			uint16_t data = static_cast<uint16_t>((pdata[0] << 8) + pdata[1]);
			i16Data.uiData = data;
			if (mSign == UNSIGNED_SIGN)
				dData = i16Data.uiData;    //streamTmp << i16Data.uiData;
			else
				//!mSign == SIGNED_SIGN
				dData = i16Data.siData; //streamTmp << i16Data.siData;
		} else { //etSize() == 2
			uint16_t data[2] = { static_cast<uint16_t>((pdata[0] << 8)
					+ pdata[1]), static_cast<uint16_t>((pdata[2] << 8)
					+ pdata[3]) };
			if (mOrder == byteOrder_type::motorola) {
				i32Data.uiData = data[0];
				i32Data.uiData <<= 16;
				i32Data.uiData += data[1];
			} else { //mOrder == INTEL_ORDER
				i32Data.uiData = data[1];
				i32Data.uiData <<= 16;
				i32Data.uiData += data[0];
			}

			if (mSign == UNSIGNED_SIGN)
				dData = i32Data.uiData;   //streamTmp << i32Data.uiData;
			else
				//!mSign == SIGNED_SIGN
				dData = i32Data.siData; //streamTmp << i32Data.siData;
		}
		dData /= pow(10, mDecimalPlaces);
		streamTmp << dData;
		strData = streamTmp.str();
	}

	//!Отсеиваем значения, которые не изменились
	const bool enableDelta = (mDelta > 0);
	if (enableDelta /*&& mOldValueAvailable*/) {
		if ((validState == validState_type::valid)
				&& (mOldValidState == validState_type::valid)
				&& (dData >= mOldValue - mDelta)
				&& (dData <= mOldValue + mDelta)) {
			PRINTDEBUG("delta => return, no value was changed.");
			return; //!Значение не вышло за пределы дельты
		}
	}

	mOldValue = dData;

	setValueToCurrentDataPipe(tp,
			(mDecimalPlaces == 0 ?
					std::to_string(static_cast<long int>(dData)) :
					strData), validState);
}

void V7ParameterModbusInt::writeParameterModbus() {

	std::string strNewValue;

	if (!mNewValueFlag || !getStrNewValueFromPipe(strNewValue)
			|| !mpDeviceModbus || !mpPort) {
		return;
	}

	confirmData_type* confirmData = new confirmData_type;
	confirmData->globalID = mServerID;
	confirmData->dateTime = bigbrother::timeStamp(); // текущее время

	if (this->mType == CURRENT_PARAM) {
		confirmData->state = inDataState_type::readonly;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	//!Заменяем запятую на точку
	strNewValue = bigbrother::replaceCommaWithDot(strNewValue);
	//!Берем пределы в целочисленном виде
	long lNewValue = static_cast<long>(atof(strNewValue.c_str())
			* pow(10, mDecimalPlaces));
#ifdef DEBUG
	cout << "lNewValue" << lNewValue << endl;
#endif

	// проверка знака
	if ((lNewValue < 0) && (mSign == UNSIGNED_SIGN)) {
		confirmData->state = inDataState_type::invalid_data;
		clog << "int => Error, incorrect unsigned value" << endl;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	// проверка диапазона
	if ((lNewValue > mMaxValue) || (lNewValue < mMinValue)) {
#ifdef DEBUG
		cout << dec << "mMaxValue" << mMaxValue << endl;
		cout << "mMinValue" << mMinValue << endl;
		cout << "(lNewValue > mMaxValue)" << boolalpha
				<< (lNewValue > mMaxValue) << endl;
		cout << "(lNewValue < mMinValue)" << boolalpha
				<< (lNewValue < mMinValue) << endl;
#endif
		clog << "int => Error, new value out of range" << endl;
		confirmData->state = inDataState_type::invalid_data;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}

	union {
		uint16_t uiValue;
		int16_t iValue;
	} i16Value;
	union {
		uint32_t uiValue;
		int32_t iValue;
	} i32Value;

	//этот массмв будем писать
	uint16_t tmpUi16Arr[2];

	if (mSign == UNSIGNED_SIGN) {
		//! формируем целое безнаковок
		i16Value.uiValue = (uint16_t) lNewValue;
		i32Value.uiValue = (uint32_t) lNewValue;
	} else {
		//! формируем целое знаковое
		i16Value.iValue = static_cast<int16_t>(lNewValue);
		i32Value.iValue = static_cast<int32_t>(lNewValue);
	}
//!формируем массив для записи
	if (mOrder == byteOrder_type::motorola) {
		tmpUi16Arr[1] = static_cast<uint16_t>(i32Value.uiValue);

		tmpUi16Arr[0] = static_cast<uint16_t>(i32Value.uiValue >> 16);
	} else { //mOrder == INTEL_ORDER
		tmpUi16Arr[0] = static_cast<uint16_t>(i32Value.uiValue);

		tmpUi16Arr[1] = static_cast<uint16_t>(i32Value.uiValue >> 16);
	}

	int mbRes;
	//!Какой функцией писать
	if (isFuncSupported(0x06)) {
		QueryMsgWriteSingle *msg = new QueryMsgWriteSingle;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x06;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress & 0xff);
		if (mSize == 1) {
			msg->mbValueHi = static_cast<uint8_t>(lNewValue >> 8);
			msg->mbValueLo = static_cast<uint8_t>(lNewValue & 0xff);
			//! Запрос
			waitForFreePort();
			mbRes = mpPort->request(msg);
			delete msg;
		} else { //mSize == 2
			msg->mbValueHi = static_cast<uint8_t>(tmpUi16Arr[0] >> 8);
			msg->mbValueLo = static_cast<uint8_t>(tmpUi16Arr[0] & 0xff);
			waitForFreePort();
			mbRes = mpPort->request(msg);
			if (mbRes == 0) {
				msg->mbAddressHi = static_cast<uint8_t>((mAddress + 2) >> 8);
				msg->mbAddressLo = static_cast<uint8_t>((mAddress + 2) & 0xff);
				msg->mbValueHi = static_cast<uint8_t>(tmpUi16Arr[1] >> 8);
				msg->mbValueLo = static_cast<uint8_t>(tmpUi16Arr[1] & 0xff);
			}
		}
	} else if (isFuncSupported(0x10)) {
		QueryMsgWriteMulti *msg = new QueryMsgWriteMulti;
		msg->mbDevAddr = static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
		msg->mbFuncCode = 0x10;
		msg->mbAddressHi = static_cast<uint8_t>(mAddress >> 8);
		msg->mbAddressLo = static_cast<uint8_t>(mAddress & 0xff);
		if (mSize == 1) {
			msg->mbQuantityRegHi = 0x00;
			msg->mbQuantityRegLo = 0x01;
			msg->mbByteCnt = 0x02;
			msg->mbValues = reinterpret_cast<uint8_t*>(&i16Value.uiValue);
			//! Запрос
			waitForFreePort();
			mbRes = mpPort->request(msg);
			delete msg;
		} else {
			msg->mbQuantityRegHi = 0x00;
			msg->mbQuantityRegLo = 0x02;
			msg->mbByteCnt = 0x04;
			msg->mbValues = (uint8_t*) tmpUi16Arr;
			waitForFreePort();
			mbRes = mpPort->request(msg);
			delete msg;

		}
	} else {
		confirmData->state = inDataState_type::invalid_function;
		setDataToSetpointConfirmBuffer(confirmData);
		return;
	}
	//!Анализируем ошибку
	if (mbRes != 0) {
		confirmData->state = cvrtErrToInputDataSTate(mbRes); //!!!!!!!
		setDataToSetpointConfirmBuffer(confirmData);
	} else {
		//!Ставим задание на вычитку
		PRINTDEBUG2("ParamInt", "Now need to read");
		mLastReading.tv_sec = 0;
		mLastReading.tv_usec = 0;
		//!Заносим результат в базу
		confirmData->state = inDataState_type::success;
		setDataToSetpointConfirmBuffer(confirmData);
	}
}
