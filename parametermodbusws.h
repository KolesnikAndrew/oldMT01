/**
 * @file      parametermodbusenum.cpp
 * @brief     Определение функций класса перечисляемого Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#ifndef PARAMETER_MODBUS_WS
#define PARAMETER_MODBUS_WS

#include <libxml++/libxml++.h>
#include <vector>
#include "parametermodbus.h"

/**
 * @brief enum for bit mask
 */
class ParameterModbusWordState: public V7ParameterModbus {
public:
	ParameterModbusWordState();
	~ParameterModbusWordState();
	bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) {};
	virtual void writeParameterModbus();
	uint16_t getMask() const;
	void setStatus(uint16_t val);
	uint16_t getStatus() const;
private:
	uint8_t mLastBit;
	uint8_t mFirstBit;
	uint16_t mMask;
	uint16_t mStatus;
	std::vector<string> mvAllowedValues; /**< Список допустимых значений */
	void makeMask();
};

#endif
