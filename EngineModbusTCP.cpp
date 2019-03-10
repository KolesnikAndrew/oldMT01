/*
 * EngineModbusTCP.cpp
 *
 *  Created on: 14 бер. 2018 р.
 *      Author: v7r
 */

#include "EngineModbus.h"

using namespace std;

EngineModbusTCP::EngineModbusTCP() :
		mTcpMbSrvAddress("10.0.0.2"), mTcpMbSrvMask("255.0.0.0"), mTcpMbSrvMAC(
				"0:0:0:0:0:0"), mTcpMbSrvPort(502) {
	std::cout << "[EngineModbusTCP]::EngineModbusTCP()." << std::endl;
}

EngineModbusTCP::~EngineModbusTCP() {
	std::cout << "[EngineModbusTCP]::~EngineModbusTCP()." << std::endl;

}

bool EngineModbusTCP::open() {
	//std::cout << "Open TCP\n";
	mpModbusContext = modbus_new_tcp(mTcpMbSrvAddress.c_str(), mTcpMbSrvPort);
	if (!mpModbusContext) {
		cerr << "\nCan't open TCP connection\n" << endl;
		return false;
	}
	cerr << "\nNew TCP connection was created.\n" << endl;
#ifdef ENGINE_MODBUS_DEBUG
	modbus_set_debug(mpModbusContext, TRUE);
#else
	modbus_set_debug(mpModbusContext, FALSE);
#endif
	mBackendMode = TCP;
	return true;
}

void EngineModbusTCP::close() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		usleep(mWaitPeriod);
	}
}

bool EngineModbusTCP::connect() {
	int16_t rc = -1;
	// const unsigned short maxRepeatsCnt = 3; //! после maxRepeatsCnt неудачных попыток пересоединяемся  сервером
	int cnt = 0;
	//while (rc == -1) {
	rc = modbus_connect(mpModbusContext);
	if (rc == -1) {
		std::cerr << "[EngineModbusTCP::connect()] rc = " << rc << std::endl;
		close();
		free();
	}
	return rc == -1 ? false : true;

}

void EngineModbusTCP::init(PortSettings* settings) {
	TcpPortSettings *pSet = dynamic_cast<TcpPortSettings *>(settings);
	mTcpMbSrvAddress = pSet->netAddress;
	mTcpMbSrvPort = pSet->servicePort;
	mMode = pSet->modbusMode;
}

void EngineModbusTCP::disconnect() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		modbus_flush(mpModbusContext);
		usleep(mWaitPeriod);
	}
}
void EngineModbusTCP::free() {
	if (mpModbusContext) {
		modbus_free(mpModbusContext);
		usleep(mWaitPeriod);
		mpModbusContext = 0;
	}
}




