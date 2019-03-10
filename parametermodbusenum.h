/**
 * @file      parametermodbusenum.h
 * @brief     Заголовочный файл класса перечисляемого Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#ifndef PARAMETERMODBUSENUM_H_
#define PARAMETERMODBUSENUM_H_

#include <libxml++/libxml++.h>
#include <vector>
#include "parametermodbus.h"

using namespace std;

/**
 * @brief класс перечисляемого Modbus-параметра
 */
class V7ParameterModbusEnum: public V7ParameterModbus {
public:
	V7ParameterModbusEnum();
	virtual ~V7ParameterModbusEnum();

	/**
	 * @fn Init
	 * @brief Инициализация перед использованием
	 * @details
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pDevice - указатель наустройство
	 * @return true - удачно, false - действие не удалось
	 */
	bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);

	/**
	 * @fn SetModbusValueToCurrentDataPipe
	 * @brief Отправить Modbus-данные в канал
	 * @details
	 * @param tp - время считывания показаний
	 * @param data - указатель на данные
	 * @param validState - Признак достоверности данных
	 * @return
	 */
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) {};
	virtual void writeParameterModbus();
private:
	std::vector<string> mvAllowedValues; /**< Список допустимых значений */
};



#endif /* PARAMETERMODBUSENUM_H_ */

