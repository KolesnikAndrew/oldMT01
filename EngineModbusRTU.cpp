/*
 * EngineModbusRTU.cpp
 *
 *  Created on: 14 бер. 2018 р.
 *      Author: v7r
 */

#include "EngineModbus.h"


EngineModbusRTU::EngineModbusRTU() :
		mBaud(0), mParity(1), mDataBit(8), mStopBit(1), mByteTimeout( { 0,
				500000 }), mResponseTimeout( { 0, 500000 }) {
	std::cout << "[EngineModbusRTU]::EngineModbusRTU()." << std::endl;
}

EngineModbusRTU::~EngineModbusRTU() {
	std::cout << "[EngineModbusRTU]::~EngineModbusRTU()." << std::endl;

}

bool EngineModbusRTU::open() {
	mpModbusContext = modbus_new_rtu(mPortName.c_str(), mBaud, mParity, mDataBit,
			mStopBit);
	if (!mpModbusContext) {
		std::cerr << "[Enginemodbus] Unable to create the libmodbus context"
				<< std::endl;
		return false;
	}

#ifdef ENGINE_MODBUS_DEBUG
	modbus_set_debug(mpModbusContext, TRUE);
#else
	modbus_set_debug(mpModbusContext, FALSE);
#endif
	modbus_set_error_recovery(mpModbusContext,
			(modbus_error_recovery_mode) (MODBUS_ERROR_RECOVERY_LINK
					| MODBUS_ERROR_RECOVERY_PROTOCOL));
	mBackendMode = RTU;
	return true;

}

void EngineModbusRTU::close() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		usleep(mWaitPeriod);
	}
}

bool EngineModbusRTU::connect() {

	if (modbus_connect(mpModbusContext) == -1) {
		return false;
	}
#ifdef ENGINE_MODBUS_DEBUG
	int r = modbus_rtu_set_serial_mode(mpModbusContext, MODBUS_RTU_RS485);
#endif
	modbus_set_slave(mpModbusContext, mAddress);
	modbus_set_byte_timeout(mpModbusContext, mByteTimeout.tv_sec,
			mByteTimeout.tv_usec);
	modbus_set_response_timeout(mpModbusContext, mResponseTimeout.tv_sec,
			mResponseTimeout.tv_usec);
	return true;
}

void EngineModbusRTU::init(PortSettings* settings) {

	RtuPortSettings *pSet = dynamic_cast<RtuPortSettings *>(settings);
	mPortName = pSet->portName;
	mBaud = pSet->baud;
	mParity = pSet->parity;
	mDataBit = pSet->dataBit;
	mStopBit = pSet->stopBit;
	mByteTimeout = pSet->byteTimeout;
	mResponseTimeout = pSet->responseTimeout;
}

void EngineModbusRTU::disconnect() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		modbus_flush(mpModbusContext);
		usleep(mWaitPeriod);
	}
}

void EngineModbusRTU::free() {
	if (mpModbusContext) {
		modbus_free(mpModbusContext);
		usleep(mWaitPeriod);
		mpModbusContext = 0;
	}
}


