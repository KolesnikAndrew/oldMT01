/**
 * @file      data_struct_definitions.h
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */
#ifndef PORTSETTINGS__H_
#define PORTSETTINGS__H_
#include <string>
#include <sys/time.h>

/**
 * Возможные типы движков
 */
typedef enum {
	RTU = 0, TCP, ASCII
} engine_backend_type;
/**
 * Конфигурационные параемтры порта rs485
 */
struct PortSettings {
	virtual ~PortSettings();
	std::string portName; /**< Имя порта в системе */
	engine_backend_type modbusBackend;
	/**
	 * @brief Печать отладочной инфоррмаци для всей иерарзии
	 * @param stream - поток ostream
	 * @return ссылку на поток
	 */
	virtual std::ostream& print(std::ostream& stream) const;
	/**
	 * @brief Перегруженный оператор вывода
	 * @param stream - поток ostream
	 * @param instance - объект QueryMsg
	 * @return ссылку на поток с данными
	 */
	friend std::ostream& operator<<(std::ostream& stream, const PortSettings& instance);
};

/**
 * Класс для работы с настройками порта RTU Modbus
 */
struct RtuPortSettings: public PortSettings {
	RtuPortSettings();
	int baud; /**< Скорость обмена */
	char parity; /**< Четность ('N' -none, 'E' - четный, 'O' - нечетный) */
	int dataBit; /**< Количество бит данных 5, 6, 7 или 8 */
	int stopBit; /**< Количество стоп-бит 1 или 2 */
	struct timeval byteTimeout; //!< максимальное время паузы между 2-мя последовательными байтами
	struct timeval responseTimeout; //!< максимальное время ожидания ответа от слэйва
	virtual std::ostream& print(std::ostream& stream) const;
};

/**
 * Класс для работы с настройками порта TCPModbus
 */
struct TcpPortSettings: public PortSettings {
	TcpPortSettings();
	std::string netAddress; /**< IPv4 адрес */
	std::string netMask; /** маска подсети */
	std::string netGateway; /**< адрес шлюза */
	int servicePort; /**< TCP modbus port - 502 по умолчанию */
	std::string modbusMode; /** "master" | "slave" */
	virtual std::ostream& print(std::ostream& stream) const;
};

#endif /* PORTSETTINGS__H_ */
