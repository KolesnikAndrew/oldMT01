/**
 * @file      timesync.cpp
 * @brief     Определение функций класса параметра синхронизации времени
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2015
 */

#include <modbus.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"
#include "timesync.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "server.h"

V7ParameterModbusInt::V7ParameterModbusInt() :
    mMaxValue(0), mMinValue(0), mSign(SIGNED_SIGN), mDecimalPlaces(0)
{

}

V7ParameterModbusInt::~V7ParameterModbusInt()
{

}

bool V7ParameterModbusInt::Init(const xmlpp::Node* pXMLNode, V7Device* pDevice)
{
    if ( !V7ParameterModbus::Init(pXMLNode, pDevice))
        return false;

    const xmlpp::Element* paramElement =
        dynamic_cast<const xmlpp::Element*>(pXMLNode);
    if ( !paramElement) {
        cerr << "Element \"ParameterModbusInteger\" not found" << endl;
        return false;
    }

    int line = paramElement->get_line();

    //!Забираем атрибуты
    const xmlpp::Attribute* attributes[5] = { NULL, NULL, NULL, NULL, NULL };
    const Glib::ustring attrNames[5] = { "sign", "unit", "decimalPlaces",
                                         "minValue", "maxValue"
                                       };
    Glib::ustring attrValues[5];
    for (int i = 0; i < 5; i++) {
        attributes[i] = paramElement->get_attribute(attrNames[i]);
        if (( !attributes[i]) && ((i == 0) || (i == 1))) { //decimalPlaces, minValue и maxValue необязательные
            cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
                 << line << ") " << endl;
            return false;
        }
        if (attributes[i])
            attrValues[i] = attributes[i]->get_value();
    }

    //!Преобразуем атрибуты
    //sign minValue maxValue --------------------------------------------------------------------
    if (attrValues[0] == "signed")
        mSign = SIGNED_SIGN;
    else if (attrValues[0] == "unsigned")
        mSign = UNSIGNED_SIGN;
    else {
        cerr << "Error in the value of the attribute \"sign\" (line=" << line
             << ") " << endl;
        return false;
    }
    //unit --------------------------------------------------------------------------------------
    mUnit = attrValues[1];
    //decimalPlaces -----------------------------------------------------------------------------
    if (attributes[2])
        istringstream(attrValues[2]) >> mDecimalPlaces;
    //minValue maxValue -------------------------------------------------------------------------
    if (attributes[3] && attributes[4]) {
        //!Заменяем запятую на точку
        int find = attrValues[3].find(",");
        if (find >= 0)
            attrValues[3].replace(find, 1, ".");
        find = attrValues[4].find(",");
        if (find >= 0)
            attrValues[4].replace(find, 1, ".");

        //!Берем пределы в целочисленном виде
        double dTmp = atof(attrValues[3].c_str());
        dTmp *= pow(10, mDecimalPlaces);
        mMinValue = (long) dTmp;
        dTmp = atof(attrValues[4].c_str());
        dTmp *= pow(10, mDecimalPlaces);
        mMaxValue = (long) dTmp;

        if (mMinValue > mMaxValue) {
            cerr
                    << "Error in attribute values (\"minValue\", \"maxValue\") (line="
                    << line << ") " << endl;
            return false;

        }
    }

    return true;
}

