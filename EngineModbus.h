/**
 * @file      EngineModbus.h
 * @brief     Класс для реализации протокола Модбас
 * @details   Класс для реализации протокола Модбас
 * @note      Реализация с учетом действующего протокола обмена
 * @author    Инженер-программист Савченко Владимир
 * @copyright © TRIOLCORP, 2017
 */
#ifndef ENGINEMODBUS_H_
#define ENGINEMODBUS_H_

#include <string>
#include <modbus.h>
#include <errno.h>
#include <libxml++/libxml++.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include "portsettings.h"
#include "QueryMsg.h"
#include "utilites.h"

/**
 * @def ENGINE_MODBUS_DEBUG
 * @brief Управление дебаг-выводом
 */
#if defined (__i386__) || defined (__amd64__)
	#define ENGINE_MODBUS_DEBUG
#endif
/**
 * @class EngineModbus
 * @brief Движок модбаса
 * @details Класс эмулирует модбасовский движок. Методы повторяют соотвествуюзие функции Модбаса.
 * Единственное отличие, результатом выполнения будет 0, если операци яуспешна или код ошибки.
 * СтандартноENGINE_MODBUS_DEBUG Мордбасовские функции возвращают -1 или длину ответа.
 * Такое поведение сделано для упрощения работы.
 * Там где необходимо, размер передается через переменную соотвествующего метода.
 *
 */
#undef ENGINE_MODBUS_DEBUG

class EngineModbus {
public:
	EngineModbus();
	virtual ~EngineModbus();
	/**
	 * @brief Инициализация параметров движка
	 */
	virtual void init(PortSettings *) = 0;
	/**
	 * @brief Открытие порта
	 */
	virtual bool open() = 0;
	/**
	 * @brief Закрытие порта
	 */
	virtual void close() = 0;
	/**
	 * @brief  Подключение к устройству
	 */
	virtual bool connect() = 0;
	/**
	 * @brief  Отключение от устройства
	 */
	virtual void disconnect() =0;
	/**
	 * @brief Освобождение контекста
	 */
	virtual void free() = 0;
	///////////////////////////////  аналоги модбасовских функций
	/**
	 * @brief  Чтение регистров 0х03
	 * @param reg_addr Адрес регистра
	 * @param nb Количество байт
	 * @param dest Массив результата
	 * @return 0 или код ошибки
	 */
	int readRegisters(int reg_addr, int nb, uint16_t *dest); // 0x03
	/**
	 * @brief Чтение входных регистров 0x04
	 * @return true, если операци я успешна
	 * @param reg_addr Адрес регистра
	 * @param nb Количество байт
	 * @param dest Массив результата
	 * @return 0 или код ошибки
	 */
	int readInputRegisters(int reg_addr, int nb, uint16_t *dest);  // 0x04
	/**
	 * @brief Запись регистра 0x06
	 * @param reg_addr Адрес регистра
	 * @param value Занчение
	 * @return 0 или код ошибки
	 */
	int writeRegister(int reg_addr, int value); // 0x06
	/**
	 * @brief Запись группы регистров 0x10
	 * @param reg_addr Адрес регистра
	 * @param nb Количество байт
	 * @param data Массив данных
	 * @return 0 или код ошибки
	 */
	int writeRegisters(int reg_addr, int nb, const uint16_t *data); // 0x10
	/**
	 * @brief ОБрабатывает сырой запрос, возвращает выходной буфер и его размер
	 * @param req Массив "запрос"
	 * @param req_len Длина массива
	 * @return 0 или код ошибки
	 */
	virtual int sendRawRequest(uint8_t *req, int req_len);
	/**
	 * @brief Получение подтверждения
	 * @param rsp Маасив "ответ"
	 * @param len  Длина массива
	 * @return 0 или код ошибки
	 */
	int receiveRawConfirmation(uint8_t **rsp, int& len);
	/**
	 * @brief Прием данных
	 * @return код исключения
	 */
	virtual	int receiveDataFromPort(uint8_t **rsp, int& len);
	/**
	 * @brief Reply exception
	 * @param req (request)
	 * @param exception_code (exception code)
	 * @return код исключения
	 */
	virtual int replyExeption(const uint8_t *req, unsigned int exception_code);
	///////////////////////////////  аналоги модбасовских функций
//    bool rawRequest(uint8_t* req, int req_len, uint8_t* rsp, int& rsp_len);
	/**
	 * @brief Возвращает указатель на модбас-контекст устройства
	 * @return Указатель на Модьас-контекст
	 */
	modbus_t* getModbusContext() const;
	/**
	 * @brief Возвращает адрес устройства
	 * @return Значение адреса устройства
	 */
	int getModbusAddress() const;
	/**
	 * @brief Возвращает адрес устройства
	 * @return Значение адреса устройства
	 */
	int getModbusFunction() const;
	/**
	 * Проверка на то, что подключены
	 * @return true, если подключен
	 */
	bool isConnected() const;
	/**
	 * Возвращает код ошибки
	 * @return значение последней ошибки
	 */
	int getModbusErrorCode() const;
	/**
	 * @brief Обработка кода ошибкм (errno)
	 * @return код Исключения Модбас
	 */
	int errHandler() const;
	/**
	 * @brief Обработка исключения
	 * @return код исключения
	 */
	int exceptHadler(uint8_t exceptionCode) const;
	/**
	 * @brief Возвращает тип движка
	 */
	engine_backend_type getBackendMode() const;
	bool isNewDataAvaliable() const;
	bool isReplyDataAvailable() const;
	virtual void setReplyDataAvailable();
	void resetReplyDataAvailable();
protected:
	std::string mPortName; //!< Имя порта в системе
	modbus_t *mpModbusContext; //!< контекст устройства модбас
	int mAddress; //!< Модбас адрес слэйва
	int mFunc; //!< Функция Модбас
	int mErrorCode; //!< Код последней ошибки Модбас
	engine_backend_type mBackendMode; //!< тип Modbus-движка
	PortSettings * mpPortSettings; //!< указатель на структуру с параметрами порта
	const useconds_t mWaitPeriod; //!< пацза после закрытия контекста
	uint8_t *pModbusBufer();
	uint8_t mQueryData[MODBUS_TCP_MAX_ADU_LENGTH];
	uint8_t mRecvdat[MODBUS_TCP_MAX_ADU_LENGTH];
	bool mIsDataReceived;
	bool mIsReplyDataAvailable;
	int16_t mQueryDataLen;

};

