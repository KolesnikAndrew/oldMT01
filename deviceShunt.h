/**
 * @file      deviceShunt.h
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */
#ifndef DEVICESHUNT_H_
#define DEVICESHUNT_H_

#include <modbus.h>
#include <errno.h>
#include <algorithm>    // std::sort
#include <cstring>
#include <sys/time.h>

#include "globals.h"
#include "port.h"
#include "devicemodbus.h"
#include "parametermodbusint.h" /**< для адреса выхода  **/

class DeviceShunt: public V7Device
{
public:
    DeviceShunt();
    virtual ~DeviceShunt();
    /**
     * @fn Init)_
     * @brief Инициализация перед использованием
     * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
     * @param pPort - указатель на порт устройства
     * @return true - удачно, false - действие не удалось
     */
    bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);
    /**
     * @fn Session
     * @brief Сеанс связи с устройством
     */
    void Session();
    /**
     * @fn Start()
     * Функция старта потока
     * @return
     */
    bool Start();
    /**
     * @fn Wait()
     * Функция ожидания выполнения потока
     * @return
     */
    bool Wait();

private:
    int mAddress; /**< адрес устройства */
    struct timeval mByteTimeout;
    struct timeval mResponseTimeout;
    /**** Состояние шунта *****/
    unsigned int mWaitTimeWDT;
    uint16_t mDataOutSafe; /**< Состояние выходов по умолчанию*/
    uint16_t mDataOutCurrent; /**< Заданное состояние выходов*/
    uint8_t mWDTState;
    /*** Функции ***/
    uint16_t getDataOutSafeState(); /**< Получаем состояние выходов, заданные по умолчанию  */
    bool setDataOutCurrentState(uint16_t state);
    uint16_t getDataOutCurrentState(); /**< Получаем состояние выходов, заданные по умолчанию  */
    uint8_t getWDTState();
    bool setWDTState(uint8_t state); /**< Взводит сторожевой таймер */
    bool resetWDTState(); /**< Сбрасывает сторожевой таймер */
    /*********/
    bool mbConnect();
    void mbDisconnect();
    uint8_t mRsp[ MODBUS_TCP_MAX_ADU_LENGTH];

    static void *ThreadFunc(void *d)
    {
        static_cast<DeviceShunt *>(d)->Session();
        return NULL;
    }
    pthread_t mThread;

};

#endif /* DEVICESHUNT_H_ */
