/**
 * @file      parametermodbusbool.h
 * @brief     Заголовочный файл класса логического Modbus-параметра
 * @details   Этот параметр заточен только по 0х05 и старт станции
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#ifndef PARAMETERMODBUSBOOL_H_
#define PARAMETERMODBUSBOOL_H_

#include <libxml++/libxml++.h>
#include <vector>
#include "parametermodbus.h"

using namespace std;

/**
 * @brief класс двоичного Modbus-параметра
 */
class V7ParameterModbusBool: public V7ParameterModbus
{
public:
    V7ParameterModbusBool();
    virtual ~V7ParameterModbusBool();

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
                uint8_t* data, const validState_type& validState) ;
    virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
    			const std::string& data, const validState_type& validState) {};

    virtual void writeParameterModbus();

private:
};

#endif /* PARAMETERMODBUSBOOL_H_ */

