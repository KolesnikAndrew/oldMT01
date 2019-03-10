/**
 * @file      QueryMsgI.h
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Савченко Владимир
 * @copyright © TRIOLCORP, 2017
 */
#ifndef QUERYMSG_H_
#define QUERYMSG_H_

#include <stdint.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <modbus.h>

//////////////////////////////////////////////////////////////////////////
/**
 * @class QueryMsg
 * @brief Структура для передачи запроса
 * @details Класс используется для построения запосов к Модбас движку @class EngineModbus @link EngineModbus.h.
 * Реализованы структуры для функций: 0x03, 0x04, 0x05, 0x06, 0x10 и функций вычитки журналов АТ27
 *
 */
struct QueryMsg
{
    QueryMsg();
    virtual ~QueryMsg();
    /**
     * @brief Собирает массив - raw request
     * @return Сконструированный массив с запросом
     */
    /// Requset
    /**
     * @brief Создаем сырой запрос к устройству
     */
    virtual void makeRawRequest() = 0;
    /**
     * @brief Размер запроса
     * @return размер запроса в байтах
     */
    int16_t getRequestSize() const;
    /**
     * @brief Возвращает ссылку на буфер запроса
     * @return ссылку на 0-й элемен массива
     */
    uint8_t *getRequestBuffer();
    /**
     * @brief  Размер ответа без CRC и преамбулы TCP
     * @return Длина массива без CRC и преамбулы TCP
     */
    int16_t getResponseSize() const;
    /**
     * @brief Устанавливаем размер ответа
     * @param size - размер массива
     */
    void setResponseSize(const int16_t size);
    /**
     * Указатель на 0-й элемент буффера ответа
     * @return указатель на буфер результатат
     */
    uint8_t *getResponseBuffer();
    /**
     * @brief Записывает ответ от бек-енда во внутренний массив
     * @param rsp - массив
     * @param size - размер
     */
    void setResponseBuffer(const uint8_t* rsp, const int16_t size);
    /**
     * @brief Текущее состояние - ошибки
     * @return 0 или код ошибки
     */
    int16_t getErrorCode() const;
    /**
     * @brief Установка кода ошибки
     * @param err - код ошибки
     */
    void setErrorCode(const int16_t err);
    /**
     * @brief Печать отладочной инфоррмаци для всей иерарзии
     * @param stream - поток ostream
     * @return ссылку на поток
     */
    virtual std::ostream& print(std::ostream& stream) const;
    /**
     * @brief Перегруженный оператор вывода
     * @param stream - поток ostream
     * @param instance - объект QueryMsg
     * @return ссылку на поток с данными
     */
    friend std::ostream& operator<<(std::ostream& stream,
            const QueryMsg& instance);
    /**
     * @brief Возвращает только массив данных из ответа
     * @return массив байт
     */
    virtual uint8_t* getDataFromResponse();
    /**
     * @brief Размер данных из ответа
     * @return размер массива в байтах
     */
    virtual int16_t getDataSize();
    /**
     * @brief Возвращает код функции запроса
     * @return код функции или 0;
     */
    uint8_t getRequestFunction() const;
    /**
     * @brief Возвращает код функции ответа
     * @return код функции или 0;
     */
    uint8_t getResponseFunction() const;
    virtual uint16_t getParamAddress() const ;
protected:
    std::vector<uint8_t> mReqBuf; //!< буфер запроса
    std::vector<uint8_t> mRspBuf; //!< буфер ответа
    int16_t mErrCode; //!< код ошибки
    int16_t mReqSize; //!< размер запорса (в байтах)
    int16_t mRspSize; //!< размер ответа  (в байтах)
    uint16_t mParamAddress;
};

/**
 * @class QueryMsgRead
 * @brief Стандартные функции
 */
struct QueryMsgRead: public QueryMsg
{
    QueryMsgRead();
    QueryMsgRead(uint8_t devAddr, uint8_t funcCode, uint8_t addressHi,
            uint8_t addressLo, uint8_t quantityHi, uint8_t quantityLo);
    uint8_t mbDevAddr;    //!< адрес модбас-устройства
    uint8_t mbFuncCode;   //!< modbus function code
    uint8_t mbAddressHi;  //!< аддресное пространство модбас
    uint8_t mbAddressLo;  //!< аддресное пространство модбас
    uint8_t mbQuantityHi; //!< количество байт
    uint8_t mbQuantityLo;
    virtual void makeRawRequest();
    virtual uint8_t* getDataFromResponse();
    virtual int16_t getDataSize();
    virtual std::ostream& print(std::ostream& stream) const;
};
/**
 * @class QueryMsgWriteSingle
 * @brief  Write 0x06 function
 */
