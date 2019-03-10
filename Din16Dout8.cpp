/**
 * @file      Din16Dout8.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Савченок Владимир
 * @copyright © TRIOLCORP, 2017
 */

#include <modbus.h>
#include <errno.h>
#include <parametermodbusfile.h>
#include <algorithm>    // std::sort
#include <cstring>
#include <string>
#include <sys/time.h>
#include "globals.h"
#include "port.h"
#include "parametermodbusint.h"
#include "parametermodbus.h"
#include "parametermodbusbool.h"
#include "parametermodbusenum.h"

/**
 * Внимание!
 * Диапазон адресов 1701-1708 для описания состояния выходов
 * 1601-1616 - для описания состония входов.
 * Это виртуальные адреса, их нельзя менять в конфиге.
 * Состояния выходов - сумма единиц:
 *  7   6   5   4   3  2  1  0
 *  128 64  32  16  8  4  2  1
 */
#include "Din16Dout8.h"

Din16Dout8::Din16Dout8() :
        mWDTState(0), mWaitTimeWDT(0), mDataOutCurrent(0), mDataOutSafe(0)
{
    PRINTDEBUG("Din16Dout8 started");
}

Din16Dout8::~Din16Dout8()
{

}

bool Din16Dout8::Init(const xmlpp::Node* pXMLNode, V7Port* pPort)
{

    if (!V7Device::Init(pXMLNode, pPort)
            || !V7DeviceModbus::Init(pXMLNode, pPort)) {
        return false;
    }

    const xmlpp::Element* deviceElement =
            dynamic_cast<const xmlpp::Element*>(pXMLNode);
    if (!deviceElement) {
        cerr << "Element \"DeviceModbus\" not found" << endl;
        return false;
    }

    int line = deviceElement->get_line();

    //!Забираем атрибуты
    const uint8_t numParams = 2;
    const xmlpp::Attribute* attributes[numParams];
    const Glib::ustring attrNames[numParams] = { "outState", "waitPeriod" };

    Glib::ustring attrValues[numParams];
    for (int i = 0; i < numParams; ++i) {
        attributes[i] = deviceElement->get_attribute(attrNames[i]);

        if (attributes[i] == NULL) {
            cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
                    << line << ") " << endl;
            return false;
        }
        if (attributes[i]) {
            attrValues[i] = attributes[i]->get_value();

        }
    }

    uint8_t index = 0;
    int val = 0;
    //!Преобразуем атрибуты

    //состояние выходов
    if (attributes[index]) {
        istringstream(attrValues[index]) >> val;
        if (val < 0 || val > 0xff) {
            val = 0; // ?
        }
        else {
            mDataOutCurrent = val;
            cout << ">>>>>>>>>>>>>>>> " << val << endl;
        }
    }
    else {
        mDataOutCurrent = 0;
    }

    mDataOutSafe = ~mDataOutCurrent; // безопасное состояние
    // wdt timeout
    if (attributes[++index]) {
        istringstream(attrValues[index]) >> val;
        if (val < 0) {
            val = 10000; // 1 sec
        }
        else {
            mWaitTimeWDT = val;
        }
    }
    return true;
}

uint8_t Din16Dout8::cvrtParamAddrToStatusBit(const uint16_t addr) const
{

    if (addr > 1700 && addr < 1709) {
        return 1 << (addr - 1700 - 1);
    }
    else {
        return 0;
    }

}