void V7ParameterModbusInt::SetModbusValueToCurrentDataPipe(struct timeval *tp,
        uint16_t* data, validState_type validState)
{
    string strData;

    if (validState == VALID_STATE) {
        if ( !data)
            return;

        union {
            uint16_t uiData;
            int16_t siData;
        } i16Data;

        union {
            uint32_t uiData;
            int32_t siData;
        } i32Data;

        double dData;

        stringstream streamTmp;

        if (GetSize() == 1) {
            i16Data.uiData = *data;
            if (mSign == UNSIGNED_SIGN)
                dData = i16Data.uiData;    //streamTmp << i16Data.uiData;
            else
                //!mSign == SIGNED_SIGN
                dData = i16Data.siData; //streamTmp << i16Data.siData;
        } else { //etSize() == 2
            if (mOrder == MOTOROLA_ORDER) {
                i32Data.uiData = data[0];
                i32Data.uiData <<= 16;
                i32Data.uiData += data[1];
            } else { //mOrder == INTEL_ORDER
                i32Data.uiData = data[1];
                i32Data.uiData <<= 16;
                i32Data.uiData += data[0];
            }
            if (mSign == UNSIGNED_SIGN)
                dData = i32Data.uiData;   //streamTmp << i32Data.uiData;
            else
                //!mSign == SIGNED_SIGN
                dData = i32Data.siData; //streamTmp << i32Data.siData;
        }
        dData /= pow(10, mDecimalPlaces);
        streamTmp << dData;
        strData = streamTmp.str();
    }
    SetValueToCurrentDataPipe(tp, strData, validState);
}

