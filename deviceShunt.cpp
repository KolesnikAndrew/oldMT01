/**
 * @file      deviceShunt.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
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
#include "devicemodbus.h"

#include "deviceShunt.h"

DeviceShunt::DeviceShunt() :
        mWDTState(0), mWaitTimeWDT(0), mDataOutCurrent(0), mDataOutSafe(0), mAddress(
                0), mThread(0)
{
    PRINTDEBUG("Din16Dout8 (Shunt) started");
}

DeviceShunt::~DeviceShunt()
{

}

bool DeviceShunt::Init(const xmlpp::Node* pXMLNode, V7Port* pPort)
{

    if (!V7Device::Init(pXMLNode, pPort)) {
        return false;
    }

    const xmlpp::Element* deviceElement =
            dynamic_cast<const xmlpp::Element*>(pXMLNode);
    if (!deviceElement) {
        cerr << "Element \"DeviceShunt\" not found" << endl;
        return false;
    }

    int line = deviceElement->get_line();

    //!Забираем атрибуты
    const uint8_t numParams = 5;
    const xmlpp::Attribute* attributes[numParams];
    const Glib::ustring attrNames[numParams] = { "address", "waitPeriod",
            "outState", "byteTimeout", "responseTimeout" };

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
    //address---------------------------------------------------------------------------------------

    istringstream(attrValues[0]) >> val;
    if ((val < 1) || (val > 247)) {
        cerr << "Error in the value of the attribute \"address\" (line=" << line
                << ") " << endl;
        return false;
    }
    else {
        mAddress = val;
    }
    // время срабытывания WDT
    if (attributes[++index]) {
        istringstream(attrValues[index]) >> val;
        if (val <= 0 || val > 0xffff) {
            val = 1000; // 1000 ms
        }
        else {
            mWaitTimeWDT = val;
        }
    }
    else {
        mWaitTimeWDT = 1000;
    }

    //состояние выходов
    if (attributes[++index]) {
        istringstream(attrValues[index]) >> val;
        if (val < 0 || val > 0xff) {
            val = 0; // ?
        }
        else {
            mDataOutSafe = val;
        }
    }
    else {
        mDataOutSafe = 0;
    }
    //byteTimeout
    if (attributes[++index]) {
        int byteTimeout;
        istringstream(attrValues[index]) >> byteTimeout;
        if (byteTimeout <= 0 || byteTimeout > 5000) { //!Должен быть больше 0 и меньше 5с
            byteTimeout = 100; //! 100 мс
        }
        mByteTimeout.tv_sec = byteTimeout / 1000;
        mByteTimeout.tv_usec = (byteTimeout % 1000) * 1000;
    }
    //responseTimeout
    if (attributes[++index]) {
        int responseTimeout;
        istringstream(attrValues[index]) >> responseTimeout;
        if (responseTimeout <= 0 || responseTimeout > 5000) { //!Должен быть больше 0 и меньше 5с
            responseTimeout = 100; //!100 мс
        }
        mResponseTimeout.tv_sec = responseTimeout / 1000;
        mResponseTimeout.tv_usec = (responseTimeout % 1000) * 1000;
    }
    //!< Установка параметров тайм-аутов
    if (pPort->getBackendType() == RTU) {
        dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->responseTimeout =
                mResponseTimeout;
        dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->byteTimeout =
                mByteTimeout;
    }
    //! сброс счетчика

    return true;
}

void DeviceShunt::Session()
{

    while (true) {
        if (mbConnect()) {
            int state = getWDTState();

            if (state != 1) {
                setWDTState(1);
                setDataOutCurrentState(0xff);
            }

            mbDisconnect();
        }
        usleep((long) mWaitTimeWDT / 1.25 * 1000);
    }

}

uint16_t DeviceShunt::getDataOutSafeState()
{

    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x12,
            0x00, 0x02); //!!!!
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

uint16_t DeviceShunt::getDataOutCurrentState()
{
    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x11,
            0x00, 0x02);

    int mbRes = mpPort->request(msg);

    uint16_t result =
            mbRes == 0 ?
                    ((msg->getResponseBuffer()[3] << 8)
                            + msg->getResponseBuffer()[4]) :
                    0xff;
    delete msg;
    return result;

}

bool DeviceShunt::setDataOutCurrentState(uint16_t state)
{
    QueryMsgWriteSingle *msg = new QueryMsgWriteSingle((uint8_t) mAddress, 0x06,
            0x00, 0x11, 0x00, (uint8_t) state);

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

uint8_t DeviceShunt::getWDTState()
{

    QueryMsgRead *msg = new QueryMsgRead((uint8_t) mAddress, 0x03, 0x00, 0x0f,
            0x00, 0x01); //!!!!

    int mbRes = mpPort->request(msg);
    uint8_t result = mbRes == 0 ? msg->getResponseBuffer()[4] : 0xff;
    delete msg;
    return result;
}

bool DeviceShunt::setWDTState(uint8_t state)
{

    QueryMsgWriteSingle *msg = new QueryMsgWriteSingle((uint8_t) mAddress, 0x06,
            0x00, 0x0f, 0x00, state == 0 ? 0 : 1);
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

bool DeviceShunt::resetWDTState()
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

bool DeviceShunt::mbConnect()
{
    while (mpPort->isBusy()) {
        usleep(200);
    }
    return true;
}

void DeviceShunt::mbDisconnect()
{

}

bool DeviceShunt::Start()
{
    if (pthread_create(&mThread, NULL, DeviceShunt::ThreadFunc, (void*) this)
            == 0)
        return true;
    else
        return false;
}

bool DeviceShunt::Wait()
{
    if (pthread_join(mThread, NULL) == 0)
        return true;
    else
        return false;
}

