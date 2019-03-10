/**
 * @file      server.h
 * @brief     Заголовочный файл класса сервера changes
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2015
 */
#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <pthread.h>
#include <deque>
#include "globals.h"

using namespace std;

/**
 * @brief флаг режима рабьота сервера
 */
class V7Modem;

class V7Server
{
public:
    V7Server(V7Modem* pModem);
    virtual ~V7Server();

    /**
     * @fn SetDataToCurrentDataBuffer
     * @brief Отправить данные в таблицу выходных данных
     * @details
     * @param data - отправляемые данные
     * @return  true - удачно, false - действие не удалось
     */
    bool SetDataToCurrentDataBuffer(outputData_type* data);
    bool GetDataFromCurrentDataBuffer(outputData_type& data);

    bool SetDataToSetpointDataBuffer(inputData_type* data);
    bool GetDataFromSetpointDataBuffer();

    bool SetDataToFileBuffer(umkaFile_type* data);
    bool GetDataFromFileBuffer(umkaFile_type& data);
    bool SetDataToFile(umkaFile_type *inputData);

    /**
     * @fn InputDataReceivingStart
     * @brief Запуск потока
     * @return true - удачно, false - действие не удалось
     */
    bool InputDataReceivingStart();

    /**
     * @fn InputDataReceivingWait
     * @brief Ожидание завершения потока
     * @return true - удачно, false - действие не удалось
     */
    bool InputDataReceivingWait();

    bool SetDataToSetpointConfirmBuffer(confirmData_type* confirmData);
    bool GetDataFromSetpointConfirmBuffer(confirmData_type& data);

//    bool GetDataFromJournalBufferConfirm(outputJournal_type& data);
//    bool SetDataFromJournalBufferConfirm(outputJournal_type* data);

//    bool GetJournalInfo(outputJournal_type &data);
    /**
     * @fn
     * @brief
     * @return
     */
    void switchMode();
    void setNormalMode();
    void setStandaloneMode();
    /**
     *
     * @return
     */
    bool getModemZipFlag() const;
    modemWorkMode_type Mode() const;

private:
    modemWorkMode_type mMode; /**< Режим работы сервера */

    pthread_t mThread; /**< Идентификатор потока */
    V7Modem* mpModem; /**< Указатель на модем */

    deque<outputData_type*> mvpCurrentData; /**< Список указателей на порции текущих данных */
    deque<inputData_type*> mvpSetpointData; /**< Список указателей на порции уставок */
    deque<confirmData_type*> mvpSetpointConfirm; /**< Список указателей на порции подтверждения уставок */
    deque<umkaFile_type*> mvpUmkaFilelStatusData;/**< Список указателей на порции чтения журнала */

    pthread_mutex_t mMutexCurrentData; /**< мьютекс канала текущих данных */
    pthread_mutex_t mMutexSetpointData; /**< мьютекс канала уставок */
    pthread_mutex_t mMutexSetpointConfirm; /**< мьютекс канала подтверждения уставок */
    pthread_mutex_t mMutexJournal; /**< мьютекс канала текущих данных */
//    pthread_mutex_t mMutexJournalCOnfirm;      /**< мьютекс канала текущих данных */

    long int mMemoryPageSize;

    /**
     * @fn InputDataReceivingCycle
     * @brief Цикл приема и обработки входных данных
     * @details Функция запускается в отдельном потоке
     */
    void InputDataReceivingCycle();

    /**
     * @fn ThreadFunc
     * @brief Функция отдельного потока
     * @param d - указатель, передаваемый в поток
     */
    static void *ThreadFunc(void *d)
    {
        ((V7Server *) d)->InputDataReceivingCycle();
        return NULL;
    }
};

#endif /* SERVER_H_ */
