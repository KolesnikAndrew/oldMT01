/**
 * @file      parameter.h
 * @brief     Заголовочный файл класса параметра устройства
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <libxml++/libxml++.h>
#include <vector>
#include <sys/time.h>
#include "server.h"

class V7Device;
class V7DeviceModbus;
class V7Port;
/**
 * @brief класс параметра устройства
 */
class V7Parameter {
public:
	V7Parameter();
	virtual ~V7Parameter();

	/**
	 * @fn initParam
	 * @brief Инициализация перед использованием
	 * @details
	 * @param pXMLNode - указатель на XML-элемент из конфигурационного файла
	 * @param pDevice - указатель наустройство
	 * @return true - удачно, false - действие не удалось
	 */
	virtual bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);

	/**
	 * @fn needToGetValue
	 * @brief Пора ли получить значение параметра
	 * @details
	 * @param time - время проверки
	 * @return true - пора, false - еще не время
	 */
	bool needToGetValue(const struct timeval* time) const;

	/**
	 * @fn setLastReading
	 * @brief Установить время последнего опроса
	 * @param time - время последнего опроса
	 */
	void setLastReading(const struct timeval* time);

	/**
	 * @fn SetValueToCurrentDataPipe
	 * @brief Отправить данные в канл текущих данных
	 * @details
	 * @param tp - время считывания показаний
	 * @param data - данные
	 * @param validState - Признак достоверности данных
	 * @return
	 */
	virtual void setValueToCurrentDataPipe(struct timeval *tp, const std::string& data, const validState_type& validState);
	/**
	 * @fn setDataToSetpointConfirmBuffer
	 * @param confirmData
	 */
	virtual void setDataToSetpointConfirmBuffer(confirmData_type* confirmData);
	/**
	 * @fn setDataToDevice
	 * @param inputData
	 * @return
	 */
	virtual bool setDataToDevice(inputData_type* inputData);
	/**
	 * @fn setDataToDevice
	 * @param inputData
	 * @return
	 */
	virtual bool setDataToDevice(umkaFile_type* inputData);
	/**
	 * @fn isNewValue
	 * @return true, if new value obtained
	 */
	bool isNewValue();
	/**
	 * @fn getNewValue
	 * @return string represantaion of new value from server
	 */
	std::string getNewValue() const;
	/**
	 * @fn resetNewValueFlag
	 * @brief reset new value flag
	 */
	void resetNewValueFlag();
	/**
	 * @fn getFileStatus
	 * @return file r/w status
	 */
	fileStatus_type getFileStatus() const;
	/**
	 * @fn setFileStatus
	 * @param status
	 */
	void setFileStatus(const fileStatus_type& status);
	/**
	 * @fn getGlobalServerId
	 * @brief getGlobalServerId
	 * @return
	 */
	unsigned int getGlobalServerId() const;
	/**
	 * @brief ожидание освобоэдения порта
	 */
	void waitForFreePort();
	/**
	 * @fn GetJrnReadingStatus
	 * @return UMCA03 file reading status
	 */
	bool isReadFileInProcess();
	/**
	 * @fn getStrNewValueFromPipe
	 * @param strNewValue
	 * @return
	 */
	bool getStrNewValueFromPipe(std::string& strNewValue);
	/**
	 * @fn setReadingUM03JrnActive
	 */
	void setReadingUM03JrnActive();
	/**
	 * @fn getParamType
	 * @return тип параметра
	 */
	paramType_type getParamType() const;
	/**
	 * @fn setParamType
	 * @param pt - утсановка типа параметра
	 */
	void setParamType(const paramType_type &pt);
	/**
	 * @fn setParameterSmartZip
	 * @brief Turning on smart traffic compression
	 * @param smartZip
	 * @details If smart traffic compression is turning on and old value is equal to new value, last one will not be transfered to the server.
	 */
	void setParameterSmartZip(const bool smartZip);
	/**
	 * @fn getParameterSmartZip
	 * @brief enable smart
	 * @return
	 */
	bool getParameterSmartZip() const;
	void resetLastReadingTime();


protected:
	enum {
		SETPOINT_PARAM, /**< Уставка */
		CURRENT_PARAM /**< Текущее значение */
	} mType; /**< Тип параметра */

	fileStatus_type mUmkaFileStatus; /**< Статус операции с файлом,  эта переменная управляет чтением файла */
	int mServerID; /**< Идентификатор параметра в БД сервера */
	std::string mName; /**< Наименование параметра (строка до 250 символов) */
	unsigned int mNumber; /**< Номер параметра в рамках устройства */
	unsigned int mPollingPeriod; /**< Минимальный период опроса [мс] */
	//bool mOldValueAvailable; /**< Наличие актуального старого значения */
	validState_type mOldValidState; /**< Старое состояние */
	std::string mStrOldValue; /**< Старое значение */
	struct timeval mLastReading; /**< Время последнего опроса */
	bool mNewValueFlag; /**< Флаг наличия нового значения */
	string mNewValue; /**< Новое значение */
	paramType_type mParamType;
	V7Device* mpDevice; /**< Указатель на устройство */
	V7Port* mpPort; /**< */
	V7Modem* mpModem; /**< */
	V7Server* mpServer; /**< */
	V7DeviceModbus* mpDeviceModbus; /**< */
	bool mParameterSmartZip; /**< */

	bool mIsReadingUmkaFile;



};

#endif /* PARAMETER_H_ */
