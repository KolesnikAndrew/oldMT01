/**
 * @file      modem.h
 * @brief     Заголовочный файл класса модема
 * @details
 * @note      В системе создается один объект этого класса
 * @author    Инженер-программист Пясецкий Владимир
 * @author    Инженер-программист Зозуля Артем
 * @author    Инженер-программист Савченко Владимир
 * @copyright © TRIOLCORP, 2014-2018
 */

#ifndef MODEM_H_
#define MODEM_H_

#include <vector>
#include <string>
#include <libxml++/libxml++.h>
#include <sys/time.h> //timeval, settimeofday
#include "server.h"
#include "curlwrapper.h"
#include "globals.h"
#include "PostToServer.h"

class V7Port;
class V7Server;
class PostToServer;

/**
 * @brief класс модема
 */
class V7Modem {
public:
	V7Modem();
	~V7Modem();

	/**
	 * @fn init()
	 * @brief Инициализация модема
	 * @return true, если успешная инициализацие
	 */
	bool initModem();
	/**
	 * @fn Run
	 * @brief Запуск
	 * @details Функция запускает потоки портов и ждет их завершения
	 * @param
	 * @return
	 */
	void runModem();
	/**
	 * @fn GetServer
	 * @brief Возвращает указатель на
	 * @details
	 * @param
	 * @return указатель на локальную базу данных
	 */
	V7Server* getServerPtr();
	/**
	 * @brief Запись уставки в устройство
	 * @param inputData - допустимый параметр V7Parameter
	 * @return true.ю если уставка записана
	 */
	bool setDataToDevice(inputData_type* inputData);
	/**
	 * @fn SetDataToDevice)
	 * @brief Запись уставки в устройство
	 * @param inputData - допустимый параметр ParameterModbusFile
	 * @return true, если уставка записана
	 */
	bool setDataToDevice(umkaFile_type* inputData);
	/**
	 * @fn getZipFlag
	 * @brief Врзвращает значение флага сжатия
	 * @return значение флага сжатия
	 */
	bool getZipFlag() const;
	/**
	 * @fn getPoolingPeriod
	 * @brief Возврат значения периода опроса
	 * @return значение времени опроса в мс
	 */
	unsigned int getPoolingPeriod() const;
	/**
	 * @fn setPoolingPeriod
	 * @brief Установка значения минимального периода опроса
	 * @param pr - время в мс
	 */
	void setPoolingPeriod(const unsigned int pr);

private:
	// << переменные >>
	std::string mConfigFileName; /**< Полное имя конфигурационного файла */
	xmlpp::DomParser mConfigFileParser; /**< Дескриптор парсера XML */
	V7Server* mpServer; /**< Указатель на объект сервера */
	std::vector<V7Port*> mvpPorts; /**< Список указателей на порты модема */
	std::string mServerAddress; /**< адрес сервера */
	std::string mServerFTP; /**< адрес сервера */
	std::string mProxyAddress; /**< адрес прокси-сервера */
	std::string mProxyPort; /**< порт прокси-сервера */
	std::string mProxyUser; /**< имя пользователя прокси-сервера */
	std::string mProxyPassword; /**< пароль пользователя прокси-сервера */
	std::string mSerialNumber; /**< Серийный номер модема */
	pthread_t mThreadDatacarrier; /**< Идентификатор потока */
	struct timeval mQueryPause; /**< пауза (латентность) запросов QueryInputData*/
	bool mIsZipTraffic; //!< сжатие траффика
	time_t mWaitConfigConfirm; /**< пауза в ожидании конфига с сервера */
	std::string mPrimaryIP; /**< IP-aдрес для eth0:0 */
	std::string mPrimaryNetmask; /**< маска сети  для eth0:0 */
	std::string mSecondaryIP; /**< IP-aдрес для eth0:1 */
	std::string mSecondaryNetmask; /**< маска сети  для eth0:0 */
	std::unique_ptr<PostToServer> mpPostToServer; //!< связь с сервером
	bool mBigbrotherStartFlag;
	modemWorkMode_type mWorkigMode;
	std::string mGsmMode;
	std::string mFirmwareVersion; /**< версия прошивки*/
	unsigned int mMinPollingPeriod; /**< Минимальный период опроса [мс] */
	// << функции >>
	/**
	 * @brief Запрос текщего времени
	 * @return время в секундах
	 */
	time_t qtime() const;
	/**
	 * @brief Переключает светодиод модема
	 * @param on - светится
	 */
	void switchGreenLed(bool on);
	/**
	 * @brief Старт потока
	 * @return true если успешно
	 */
	bool startDatacarrier();
	/**
	 * @brief Присоединение потока
	 * @return true если успешно
	 */
	bool waitDatacarrier();
	/**
	 * @brief Старт обмена с сервером
	 */
	void runDatacarrier();
	/**
	 *@brief Обмен конфигами
	 * @return true, если успешно
	 */
	bool xchangeConfigWithServer();
	/**
	 *@brief Обмен конфигами
	 * @return true, если успешно
	 */
	bool xchangeConfigWithServerStandalone();
	/**
	 *@brief Обмен конфигами
	 * @return true, если успешно
	 */
	bool xchangeConfigWithServerStandaloneTmp();
	/**
	 * @brief  Создание потока
	 * @param d
	 */
	static void *threadFunc(void *d) {
		((V7Modem *) d)->runDatacarrier();
		return NULL;
	}
	/**
	 * @fn PreCheckModemConfig
	 * @brief Проверка корректности конфига
	 * @return true - удачно, false - действие не удалось
	 */
	bool preCheckModemConfig();