struct QueryMsgWriteSingle: public QueryMsg
{
    QueryMsgWriteSingle();
    QueryMsgWriteSingle(uint8_t devAddr, uint8_t funcCode, uint8_t addressHi,
            uint8_t addressLo, uint8_t valueHi, uint8_t valueLo);
    uint8_t mbDevAddr;    //!< адрес модбас-устройства
    uint8_t mbFuncCode;   //!< modbus function code
    uint8_t mbAddressHi;  //!< старший байт
    uint8_t mbAddressLo;  //!< младший байт
    uint8_t mbValueHi;     //!< старший байт
    uint8_t mbValueLo;     //!< младший байт
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
};

/**
 * @class QueryMsgWriteMulti
 * @brief write 0x10 function
 */
struct QueryMsgWriteMulti: public QueryMsg
{
    QueryMsgWriteMulti();
    QueryMsgWriteMulti(uint8_t devAddr, uint8_t funcCode, uint8_t addressHi,
            uint8_t addressLo, uint8_t quantityRegHi, uint8_t quantityRegLo,
            uint8_t byteCnt, uint8_t * values);
    uint8_t mbDevAddr;       //!< адрес модбас-устройства
    uint8_t mbFuncCode;      //!< modbus function code
    uint8_t mbAddressHi;     //!< старший байт
    uint8_t mbAddressLo;     //!< младший байт
    uint8_t mbQuantityRegHi; //!< кол-во байт - старший байт всегда 0?
    uint8_t mbQuantityRegLo; //!< младший байт - 0x01 .. 0x7b
    uint8_t mbByteCnt; //!< количество байт 2 * (mbQuantityRegHi << 8 + mbQuantityRegLo)
    uint8_t * mbValues;      //!< данные
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;

};
/**
 * @class QueryMsgJrnAT27
 * @brief Функции работы с файлами АТ27 (raw request)
 */
struct QueryMsgJrnAT27: public QueryMsg
{
    QueryMsgJrnAT27();
    QueryMsgJrnAT27(uint8_t devAddr, uint8_t funcCode, uint8_t fileNumber,
            uint8_t startAddrHiWordHi, uint8_t startAddrHiWordLo,
            uint8_t startAddrLoWordHi, uint8_t startAddrLoWordLo,
            uint8_t cntRecords);
    uint8_t mbDevAddr;            //!< адрес модбас-устройства
    uint8_t mbFuncCode;           //!< modbus function code
    uint8_t mbFileNumber;         //!< аддресное пространство модбас
    uint8_t mbStartAddrHiWordHi;  //!< аддрес старший байт
    uint8_t mbStartAddrHiWordLo;  //!< аддрес
    uint8_t mbStartAddrLoWordHi;  //!< аддрес
    uint8_t mbStartAddrLoWordLo;  //!< аддрес младший байт
    uint8_t mbCntRecords;         //!< количество записей (1..122))
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
};
/**
 * @class QueryMsgCfgAT27
 * @brief Функции работы с конфигами АТ27 (raw request)
 */
struct QueryMsgCfgAT27: public QueryMsgJrnAT27
{
    QueryMsgCfgAT27();
    QueryMsgCfgAT27(uint8_t devAddr, uint8_t funcCode, uint8_t fileNumber,
            uint8_t startAddrHiWordHi, uint8_t startAddrHiWordLo,
            uint8_t startAddrLoWordHi, uint8_t startAddrLoWordLo,
            uint8_t cntRecords, uint8_t *values);
    uint8_t *mbValues;            //!< массив данных (только для записи)
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
};

/**
 * @class QueryMsgRaw
 * @brief Raw request
 */
struct QueryMsgRaw: public QueryMsg
{
    QueryMsgRaw();
    QueryMsgRaw(uint8_t reqLength, uint8_t* reqBuf);
    uint8_t mbReqLength; //!< кол-во байт в посылке или ответе (CRC не учитывается)
    uint8_t* req;       //!< данные запроса - массив байт
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
    // MODBUS_TCP_MAX_ADU_LENGTH = 260
    // MODBUS_RTU_MAX_ADU_LENGTH  256
};

/**
 * @brief QueryMsgWriteSingleCoil
 * @brief  Raw request
 */
