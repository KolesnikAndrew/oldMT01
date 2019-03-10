/*
 * SendData.h
 *
 *  Created on: 10 сент. 2014
 *      Author: rtem
 */

#ifndef SENDDATA_H_
#define SENDDATA_H_

#include <string>

#include "globals.h"
#include "curlwrapper.h"
#include "ConfigFile.h"
#include "DataTransfer.h"
#include "XMLAttribute.h"
#include "utilites.h"

class V7Server;

/**
 * @brief класс связи с сервером
 */
class PostToServer
{
public:
    PostToServer(const std::string &serverAddress, const std::string &serverFTP,
            const std::string &proxyAddress, const std::string &proxyPort,
            const std::string &proxyUser, const std::string &proxyPassword,
            const std::string &serialNumber, V7Server* server);
    virtual ~PostToServer();

public:
    /**
     * @fn QueryInputData
     * @brief Запрос входных данных с сервера
     * @details
     * @param Address - Адрес запроса
     * @return long - время выполнения запроса
     */
    void queryInputData();

    /**
     * @fn SendValues
     * @brief Отправка значений параметров на сервер
     * @details
     * @param Address - Адрес запроса
     * @return long - время выполнения запроса
     */
    void sendValues();
    void sendValuesArchive();
    void sendValuesBinaryArray();
    void sendValuesKN24CrashLog();
    /**
     * @fn Authorized
     * @brief Авторизация на сервере
     * @details
     * @param Modem - найстройки модема(прокси, адрес сервера)
     * @return true - удачно, false - действие не удалось
     */
    bool queryAuthorize(bool first);
    /**
     * @fn ConnectionStatus
     * @brief Посылка статистики связи на сервер
     * @details
     * @param Speed - скорость работы системы в байт/сек, Adress - адрес сервера
     * @return
     */
    //void ConnectionStatus(const std::string& Speed);
    friend std::ostream &operator<< (std::ostream& stream, const PostToServer& obj);
    /**
     * @brief
     */
    void setZipTrafficFlag(bool flag);
    std::string getServerTime();

private:
    enum flagProcess
    {
        flagNone = 0, //!< нет новых флагов
        flagLogFile,  //!< отослать лог
        flagCash,     //!< отправить состояние счета
        flagReboot,   //!< перезагрузка
        flagNewFile,  //!< новая прошивка
        flagNewRoot   //!< новое ядро (на 11.2017) не используется
    };

    /**
     * @fn ConfirmInputDataProcess
     * @brief Подтверждение обработки входных данных
     * @details
     * @param Address - Адрес запроса, JustNow - флаг состояния запроса
     * @return long - время выполнения запроса
     */
    void confirmInputDataProcess(bool JustNow);
    /**
     * @fn ReadyCash
     * @brief Посылка состояние счета на сервер
     * @details
     * @param Address - Адрес запроса
     * @return
     */
    void processCashQuery(const string& Addressm);

    /**
     * @fn SendFile
     * @brief Посылка лог файла на сервер
     * @details
     * @param Address - Адрес запроса
     * @return
     */
    bool sendFile(const string& Address, const string&FilePath,
            const string& Message, unsigned int par_id);

    /**
     * @fn ScanFirmwareFile
     * @brief Сканирование наличие прошивки и даты последнего изменения
     * @details
     * @param
     * @return string - имя=дата создания
     */
    string scanFirmwareFile();
    /**
     * @fn ReadyModemReboot
     * @brief функция удаленной перезагрузки модема
     * @details
     * @param Address - Адрес запроса
     * @return
     */
    void processRebootModem(const string& Address);

    /**
     * @fn GetCash
     * @brief запрос счета с помощью внутр скрипта сash_SQ.php
     * @details
     * @param
     * @return true - удачно, false - действие не удалось
     */
    bool getCashFromModem();
    /**
     * @fn ParseQueryInputData
     * @brief Парсинг ответа от сервера скрипта QueryInputData
     * @details
     * @param Message - Ответ от сервера
     * @return
     */
    void parseQueryInputData(const string& Message);
    /**
     * @brief Подтверждение нового файла
     */
    void ReadyNewFile();


    /**
     * @fn GetFirmware
     * @brief Скачивание новых файлов с сервера
     * @details
     * @param true - удачно, false - действие не удалось
     * @return
     */
    void GetNewFile(const std::string& fileName, bool ResumeFrom);
    /**
     *
     * @param name
     * @param NodePtrLvl1
     * @param set_flag
     */
    void checkNewFlagProcess(string name, xmlNodePtr NodePtrLvl1,
            flagProcess set_flag);
    /**
     *
     */
    void manageFlags();
    /**
     *
     */
    void writeArchiveFile();
    /**
     * Return true if file loaded
     * @param id
     * @return
     */
    std::string GetNewAT27Config(unsigned int id);
    ///////////////////////  local vars ///////////////////////
    LibCurlWrapper *mpCurlWrapper;
    char ConnectionTime[20]; /**< Хранит время предидущего запроса статистики связи(для предотвращения   многочисленных запросов) */
    bool mConnect; /**< Флаг наличия связи */
    bool mIsNeedAuthorize; /**< Флаг повторной авторизации*/
    struct timeval mTv; /**< Значение времени */
    struct timezone mTz; /**< Структура представляющая часовой пояс.  */
    struct tm *mpTm; /**< Структура хранящая полную дату и время до секунд */
    //string mBuffer; /**< буфер принимаемых данных */
    string mCommentConnect; /**< Строка содержит сообщение для комментария статистики связи*/
    flagProcess mChoosenFlag;
    CURLcode mCurlResult; /**< результат запроса на сервер */
    string mSerialNumber; /**< Серийный номер модема */
    V7Server* mpServer; /**< Указатель на объект сервера */
    DataTransfer *mpDataTransfer; /**< Указатель класса передачи данных */
    ConfigFile *mpConfigFile; /**< */
    XMLAttribute *mpXMLAttr; /**< */
    bool mIsNeedToSendFile; /**< */
    umkaFile_type mSavedData; /**< */
    bool mIsZipTraffic;
    int8_t mCntCashRead;
    bool mBadParamID;
    bool mSkipQueryImputData;
};

#endif /* SENDDATA_H_ */
