/**
 * @file      Din16Dout8.h
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */
#ifndef DIN16DOUT8_H_
#define DIN16DOUT8_H_

#include "device.h"
#include "deviceShunt.h"
/**
 * @class Din16Dout8
 * @brief Класс для работы с Din16Dout8
 */
class Din16Dout8: public V7DeviceModbus
{
public:
    Din16Dout8();
    virtual ~Din16Dout8();
    virtual bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);
    virtual void Session();

protected:

    /**** Состояние шунта *****/
    unsigned int mWaitTimeWDT;
    uint16_t mDataOutSafe; /**< Состояние выходов по умолчанию*/
    uint16_t mDataOutCurrent; /**< Заданное состояние выходов*/
    uint8_t mWDTState;
    uint8_t mRsp[ MODBUS_TCP_MAX_ADU_LENGTH];
    /*** Функции ***/

protected:
    /**
     * @brief Установка значений в параметры за счет расшифровки слова-состояния
     * @param values
     */
    void setParametersValues(uint16_t values);

    uint16_t getDataOutSafeState(); /**< Получаем состояние выходов, заданные по умолчанию  */
    bool setDataOutCurrentState(uint16_t state);
    uint16_t getDataOutCurrentState(int &state); /**< Получаем состояние выходов, заданные по умолчанию  */
    uint16_t getDataInCurrentState(int &state); /**< Получаем состояние выходов, заданные по умолчанию  */
    bool setDataInCurrentState(uint16_t state); /**< Получаем состояние выходов, заданные по умолчанию  */
    uint8_t getWDTState();
    bool setWDTState(uint8_t state); /**< Взводит сторожевой таймер */
    bool resetWDTState(); /**< Сбрасывает сторожевой таймер */
    /*********/
    bool mbConnect();
    void mbDisconnect();
    /**
     * Коонвертирует адрес в 2^(Номер_бита)
     * @param addr виртуальный адрес параметра
     * @return число, соответсвующее номеру бита в слове состоянии
     */
    uint8_t cvrtParamAddrToStatusBit(uint16_t addr) const;
    /**
     * Установка бита в слове состояния
     * @param wordState
     * @param bitMask
     * @return
     */
    uint16_t setWordStateBit(uint16_t wordState, const uint16_t bitMask) const;
    /**
     * Сброс бита в слове состояния
     * @param wordState
     * @param bitMask
     * @return
     */
    uint16_t resetWordStateBit(uint16_t wordState, const uint16_t bitMask) const;
    /**
     * @brief Ждем пока порт занят
     */
    void waitFreePort();

};

class Din16Dout8v1: public Din16Dout8
{
public:
    virtual void Session();

private:
    bool setOuts(uint8_t state);
    uint16_t getDataOutCurrentState(int & state); /**< Получаем состояние выходов, заданные по умолчанию  */
};

#endif /* DIN16DOUT8_H_ */
