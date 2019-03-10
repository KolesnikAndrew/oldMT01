/**
 * @file      server.cpp
 * @brief     Определение функций класса сервера
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2015
 */

#include "globals.h"
#include "modem.h"

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <utilites.h>

#define MAX_SETPOINT_DATA_BUFFER_SIZE 2500
#define MAX_SETPOINT_CONFIRM_BUFFER_SIZE 2500

V7Server::V7Server(V7Modem* pModem) :
        mThread(0), mpModem(pModem), mMode(modemWorkMode_type::normal)
{
    pthread_mutex_init(&mMutexCurrentData, NULL);
    pthread_mutex_init(&mMutexSetpointData, NULL);
    pthread_mutex_init(&mMutexSetpointConfirm, NULL);
    pthread_mutex_init(&mMutexJournal, NULL);
    mMemoryPageSize = sysconf(_SC_PAGE_SIZE);
}

V7Server::~V7Server()
{
    while (!mvpCurrentData.empty()) {
        delete mvpCurrentData.back();
        mvpCurrentData.pop_back();
    }
    while (!mvpSetpointData.empty()) {
        delete mvpSetpointData.back();
        mvpSetpointData.pop_back();
    }
    while (!mvpSetpointConfirm.empty()) {
        delete mvpSetpointConfirm.back();
        mvpSetpointConfirm.pop_back();
    }
    while (!mvpUmkaFilelStatusData.empty()) {
        delete mvpUmkaFilelStatusData.back();
        mvpUmkaFilelStatusData.pop_back();
    }

    pthread_mutex_destroy(&mMutexCurrentData);
    pthread_mutex_destroy(&mMutexSetpointData);
    pthread_mutex_destroy(&mMutexSetpointConfirm);
    pthread_mutex_destroy(&mMutexJournal);
}

bool V7Server::SetDataToCurrentDataBuffer(outputData_type* data)
{
    if (!data || data->globalID == 0) {
        return false;
    }

    long int freeMemory = sysconf(_SC_AVPHYS_PAGES) * mMemoryPageSize;

    pthread_mutex_lock(&mMutexCurrentData);
    if ((freeMemory <= 20000000) && (!mvpCurrentData.empty())) {
        delete mvpCurrentData.front();
        mvpCurrentData.pop_front();
    }
    mvpCurrentData.push_back(data);
    pthread_mutex_unlock(&mMutexCurrentData);

    return true;
}

bool V7Server::GetDataFromCurrentDataBuffer(outputData_type& data)
{
    if (mvpCurrentData.empty())
        return false;

    pthread_mutex_lock(&mMutexCurrentData);
    outputData_type* pData = mvpCurrentData.back();
    mvpCurrentData.pop_back();
    pthread_mutex_unlock(&mMutexCurrentData);

    data = *pData;
    delete pData;

    return true;
}

bool V7Server::SetDataToSetpointDataBuffer(inputData_type* data)
{
    if (!data)
        return false;
    if (mvpCurrentData.size() >= MAX_SETPOINT_DATA_BUFFER_SIZE) {
        cerr << "Overflow of the setpoint buffer buffer!" << endl;
        delete data;
        return false;
    }
    pthread_mutex_lock(&mMutexSetpointData);
    mvpSetpointData.push_back(data);
    pthread_mutex_unlock(&mMutexSetpointData);

    return true;
}

bool V7Server::GetDataFromSetpointDataBuffer()
{
    if (mvpSetpointData.empty())
        return false;

    pthread_mutex_lock(&mMutexSetpointData);
    inputData_type* inputData = mvpSetpointData.front();
    mvpSetpointData.pop_front();
    pthread_mutex_unlock(&mMutexSetpointData);

    if (!inputData)
        return false;

    if (mpModem->setDataToDevice(inputData)) {
        delete inputData;
        return true;
    }

    confirmData_type* confirmData = new confirmData_type;
    confirmData->globalID = inputData->globalID;
    delete inputData;

    //!Берем текущее время
    struct timeval readingTime;
    confirmData->dateTime = bigbrother::timeStamp();
    confirmData->state = inDataState_type::unknown;
    SetDataToSetpointConfirmBuffer(confirmData);

    return false;
}

bool V7Server::InputDataReceivingStart()
{
    if (pthread_create(&mThread, NULL, V7Server::ThreadFunc, (void*) this) == 0)
        return true;
    else
        return false;
}

bool V7Server::InputDataReceivingWait()
{
    if (pthread_join(mThread, NULL) == 0)
        return true;
    else
        return false;
}

