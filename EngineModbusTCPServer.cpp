/*
 * EngineModbusTCPServerServer.cpp
 *
 *  Created on: 14 бер. 2018 р.
 *      Author: v7r
 */

#include "EngineModbus.h"

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <globals.h>

using namespace std;

EngineModbusTCPServer::EngineModbusTCPServer() :
		mTcpMbSrvAddress("10.0.0.2"), mTcpMbSrvMask("255.0.0.0"), mTcpMbSrvMAC(
				"0:0:0:0:0:0"), mTcpMbSrvPort(502), mListenConnections(0), mListenSocket(
				0), mTCPServerCarrier(0), mFdMax(0), mMasterSocket(0), mpQueryData(
				0), mRespLen(0) {
	std::cout << "[EngineModbusTCPServer]::EngineModbusTCPServer()."
			<< std::endl;
}

EngineModbusTCPServer::~EngineModbusTCPServer() {
	std::cout << "[EngineModbusTCPServer]::~EngineModbusTCPServer()."
			<< std::endl;

}

bool EngineModbusTCPServer::open() {
	std::cout << "New TCP server \n";
	cout << mTcpMbSrvAddress << ":::" << mTcpMbSrvPort << endl;
	mpModbusContext = modbus_new_tcp(mTcpMbSrvAddress.c_str(), mTcpMbSrvPort);
	if (!mpModbusContext) {
		return false;
	}

#ifdef ENGINE_MODBUS_DEBUG
	modbus_set_debug(mpModbusContext, TRUE);
#else
	modbus_set_debug(mpModbusContext, FALSE);
#endif
	mBackendMode = TCP;
	start();
	return true;
}

void EngineModbusTCPServer::close() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		usleep(mWaitPeriod);
	}
}

bool EngineModbusTCPServer::connect() {
	return true;
}

//TODO доделать передачу тайм-аутов в либмодбас из конфига.
void EngineModbusTCPServer::init(PortSettings* settings) {
	cout << "EngineModbusTCPServer init()" << endl;
	TcpPortSettings *pSet = dynamic_cast<TcpPortSettings *>(settings);
	mTcpMbSrvAddress = pSet->netAddress;
	mTcpMbSrvPort = pSet->servicePort;
	mMode = pSet->modbusMode;
	cout << mTcpMbSrvAddress << ":::" << mTcpMbSrvPort << endl;

}

void EngineModbusTCPServer::disconnect() {
	if (mpModbusContext) {
		modbus_close(mpModbusContext);
		modbus_flush(mpModbusContext);
		usleep(mWaitPeriod);
	}
}
void EngineModbusTCPServer::free() {
	if (mpModbusContext) {
		modbus_free(mpModbusContext);
		usleep(mWaitPeriod);
		mpModbusContext = 0;
	}
}

bool EngineModbusTCPServer::start() {
	{
		if (pthread_create(&mTCPServerCarrier, NULL,
				EngineModbusTCPServer::ThreadFunction, (void*) this) == 0) {
			clog << "[EngineModbusTCPServer::Start] server thread added."
					<< endl;
			sleep(2);
			return true;
		} else {

			clog << "[EngineModbusTCPServer::Start] server thread not added."
					<< endl;
			return false;
		}
	}
}

void EngineModbusTCPServer::listen() {

	mListenSocket = modbus_tcp_listen(mpModbusContext, mListenConnections);

	FD_ZERO(&mRefSet);
	FD_SET(mListenSocket, &mRefSet);
	mFdMax = mListenSocket;

	if (mListenSocket == -1) {
		fprintf(stderr, "Unable to listen TCP connection\n");
		modbus_free(mpModbusContext);
		return;
	}
	for (;;) {
		mRdSet = mRefSet;
		if (select(mFdMax + 1, &mRdSet, NULL, NULL, NULL) == -1) {
			::close(mListenSocket);
		}
		/* Run through the existing connections looking for data to be
		 * read */
		for (mMasterSocket = 0; mMasterSocket <= mFdMax; ++mMasterSocket) {

			if (!FD_ISSET(mMasterSocket, &mRdSet)) {
				continue;
			}

			if (mMasterSocket == mListenSocket) {
				/* A client is asking a new connection */
				socklen_t addrlen;
				struct sockaddr_in clientaddr;
				int newfd;

				/* Handle new connections */
				addrlen = sizeof(clientaddr);
				memset(&clientaddr, 0, sizeof(clientaddr));
				newfd = accept(mListenSocket, (struct sockaddr *) &clientaddr,
						&addrlen);
				if (newfd == -1) {
					perror("Server accept() error");
				} else {
					FD_SET(newfd, &mRefSet);
					if (newfd > mFdMax) {
						mFdMax = newfd;
					}
					printf("New connection from %s:%d on socket %d\n",
							inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port,
							newfd);
				}
			} else {

				modbus_set_socket(mpModbusContext, mMasterSocket);
				mQueryDataLen = modbus_receive(mpModbusContext, mQueryData);
				if (mQueryDataLen > 0) {

					pSemaphoreTCP->Post();
					pSemaphoreScada->Wait();

					int r = (mFunc != mpRespData[1]) ?
							modbus_reply_exception(mpModbusContext, mQueryData,
									mRespLen) :
							modbus_send_raw_reply(mpModbusContext, mQueryData,
									mQueryDataLen, mpRespData, mRespLen);

					mQueryDataLen = mRespLen = 0;
					memset(mQueryData, 0, MODBUS_TCP_MAX_ADU_LENGTH);
					memset(mpRespData, 0, MODBUS_TCP_MAX_ADU_LENGTH);

				} else if (mQueryDataLen == -1) {
					/* This example server in ended on connection closing or
					 * any errors. */
					printf("Connection closed on socket %d\n", mMasterSocket);
					::close(mMasterSocket);

					/* Remove from reference set */
					FD_CLR(mMasterSocket, &mRefSet);
					if (mMasterSocket == mFdMax) {
						--mFdMax;
					}
					mQueryDataLen = 0;
				}
			}
		}
	}

}

int EngineModbusTCPServer::receiveDataFromPort(uint8_t **rsp, int& len) {
	using namespace std;
	if (mQueryDataLen > 0) {
		*rsp = mQueryData + 6;
		len = mQueryDataLen - 6;
		mAddress = *(*rsp);
		mFunc = *(*rsp + 1);
		return (mErrorCode = 0);
	} else {
		return (mErrorCode = 0x06);
	}
}

int EngineModbusTCPServer::sendRawRequest(uint8_t *req, int req_len) {

	mRespLen = req_len;
	if (req) {
		memcpy(mpRespData, req, mRespLen);
	}
	return 0;
}

int EngineModbusTCPServer::replyExeption(const uint8_t *req,
		unsigned int exception_code) {
	return 0;
}

void EngineModbusTCPServer::setReplyDataAvailable() {
	mIsReplyDataAvailable = true;
}
