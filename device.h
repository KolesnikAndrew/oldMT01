/**
 * @file      device.h
 * @brief     Заголовочный файл класса устройства на порту
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
#ifndef DEVICE_H_
#define DEVICE_H_

#include <vector>
#include <libxml++/libxml++.h>
#include "server.h"
#include "parametermodbusfile.h"

using namespace std;

class V7Parameter;
class V7Port;
class ParameterModbusFile;

/**
 * @brief класс устройства на порту
 */
class V7Device
{
public:
    V7Device();
    virtual ~V7Device();

    /**
     * @fn Init
     * @brief Инициализация перед использованием
     * @details
     * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
     * @param pPort - указатель на порт устройства
     * @return true - удачно, false - действие не удалось
     */
    virtual bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);

    /**
     * @fn GetPort
     * @brief Получить указатель на порт
     * @details
     * @param
     * @return указатель или NULL
     */
    V7Port* getPort();

    /**
     * @fn Session
     * @brief Сеанс связи с устройством
     * @details
     * @param
     * @return
     */
    virtual void Session() = 0;

    bool setDataToDevice(inputData_type* inputData);
    bool setDataToDevice(umkaFile_type* inputData);

    unsigned int mSessionNumber; /**< Номер сессии опроса параметра (служебный параметр для внутренних механизмов) */
    unsigned int  getUmkaConfigFileIdAndSetConfigBuffer() const;

    //validState_type cvrtErrToValidState(int errCode) const;
//    int getTypeId() const;
protected:
    std::vector<V7Parameter*> mvpParameters; /**< Список указателей на параметры устройства */
    std::vector<V7Parameter*> mvpUmkaFiles; /**< Список указателей на параметры устройства */
    std::vector<V7Parameter*> mvpStatusWords; /**< Список указателей на параметры устройства */
    std::vector<V7Parameter*> mvpByteSingleArrays; /**< Список указателей на параметры устройства */
    std::vector<V7Parameter*> mvpSingleMatrix;
    unsigned int mNumber; /**< Номер устройства в рамках модема */
    int mBaud; /**< Скорость обмена */
    char mParity; /**< Четность ('N' -none, 'E' - четный, 'O' - нечетный) */
    int mDataBit; /**< Количество бит данных 5, 6, 7 или 8 */
    int mStopBit; /**< Количество стоп-бит 1 или 2 */
    string mSerialNumber; /**< Серийный номер */
    string mName; /**< Наименование (строка до 250 символов) */
    V7Port* mpPort; /**< Указатель на порт устройства */
    std::string mConfigAT27Buffer;
    int mTypeID; //! тип устройства.
                //! 1001 - DinDout_v1
                //! 1002 - DinDout_v2


public:
    validState_type cvrtErrToValidState(int errCode) const;
    inDataState_type cvrtErrToInputDataSTate(int16_t err);
};

#endif /* DEVICE_H_ */
