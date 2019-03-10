/**
 *  @file: ConfigFile.h
 *
 *  @date: 26 нояб. 2014
 *  @author: rtem, vl
 */

#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_


#include <fcntl.h>
#include <unistd.h>
#include "string.h"
#include <fstream>
#include <string>
#include "iostream"
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include "server.h"
#include "utilites.h"
#include "XMLAttribute.h"
#include "curlwrapper.h"

class ConfigFile
{
public:
    /**
     * Конструктор класса
     * @param serverAddress - адрес сервер ИТА
     */
    ConfigFile(const string& serverAddress);
    /**
     * Конструктор класса
     * @param serverAddress - адрес сервера ИТА
     * @param serverInstance - локальный сервер
     */
    ConfigFile(const string& serverAddress, V7Server *serverInstance);
    /**
     * @brief Деструктор класса
     */
    ~ConfigFile();
    /**
     * @fn GetConfigFile
     * @brief Запрос конфигурационного файла с сервера
     * @details
     * @param Address - Адрес запроса
     * @return true - удачно, false - действие не удалось
     */
    bool GetConfigFile(bool isZipped);
    /**
     * @fn ReadyConfigFile
     * @brief Сброс флага нового конфигурационного файла на сервере
     * @details
     * @param Address - Адрес запроса
     * @return
     */
    void ReadyConfigFile();
    /**
     * @fn WriteFileConfig
     * @brief Формирование конфигурационного файла для посылки на сервер
     * @details
     * @param
     * @return
     */
    void WriteFileConfig();
    /**
     * @fn SendModemConfig
     * @brief Отправить конфигурацию модема на сервер
     * @details
     * @param Address - Адрес запроса
     * @return true - удачно, false - действие не удалось
     */
    bool SendModemConfig(bool isZipped = false);
    /**
     * @fn ParseAnswer
     * @brief Парсинг ответа от сервера любого скрипта
     * @details
     * @param Answer - ответ от сервера Answer - ОК или код ошибки с описанием. Function - в какой ф-ции вызываем
     * @return string - ответ от сервера(преобразованный и распознанный)
     */
    string ParseAnswer(const string& Answer, const string& Function);

private:
    /**
     * @fn ParseConfigFile
     * @brief Парсинг ответа от сервера скрипта SendModemConfig
     * @details
     * @param Answer - ответ сервера
     * @return
     */
    void ParseConfigFile(const string& Answer);
    /**
     * @fn WriteLocalFileConfig
     * @brief Актуализация локального конфигурационного файла
     * @details
     * @param
     * @return
     */
    void WriteLocalFileConfig();
    /**
     * @fn DeleteParams
     * @brief Удаление 1 елемента из строки с ID параметрами
     * @details В строке хранятся ID актуальных параметров на сервере
     * @param
     * @return string - актуальные ID параметров
     */
    string DeleteParams();


    string mBuffer; //!< буффер запросов - ответов
    CURL *mpCurl; /**< указатель для работы с запросами на сервер */
    CURLcode mResult; /**< результат запроса на сервер */
    //struct timeval tv; /**< Значение времени */
    //struct timezone tz; /**< Структура представляющая часовой пояс.  */
    //struct tm *tm; /**< Структура хранящая полную дату и время до секунд */
    string IdFromServerToModem; /**< Строка содержащая актуальные ID параметров */
    XMLAttribute *mpXMLAttr;
    string mServerAddress; /**< адрес сервера */
    V7Server *mpServerInstance;
    LibCurlWrapper* mpServerITA;

};

#endif /* CONFIGFILE_H_ */
