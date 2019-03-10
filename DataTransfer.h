/*
 * PostToDB.h
 *
 *  Created on: 12 сент. 2014
 *      Author: rtem
 */

#ifndef DATATRANSFER_H_
#define DATATRANSFER_H_

//#include <sqlite3.h>
#include <fstream>
#include <sys/time.h>   /* gettimeofday, timeval */
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlstring.h>
#include <algorithm>
#include "XMLAttribute.h"
#include "server.h"
#include "globals.h"

using namespace std;

class V7Server;

/**
 * @brief данные для авторизации и работы
 */
class DataTransfer
{
public:
    DataTransfer(V7Server* server);
    virtual ~DataTransfer();

//	/**
//	 * @fn SendData
//	 * @brief Помещение входных данных с сервера в БД
//	 * @details
//	 * @param ID - идентификатор параметра, Value - значение параметра
//	 * @return true - удачно, false - действие не удалось
//	 */
//	bool SendData(string ID, string Value);
    /**
     * @fn DTSendValues
     * @brief Вычитка данных и формирование файла для запроса SendValues
     * @details
     * @param
     * @return
     */
    void DTPrepareSendValues();
    /**
     * @fn DTConfirmInputDataProcess
     * @brief Вычитка данных и формирование файла для запроса ConfirmInputDataProces
     * @details
     * @param
     * @return int - количество строк для отправки на сервер
     */
    umkaFile_type DTConfirmInputDataProcess();
    /**
     * @fn DTStatisticsSelect
     * @brief Вычитка из БД данных и формирование файла для запроса SendConnectionStatus
     * @details
     * @param
     * @return
     */
    void DTStatisticsSelect(const std::string& Date, const std::string& Speed, bool Connect);
    /**
     * @fn RemoveLines
     * @brief удаление строк из файла
     * @details
     * @param filename - путь к файлу и имя файла с раширением(если есть) , StrCount - количество удаляемых строк
     * @return
     */
    void RemoveLines(const char *filename, int StrCount);
    /**
     * @fn writeConfirmDataArchive
     * @brief write confirm data archive file
     *
     */
    void writeConfirmDataArchive();
    /**
     * @fn writeArchiveFile
     * @brief write  archive file
     */
    void writeArchiveFile();
    /**
     * @fn writeArchiveStatInfo
     * @brief write archive statistic info
     * @param comment
     * @param speed
     */
    void writeArchiveStatInfo(const std::string& comment, double speed);
    /**
     * @fn writeByteArrayFile
     * @brief write
     */
    void writeByteArrayFile(const int paramId, const std::string& tSatamp, const std::string& data);

    bool writeSendValueFile(const std::ostringstream& data, const std::string& fname=F_SEND_VALUES);

    void writeArchiveSendValues();

    /**
     * @fn DTSendArchiveValues
     * @brief создает архивный файл
     */
    void DTSendArchiveValues();
    /**
     * @fn CreatFile
     * @brief Создание файла для работы
     * @details
     * @param NameFile - имя файла
     * @return
     */
//    void CreatFile(const string& NameFile);
//    /**
//     * @fn CreatingFile
//     * @brief Создание необходимых файлов
//     * @details
//     * @param
//     * @return
//     */
//    void CreatingFile();

    void DTPrepareSendArchiveValues();

//	int Get_data_from_journal_status();

private:
//
//    bool SetDataToArchiveBuffer(umkaFile_type* data);
//    bool GetDataToArchiveBuffer(umkaFile_type& data);

    struct timeval tv; /**< Значение времени */
    struct timezone tz; /**< Структура представляющая часовой пояс.  */
    struct tm *tm; /**< Структура хранящая полную дату и время до секунд */
    XMLAttribute *XMLAttr;
    deque<umkaFile_type*> mvpJournalStatusData;/**< Список указателей на порции чтения журнала */
    long int mMemoryPageSize;
    bool mFlagReadyFile;

//    pthread_mutex_t mMutexJournal; /**< мьютекс канала текущих данных */

    V7Server* mpServer; /**< Указатель на объект сервера */

    void PrepareRegularData(ostringstream& data);
    void PrepareByteArrayData( outputData_type& data);
    void PrepareArchiveRegularData( std::ostringstream& data);
    void PrepareArchiveByteArrayData(const outputData_type& data);

    bool mNeedWaitResponce;
public:
    bool isNeedWaitResponce();

};
#endif /* DATATRANSFER_H_ */
