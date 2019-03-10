/**
 * @file      parametermodbusint.h
 * @brief     Заголовочный файл класса целочисленного Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#ifndef PARAMETERMODBUSINT_H_
#define PARAMETERMODBUSINT_H_

#include <libxml++/libxml++.h>
#include <vector>
#include "parametermodbus.h"

using namespace std;

/**
 * @brief класс целочисленного Modbus-параметра
 */
class V7ParameterModbusInt: public V7ParameterModbus {
public:
	V7ParameterModbusInt();
	virtual ~V7ParameterModbusInt();

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
			const std::string& data, const validState_type& validState) {
	}
	virtual void writeParameterModbus();

private:
	string mUnit; /**< Единицы измерения */
	long mMaxValue; /**< Значение мин. */
	long mMinValue; /**< Значение макс. */
	double mDelta; /**< Отклонение значения от текущего, которое инициирует запись нового значения в БД. */
	double mOldValue; /**< Старое значение */
	enum {
		SIGNED_SIGN, /**< Знаковый */
		UNSIGNED_SIGN /**< Беззнаковый */
	} mSign; /**< Наличие знака */
	unsigned int mDecimalPlaces; /**< Количество знаков после запятой */
};

#endif /* PARAMETERMODBUSINT_H_ */
