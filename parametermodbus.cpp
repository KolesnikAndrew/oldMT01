/**
 * @file      parametermodbus.cpp
 * @brief     Определение функций класса Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
#include <algorithm>
#include "globals.h"
#include "parametermodbus.h"
#include "utilites.h"

V7ParameterModbus::V7ParameterModbus() :
		mAddress(0), mSize(1), mOrder(byteOrder_type::motorola), mSessionNumber(0) {

}

V7ParameterModbus::~V7ParameterModbus() {

}

bool V7ParameterModbus::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {
	if (!V7Parameter::initParam(pXMLNode, pDevice)) {
		return false;
	}

	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusXxx\" not found" << endl;
		return false;
	}

	const int line = paramElement->get_line();

	//!Забираем атрибуты
	const uint8_t numOfParams = 4;
	const xmlpp::Attribute* attributes[numOfParams];
	const Glib::ustring attrNames[numOfParams] = { "functions", "address",
			"size", "order" };
	Glib::ustring attrValues[numOfParams];
	for (int i = 0; i < numOfParams; ++i) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		/*if ((!attributes[i]) && (i == 0)) {
		 cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
		 << line << ") " << endl;
		 return false;
		 }*/
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
		}
	}

	//!Преобразуем атрибуты
	//mbFunctions ----------------------------------------------------------------------------------
	istringstream tmpStream(attrValues[0]);
	int tmpFunc(0);
	while (tmpStream >> tmpFunc) {
		if (tmpFunc > 0 && tmpFunc < 256) {
			mvModbusFunctions.push_back(tmpFunc);
		}
	}
	if (mvModbusFunctions.empty()) {
		cerr << "Error in the value of the attribute \"functions\" (line="
				<< line << "). Add function 0x03 as default. " << endl;
		mvModbusFunctions.push_back(0x03);
	}
	//address --------------------------------------------------------------------------------------
	istringstream(attrValues[1]) >> mAddress;
	if (mAddress > 0xFFFF) {
		cerr << "Error in the value of the attribute \"address\" (line=" << line
				<< "). Set \"device address = 1\" as default. " << endl;
		mAddress = 1;
	}
	//size -----------------------------------------------------------------------------------------
	istringstream(attrValues[2]) >> mSize;
	if (mSize == 0 || mSize > 2) {
		cerr << "Error in the value of the attribute \"size\" (line=" << line
				<< "). Set \"size = 1\" as default." << endl;
		mSize = 1;
	}
	//order ----------------------------------------------------------------------------------------
	if (bigbrother::toUpperCase(attrValues[3]) == "INTEL")
		mOrder = byteOrder_type::intel;
	else if (bigbrother::toUpperCase(attrValues[3]) == "MOTOROLA")
		mOrder = byteOrder_type::motorola;
	else if (bigbrother::toUpperCase(attrValues[3]) == "HB_LW")
		mOrder = byteOrder_type::high_byte_low_word;
	else if (bigbrother::toUpperCase(attrValues[3]) == "HB_HW")
		mOrder = byteOrder_type::high_byte_high_word;
	else if (bigbrother::toUpperCase(attrValues[3]) == "LB_HW")
		mOrder = byteOrder_type::low_byte_high_word;
	else if (bigbrother::toUpperCase(attrValues[3]) == "LB_LW")
		mOrder = byteOrder_type::low_byte_low_word;
	else {
		cerr << "Error in the value of the attribute \"order\" (line=" << line
				<< "). Set \"Motorola order\" as default." << endl;
		mOrder = byteOrder_type::motorola;
	}

	return true;
}

unsigned int V7ParameterModbus::getAddress() const {
	return mAddress;
}

unsigned int V7ParameterModbus::getSize() const {
	return mSize;
}

bool V7ParameterModbus::isFuncSupported(unsigned int func) {
	return std::find(mvModbusFunctions.begin(), mvModbusFunctions.end(), func)
			!= mvModbusFunctions.end();
}

byteOrder_type V7ParameterModbus::getParamByteOrder() const{
	return mOrder;
}

void V7ParameterModbus::setParamByteOrder(const byteOrder_type& paramOrder){
	mOrder = paramOrder;
}

validState_type V7ParameterModbus::cvrtErrToValidState(int errCode) const {
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

inDataState_type V7ParameterModbus::cvrtErrToInputDataSTate(int16_t err) {
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

