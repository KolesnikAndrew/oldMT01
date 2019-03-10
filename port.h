/**
 * @file      Port.h
 * @brief     Заголовочный файл класса порта модема
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
#ifndef PORT_H_
#define PORT_H_

#include <vector>
#include <pthread.h>
#include <string>
#include <libxml++/libxml++.h>
#include "server.h"
#include "EngineModbus.h"
#include <queue>
#include "QueryMsg.h"

using namespace std;

class V7Modem;
class V7Device;
class V7Scada;
class DeviceShunt;

//////////////////////////////////////////////////////////////////////////
/**
 * @brief класс порта модема
 */
class V7Port {
public:
	V7Port();
	virtual ~V7Port();

	/**
	 * @fn Init
	 * @brief Инициализация перед использованием
	 * @details
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pServer - указатель на локальную БД
	 * @return true - удачно, false - действие не удалось
	 */
	virtual bool Init(const xmlpp::Node* pXMLNode, V7Modem* pModem);

	/**
	 * @fn Start
	 * @brief Создание потока порта
	 * @details
	 * @param
	 * @return true - удачно, false - действие не удалось
	 */
	bool Start();

	/**
	 * @fn Wait
	 * @brief Ожидание завершения потока порта
	 * @details
	 * @param
	 * @return true - удачно, false - действие не удалось
	 */
	bool Wait();

	/**
	 * @fn GetModem
	 * @brief Получить указатель на модем порта
	 * @details
	 * @param
	 * @return указатель на модем порта или NULL
	 */
	V7Modem* GetModem();

	/**
	 * @fn GetName
	 * @brief Получить Имя файла последовательного порта в системе
	 * @details
	 * @param
	 * @return Имя файла последовательного порта в системе
	 */

	bool setDataToDevice(inputData_type* inputData);
	/**
	 *
	 * @param inputData
	 * @return
	 */
	bool setDataToDevice(umkaFile_type* inputData);

//    bool GetJournalInfo(outputJournal_type &data);

	std::string mName; /**< Имя файла последовательного порта в системе */
	pthread_mutex_t mMutexInputData; /**< мьютекс доступа к данным последовательного порта */
	pthread_mutex_t mMutexTTY; /**< мьютекс доступа к железу последовательного порта*/
	bool mNecessaryToFree; /**< флаг, сигнализирующий о необходимости освободить порт для потока SCADA*/
	/////////////////////// данные в порт    ////////////////////////////
	// bool write(uint8_t fucnCode, int data); //0x06
	// bool write(uint8_t fucnCode, const uint16_t* data); //0x10
	// bool read(uint8_t fucnCode, uint16_t *data); //0x03, 0x04
	//////////////////////////////////////////////////////////////////////
	/**
	 *
	 * @param eng
	 */
	void setModbusEngine(EngineModbus* eng);
	/**
	 *
	 * @return
	 */
	EngineModbus* getModbusEngine() const;
	/**
	 *
	 * @param
	 */
	void setPortSettings(PortSettings*);
	/**
	 * @brief Возвращает указатель на структуру с  параметрами порта
	 * @return 0 или указатель
	 */
	PortSettings* getPortSettings() const;
	/**
	 * @brief Возвращает тип движка
	 * @return
	 */
	engine_backend_type getBackendType() const;
	/**
	 * @brief Принимает от клиента и отправляет в устройство данные для raw request;
	 * @param buf
	 * @param szie
	 * @return 0 или код ошибка
	 */
	int request(QueryMsg * msg);
	/**
	 * @brief Флаг занятости порта
	 * @return значение flagBusy
	 */
	bool isBusy() const;
	/**
	 * @fn setTriesNumber
	 * @brief Устанавливает значение количества повторов ри запросе
	 */
	void setTriesNumber(const int val);

private:
	pthread_t mThread; /**< Идентификатор потока */
	std::vector<V7Device*> mvpDevices; /**< Списокуказателей на устройства порта */
	V7Scada* mpScada; /**< Указатель на внешнюю SCADA*/
	DeviceShunt *mpShunt; /**< Шунт */

	bool flagBusy;
	/**
	 * @fn Run
	 * @brief Цикл работы порта
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
	static void *ThreadFunc(void *d) {
		((V7Port *) d)->Run();
		return NULL;
	}

protected:
	/**
	 * @brief Инициалицая TCP порта
	 * @param portElement
	 * @return
	 */
	virtual bool initTcpBackendConfig(const xmlpp::Element* portElement);
	/**
	 * @brief Инициализация RTU порта
	 * @param portElement
	 * @return
	 */
	virtual bool initRtuBackendConfig(const xmlpp::Element* portElement);
	V7Modem* mpModem; /**< Указатель на модем */
	PortSettings *mpPortSettings; //!< структура для хранения настроек порта
	EngineModbus *mpModbusEngine; //!<  движок
	engine_backend_type mBackendType; //!< тип движка
	modbus_t *mpPortToScada;
	/**
	 * @brief Возвращаем тип бекенда
	 * @param portElement
	 * @return
	 */
	engine_backend_type getEngineTypeConfig(const xmlpp::Element* portElement);
	int mTriesNumber;

};

class ScadaPort: public V7Port {
	//TODO удалить класс после приведения парсинга на сервере к единому виду
	/**
	 * @brief Инициализация параметров порта СКАДА
	 * @attention Если конфиг порта ошибочный, то устанавливаются параметры по умолчанию (9600, Ņ, 1, 8)
	 */
	virtual bool initRtuBackendConfig(const xmlpp::Element* portElement);
//	/**
//	 * @brief Инициалицая TCP порта
//	 * @param portElement
//	 * @return
//	 */
//	virtual bool initTcpBackendConfig(const xmlpp::Element* portElement);
	bool isTcpMasterMode(const xmlpp::Element* portElement);
public:
	/**
	 * @fn Init
	 * @brief Инициализация перед использованием
	 * @details
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pServer - указатель на локальную БД
	 * @return true - удачно, false - действие не удалось
	 */
	virtual bool Init(const xmlpp::Node* pXMLNode, V7Modem* pModem);

};

#endif /* PORT_H_ */