bool V7Server::SetDataToSetpointConfirmBuffer(confirmData_type* confirmData)
{
    if (!confirmData)
        return false;

    if (mvpSetpointConfirm.size() >= MAX_SETPOINT_CONFIRM_BUFFER_SIZE) {
        cerr << "Overflow of the confirm buffer!" << endl;
        delete confirmData;
        return false;
    }
    pthread_mutex_lock(&mMutexSetpointConfirm);
    mvpSetpointConfirm.push_back(confirmData);
    pthread_mutex_unlock(&mMutexSetpointConfirm);

    return true;
}

bool V7Server::GetDataFromSetpointConfirmBuffer(confirmData_type& data)
{
    if (mvpSetpointConfirm.empty())
        return false;

    pthread_mutex_lock(&mMutexSetpointConfirm);
    confirmData_type* pData = mvpSetpointConfirm.front();
    mvpSetpointConfirm.pop_front();
    pthread_mutex_unlock(&mMutexSetpointConfirm);

    data = *pData;
    delete pData;

    return true;
}

void V7Server::InputDataReceivingCycle()
{
    while (!endWorkFlag) {
        if (!GetDataFromSetpointDataBuffer())
            sleep(1);
    }
}

bool V7Server::SetDataToFileBuffer(umkaFile_type* data)
{
    if (!data)
        return false;

    if (mvpUmkaFilelStatusData.size() >= MAX_SETPOINT_CONFIRM_BUFFER_SIZE) {
        cerr << "Overflow of the journal buffer!" << endl;
        delete data;
        return false;
    }

    pthread_mutex_lock(&mMutexJournal);
    mvpUmkaFilelStatusData.push_front(data);
    pthread_mutex_unlock(&mMutexJournal);

    return true;
}

bool V7Server::SetDataToFile(umkaFile_type *inputData)
{
	//PRINTDEBUG2("V7Server::SetDataToFile::: inputData SET DATA TO FILE  ", (int)inputData->umkaFileStatus);
    if (!inputData) {
        return false;
    }
    if (mpModem->setDataToDevice(inputData)) {
        delete inputData;
        return true;
    }

    delete inputData;

    return false;
}

bool V7Server::GetDataFromFileBuffer(umkaFile_type& data)
{
    if (mvpUmkaFilelStatusData.empty())
        return false;

    pthread_mutex_lock(&mMutexJournal);
    umkaFile_type* pData = mvpUmkaFilelStatusData.back();
    mvpUmkaFilelStatusData.pop_back();
    pthread_mutex_unlock(&mMutexJournal);

    data = *pData;
    delete pData;

    return true;
}

void V7Server::switchMode()
{
    mMode = (mMode == modemWorkMode_type::normal) ? modemWorkMode_type::standalone : modemWorkMode_type::normal;
}

void V7Server::setNormalMode()
{
    mMode = modemWorkMode_type::normal;
}

void V7Server::setStandaloneMode()
{
    mMode = modemWorkMode_type::standalone;
}

modemWorkMode_type V7Server::Mode() const
{
    return mMode;
}

bool V7Server::getModemZipFlag() const
{
    return mpModem->getZipFlag();
}
//bool V7Server::GetDataFromJournalBufferConfirm(outputJournal_type& data){
//    if(mvpJournalStatusCOnfirmData.empty())
//        return false;
//
//    pthread_mutex_lock(&mMutexJournal);
//    outputJournal_type* pData = mvpJournalStatusData.back();
//    mvpJournalStatusData.pop_back();
//    pthread_mutex_unlock(&mMutexJournal);
//
//    data = *pData;
//    delete pData;
//
//    return true;
//}
//
//
//bool V7Server::SetDataFromJournalBufferConfirm(outputJournal_type* data){
//    if(!data) return false;
//
//    long int freeMemory = sysconf(_SC_AVPHYS_PAGES) * mMemoryPageSize;
//
//    pthread_mutex_lock(&mMutexJournalCOnfirm);
//    if((freeMemory <= 20000000) && (!mvpJournalStatusCOnfirmData.empty())){
//        delete mvpJournalStatusCOnfirmData.front();
//        mvpJournalStatusCOnfirmData.pop_front();
//    }
//    mvpJournalStatusCOnfirmData.push_back(data);
//    pthread_mutex_unlock(&mMutexJournalCOnfirm);
//
//    return true;
//}
//bool V7Server::GetJournalInfo(outputJournal_type &data)
//{
//    if(mpModem->GetJournalInfo(&data)){
//        return true;
//    }
//    return false;
//}