void Din16Dout8::Session()
{
    ++mSessionNumber;
    uint32_t sleepTime = 0;
    struct timeval sessionTime;
    struct timeval readingTime;
//!Берем текущее время
    gettimeofday(&sessionTime, NULL);
    uint16_t outState(mDataOutCurrent), inState(0);
    validState_type validState = validState_type::valid;
    int errCode(0);
    if (mbConnect()) {
        if (getWDTState() != 1) {
            setWDTState(1);
            setDataOutCurrentState(mDataOutCurrent);
        }
        mDataOutCurrent = outState = getDataOutCurrentState(errCode);
        inState = getDataInCurrentState(errCode);

        gettimeofday(&readingTime, NULL);
    }
    // запись состояния выходом
    validState = static_cast<validState_type>(errCode);
    uint8_t val[] = { 0, 0 };
    uint8_t *pData = val;
    typedef std::vector<V7Parameter*>::iterator param_it;
    for (param_it it = mvpParameters.begin(); it != mvpParameters.end(); ++it) {
        V7ParameterModbus* curParam = dynamic_cast<V7ParameterModbus*>(*it);

        if (curParam->isNewValue()
                && (curParam->getAddress() > 1700
                        && curParam->getAddress() < 1709)) {
            confirmData_type *confirmData = new confirmData_type;
            confirmData->globalID = curParam->getGlobalServerId();
            confirmData->dateTime = bigbrother::timeStamp();
            // фейковый параметр
            int newVal = 0;
            istringstream(curParam->getNewValue()) >> newVal;
            if (newVal < 0 || newVal > 1) {
                confirmData->state = inDataState_type::invalid_data;
                curParam->setDataToSetpointConfirmBuffer(confirmData);
            }
            else {
                //!Заносим результат в базу
                if (newVal == 1) {
                    outState |= cvrtParamAddrToStatusBit(
                            curParam->getAddress());
                }
                else {
                    outState &= ~cvrtParamAddrToStatusBit(
                            curParam->getAddress());
                }
                if (!setDataOutCurrentState(outState)) {
                    confirmData->state = inDataState_type::unknown;
                }
                else {
                    confirmData->state = inDataState_type::success;
                    mDataOutCurrent = outState;
                    curParam->resetNewValueFlag();
                }
                curParam->setDataToSetpointConfirmBuffer(confirmData);
            }
        }

        if (curParam->isNewValue() && curParam->getAddress() == 0x11) {
            unsigned int newVal = 0;
            istringstream(curParam->getNewValue()) >> newVal;
            bool res = setDataOutCurrentState(newVal);
            outState = mDataOutCurrent = getDataOutCurrentState(errCode);
            confirmData_type *confirmData = new confirmData_type;
            confirmData->globalID = curParam->getGlobalServerId();
            confirmData->dateTime = bigbrother::timeStamp();
            inDataState_type state = (inDataState_type) (
                    errCode == 0 ? inDataState_type::success : static_cast<inDataState_type>(errCode));
            confirmData->state = state;
            if (state == inDataState_type::success) {

                curParam->resetNewValueFlag();
            }
            curParam->setDataToSetpointConfirmBuffer(confirmData);
        }
        //передача состояния статусов

        if (curParam->getAddress() == 0x10) {
            pData[0] = static_cast<uint8_t>(inState >> 8);
            pData[1] = static_cast<uint8_t>(inState & 0xff);

            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        if (curParam->getAddress() == 0x11) {
            pData[0] = static_cast<uint8_t>(outState >> 8);
            pData[1] = static_cast<uint8_t>(outState & 0xff);
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        //read ins
        if (curParam->getAddress() > 1600 && curParam->getAddress() < 1617) {
            uint16_t addr = (curParam->getAddress() - 1600) - 1; // номер входа
            uint16_t mask = 1 << addr;
            pData[0] = 0;
            pData[1] = (((inState & mask) >> addr) & 1) ? 1 : 0;
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        //read outs
        if (curParam->getAddress() > 1700 && curParam->getAddress() < 1709) {
            uint16_t addr = (curParam->getAddress() - 1700) - 1; // номер входа
            uint16_t mask = 1 << addr; // номер входа
            pData[0] = 0;
            pData[1] = (((outState & mask) >> addr) & 1) ? 1 : 0;
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }

    }
    usleep(2500); // маленькая пауза в 2,5 мс
}

uint16_t Din16Dout8::getDataOutSafeState()
{

    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x12,
            0x00, 0x02); //!!!!
    waitFreePort();
    int mbRes = mpPort->request(msg);
    if (mbRes != 0) {
        delete msg;
        return 0xffff;
    }
    else {
        delete msg;
        return ((msg->getResponseBuffer()[3] << 8) + msg->getResponseBuffer()[4]);
    }

}

uint16_t Din16Dout8::getDataOutCurrentState(int &errCode)
{
    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x11,
            0x00, 0x02);

    waitFreePort();
    int mbRes = mpPort->request(msg);

    uint16_t result;
    if (mbRes == 0) {
        result = ((msg->getResponseBuffer()[3] << 8)
                + msg->getResponseBuffer()[4]);
        errCode = 0;
    }

    else {
        result = 0;
        errCode = mbRes;
    }
    delete msg;
    return result;

}

bool Din16Dout8::setDataOutCurrentState(uint16_t state)
{
    QueryMsgWriteSingle *msg = new QueryMsgWriteSingle((uint8_t) mAddress, 0x06,
            0x00, 0x11, 0x00, (uint8_t) state);

    waitFreePort();
    int mbRes = mpPort->request(msg);
    if (mbRes != 0) {
        delete msg;
        return false;
    }
    else {
        delete msg;
        return true;
    }
}

uint8_t Din16Dout8::getWDTState()
{

    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x0f,
            0x00, 0x01); //!!!!

    waitFreePort();
    int mbRes = mpPort->request(msg);
    uint8_t result = mbRes == 0 ? msg->getDataFromResponse()[1] : 0xff;
    delete msg;
    return result;
}

