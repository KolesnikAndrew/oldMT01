/**
 * @file      parametermodbus.h
 * @brief     Заголовочный файл класса Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
#ifndef PARAMETERMODBUS_H_
#define PARAMETERMODBUS_H_

#include <modbus/modbus.h>
#include <libxml++/libxml++.h>
#include <vector>
#include "parameter.h"

/**
 * @brief класс Modbus-параметра
 */
class V7ParameterModbus: public V7Parameter {
public:
	V7ParameterModbus();
	virtual ~V7ParameterModbus();

	/**
	 * @fn Init
	 * @brief Инициализация перед использованием
	 * @details
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pDevice - указатель наустройство
	 * @return true - удачно, false - действие не удалось
	 */
	virtual bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	/**
	 * @fn SetModbusValueToCurrentDataPipe
	 * @brief Отправить Modbus-данные в канал текущих данных
	 * @details
	 * @param tp - время считывания показаний
	 * @param data - указатель на данные
	 * @param validState - Признак достоверности данных
	 * @return
	 */
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState) = 0;
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			const std::string& data, const validState_type& validState) = 0;
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState) = 0;
	/**
	 * @fn GetAddress
	 * @brief Получить адрес параметра
	 * @return адрес параметра
	 */
	unsigned int getAddress() const;

	/**
	 * @fn GetSize
	 * @brief Получить количество ячеек, занимаемых параметром
	 * @return количество ячеек, занимаемых параметром
	 */
	unsigned int getSize() const;

	/**
	 * @fn FunctionIsSupported
	 * @brief Поддерживается ли функция?
	 * @param func - проверяемая функция
	 * @return true - поддерживается, false - не поддерживается
	 */
	bool isFuncSupported(unsigned int func);
	/**
	 * Запись параметров в порт
	 */
	virtual void writeParameterModbus() = 0; //! Запись в порт
	inDataState_type cvrtErrToInputDataSTate(int16_t err);
	validState_type cvrtErrToValidState(int errCode) const;

	unsigned int mSessionNumber; /**< Номер сессии опроса параметра (служебный параметр для внутренних механизмов) */ //
	byteOrder_type getParamByteOrder() const;
	void setParamByteOrder(const byteOrder_type& paramOrder);

protected:
	std::vector<unsigned int> mvModbusFunctions; /**< Список допустимых функций ModBus */
	unsigned int mAddress; /**< Адрес */
	unsigned int mSize; /**< Количество ячеек, занимаемых параметром */
	byteOrder_type mOrder;

};

#endif /* PARAMETERMODBUS_H_ */