/**
 * @class EngineModbusRTU
 * @brief Движок модбаса по 485 порту в RTU
 */
class EngineModbusRTU: public EngineModbus {
public:
	EngineModbusRTU();
	virtual ~EngineModbusRTU();
	virtual void init(PortSettings *);
	virtual bool open();
	virtual void close();
	virtual bool connect();
	virtual void disconnect();
	virtual void free();


private:
	/**
	 * Параметры для устройства RTU
	 */
	int mBaud; //!< скорость
	int mParity; //!< четность
	int mDataBit; //!< количество бит
	int mStopBit; //!< стоповый бит
	struct timeval mByteTimeout; //!< тайм-аут между двумя последовательными байтами
	struct timeval mResponseTimeout; //!< таймаут ответа

};

/**
 * @class EngineModbusTCP
 * @brief Движок модбаса по порту Ethernet (ModbusTCP)
 */
class EngineModbusTCP: public EngineModbus {
public:
	EngineModbusTCP();
	~EngineModbusTCP();
	virtual void init(PortSettings *);
	virtual bool open();
	virtual void close();
	virtual bool connect();
	virtual void disconnect();
	virtual void free();

private:
	std::string mTcpMbSrvAddress; //!< ipv4 19 байт
	std::string mTcpMbSrvMask; //!< маска подсети
	std::string mTcpMbSrvMAC; //!< MAC адрес устройства, в дальнешем расширить
							  //!< проверку в целях безопасности или привязки по макам
	int mTcpMbSrvPort; //!< 502 по умолчанию, максимум 65535
	std::string mMode; //!< master (по умолчанию), slave (на будущее)

//    TODO доработать работу с тайм-аутами.
//    struct timeval mByteTimeout; //!< тайм-аут между двумя последовательными байтами
//    struct timeval mResponseTimeout; //!< таймаут ответа

};

/**
 * @def DEFAULT_TCP_LISTEN_ADRESS
 * @brief По умолчанию слушаем на всех адресах
 */
#define DEFAULT_TCP_LISTEN_ADRESS "0.0.0.0"
/**
 * @def DEFAULT_TCP_LISTEN_PORT
 * @brief Стандартный порт 502
 */
#define DEFAULT_TCP_LISTEN_PORT 502
/**
 * @def DEFAULT_TCP_GW_CONNECTIONS
 * @brief По умолчанию 16 соединений
 */
#define DEFAULT_TCP_GW_CONNECTIONS 16
/**
 * @class EngineModbusTCPServer
 * @brief Движок модбаса на порту Ethernet (ModbusTCP)
 */
class EngineModbusTCPServer: public EngineModbusTCP {
public:
	EngineModbusTCPServer();
	~EngineModbusTCPServer();
	virtual void init(PortSettings *);
	virtual bool open();
	virtual void close();
	virtual bool connect();
	virtual void disconnect();
	virtual void free();
	void listen();
	bool start();
	virtual	int receiveDataFromPort(uint8_t **rsp, int& len);
	virtual int sendRawRequest(uint8_t *req, int req_len);
	virtual int replyExeption(const uint8_t *req, unsigned int exception_code);
	void setReplyDataAvailable();

private:
	std::string mTcpMbSrvAddress; //!< ipv4 19 байт
	std::string mTcpMbSrvMask; //!< маска подсети
	std::string mTcpMbSrvMAC; //!< MAC адрес устройства, в дальнешем расширить
							  //!< проверку в целях безопасности или привязки по макам
	int mTcpMbSrvPort; //!< 502 по умолчанию, максимум 65535
	std::string mMode; //!< master (по умолчанию), slave (на будущее)
	int mListenSocket;
	int mMasterSocket;
	int mListenConnections;
	fd_set mRefSet;
	fd_set mRdSet;
	int mFdMax;
	pthread_t mTCPServerCarrier;
	uint8_t mpRespData[MODBUS_TCP_MAX_ADU_LENGTH];
	int16_t mRespLen;
	uint8_t *mpQueryData;

//    TODO доработать работу с тайм-аутами.
//    struct timeval mByteTimeout; //!< тайм-аут между двумя последовательными байтами
//    struct timeval mResponseTimeout; //!< таймаут ответа
    static void* ThreadFunction(void *d)
    {
        static_cast<EngineModbusTCPServer *>(d)->listen();
        return NULL;
    }

};

#endif /* ENGINEMODBUS_H_ */
