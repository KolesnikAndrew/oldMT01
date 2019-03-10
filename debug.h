/*
 * debug.h
 *
 *  Created on: 2 лип. 2018 р.
 *      Author: v7r
 */

#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

/**
 * @def DEBUG
 * @brief По умолчанию включает режим отладки на ПК
 */
#if defined (__i386__)|| defined (__amd64__)
	#define DEBUG
	#define DEBUG_PC
	#define DEBUG_ALL
#endif

/**
 * @def DEBUG_MSG
 * @brief display debug msg to console
 */
#if defined (DEBUG) && !defined (DEBUG_MSG)
	#define DEBUG_MSG
#endif
/**
 * @def DEBUG_MODBUS
 * @brief display modbus debug outputs
 */
#if defined (DEBUG) && ! defined (DEBUG_MODBUS)
	#define DEBUG_MODBUS
#endif
/**
 * DEBUG_LEVEL 0, 1, 2
 */
#if defined (DEBUG) && ! defined (DEBUG_LEVEL)
	#define DEBUG_LEVEL 0
#endif


#endif /* SRC_DEBUG_H_ */