	/**
	 * @fn PreCheckBaseData
	 * @brief Проверка корректности Basedata
	 * @return true - удачно, false - действие не удалось
	 */
	bool preCheckBaseData();
	/**
	 * @brief Вспомогательная функция для работы в режиме Онлайн
	 */
	void runModemOnline();
	/**
	 * @brief Вспомогательная функция для работы в Офлайн
	 */
	void runModemOffline();
	/**
	 * @brief Вспомогательная функция для запуска стандалон
	 */
	void runModemStandAlone();
	/**
	 * @fn getWorkingMode
	 * @brief Вычитка режима работы модема
	 * @details Вычитка режима работы модема. Реализовано 3 режима:
	 * normal (117.003), standalone (117.300). offline (без подключения к интернету)
	 * Режимы определяются в ModemConfig.xml (элемент "Mode"),
	 * @param modemElement
	 */
	bool getModeFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @fn preInitCheck
	 * @brief Проверка наличия и целостности файлов конфигурации модема
	 */
	bool preInitCheck();
	/**
	 * @fn getServerFromConfig
	 * @brief Вычитывает параметры сервера из файла настроек
	 * @param modemElement
	 * @return true, если операция успешна
	 */
	bool getServerFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @fn getRootElementFromConfig
	 * @brief Создает структуру, соотвествующую файлу настрек
	 * @param modemElement
	 * @return true, если операция успешна
	 */
	bool getRootElementFromConfig(const xmlpp::Element *&modemElement);
	/**
	 * @brief Вычитка флага сжатия траффика
	 * @param modemElement
	 * @return true, если сжатие включено
	 */
	bool getZipTrafficFlagFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @brief Получаем задержку посылки запросов к серверу
	 * @param modemElement
	 * @return true, если сжатие включено
	 */
	bool getQueryLatencyFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @brief Вычитка режима работы модема
	 * @param modemElement
	 * @return true, если сжатие включено
	 */
	bool getGsmModeFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @brief Вычитка параметров портов
	 * @param modemElement
	 * @return true, если сжатие включено
	 */
	bool getAndInitPortsFromConfig(const xmlpp::Element *modemElement);
	/**
	 * @brief Запуск приложения в режиме онлайн
	 */
	void runDataCarrierOnline();
	/**
	 * @brief Запуск приложения в режиме стэндэлоне
	 */
	void runDataCarrierStandalone();
	/**
	 * @brief Запуск приложения в режиме оффлайн
	 */
	void runDataCarrierOffile();
	/**
	 * @brief Переключение режимов работы модема
	 */
	void switchGsmMode();
	/**
	 * @brief gsm mode from ppp-config file
	 * @return 0 - 2g, 1 - 3g, 2 - old scripts
	 */
	uint8_t testGsmMode() const;
	/**
	 * @brief Парсинг конфига на поиск IP-алресов
	 * @param modemElement
	 * @return true, если успешно
	 */
	bool getIPformConfig(const xmlpp::Element *modemElement);
	/**
	 * @brief Реконфигурирует сетевые интерфейсы
	 */
	void reconfNetworkInterfaces();
	/**
	 * @brief extract firmware info from BaseData file
	 * @param basedata
	 * @return строку с версией прошивки
	 */
	std::string getFirmwareVersion(std::string basedata);


};

#endif /* MODEM_H_ */
