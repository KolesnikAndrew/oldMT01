/*
 * global_types.h
 *
 *  Created on: 24 вер. 2018 р.
 *      Author: v7r
 */

#ifndef SRC_GLOBAL_TYPES_H_
#define SRC_GLOBAL_TYPES_H_

#include <string>
/**
 * @brief флаг режима работы модема
 */
enum class modemWorkMode_type {
	normal, /**< есть подключение к серверу*/
	standalone, /**< автономный режим c атопереходом в онлайн*/
	offline, /**< офлайн режим*/
	local /**< локальный режим*/
};

/**
 * @brief parameter modbus tyoe identifier
 */
enum class paramType_type {
	p_int,              	//!< p_int
	p_float,            	//!< p_float
	p_single,              	//!< p_single
	p_boolean,          	//!< p_boolean
	p_string,           	//!< p_string
	p_enum,             	//!< p_enum
	p_bitfield,         	//!< p_bitfield
	p_byte_array_single,    //!< p_byte_array_single
	p_file,					//!< p_file
	p_sizefull,         	//!< p_sizefull
	p_sizeavailable,     	//!< p_sizeavailable
	p_word_state,			//!< p_word_state
	p_single_matrix,		//!< p_single_matrix
	p_crash_kn24,
	p_undefined 	//!<p_undefined
};
/**
 * @brief Признак достоверности данных (см. протокол обмена)
 */
enum class validState_type {
	valid, /**< 0 - Значение корректно */
	not_responding, /**< 1 - Значение не корректно. (Устройство не отвечает) */
	invalid_address, /**< 2 - Значение не корректно. (Неверное значение адреса) */
	invalid_function, /**< 3 - Значение не корректно. (Неверная функция запроса) */
	device_busy, /**< 4 - Значение не корректно. (Устройство занято) */
	invalid_answer, /**< 5 - Значение не корректно. (Ошибочный ответ устройства) */
	invalid_unknown /**< 6 - Значение не корректно. (Неизвестная ошибка) */
};

/**
 * @brief Состояние входного параметра (см. протокол обмена)
 */
enum class inDataState_type {
	new_input, /**< 0 - Новая уставка */
	processing, /**< 1 - Уставка в обработке */
	success, /**< 2 - Уставка успешно установлена */
	not_responding, /**< 3 - Уставка не установлена (Устройство не отвечает) */
	invalid_address, /**< 4 - Уставка не установлена (Неверное значение адреса)  */
	readonly, /**< 5 - Уставка не установлена (Параметр только для чтения) */
	invalid_data, /**< 6 - Уставка не установлена (Значение уставки некорректно) */
	invalid_function, /**< 7 - Уставка не установлена (Неверная функция запроса) */
	device_busy, /**< 8 - Уставка не установлена (Устройство занято) */
	invalid_answer, /**< 9 - Уставка не установлена (Ошибочный ответ устройства) */
	unknown /**< 10- Уставка не установлена (Неизвестная ошибка) */
};

/**
 * @brief данные для записи в канал текущих данных
 */
struct outputData_type {
	int globalID; /**< Идентификатор параметра в БД сервера */
	std::string value; /**< Текущее значение */
	std::string dateTime; /**< Временной штамп значения в формате "ГГГГ-ММ-ДД ЧЧ:ММ:СС" */
	int ms; /**< Временной штамп значения [мс]*/
	validState_type validState; /**< Признак достоверности данных (см. протокол обмена) */
	paramType_type paramType;
};

/**
 * @brief данные для чтения из канала уставок
 */
struct inputData_type {
	int globalID; /**< Идентификатор параметра в БД сервера */
	std::string value; /**< Значение */
};

/**
 * @brief данные для записи в канал состояния уставок
 */
struct confirmData_type {
	confirmData_type() :
			globalID(0), dateTime(""), state(inDataState_type::unknown) {
	}
	unsigned int globalID; /**< Идентификатор параметра в БД сервера */
	std::string dateTime; /**< Временной штамп значения в формате "ГГГГ-ММ-ДД ЧЧ:ММ:СС" */
	inDataState_type state; /**< Состояние входного параметра (см. протокол обмена) */
};

/**
 * @brief Состояние вычитки журнала (см. протокол обмена)
 */
enum class fileStatus_type {
	start_process = 0, /**< 0 - Oжидает продолжения */
	in_process, /**< 1 - В процессе вычитки */
	read_success, /**< 2 - Успешно выполнено  */
	need_delete, /**< 3 */
	delete_success, /**< 4 */
	need_stop, /**< 5 */
	stop_success, /**< 6 - Ошибка чтения из канала АСУ( подробности см. validState_type) */
	modbus_error, /**< 7 - Ошибка чтения из канала АСУ( подробности см. validState_type) */
	rw_error, /**< 8 - Ошибка записи или чтения файла на МТ01(битый файл) */
	ready_file, /**< 9 READY FILE*/
	unknown, /** 10 */
	send_success, /**< 11 Файл был отослан*/
	need_restart, /**12**/
	need_write = 15, /**< в процессе записи**/
	write_success, /**< 16 успешно записан */
	write_confirm_success, /**< 17 подтверждение вычиткой успешное */
	write_fail, /**< 18 ошибка записи конфига*/
	write_confirm_fail, /**< 19 ошибка подтверждения записи*/
	bad_id /**< 20 id in request is not exists**/
};

std::ostream& operator <<(std::ostream & stream, const fileStatus_type& state);

/**
 * @brief данные для записи в канал вычитки журнала
 */
struct umkaFile_type {
	umkaFile_type() :
			isWriteMode(false), globalID(0), deviceNumber(0), cur_fileSize(0), cur_filePath(
					""), config_buffer(""), umkaFileStatus(
					fileStatus_type::unknown), validState(
					validState_type::invalid_unknown)/*, isFileSended(false), isNeedReadFile(
	 false), isNeedStopReading(false), isNeedDeleteFile(false)*/{
	}

	bool isWriteMode; /**< write or read mode **/
	unsigned int globalID; /**< Идентификатор параметра в БД сервера */
	unsigned int deviceNumber; /**< Идентификатор параметра в БД сервера */
	long cur_fileSize; /**< Текущее значение */
	std::string cur_filePath;
	std::string config_buffer; /**< config file data**/
	fileStatus_type umkaFileStatus; /**< Состояние вычитки журнала */
	validState_type validState; /**< Признак достоверности данных (см. протокол обмена) */
};

enum class byteOrder_type {
	intel, /**< Младшие байты имеют меньший адрес*/
	motorola, /**< Старшие байты имеют меньший адрес*/
	high_byte_low_word,
	high_byte_high_word,
	low_byte_high_word,
	low_byte_low_word
};
/**< Порядок следования байт */

/**
 * Разновидность журнала
 */
enum class umkaFileClass_type {
	umka03, 	//!< umka03
	umka27, 	//!< umka27
	umka27cfg, //!< umka27 config
	kn24,   //!< kn24
	undefined //!< не поддерживаемый тип
};
/**
 * @brief descritor for Modbus device type
 */
enum class deviceModbusTypeID_type {
	regular,//!< regular
	stat,   //!< stat
	din16v1,//!< din16v1
	din16v2,//!< din16v2
	kn24    //!< kn24
};

#endif /* SRC_GLOBAL_TYPES_H_ */