bool Din16Dout8::setWDTState(uint8_t state)
{

    QueryMsgWriteSingle *msg = new QueryMsgWriteSingle((uint8_t) mAddress, 0x06,
            0x00, 0x0f, 0x00, state == 0 ? 0 : 1);
    waitFreePort();
    int mbRes = mpPort->request(msg);
    if (mbRes != 0) {
        delete msg;
        return false;
    }
    else {
        delete msg;
        return true;
    }
}

bool Din16Dout8::resetWDTState()
{

    QueryMsgWriteSingle *msg = new QueryMsgWriteSingle((uint8_t) mAddress, 0x06,
            0x00, 0x0f, 0x00, 0x01);

    int mbRes = mpPort->request(msg);
    if (mbRes != 0) {
        delete msg;
        return false;
    }
    else {
        delete msg;
        return true;
    }

}

bool Din16Dout8::mbConnect()
{
    while (mpPort->isBusy()) {
        usleep(200);
    }
    return true;
}

void Din16Dout8::setParametersValues(uint16_t values)
{

}

void Din16Dout8::mbDisconnect()
{

}

uint16_t Din16Dout8::getDataInCurrentState(int &errCode)
{
    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x10,
            0x00, 0x02);

    waitFreePort();
    int mbRes = mpPort->request(msg);

    uint16_t result;
    if (mbRes == 0) {
        result = ((msg->getResponseBuffer()[3] << 8)
                + msg->getResponseBuffer()[4]);
        errCode = 0;

    }
    else {
        result = 0;
        errCode = mbRes;
    }

    delete msg;
    return result;
}

void Din16Dout8::waitFreePort()
{
    while (mpPort->isBusy()) {
        sched_yield();
    }
    // std::cout << std::endl;
}

