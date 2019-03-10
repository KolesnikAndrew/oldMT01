/**
 * @file      globals.cpp
 * @brief     Модуль глобальных переменных
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include "globals.h"

bool endWorkFlag = false; /**< //флаг окончания работы */
bool standAloneMode = false; /**< режим ожидания подключения к сети */
bool offlineMode = false; /**< без пдключения к сети */
bool switchToNormalMode = false; /**< не было переключения */

IPC::ISemaphore* pSemaphoreTCP = 0;
IPC::ISemaphore* pSemaphoreScada = 0;

std::ostream& operator <<(std::ostream & stream, const fileStatus_type& state){
	return stream << static_cast<int>(state);
}
