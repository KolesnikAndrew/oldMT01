/**
 * @file      devicemodbus.h
 * @brief     Заголовочный файл класса Modbus-устройства
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
#ifndef DEVICEMODBUS_H_
#define DEVICEMODBUS_H_

#include <libxml++/libxml++.h>
#include <modbus.h>
#include "device.h"

class V7ParameterModbus;

/**
 * @brief класс Modbus-устройства
 */
class V7DeviceModbus: public V7Device {
public:
	V7DeviceModbus();
	virtual ~V7DeviceModbus();

	/**
	 * @fn Init
	 * @brief Инициализация перед использованием
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pPort - указатель на порт устройства
	 * @return true - удачно, false - действие не удалось
	 */
	virtual bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);

	/**
	 * @fn GetModbusContext
	 * @brief Получение указателя на описатель modbus-соединения
	 * @return указатель или NULL
	 */
	modbus_t* GetModbusContext();

	/**
	 * @fn GetAddress
	 * @brief Получение адреса устройства
	 * @return адрес устройства
	 */
	unsigned int getDeviceAddress();

	/**
	 * @fn Session
	 * @brief Сеанс связи с устройством
	 */
	virtual void Session();
	void setTypeID(const deviceModbusTypeID_type &id);
	void setTypeID(const int id);
	deviceModbusTypeID_type getTypeID() const;

protected:
	unsigned int mAddress; /**< адрес устройства */
	struct timeval mByteTimeout;
	struct timeval mResponseTimeout;
	int mTimeoutAfterReading; /**< Интервал времени ожидания после вычитки данных из устройства [мкс]*/
	int mTimeoutAfterWriting; /**< Интервал времени ожидания после записи данных в устройство [мкс] */
	modbus_t* mpModbusContext; /**< Указатель на описатель modbus-соединения */
	bool mIsConnected; /**< Установлено ли соединение с устройством */
	bool mTimeSync; /**< Использование механизма синхронизации времени*/
	unsigned int mTimeSyncAddress; /**< Адрес ячейки для синхрнизации времени*/
	unsigned int mTimeSyncPeriod; /**< Период синхронизации времени [с]*/
	__time_t mLastSync; /**< Время последней синхронизации */
	bool mIsTimeSyncAvailable; /**< Доступность синхронизации времени */

	int mTriesNumber; /**< Количество повторов при опросе устройства*/
	bool mSmartZIP;
	struct timeval mSessionTime; //!< time stamp
	deviceModbusTypeID_type mDeviceTypeID;
	int32_t mCrashFileParamIdx;
	bool mIsInCrashHandleProcess;
	uint32_t mResetTime; //!< unix time stamp
	/**
	 * @fn Connect
	 * @brief Соединиться с устройством
	 * @return true - удачно, false - действие не удалось
	 */
	bool Connect();
	/**
	 * @fn Disconnect
	 * @brief Отсосединиться от устройства
	 * @return
	 */
	void Disconnect();
	/**
	 * @fn WriteTimeSync()
	 * @brief Синхронизация времени модема и контроллера
	 * @param sec
	 */
	void WriteTimeSync(__time_t sec);
	/**
	 * @fn readUmkaFile()
	 * @brief Чтение файла из контроллера
	 */
	void readUmkaFile();
	/**
	 * @fn waitFreePort()
	 * @brief Ждем освобождения порта
	 */
	void waitFreePort();
	/**
	 * @fn initCommonDeviceModbus()
	 * @brief Инициализация устройства
	 * @param pXMLNode файл конфигурации
	 * @param pPort указатель на порт
	 * @return true, если удачно инициализировались
	 */
	bool initCommonDeviceModbus(const xmlpp::Node* pXMLNode, V7Port* pPort);
	/**
	 * @fn setStatusWordParameters()
	 * @brief Установка слова состояния
	 */
	void setStatusWordParameters();
	/**
	 * @fn readByteArrays()
	 * @brief вычитка одномерных массивов из ШЛБ
	 */
	void readByteArrays();
	/**
	 * @fn readMatrix()
	 * @brief Формирование матрицы динамограмм ШЛБ
	 */
	void readMatrix();
	/**
	 * @brief crash log reader
	 */
	bool KN24CrashHandler();
	/**
	 * @brief setCrashFileID
	 */
	void setKN24CrashFileID();
	/**
	 * @brief chkKN24crash
	 * @return
	 */
	bool isKN24CrashEvent();
	/**
	 * @fn
	 */
	void resetTimeReadFlag();
	/**
	 *
	 * @return
	 */
	bool isKN24device();
	/**
	 *
	 * @return
	 */
	uint16_t KN24CrashRegAddr();
	/**
	 *
	 * @return
	 */
	uint16_t KN24CrashMask();
};

#endif /* DEVICEMODBUS_H_ */