void Din16Dout8v1::Session()
{

    ++mSessionNumber;
    struct timeval sessionTime;
    struct timeval readingTime;
//!Берем текущее время
    gettimeofday(&sessionTime, NULL);
    uint16_t outState(0), inState(0);
    validState_type validState = validState_type::valid;
    int errCode(0);
    if (mbConnect()) {
        int state = getWDTState();
        if (state != 1) {
            setWDTState(1);
            setDataOutCurrentState(mDataOutCurrent);
        }
        outState = getDataOutCurrentState(errCode);
        inState = getDataInCurrentState(errCode);
        gettimeofday(&readingTime, NULL);
    }
    // запись состояния выходом
    validState = static_cast<validState_type>(errCode);
    uint8_t val[] = { 0, 0 };
    uint8_t *pData = val;
    typedef std::vector<V7Parameter*>::iterator param_it;
    for (param_it it = mvpParameters.begin(); it != mvpParameters.end(); ++it) {
        V7ParameterModbus* curParam = dynamic_cast<V7ParameterModbus*>(*it);
        // outs

        if (curParam->isNewValue()
                && (curParam->getAddress() > 1700
                        && curParam->getAddress() < 1709)) {
            confirmData_type *confirmData = new confirmData_type;
            confirmData->globalID = curParam->getGlobalServerId();
            confirmData->dateTime = bigbrother::timeStamp();
            curParam->resetNewValueFlag(); // resert new flag
            // фейковый параметр
            int newVal = 0;
            istringstream(curParam->getNewValue()) >> newVal;
            if (newVal < 0 || newVal > 1) {
                confirmData->state = inDataState_type::invalid_data;
                curParam->setDataToSetpointConfirmBuffer(confirmData);
            }
            else {
                //!Заносим результат в базу
                if (newVal == 1) {
                    outState |= cvrtParamAddrToStatusBit(
                            curParam->getAddress());
                }
                else {
                    outState &= ~cvrtParamAddrToStatusBit(
                            curParam->getAddress());
                }
                //записываем в дин
                if (!setDataOutCurrentState(outState)) {
                    confirmData->state = inDataState_type::unknown;
                }
                else {
                    confirmData->state = inDataState_type::success;
                    mDataOutCurrent = outState;
                }
                curParam->setDataToSetpointConfirmBuffer(confirmData);
            }
        }

        if (curParam->isNewValue() && curParam->getAddress() == 0x11) {
            unsigned int newVal = 0;
            istringstream(curParam->getNewValue()) >> newVal;
            bool res = setDataOutCurrentState(newVal);
            outState = mDataOutCurrent = getDataOutCurrentState(errCode);
            confirmData_type *confirmData = new confirmData_type;
            confirmData->globalID = curParam->getGlobalServerId();
            confirmData->dateTime = bigbrother::timeStamp();
            inDataState_type state = (inDataState_type) (
                    errCode == 0 ? inDataState_type::success : static_cast<inDataState_type>(errCode));
            confirmData->state = state;
            if (state == inDataState_type::success) {

                curParam->resetNewValueFlag();
            }
            curParam->setDataToSetpointConfirmBuffer(confirmData);
        }
        //передача состояния статусов

        if (curParam->getAddress() == 0x10) {

            pData[0] = static_cast<uint8_t>(inState >> 8);
            pData[1] = static_cast<uint8_t>(inState & 0xff);

            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        if (curParam->getAddress() == 0x11) {
            pData[0] = static_cast<uint8_t>(outState >> 8);
            pData[1] = static_cast<uint8_t>(outState & 0xff);
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        //read ins
        if (curParam->getAddress() > 1600 && curParam->getAddress() < 1617) {
            uint16_t addr = (curParam->getAddress() - 1600) - 1; // номер входа
            uint16_t mask = 1 << addr;
            pData[0] = 0;
            pData[1] = (((inState & mask) >> addr) & 1) ? 1 : 0;
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }
        //read outs
        if (curParam->getAddress() > 1700 && curParam->getAddress() < 1709) {
            uint16_t addr = (curParam->getAddress() - 1700) - 1; // номер входа
            uint16_t mask = 1 << addr; // номер входа
            pData[0] = 0;

            pData[1] = (((outState & mask) >> addr) & 1) ? 1 : 0;
            curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
                    validState);
        }

    }
    usleep(2500); // пауза в 2,5 мс
}

bool Din16Dout8v1::setOuts(uint8_t state)
{
    QueryMsg0x42 *msg = new QueryMsg0x42((uint8_t) mAddress, 0x42, 0x00, state);
    waitFreePort();
    int mbRes = mpPort->request(msg);

    if (mbRes != 0) {
        delete msg;
        return false;
    }
    else {
        delete msg;
        return true;
    }
}

uint16_t Din16Dout8v1::getDataOutCurrentState(int &errCode)
{
    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x11,
            0x00, 0x01); ////!!!!!!!

    waitFreePort();
    int mbRes = mpPort->request(msg);

    uint16_t result;
    if (mbRes == 0) {
        result = ((msg->getResponseBuffer()[3] << 8)
                + msg->getResponseBuffer()[4]);
        errCode = 0;

    }
    else {
        result = 0;
        errCode = mbRes;
    }

    delete msg;
    return result;
}

uint16_t Din16Dout8::setWordStateBit(uint16_t wordState,
        const uint16_t bitMask) const
{
    return wordState | bitMask;
}

uint16_t Din16Dout8::resetWordStateBit(uint16_t wordState,
        const uint16_t bitMask) const
{
    return wordState & (~bitMask);
}