void V7ParameterModbusInt::WriteParameterModbus(modbus_t* pModbusContext)
{
    if ( !mpNewValueFlag)
        return;
    V7DeviceModbus* pDeviceModbus = dynamic_cast<V7DeviceModbus*>(mpDevice);
    if ( !pDeviceModbus)
        return;
    V7Port* pPort = dynamic_cast<V7Port*>(pDeviceModbus->GetPort());
    if ( !pPort)
        return;

    pthread_mutex_lock( &pPort->mMutexInputData);
    string strNewValue = mpNewValue;
    mpNewValueFlag = false;
    pthread_mutex_unlock( &pPort->mMutexInputData);

    confirmData_type* confirmData = new confirmData_type;
    confirmData->globalID = mServerID;

    //!Берем текущее время
    struct timeval readingTime;
    gettimeofday( &readingTime, NULL);
    char strTmp[DATE_TIME_SIZE];
    struct tm* tm = gmtime( &readingTime.tv_sec);
    sprintf(strTmp, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
            tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    confirmData->dateTime = string(strTmp);

    if (this->mType == CURRENT_PARAM) {
        confirmData->state = ERR_READONLY_ISTATE;
        SetDataToSetpointConfirmBuffer(confirmData);
        return;
    }

    //!Заменяем запятую на точку
    int find = strNewValue.find(",");
    if (find >= 0)
        strNewValue.replace(find, 1, ".");

    //!Берем пределы в целочисленном виде
    double dNewValue = atof(strNewValue.c_str());
    dNewValue *= pow(10, mDecimalPlaces);
    long lNewValue = (long) dNewValue;

    if ((lNewValue < 0) && (mSign == UNSIGNED_SIGN)) {
        confirmData->state = ERR_INVALID_DATA_ISTATE;
        SetDataToSetpointConfirmBuffer(confirmData);
        return;
    }

    if (((mMinValue != mMaxValue) || (mMinValue != 0))
            && ((lNewValue > mMaxValue) || (lNewValue < mMinValue))) {
        confirmData->state = ERR_INVALID_DATA_ISTATE;
        SetDataToSetpointConfirmBuffer(confirmData);
        return;
    }

    union {
        uint16_t uiValue;
        int16_t iValue;
    } i16Value;
    union {
        uint32_t uiValue;
        int32_t iValue;
    } i32Value;
    uint16_t tmpUi16Arr[2];

    if (mSign == UNSIGNED_SIGN) {
        i16Value.uiValue = (uint16_t) lNewValue;
        i32Value.uiValue = (uint32_t) lNewValue;
    } else { //!mSign == SIGNED_SIGN
        i16Value.iValue = (int16_t) lNewValue;
        i32Value.iValue = (int32_t) lNewValue;
    }

    if (mOrder == MOTOROLA_ORDER) {
        tmpUi16Arr[1] = (uint16_t) i32Value.uiValue;
        i32Value.uiValue >>= 16;
        tmpUi16Arr[0] = (uint16_t) i32Value.uiValue;
    } else { //mOrder == INTEL_ORDER
        tmpUi16Arr[0] = (uint16_t) i32Value.uiValue;
        i32Value.uiValue >>= 16;
        tmpUi16Arr[1] = (uint16_t) i32Value.uiValue;
    }

    int mbRes;
    //!Какой функцией писать
    if (FunctionIsSupported(6)) {
        if (mSize == 1)
            for (int i = 0; i < TRIES_NUMBER; i++) {
                mbRes = modbus_write_register(pModbusContext, mAddress,
                                              (int) lNewValue);
                if (mbRes != -1)
                    break;
                usleep(500000);
            }
        else { //mSize == 2
            for (int i = 0; i < TRIES_NUMBER; i++) {
                mbRes = modbus_write_register(pModbusContext, mAddress,
                                              (int) tmpUi16Arr[0]);
                if (mbRes != -1)
                    break;
                usleep(500000);
            }
            if (mbRes != -1)
                for (int i = 0; i < TRIES_NUMBER; i++) {
                    mbRes = modbus_write_register(pModbusContext, mAddress + 1,
                                                  (int) tmpUi16Arr[1]);
                    if (mbRes != -1)
                        break;
                    usleep(500000);
                }
        }
    } else if (FunctionIsSupported(0x10)) {
        if (mSize == 1)
            for (int i = 0; i < TRIES_NUMBER; i++) {
                mbRes = modbus_write_registers(pModbusContext, mAddress, 1,
                                               &i16Value.uiValue);
                if (mbRes != -1)
                    break;
                usleep(500000);
            }
        else
            //mSize == 2
            for (int i = 0; i < TRIES_NUMBER; i++) {
                mbRes = modbus_write_registers(pModbusContext, mAddress, 2,
                                               tmpUi16Arr);
                if (mbRes != -1)
                    break;
                usleep(500000);
            }
    } else {
        confirmData->state = ERR_INVALID_FUNCTION_ISTATE;
        SetDataToSetpointConfirmBuffer(confirmData);
        return;
    }

    if (mbRes == -1) {
        //!Анализируем ошибку
        switch (errno) {
        case EMBXILFUN: //Illegal function
            confirmData->state = ERR_INVALID_FUNCTION_ISTATE; //Неверная функция запроса
            break;
        case EMBXILADD: //Illegal data address
            confirmData->state = ERR_INVALID_ADDRESS_ISTATE; //Неверное значение адреса
            break;
        case EMBXILVAL: //Illegal data value
            confirmData->state = ERR_INVALID_DATA_ISTATE;
            break;
        case EMBXSFAIL: //Slave device or server failure
        case 110:       //Connection timed out
            confirmData->state = ERR_NOT_RESPONDING_ISTATE; //Устройство не отвечает
            break;
        case EMBXSBUSY: //Slave device or server is busy
            confirmData->state = ERR_DEVICE_BUSY_ISTATE; //Устройство занято
            break;
        case EMBXGTAR: //Target device failed to respond
        case EMBBADCRC: //Invalid CRC
        case EMBBADDATA: //Invalid data
        case EMBBADEXC: //Invalid exception code
            confirmData->state = ERR_INVALID_ANSWER_ISTATE; //Ошибочный ответ устройства
            break;
        default:
            confirmData->state = ERR_UNKNOWN_ISTATE; //Неизвестная ошибка
            break;
        }
        SetDataToSetpointConfirmBuffer(confirmData);
        return;
    }

    //!Ставим задание на вычитку
    mLastReading.tv_sec = 0;
    mLastReading.tv_usec = 0;

    //!Заносим результат в базу
    confirmData->state = SUCCESS_SET_ISTATE;
    SetDataToSetpointConfirmBuffer(confirmData);
}

