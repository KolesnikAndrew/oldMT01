/**
 * @file      scada.h
 * @brief     Заголовочный файл класса внешней SCADA
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2015
 */
#ifndef SCADA_H_
#define SCADA_H_

#include <libxml++/libxml++.h>
#include <pthread.h>
//#include <serialv.h>
#include <fstream>
#include <modbus.h>
#include "devicemodbus.h"
#include <map>
#include <set>

class V7Port;

using namespace std;

class V7Scada
{
public:
    V7Scada();
    virtual ~V7Scada();

    /**
     * @fn Init
     * @brief Инициализация перед использованием
     * @details
     * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
     * @param pPort - указатель на порт, на который ретранслируются данные
     * @return true - удачно, false - действие не удалось
     */
    bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);
    /**
     * @brief Инициализация с привязкой к иодбас-контексту устройства
     * @param pXMLNode
     * @param pPort
     * @param pDeviceModBus
     * @return
     */
    bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort, V7DeviceModbus* pDeviceModBus);
    /**
     * @brief Инициализация с привязкой к пулу
     * @param pXMLNode
     * @param pPort
     * @param pDevices
     * @return
     */
    bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort, std::vector<V7Device*> pDevices);
    /**
     * @fn Start
     * @brief Создание потока
     * @details
     * @param
     * @return true - удачно, false - действие не удалось
     */
    bool Start();

    /**
     * @fn Wait
     * @brief Ожидание завершения потока
     * @details
     * @param
     * @return true - удачно, false - действие не удалось
     */
    bool Wait();

private:
    pthread_t mThread; /**< Идентификатор потока */
    V7Port* mpPort;
    ScadaPort* mpScadaPort;
    //modbus_t* mpPortToScada; /**< Описатель порта в файловой системе, к которому подключена внешняя SCADA */
    //modbus_t* mpPortToDevice; /**< Описатель порта в файловой системе, к на который ретранслируются запросы SCADA */
    int numOfDevices; /**< Количество устроств на порту*/
    /**
     * список устройств модбас на порту.
     * модбас-контекс связывает с адресом устройства
     */
    std::map<uint8_t, modbus_t*> mpDevicePool; /**< Списокуказателей на устройства порта */
    /**
     * @brief
     */
    static uint8_t mKnownModbusFubctions[];
    /**
     * @brief
     * @param func
     * @param knownFunc
     * @param sz
     * @return
     */
    bool checkModbusFunction(uint8_t func, uint8_t *knownFunc, uint16_t sz);
    /**
     * @fn Run
     * @brief Цикл приема и обработки запросов
     * @details Функция запускается в отдельном потоке
     * @param
     * @return
     */
    virtual void Run();

    /**
     * @fn ThreadFunc
     * @brief Функция потока порта
     * @details
     * @param d - указатель, передаваемый в поток
     * @return
     */
    static void *ThreadFunc(void *d)
    {
        ((V7Scada *) d)->Run();
        return NULL;
    }
    /**
     * @fn timeStamp
     * @brief вспомогательная функция, лог перезагрузки и времени
     * @details
     * @param
     * @return
     */
    void errStamp(const char * source) const;
    /**
     * @fn cntLog
     * @brief вспомогательная функция, лог перезагрузки и времени
     * @details
     * @param
     * @return
     */
    void cntLog();
    /**
     * @fn matchSlaveId
     * @brief looking for id in buf
     * @param buf
     * @param id
     * @return
     */
    bool matchSlaveId(const uint8_t *buf, const uint id);
    /**
     * @brief ищет зданный ID  в пуле
     * @param id
     * @return
     */
    bool isDevIDinPool(const uint id);

    /**
     * @brief Групповая команда записи в пул устройств
     * @param mpDevicePool
     * @param cmdBuffer
     */
    void writePool(std::map<uint8_t, modbus_t*> mpDevicePool, uint8_t *cmdBuffer, uint8_t cmdBufferLen);

    std::ofstream mCntPacketsFile; //!< файл для хранения статистики
    long int mCntReceived; //!< все принято пакетов
    long int mCntTransmittedToDevice; //!< принято с устройства
    long int mCntReceivedFromDevice; //!< принято с устройства
    long int mCntTransmittedToScada; //!< отправлено в АСУ


};

#endif /* SCADA_H_ */
