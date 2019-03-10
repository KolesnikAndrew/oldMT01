/**
 * @file      timesync.h
 * @brief     Заголовочный файл класса параметра синхронизации времени
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2015
 */

#ifndef TIMESYNC_H_
#define TIMESYNC_H_

#include <libxml++/libxml++.h>
//#include <vector>
//#include "parametermodbus.h"

using namespace std;

/**
 * @brief класс параметра синхронизации времени
 */
class V7ParameterModbusTimeSync: public V7ParameterModbus
{
public:
    V7ParameterModbusTimeSync();
    virtual ~V7ParameterModbusTimeSync();

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
    virtual void SetModbusValueToCurrentDataPipe(struct timeval *tp,
            uint16_t* data, validState validState);

    virtual void WriteParameterModbus(modbus_t* pModbusContext);
};

#endif /* TIMESYNC_H_ */
