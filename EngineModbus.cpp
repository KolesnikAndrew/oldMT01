/**
 * @file      EngineModbus.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Савченко Владимир
 * @copyright © TRIOLCORP, 2017
 */

#include <iostream>
#include "EngineModbus.h"


EngineModbus::EngineModbus() :
		mPortName(""), mpModbusContext(0), mAddress(0), mErrorCode(0), mBackendMode(
				RTU), mpPortSettings(0), mWaitPeriod(300), mFunc(0), mIsDataReceived(false), mIsReplyDataAvailable(false),mQueryDataLen(0) {
	std::cout << "[EngineModbus] started." << std::endl;

}

EngineModbus::~EngineModbus() {
	std::cout << "exit form modbus" << std::endl;
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		modbus_free(mpModbusContext);
		usleep(mWaitPeriod);
	}
}

int EngineModbus::readRegisters(int reg_addr, int nb, uint16_t* dest) {
	int rez = modbus_read_registers(mpModbusContext, reg_addr, nb, dest);
	if (rez < 0) {
		return (mErrorCode = errHandler());
	}
	return (mErrorCode = 0);
}

int EngineModbus::readInputRegisters(int reg_addr, int nb, uint16_t* dest) {
	int res = modbus_read_registers(mpModbusContext, reg_addr, nb, dest);
	if (res < 0) {
		return (mErrorCode = errHandler());
	}
	return (mErrorCode = 0);
}

int EngineModbus::writeRegister(int reg_addr, int value) {
	int res = modbus_write_register(mpModbusContext, reg_addr, value);
	if (res < 0) {
		// обработка ошибок
		return (mErrorCode = errHandler());
	}
	mErrorCode = 0;
	return (mErrorCode = 0);

}

int EngineModbus::writeRegisters(int reg_addr, int nb, const uint16_t* data) {
	int res = modbus_write_registers(mpModbusContext, reg_addr, nb, data);
	if (res < 0) {
		// обработка ошибок
		return (mErrorCode = errHandler());
	}

	return (mErrorCode = 0);
}

//bool EngineModbus::rawRequest(uint8_t* req, int req_len, uint8_t* rsp,
//		int& rsp_len) {
//	int res = modbus_send_raw_request(mpContext, req, sizeof(req));
//	if (res < 0) {
//		// обработка ошибок
//		return !(mErrorCode = errHandler());
//	}
//	rsp_len = modbus_receive_confirmation(mpContext, rsp);
//	if (res < 0) {
//		// обработка ошибок
//
//		return !(mErrorCode = exceptHadler(rsp[2]));
//	}
//	mErrorCode = 0;
//	return !mErrorCode;
//}

int EngineModbus::sendRawRequest(uint8_t* req, int req_len) {

	if (mpModbusContext) {
		if (modbus_send_raw_request(mpModbusContext, req, req_len) == -1) {
			return (mErrorCode = errHandler());
		}
	} else {
		return (mErrorCode = 0x04); //!SLAVE DEVICE FAILURE
	}
	return (mErrorCode = 0);
}

int EngineModbus::receiveRawConfirmation(uint8_t **rsp, int& len) {
	uint8_t *pRsp = *rsp;
	if (mpModbusContext) {
		int res = modbus_receive_confirmation(mpModbusContext, pRsp);
		if (res == -1) {
			return (mErrorCode = errHandler());
		}
		if (mBackendMode == RTU) {
			len = res - 2; //! обрезаем crc
		}
		if (mBackendMode == TCP) {
			len = res - 6; //! обрезаем заголовок
		}
	} else {
		return (mErrorCode = 0x04);  //!SLAVE DEVICE FAILURE
	}
	return (mErrorCode = 0);
}

modbus_t* EngineModbus::getModbusContext() const {
	return mpModbusContext;
}

int EngineModbus::getModbusAddress() const {
	return mAddress;
}

bool EngineModbus::isConnected() const {
	return mpModbusContext != 0 ? true : false;
}

int EngineModbus::getModbusErrorCode() const {

	return mErrorCode;

}

int EngineModbus::errHandler() const {
	switch (errno) {
	case EMBXILFUN: //Illegal function
		return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
	case EMBXILADD: //Illegal data address
		return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	case EMBXSFAIL: //Slave device or server failure
		return MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE;
	case 110:       //Connection timed out
		return MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY;
	case EMBXSBUSY: //Slave device or server is busy
		return MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY;
	case EMBXGTAR: //Target device failed to respond
		return MODBUS_EXCEPTION_GATEWAY_TARGET;
	case EMBBADCRC: //Invalid CRC
		return MODBUS_EXCEPTION_GATEWAY_TARGET + 1;
	case EMBBADDATA: //Invalid data
		return MODBUS_EXCEPTION_GATEWAY_TARGET + 2;
	case EMBBADEXC: //Invalid exception code
		return MODBUS_EXCEPTION_GATEWAY_TARGET + 3;
	default:
		return MODBUS_EXCEPTION_NOT_DEFINED;
	}
}

int EngineModbus::replyExeption(const uint8_t* req,
		unsigned int exception_code) {
	if (mpModbusContext) {
		int r = modbus_reply_exception(mpModbusContext, req, exception_code);
		if (r == -1) {
			return (mErrorCode = errHandler());
		} else {
			return (mErrorCode = 0x04);
		}
	}
	return (mErrorCode = 0);
}

int EngineModbus::getModbusFunction() const {
	return mFunc;
}

int EngineModbus::exceptHadler(uint8_t exceptionCode) const {
	switch (exceptionCode) {
	case 0x01: //ILLEGAL FUNCTION
		return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
	case 0x02: //ILLEGAL DATA ADDRESS
		return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	case 0x03: //ILLEGAL DATA VALUE
		return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
		break;
	case 0x04: //SLAVE DEVICE FAILURE
		return MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE;
	default:
		return MODBUS_EXCEPTION_NOT_DEFINED;
	}
}

int  EngineModbus::receiveDataFromPort(uint8_t** rsp, int& len) {
	using namespace std;
	uint8_t *pRsp = *rsp;

	if (mpModbusContext) {

		int r = modbus_receive(mpModbusContext, pRsp);
		if (r == -1) {
			return (mErrorCode = errHandler());
		}
		if (mBackendMode == TCP) {
			pRsp += 6;
			len = r - 6;
		} else if (mBackendMode == RTU) {
			len = r - 2;
		}
		mAddress = pRsp[0];
		mFunc = pRsp[1];
	} else {
		return (mErrorCode = 0x04);
	}
	return (mErrorCode = 0);
}


engine_backend_type EngineModbus::getBackendMode() const {
	return mBackendMode;
}

bool EngineModbus::isNewDataAvaliable() const
{
	return mIsDataReceived;
}

bool EngineModbus::isReplyDataAvailable() const
{
	return mIsReplyDataAvailable;
}

void EngineModbus::setReplyDataAvailable()
{
	mIsReplyDataAvailable = true;
}
void EngineModbus::resetReplyDataAvailable()
{
	mIsReplyDataAvailable = false;
}