struct QueryMsgWriteSingleCoil: public QueryMsg
{
    QueryMsgWriteSingleCoil();
    QueryMsgWriteSingleCoil(uint8_t devAddr, uint8_t funcCode,
            uint8_t addressHi, uint8_t addressLo, uint8_t valueHi,
            uint8_t valueLo);
    uint8_t mbDevAddr;    //!< адрес модбас-устройства
    uint8_t mbFuncCode;   //!< modbus function code
    uint8_t mbAddressHi;  //!< старший байт
    uint8_t mbAddressLo;  //!< младший байт
    uint8_t mbValueHi;     //!< старший байт
    uint8_t mbValueLo;     //!< младший байт
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
};

/**
 * @class QueryMsgReadFileRecords
 * @brief Файловое стение 0x14
 */
struct QueryMsgReadFileRecords: public QueryMsg
{
    QueryMsgReadFileRecords();
    QueryMsgReadFileRecords(uint8_t devAddr, uint8_t funcCode, uint8_t byteCnt,
            uint8_t refType, uint8_t fileNumberHi, uint8_t fileNumberLo,
            uint8_t recAddressHi, uint8_t recAaddressLo, uint8_t recLenHi,
            uint8_t recLenLo);
    uint8_t mbDevAddr;        //!< адрес модбас-устройства
    uint8_t mbFuncCode;       //!< modbus function code
    uint8_t mbByteCnt;        //!< количество байт 0x07..0xF5
    uint8_t mbRefType;        //!< тип - всегда 0x06
    uint8_t mbFileNumberHi;   //!< номер файла
    uint8_t mbFileNumberLo;   //!< номер файла
    uint8_t mbRecordNumberHi; //!< номер записи
    uint8_t mbRecordNumberLo; //!< номер записи
    uint8_t mbRecordLengthHi; //!< размер записи
    uint8_t mbRecordLengthLo; //!< разсер записи
    virtual void makeRawRequest();
    virtual uint8_t *getDataFromResponse();
    virtual int16_t getDataSize();
    virtual std::ostream& print(std::ostream& stream) const;
    /**
     * @brief Проверяет на заполненность массив секций памяти для вычитки журнала из УМКА03
     * @return состояние ячейки памяти журанлов УМКА03 - 1 - заполнена, 0 - пустая.
     */
    bool checkUM03JournalFilling();

};
/**
 * @class QueryMsg0x42
 * @brief Класс для раоты с запросами к Din16Dout8
 */
struct QueryMsg0x42: public QueryMsg
{
    QueryMsg0x42();
    QueryMsg0x42(uint8_t devAddr, uint8_t funcCode, uint8_t dataHi,
            uint8_t dataLo);

    uint8_t mbDevAddr;        //!< адрес модбас-устройства
    uint8_t mbFuncCode;       //!< modbus function code
    uint8_t mbDataHi;
    uint8_t mbDataLo;
    virtual void makeRawRequest();
    virtual std::ostream& print(std::ostream& stream) const;
};

/**
 * @brief Read Device Identification
 */
struct QueryMsg0x2b: public QueryMsg
{
    QueryMsg0x2b();
    QueryMsg0x2b(int8_t devAddr, uint8_t funcCode, uint8_t MEIType,
            uint8_t mReadDeviceIDcode, uint8_t mObjectId);
    uint8_t mbDevAddr;        //!< адрес модбас-устройства
    uint8_t mbFuncCode;       //!< modbus function code
    uint8_t mbMEIType;         //!< 0x0e
    uint8_t mbReadDeviceIDcode; //!< 0x01, 0x02, 0x03, 0x04
    uint8_t mbObjectId;
    virtual void makeRawRequest();
    virtual uint8_t* getDataFromResponse();
    virtual int16_t getDataSize();
    virtual std::ostream& print(std::ostream& stream) const;
    unsigned short getUMKA03Id();
    int getUMKA27Id();
};

//struct QueryMsgReadSingleArray: public QueryMsg
//{
//	QueryMsgReadSingleArray();
//	QueryMsgReadSingleArray(uint8_t devAddr, uint8_t funcCode, uint8_t addressHi,
//            uint8_t addressLo, uint16_t arrSize);
//    uint8_t mbDevAddr;    //!< адрес модбас-устройства
//    uint8_t mbFuncCode;   //!< modbus function code
//    uint8_t mbAddressHi;  //!< аддресное пространство модбас
//    uint8_t mbAddressLo;  //!< аддресное пространство модбас
//    virtual void makeRawRequest();
//    virtual uint8_t* getDataFromResponse();
//    virtual int16_t getDataSize();
//    virtual std::ostream& print(std::ostream& stream) const;
//};


#endif /* QUERYMSG_H_ */
