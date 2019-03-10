/**
 * @file      globals.h
 * @brief     Заголовок модуля глобальных переменных
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 * @version: 117.300 (2017)
 */
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <map>
#include <semaphore.h>
#include <sched.h>
#include "semaphors.h"
#include "debug.h"
#include "global_defines.h"
#include "global_types.h"

/**
 * Вспомогательные переменные для управления режимом работы модема
 * */
extern bool endWorkFlag; /**< флаг окончания работы */
extern bool standAloneMode; /**<  флаг режима работы без подключения к серверу */
extern bool offlineMode; /**< оффлановый режим*/
extern bool switchToNormalMode; /**< признак переключения в нормальный режим из режима без подключения*/

extern bool gStopConfirmSend;

/**
 * @brief only for docs
 */
const std::map<int, std::string> modbusDeviceTypes = { { 3, "Any" }, { 1001,
		"Din16Dout8v1" }, { 1002, "Din16Dot8v2" }, { 1003, "StatMT01" }, };

extern IPC::ISemaphore* pSemaphoreTCP;
extern IPC::ISemaphore* pSemaphoreScada;

#endif /* GLOBALS_H_ */
