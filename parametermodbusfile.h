/**
 * @file      parametermodbusjournal.h
 * @brief     Заголовочный файл класса Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Зозуля Артем
 * @copyright © TRIOLCORP, 2016
 */

#ifndef PARAMETERMODBUSFILE_H_
#define PARAMETERMODBUSFILE_H_

#include <libxml++/libxml++.h>
#include <vector>
#include <pthread.h>
#include <serialv.h>
#include "utilites.h"

#include "UMKA03utilites.h"
#include "parametermodbus.h"
#include <memory>
//#include "parameter.h"
using namespace std;

/**
 * @def  MAX_STRING_SIZE
 */
#define MAX_STRING_SIZE 250   //!< Размер строки
/**
 * @def MAX_REGISTER_COUNT
 */
#define MAX_REGISTER_COUNT 0x7A  //!< Количество регистров в одном запросе

/**
 * @def MAX_RECORDS_TO_WRITE
 * @brief  Максимальное число записей выбираем из условия попадания на одну страницу рзмером 528 байт.
 * Для конфигов АТ27
 */
#define MAX_RECORDS_TO_WRITE 0x58 //!< Количество регистров взапросе записи конфига
/**
 * @def BUFFER_LENGTH
 */
#define BUFFER_LENGTH  MODBUS_RTU_MAX_ADU_LENGTH //!< Размер буфера
/**
 * @def * FN_WRITE_CONFIG_AT27
 */
#define FN_WRITE_CONFIG_AT27 0x69 //!< Функция записи конфигов АТ27
/**
 * @def FN_READ_CONFIG_AT27
 */
#define FN_READ_CONFIG_AT27 0x68 //!< Функция чтения конфигов АТ27
/**
 * @def CONFIG_AT27_NUMBER
 */
#define CONFIG_AT27_NUMBER 0x1d //!< Функция чтения конфигов АТ27

class V7Port;
class V7Parameter;

/**
 * @brief класс Modbus-параметра
 */
class ParameterModbusFile: public V7ParameterModbus {
public:
	ParameterModbusFile();
	virtual ~ParameterModbusFile();
	bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	/**
	 * @fn Run
	 * @brief Цикл приема и обработки запросов
	 * @details Функция запускается в отдельном потоке
	 * @param
	 * @return
	 */
	virtual void Run();

	void SetDataToFileBuffer(umkaFile_type* confirmData);

	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState);

	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);

	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			const std::string& data, const validState_type& validState);

	/**
	 * @fn WriteParameterModbus
	 * @brief Запись конфига в устройство АТ27
	 */
	virtual void writeParameterModbus();
	/**
	 * @fn DeleteFile
	 * @brief Удаление недочитанного файла
	 */
	void DeleteFile();
	/**
	 * @fn SetProcessState
	 * @brief Установка состояния работы с файлом
	 */
	void SetProcessState(const fileStatus_type& status);
	/**
	 * @fn StopReadJrn
	 * @brief Останавливает вычитку журналов
	 */
	void StopReadJrn();
	/**
	 * @fn WriteConfig
	 * @brief Запись файла конфигурации в АТ27
	 */
	void WriteConfig();
	/**
	 * @fn isConfigFile
	 * @return true, если текущий файл - конфиг
	 */
	bool isConfigFile() const;
	/**
	 * @fn setConfigFileAT27Buffer
	 * @param str
	 */
	void setConfigFileAT27Buffer(const std::string &str);
	/**
	 * @fn setDeviceConfigNumber
	 * @param devNumber - номер устройства, для которого записывается конфиг с сервера
	 */
	void setDeviceConfigNumber(const unsigned devNumber);


	/**
	 * @brief Set a flag to delete a file
	 */
	void setDeleteFlag();
	/**
	 *
	 */
	void resetDeleteFlag();
	/**
	 * @brief Set a flag to stop read a file
	 */
	void setStopFlag();
	/**
	 *
	 */
	void resetStopFlag();
	/**
	 * @brief Set a flag to read a file
	 */
	void setReadFlag();
	/**
	 * @brief Reset a flag to stop read a file
	 */
	void resetReadFlag();
	/**
	 * @brief Set a flag to write read a file
	 */
	void setWriteFlag();
	void resetWriteFlag();
	/**
	 * @brief Return state of the delete file flag
	 */
	bool getDeletFlag() const;
	/**
	 * @brief Return state of the stop reading file flag
	 */
	bool getStopFlag() const;
	/**
	 * @brief Return state of the read file flag
	 */
	bool getReadFlag() const;
	/**
	 * @brief Return state of the write file flag
	 */
	bool getWriteFlag() const;
	/**
	 *
	 */
	void resetAllStateFlags();
	void badIdAnswer(uint32_t bad_id);

protected:
	int mRequestCount; //!< Количество операций с файлом в одном цикле
	fileStatus_type mProcessState; //!< Статус текущей операции с файлом
	string mFileExtension; //!< Расшширение файла.
	/**
	 * UM27, UM03_Triol, UM03_Rosneft
	 */
	umkaFileClass_type mFileClassType; //!< Тип журнала. УМКА03_старый, УМКА03_новый, УМКА27.
	string mFilePath; //!< Путь к файлу
	string mConfigAT27Buffer; /**< буфер длясчитанного файла */
	unsigned int mDevConfigNumber; //!< Номер устройства (сервер)
	unsigned int mAddressInJournal; //!< Адрес параметра на устройстве
	unsigned int mFileNumber; //!< Номер файла в устройстве
	unsigned int mFileSize; //!< Размер файла в устройстве
	std::unique_ptr<UMKA03utilites> mpUM03utl; //!<Указатель на объект утилит для работы с УМКА03
	bool mFlagReadingJrn;
	bool mIsNeedDeleteFile;
	bool mIsNeedStopReadFile;
	bool mIsNeedReadFile;
	bool mIsNeedWriteFile;
	/**
	 * @fn readConfigAT27
	 * @return Строковое представление конфига
	 */
	std::string readConfigAT27();
private:
	/**
	 * @brief Чтение файла функцие 0х68
	 */
	void readFile0x68();
	/**
	 * @brief Чтение файла функцие 0х14
	 *
	 */
	void readFile0x14();

};



#endif /* PARAMETERMODBUSFILE_H_ */
