/**
 * @mainpage  Модуль обмена данными модема МТ01 с преиферийным оборудованием
 * @note
 * @author    Инженер-программист: Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 * @todo рефакторинг классов, которые работают с обменом, отделение логики на порту,
 * разделение логики по модбасовким девайсам, реализация проверки и установки типов,
 * перенос логики с девайс модбаса на порт (настройки на порт)
 */
/////* Includes ---------------------------------------------------------------*/
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <modbus.h>
#include <cstdlib>
#include <utilites.h>
#include <dirent.h>

#include "semaphors.h"
#include "modem.h"
#include "KN24defs.h"

using namespace std;

/**
 * @fn SigintHandler
 * @brief Вызывается по сигналу SIGINT (Ctrl-C).
 * @details
 * @param signum (не используется)
 * @return
 */
void SigintHandler(int signum) {
	endWorkFlag = true;
}

/**
 * @fn main
 * @brief Главная функция программы
 * @details
 * @param argc
 * @param argv[]
 * @return int
 */

using namespace IPC;

int main() {

	pSemaphoreScada = new CAnoymousSemaphore(1, INSIDE_PROCESS);
	pSemaphoreTCP = new CAnoymousSemaphore(1, INSIDE_PROCESS);

	if (signal(SIGINT, SigintHandler) == SIG_ERR) {
		cerr << "Impossible to get SIGINT" << endl;
		return 1;
	}

	V7Modem* modem = new V7Modem;
	if (modem->initModem()) {
		modem->runModem();
	} else {
		delete modem;
		delete pSemaphoreTCP;
		delete pSemaphoreScada;
		return -1;
	}

	delete modem;
	delete pSemaphoreTCP;
	delete pSemaphoreScada;

	return 0;
}

//#include <iostream>
//#include <unistd.h>
//#include <signal.h>
//#include <modbus.h>
//#include <cstdlib>
//#include "globals.h"
//#include "modem.h"
//#include "EngineModbus.h"
//#include "device.h"
//#include "globals.h"
//#include "semaphors.h"
//#include <utilites.h>
//#include <dirent.h>
//#include <fstream>
//#include <cstring>
//
//#include "KN24defs.h"
//
//using namespace std;
//
//int requestPort(QueryMsg * msg, EngineModbusRTU *mpModbusEngine) {
//
//	//flagBusy = true;
//	const useconds_t pauseInUs = 300;
//	int exitCode(0); //нет ошибки
//	// подключение к движку
//	if (!mpModbusEngine->getModbusContext())
//		mpModbusEngine->open();
//	// если подключились
//	if (mpModbusEngine->connect()) {
//		//PRINTDEBUG2("\n[Port::request()] Modbus ctx is: ", mpModbusEngine->getModbusContext());
//		msg->makeRawRequest();
//		int retCode = mpModbusEngine->sendRawRequest(msg->getRequestBuffer(),
//				msg->getRequestSize());
//		//cout << "reqtCode: " << (retCode) << "  :  ";
//		uint8_t reqFunc = msg->getRequestFunction();
//		//cout << "reqFunc: " << (int) reqFunc << endl;
//		;
//		//int addr = msg->getRequestBuffer()[0];
//
//		if (retCode != 0) {
//			// error & exit
//			PRINTDEBUG("[Port::request()] error & exit");
//			usleep(pauseInUs);
//			msg->setErrorCode(retCode);
//		} else {
//
//			uint8_t tmp[MODBUS_TCP_MAX_ADU_LENGTH];
//			uint8_t* pTmp = tmp;
//			int tmpSize(0);
//			retCode = mpModbusEngine->receiveRawConfirmation(&pTmp, tmpSize);
//
//			if (retCode != 0) {
//				cout << "Error!!!!!!\n";
//				msg->setErrorCode(retCode); // exit
//			} else {
//				engine_backend_type mBackendType = RTU;
//				if (mBackendType == RTU) {
//					if (reqFunc != tmp[1]) {
//						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[2]));
//					} else {
//
//						msg->setErrorCode(retCode);
//						msg->setResponseBuffer(tmp, tmpSize);
//					}
//
//				} else if (mBackendType == TCP) {
//
//					if (reqFunc != tmp[7]) {
//						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[8]));
//					} else {
//						msg->setErrorCode(retCode);
//						msg->setResponseBuffer(tmp + 6, tmpSize); ///
//					}
//				}
//
//			}
//
//		}
//		//        PRINTDEBUG2("\n[Port::request()] Request result:\n", *msg);
//		exitCode = msg->getErrorCode();
//		mpModbusEngine->close();
//	} else {
//		exitCode = 0x04; // slave device failure
//	}
//	// flagBusy = false;
//	return exitCode;
//}
//
//QueryMsgJrnAT27 request0x68(uint8_t fileNumber, uint8_t deviceAddress,
//		uint32_t recAddress, uint8_t recCnt) {
//	return QueryMsgJrnAT27(deviceAddress, 0x68, fileNumber,
//			(uint8_t) (recAddress >> 24), (uint8_t) (recAddress >> 16),
//			(uint8_t) (recAddress >> 8), (uint8_t) recAddress, recCnt);
//
//}
//
//bool setParamDescHeaderData(EngineModbusRTU* eng,
//		headOfParamDescFile_type& pdsHeader) {
//	const uint8_t devAddr = static_cast<uint8_t>(0x01);
//	QueryMsgJrnAT27 msg = request0x68(0x01, devAddr, 0, 64);
//	QueryMsg *pMsg = &msg;
//	int mbRes = requestPort(pMsg, eng);
//	if (mbRes == 0) {
//		memcpy(reinterpret_cast<uint8_t*>(&pdsHeader),
//				msg.getDataFromResponse() + 8, 128);
//	} else {
//		PRINTDEBUG("Error read file ")
//		return false;
//	}
//	return true;
//}
//
//bool setLogFileHEader(EngineModbusRTU* eng, headOfLogFile_type& logHeader) {
//	const uint8_t devAddr = static_cast<uint8_t>(0x01);
//	QueryMsgJrnAT27 msg = request0x68(0x18, devAddr, 0, 64);
//	QueryMsg *pMsg = &msg;
//	int mbRes = requestPort(pMsg, eng);
//	if (mbRes == 0) {
//		memcpy(reinterpret_cast<uint8_t*>(&logHeader),
//				msg.getDataFromResponse() + 8, 128);
//	} else {
//		PRINTDEBUG("Error read file ")
//		return false;
//	}
//	return true;
//}
//
//using namespace bigbrother;
//
//std::string readFile0x01(int32_t mParamDescrFileSize, EngineModbusRTU *emb) {
//	if (mParamDescrFileSize == 0) {
//		//setParamDescHeaderData();
//		//mParamDescrFileSize = getParamDescrFileSize();
//		if (mParamDescrFileSize == 0) {
//			return "";
//		}
//	}
//	const uint8_t devAddr = 0x01;
//	//static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
//	const int maxAttempts = 3;
//	int8_t cntErrorConnection(0);
//	std::string fileBuffer = "";
//	const uint8_t fileNumber = 0x01;
//	int32_t curSize(0);
//	do {
//
//		uint8_t recCnt =
//				curSize + MAX_REGISTER_COUNT < mParamDescrFileSize ?
//						static_cast<uint8_t>(MAX_REGISTER_COUNT) :
//						mParamDescrFileSize - curSize;
//		//	PRINTDEBUG2("size: ", mParamDescrFileSize)
//		//	PRINTDEBUG2("fb size: ", fileBuffer.length())
//		//	PRINTDEBUG2("cur size: ", curSize)
//		//	PRINTDEBUG2("recCnt: ", (int )recCnt)
//		QueryMsgJrnAT27 msg = request0x68(fileNumber, devAddr, curSize, recCnt);
//		//waitForFreePort();
//		int mbRes = requestPort(&msg, emb);
//		if (mbRes == 0) {
//			uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
//			uint8_t rspSize = msg.getResponseBuffer()[7] * 2;
//			memcpy(rspBuf, msg.getResponseBuffer() + 8,
//					msg.getResponseBuffer()[7] * 2);
//			fileBuffer.append(rspBuf, rspBuf + rspSize);
//
//		} else {
//			++cntErrorConnection;
//		}
//		curSize = fileBuffer.length() / 2;
//		//	PRINTDEBUG2(">>>>>>>> curSize: ", curSize)
//	} while (curSize < mParamDescrFileSize && cntErrorConnection != maxAttempts);
//	if (cntErrorConnection == maxAttempts) {
//		std::ofstream tmp("/home/v7r/projects/0x18err.hex",
//				std::ios_base::binary);
//		tmp.write(fileBuffer.c_str(), fileBuffer.size());
//		tmp.close();
//		PRINTDEBUG("error")
//		return "";
//	}
//	std::ofstream tmp("/home/v7r/projects/0x01.hex", std::ios_base::binary);
//	tmp.write(fileBuffer.c_str(), fileBuffer.size());
//	tmp.close();
//	return fileBuffer;
//}
//
//std::string readFile0x18(int32_t mParamDescrFileSize, EngineModbusRTU *emb) {
//	if (mParamDescrFileSize == 0) {
//		//setParamDescHeaderData();
//		//mParamDescrFileSize = getParamDescrFileSize();
//		if (mParamDescrFileSize == 0) {
//			return "";
//		}
//	}
//	const uint8_t devAddr = 0x01;
//	//static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
//	const int maxAttempts = 3;
//	int8_t cntErrorConnection(0);
//	std::string fileBuffer = "";
//	const uint8_t fileNumber = 0x18;
//	int32_t curSize(0);
//	do {
//
//		uint8_t recCnt =
//				curSize + MAX_REGISTER_COUNT < mParamDescrFileSize ?
//						static_cast<uint8_t>(MAX_REGISTER_COUNT) :
//						mParamDescrFileSize - curSize;
//		//	PRINTDEBUG2("size: ", mParamDescrFileSize)
//		//	PRINTDEBUG2("fb size: ", fileBuffer.length())
//		//	PRINTDEBUG2("cur size: ", curSize)
//		//	PRINTDEBUG2("recCnt: ", (int )recCnt)
//		QueryMsgJrnAT27 msg = request0x68(fileNumber, devAddr, curSize, recCnt);
//		//waitForFreePort();
//		int mbRes = requestPort(&msg, emb);
//		if (mbRes == 0) {
//			uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
//			uint8_t rspSize = msg.getResponseBuffer()[7] * 2;
//			memcpy(rspBuf, msg.getResponseBuffer() + 8,
//					msg.getResponseBuffer()[7] * 2);
//			bigbrother::swapBytesInWordsInBuffer(rspBuf, rspSize);
//			fileBuffer.append(rspBuf, rspBuf + rspSize);
//
//		} else {
//			++cntErrorConnection;
//		}
//		curSize = fileBuffer.length() / 2;
//		//	PRINTDEBUG2(">>>>>>>> curSize: ", curSize)
//	} while (curSize < mParamDescrFileSize && cntErrorConnection != maxAttempts);
//	if (cntErrorConnection == maxAttempts) {
//		std::ofstream tmp("/home/v7r/projects/0x18err.hex",
//				std::ios_base::binary);
//		tmp.write(fileBuffer.c_str(), fileBuffer.size());
//		tmp.close();
//		PRINTDEBUG("error")
//		return "";
//	}
//	std::ofstream tmp("/home/v7r/projects/0x18.hex", std::ios_base::binary);
//	tmp.write(fileBuffer.c_str(), fileBuffer.size());
//	tmp.close();
//	return fileBuffer;
//}
//
//std::vector<uint16_t> getPntPrm(const headOfLogFile_type &log_header) {
//	const int n_params = 9;
//	std::vector<uint16_t> res(n_params);
//	for (int i = 0; i < n_params; ++i) {
//		res[i] = log_header.pntPrm[i];
//	}
//	return res;
//}
//
//std::vector<uint32_t> getKAmp(const headOfLogFile_type &log_header) {
//	const int n_params = 9;
//	std::vector<uint32_t> res(n_params);
//	for (int i = 0; i < n_params; ++i) {
//		res[i] = log_header.kAmp[i];
//	}
//	return res;
//}
//
///**
// * @brief Return param from PDS file
// * @param fileBuffer - file buffer
// * @param pdsHeader - pointer to the PDS file header
// * @param param_mun - parameter number (absolute)
// * @return structure with parameter data
// */
//DescriptorPrm_type getParamDescr(headOfParamDescFile_type* pdsHeader,
//		uint16_t param_num) {
//	uint8_t id_gr = param_num >> 7;
//	uint8_t id_pr = param_num - (id_gr << 7);
//	DescriptorPrm_type* pds = (((DescriptorPrm_type *) ((size_t) (pdsHeader)
//			+ (size_t) ((pdsHeader->PrmStart << 1)))));
//
//	const size_t finish = (size_t) (pdsHeader)
//			+ (size_t) (pdsHeader->SizeOfFileDescr << 1);
//	while ((size_t) pds < (size_t) finish && pds->IDGrp != id_gr) {
//		++pds;
//	}
//	return *(pds + id_pr);
//}
///**
// * @brief Extract unit code from param
// * @param param
// * @return
// */
//uint16_t getUnit(const DescriptorPrm_type& param) {
//	return param.Unit;
//}
//
///**
// * @brief Convert units index to ansi name
// * @warning string is not human readable in *nix with utf8 locales
// * @param nameIdx
// * @return
// */
//const char* getUnitName(uint16_t nameIdx) {
//	return unitsName[nameIdx].c_str();
//}
//
///**
// * @brief extract Power muultiplyer (0..7)
// * @param param
// * @return
// */
//uint16_t getPower(const DescriptorPrm_type& param) {
//	return param.FlgPrm.Power;
//}
//
//void makeTdriveLogHeader(uint8_t *kn24log) {
//}
//
//void makeTdriveLogFile() {
//}

//using namespace bigbrother;
//
//int main() {
//	const int pdsHeaderSz = 1 << 7; // file size in first 128 bytes
//	ifstream pdsFile("/home/v7r/projects/0x01.hex");
//	uint8_t pdsHeadBuffer[pdsHeaderSz];
//	pdsFile.read((char*) pdsHeadBuffer, pdsHeaderSz);
//	pdsFile.clear();
//	pdsFile.seekg(0, ios::beg);
//	headOfParamDescFile_type *pHof = (headOfParamDescFile_type *) pdsHeadBuffer;
//	uint16_t pdsFileSize = (pHof->SizeOfFileDescr << 1) + pdsHeaderSz;
//	cerr << *pHof << flush << endl;
//
//	PRINTDEBUG2("pdsFileSize: ", pdsFileSize)
//	PRINTDEBUG2("from header: ", pHof->SizeOfFileDescr << 1)
//	PRINTDEBUG2("sizeof head pds struct: ", sizeof(headOfParamDescFile_type))
//	PRINTDEBUG2("sizeof descr param struct: ", sizeof(DescriptorPrm_type))
//	PRINTDEBUG2("prmSatrt: ", pHof->PrmStart)
//
//	std::unique_ptr<uint8_t> pdsFileBufferFull = std::unique_ptr<uint8_t>(
//			new uint8_t[pdsFileSize]);
//
//	pdsFile.read((char*) pdsFileBufferFull.get(), pdsFileSize);
//
//	headOfParamDescFile_type *pdsData =
//			(headOfParamDescFile_type *) pdsFileBufferFull.get();
//
//	void *Addr = (void *) ((DescriptorPrm_type *) ((size_t) pdsData
//			+ ((pdsData->PrmStart) << 1)));
//
////	DescriptorPrm_type *ppPrm = (DescriptorPrm_type *) (Addr);
////	for (int j = 0; j < 302; j++) {
////		cout << "=================" << j <<"======================" << endl;
////		cout << *ppPrm << endl;
////		ppPrm++;
////	}
//
//	/////////////////////// LOG ////////////////////////////////
//	const int logHeaderSize = 1 << 7;
//	//std::ifstream logFile("/home/v7r/projects/0x18.hex");
//	std::ifstream logFile("/home/v7r/projects/damp.bin");
//	if (!logFile.is_open()) {
//		cerr << "can't read duump" << endl;
//		return EXIT_FAILURE;
//	}
//	uint8_t logHeaderBuffer[logHeaderSize];
//	logFile.read((char*) logHeaderBuffer, logHeaderSize);
//	logFile.clear();
//	logFile.seekg(0, ios::beg);
//	headOfLogFile_type *pLog = (headOfLogFile_type *) logHeaderBuffer;
//	cout << *pLog << endl;
//	uint16_t logFileSize = ((pLog->Sz * pLog->Nvar) << 1) + logHeaderSize;
//	std::unique_ptr<uint8_t> logFileBufferFull = std::unique_ptr<uint8_t>(
//			new uint8_t[logFileSize]);
//	PRINTDEBUG2("log file buffer size: ", logFileSize)
//	logFile.read((char*) logFileBufferFull.get(), logFileSize);
//	pLog = (headOfLogFile_type *) logFileBufferFull.get();
//
//	std::vector<uint16_t> pntPrm = getPntPrm(*pLog);
//	std::vector<uint32_t> kAmp = getKAmp(*pLog);
//
//	////////////////////// TDRIVE LOg ///////////////////////////
//	TPrm par;
//	logFileSave_type log_tdrive;
//	headOfLogFileSave_type logSaveHead;
//	logSaveHead.CRC = 0;
//	logSaveHead.Ntic = pLog->Ntic;
//	logSaveHead.Nvar = pLog->Nvar;
//	logSaveHead.TimeOneTick = pLog->TimeOneTick;
//	logSaveHead.LogInd = pLog->LogInd;
//	logSaveHead.Sz = pLog->Sz;
//	logSaveHead.DeviceType = pHof->DeviceType;
//	logSaveHead.UnixTime = bigbrother::getUnixTimeStamp(); //UTC !!!
//	memcpy(logSaveHead.DeviceName, pHof->DeviceName, 32);
//	memcpy(logSaveHead.VendorName, pHof->VendorName, 32);
//	////////////////////// header   /////////////////////////////
//	for (int i = 0; i < pLog->Nvar; ++i) {
//		DescriptorPrm_type param = getParamDescr(pdsData, pntPrm[i]);
////		cerr << param << endl;
//		memcpy(par.Name, param.Name, 16);
//		memcpy(par.EdIzm, getUnitName(param.Unit), 16);
//		par.IDgrp = param.IDGrp;
//		par.IDPrm = param.IDPrm;
//		par.Power = getPower(param);
//		par.kAmp = kAmp[i];
//		logSaveHead.PrmLog[i] = par;
//	}
//	for (int i = pLog->Nvar; i < 20; ++i) {
//		logSaveHead.PrmLog[i] = TPrm();
//	}
//
//	const size_t logDataSz = logFileSize - logHeaderSize;
//	const size_t logSaveSz = sizeof(headOfLogFileSave_type) + logDataSz;
//
//	PRINTDEBUG2("logDataSz: ", logDataSz)
//	PRINTDEBUG2("logSaveSz: ", logSaveSz)
//
//	std::unique_ptr<uint8_t> saveLogFileBuffer = std::unique_ptr<uint8_t>(
//			new uint8_t[logSaveSz]);
//
//	memcpy((char*) saveLogFileBuffer.get(), (char*) &logSaveHead,
//			sizeof(headOfLogFileSave_type));
//
//	memcpy((char*) saveLogFileBuffer.get() + sizeof(headOfLogFileSave_type),
//			(char*) (logFileBufferFull.get() + logHeaderSize), logDataSz);
//
//	uint16_t temp_crc = bigbrother::CalcCRC16_tdrive(saveLogFileBuffer.get() + 2, logSaveSz - 2);
//
//	cout << hex << "crc1: " << temp_crc << endl;
//
//	memcpy(saveLogFileBuffer.get(), (char*) &temp_crc, 2);
//
//	ofstream logTdrive("/home/v7r/projects/logTdrive.log", std::ios::binary);
//	logTdrive.write((char*) saveLogFileBuffer.get(), logSaveSz);
//
//	return 0;
//}

/**
 * @mainpage  Модуль обмена данными модема МТ01 с преиферийным оборудованием
 * @note
 * @author    Инженер-программист: Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */
/////* Includes ---------------------------------------------------------------*/
//#include <iostream>
//#include <unistd.h>
//#include <signal.h>
//#include <modbus.h>
//#include <cstdlib>
//#include "globals.h"
//#include "modem.h"
//#include "EngineModbus.h"
//#include "device.h"
//#include "globals.h"
//#include "semaphors.h"
//#include <utilites.h>
//#include <dirent.h>
//
//using namespace std;
//
//V7Modem* modem = NULL;
//
///**
// * @fn SigintHandler
// * @brief Вызывается по сигналу SIGINT (Ctrl-C).
// * @details
// * @param signum (не используется)
// * @return
// */
//void SigintHandler(int signum) {
//	endWorkFlag = true;
//}
//
//
//
//
//    /*====================================================================================================*/
//    /* Program to access the RTS,of a Serial Port(ttyS* or ttyUSB0) using ioctl() system call.            */
//    /*                                                                                                    */
//    /*====================================================================================================*/
//    /*                                                                                                    */
//    /* If your are using FT232 based USB to Serial Converter all the pins are inverted internally         */
//    /* Modem Pins of FT232 based Serial Port -> ~RTS,,~DTR.                                               */
//    /* SETTING the Pins will make it LOW,while CLEARING the Pins will make it HIGH.                       */
//    /*----------------------------------------------------------------------------------------------------*/
//
//    /*----------------------------------------------------------------------------------------------------*/
//    /* Compiler/IDE  : gcc 4.6.3                                                                          */
//    /* Library       :                                                                                    */
//    /* Commands      : gcc -o rts rts.c                                                                   */
//    /* OS            : Linux(x86) (Linux Mint 13 Maya/Ubuntu 14.04)(Linux Kernel 3.x.x)                  */
//    /* Programmer    : Rahul.S                                                                            */
//    /* Date          : 27-December-2014                                                                   */
//    /*====================================================================================================*/
//
//    /*====================================================================================================*/
//    /* Running the executable                                                                             */
//    /* ---------------------------------------------------------------------------------------------------*/
//    /* 1) Compile the  serialport_read.c  file using gcc on the terminal (without quotes)                 */
//        /*                                                                                                    */
//    /*  " gcc -o rts rts.c  "                                                                         */
//    /*                                                                                                    */
//        /* 2) Linux will not allow you to access the serial port from user space,you have to be root.So use   */
//        /*    "sudo" command to execute the compiled binary as super user.                                    */
//        /*                                                                                                    */
//        /*       "sudo ./rts"                                                                   */
//    /*                                                                                                    */
//    /*====================================================================================================*/
//
//    /*====================================================================================================*/
//    /* Sellecting the Serial port Number on Linux                                                         */
//    /* ---------------------------------------------------------------------------------------------------*/
//    /* /dev/ttyUSBx - when using USB to Serial Converter, where x can be 0,1,2...etc                      */
//    /* /dev/ttySx   - for PC hardware based Serial ports, where x can be 0,1,2...etc                      */
//        /*====================================================================================================*/
//
//    /*==================================*/
//    /*       MODEM bits                 */
//    /*------------------- --------------*/
//    /*   TIOCM_DTR --- DTR Line         */
//    /*   TIOCM_RTS --- RTS Line         */
//    /*==================================*/
//
//    #include <stdio.h>
//        #include <fcntl.h>       /* File Control Definitions           */
//        #include <termios.h>     /* POSIX Terminal Control Definitions */
//        #include <unistd.h>      /* UNIX Standard Definitions          */
//        #include <errno.h>       /* ERROR Number Definitions           */
//    #include <sys/ioctl.h>   /* ioctl()                            */
//        int main(void)
//        {
//            int fd; /*File Descriptor*/
//        printf("\n  +--------------------------------------------+");
//        printf("\n  | Program to Control RTS pins of Serial Port |");
//        printf("\n  +--------------------------------------------+");
//
//        /*------------------ Opening the Serial port ------------------*/
//
//        /* Change /dev/ttyUSB0 to the one corresponding to your system */
//
//            fd = open("/dev/ttyAPP0",O_RDWR | O_NOCTTY );           /* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
//                                    /* O_RDWR Read/Write access to serial port           */
//                                    /* O_NOCTTY - No terminal will control the process   */
//                                    /* Blocking Mode  */
//
//            if(fd == -1)                        /* Error Checking */
//                   printf("\n    Error! in Opening ttyAPP0  ");
//            else
//                   printf("\n    ttyAPP0 Opened Successfully \n");
//
//
//        /*--------- Controlling the RTS pins  of Serial Port --------*/
//
//        int RTS_flag;
//
//        RTS_flag = TIOCM_RTS;   /* Modem Constant for RTS pin */
//
//
//        /*--------------------- Controlling the RTS pin using ioctl() system call ---------------------*/
//
//        /* setting RTS = 1,~RTS = 0 */
//
//        ioctl(fd,TIOCMBIS,&RTS_flag);       /* fd -file descriptor pointing to the opened Serial port */
//                                /* TIOCMBIS - Set the bit corrosponding to  RTS_flag      */
//        printf("\n    Setting RTS = 1,~RTS = 0 ");
//        printf("\n\n    Press any Key...");
//        getchar();
//
//        /* setting RTS = 0,~RTS = 1 */
//
//        ioctl(fd,TIOCMBIC,&RTS_flag);        /* fd -file descriptor pointing to the opened Serial port */
//                                 /* TIOCMBIC - Clear the bit corrosponding to  RTS_flag    */
//        printf("\n    Setting RTS = 0,~RTS = 1 ");
//        printf("\n\n    Press any Key...");
//        getchar();
//
//        close(fd); /* Close the Opened Serial Port */
//        printf("\n  +--------------------------------------------+\n\n");
//    }
//#include <iostream>
//#include <unistd.h>
//#include <signal.h>
//#include <modbus.h>
//#include <cstdlib>
//#include "globals.h"
//#include "modem.h"
//
//void print_errno()
//{
//    //!Анализируем ошибку
//    switch (errno) {
//    case EMBXILFUN: //Illegal function
//        cerr << 10 << endl;
//        break;
//    case EMBXILADD: //Illegal data address
//        cerr << 11 << endl;
//        break;
//    case EMBXSFAIL: //Slave device or server failure
//    case 110:       //Connection timed out
//        cerr << 12 << endl;
//        break;
//    case EMBXSBUSY: //Slave device or server is busy
//        cerr << 13 << endl;
//        break;
//    case EMBXGTAR: //Target device failed to respond
//    case EMBBADCRC: //Invalid CRC
//    case EMBBADDATA: //Invalid data
//    case EMBBADEXC: //Invalid exception code
//        cerr << 14 << endl;
//        break;
//    default:
//
//        break;
//    }
//}
//int main()
//{
//    modbus_t * mServer = modbus_new_tcp("10.0.0.5", 502);
//    int16_t rc = -1;
//    while (rc == -1) {
//        PRINTDEBUG("Client. Try to connect to Modbus Server...")
//        rc = modbus_connect(mServer);
//    }
//
//    PRINTDEBUG("Client init.")
//    uint16_t buf[120];
//    int mbRes = modbus_read_input_registers(mServer, 11392, 3, buf);
//    cerr << mbRes << endl;
//    if (mbRes == -1) {
//        print_errno();
//    }
//
//    cout << (int) buf[0] << ":" << (int) buf[1] << ":" << (int) buf[2] << ":"
//            << (int) buf[1] << endl;
//    uint8_t req[] = { 0, 0x03, 89, 0, 0, 3 };
//    int rez = modbus_send_raw_request(mServer, req, 6);
//    uint8_t rsp[255];
//    rez = modbus_receive_confirmation(mServer, rsp);
//    cerr << rez << endl;
//    for (int i = 0; i < rez; ++i) {
//        cout << (int) rsp[i] << " : ";
//    }
//    cout << endl;
//    cout <<" ok "<< endl;
//    return 0;
//}
//#include <iostream>
//#include <unistd.h>
//#include <signal.h>
//#include <modbus.h>
//#include <cstdlib>
//#include "globals.h"
//#include "modem.h"
//#include "EngineModbus.h"
//#include "device.h"
//#include "globals.h"
//#include "semaphors.h"
//#include <utilites.h>
//#include <dirent.h>
//
//std::set<std::string> getDirList(const std::string& dirName) {
//	struct dirent *de;
//	DIR *dr = opendir(dirName.c_str());
//	std::set<std::string> ls;
//	if (dr == NULL) {
//		std::cerr << "[debug] " << dirName << " is empty." << std::endl;
//		return ls;
//	}
//	while ((de = readdir(dr)) != NULL)
//		ls.insert(de->d_name);
//	return ls;
//}
//
//int main(){
//	std::set<std::string> mls;
//	mls = bigbrother::getDirList("/tmp/MT01/dyn/");
//	std::cout << *(mls.begin()) << std::endl;
//	return 0;
//}
/*
 int requestPort(QueryMsg * msg, EngineModbusRTU *mpModbusEngine)
 {

 //flagBusy = true;
 const useconds_t pauseInUs = 300;
 int exitCode(0); //нет ошибки
 // подключение к движку
 if (!mpModbusEngine->getModbusContext())
 mpModbusEngine->open();
 // если подключились
 if (mpModbusEngine->connect()) {
 PRINTDEBUG2("\n[Port::request()] Modbus ctx is: ",
 mpModbusEngine->getModbusContext());
 msg->makeRawRequest();
 int retCode = mpModbusEngine->sendRawRequest(msg->getRequestBuffer(),
 msg->getRequestSize());
 cout << "reqtCode" << (retCode);
 uint8_t reqFunc = msg->getRequestFunction();
 cout << "reqFunc" << (int)reqFunc<<endl;;
 //int addr = msg->getRequestBuffer()[0];

 if (retCode != 0) {
 // error & exit
 PRINTDEBUG("[Port::request()] error & exit");
 usleep(pauseInUs);
 msg->setErrorCode(retCode);
 }
 else {

 uint8_t tmp[MODBUS_TCP_MAX_ADU_LENGTH];
 uint8_t* pTmp = tmp;
 int tmpSize(0);
 retCode = mpModbusEngine->receiveRawConfirmation(&pTmp, tmpSize);

 if (retCode != 0) {
 cout << "Error!!!!!!\n";
 msg->setErrorCode(retCode); // exit
 }
 else {
 engine_backend_type mBackendType = RTU;
 if (mBackendType == RTU) {
 if (reqFunc != tmp[1]) {
 msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[2]));
 }
 else {

 msg->setErrorCode(retCode);
 msg->setResponseBuffer(tmp, tmpSize);
 }

 }
 else if (mBackendType == TCP) {

 if (reqFunc != tmp[7]) {
 msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[8]));
 }
 else {
 msg->setErrorCode(retCode);
 msg->setResponseBuffer(tmp + 6, tmpSize); ///
 }
 }

 }

 }
 //        PRINTDEBUG2("\n[Port::request()] Request result:\n", *msg);
 exitCode = msg->getErrorCode();
 mpModbusEngine->close();
 }
 else {
 exitCode = 0x04; // slave device failure
 }
 // flagBusy = false;
 return exitCode;
 }


 int main()
 {
 RtuPortSettings *mbrtu = new RtuPortSettings();
 mbrtu->baud = 115200;
 mbrtu->dataBit = 8;
 mbrtu->parity = 'N';
 mbrtu->portName = "/dev/ttyUSB0";
 mbrtu->stopBit = 1;
 mbrtu->modbusBackend = RTU;

 EngineModbusRTU *emb = new EngineModbusRTU();
 emb->init(mbrtu);
 emb->open();

 QueryMsg0x2b *qm2b = new QueryMsg0x2b();
 qm2b->mbDevAddr = 0x01;
 //qm2b->mbReadDeviceIDcode = 0x00;
 qm2b->mbObjectId = 0x00;
 qm2b->makeRawRequest();
 requestPort(qm2b, emb);
 cout << *qm2b << endl;
 cout << dec << qm2b->getUMKA27Id()<< endl;
 sleep(1000);
 return 0;
 }
 */
//
//#include <iostream>
//#include <iomanip>
//#include <string.h>
//#include <errno.h>
//#include <modbus.h>
//#include <unistd.h>
//#include <time.h>
//
//using namespace std;
//
//#define BUFFER_LENGTH 260
//
//void print_buffer(uint8_t *buffer, int length)
//{
//    for (int i = 0; i < 40; ++i) {
//
//        cerr << setw(2) << setfill('0') << hex << (int) (buffer[i]) << " ";
//    }
//    cerr << dec << endl;
//}
//
//int main()
//{
//    uint8_t bufferSCADA[BUFFER_LENGTH] = { 0 };
//    uint8_t bufferDevice[BUFFER_LENGTH] = { 0 };
//    int length = 0;
//    uint16_t func = 0;
//    modbus_t *scada = NULL;
//    modbus_t *device = NULL;
//#ifdef DEBUG
//    scada = modbus_new_rtu("/dev/ttyUSB2", 38400, 'N', 8, 1);
//    device = modbus_new_rtu("/dev/ttyUSB3", 38400, 'N', 8, 1);
//#else
//    scada = modbus_new_rtu("/dev/ttyAPP0", 38400, 'N', 8, 1);
//    //device = modbus_new_rtu("/dev/ttyAPP3", 38400, 'N', 8, 1);
//    device = modbus_new_rtu("/dev/ttyUSB0", 38400, 'N', 8, 1);
//#endif
//    if (!scada) {
//        cerr << "Scada not created" << endl;
//     //   return -1;
//    }
//    if (!device) {
//        cerr << "Scada not created" << endl;
//      //  return -1;
//    }
//    int byteTimeout = 500;
//    int respTimeout = 4500;
//    struct timeval to;
//    to.tv_sec = byteTimeout / 1000;
//    to.tv_usec = (byteTimeout % 1000) * 1000;
//    modbus_set_byte_timeout(scada, &to);
//    modbus_set_byte_timeout(device, &to);
//
//    to.tv_sec = respTimeout / 1000;
//    to.tv_usec = (respTimeout % 1000) * 1000;
//    modbus_set_response_timeout(scada, &to);
//    modbus_set_response_timeout(device, &to);
//
//    modbus_set_debug(scada, TRUE);
//    modbus_set_debug(device, TRUE);
//    if (modbus_connect(scada) == -1) {
//        cerr << "scada: Unable to connect " << strerror(errno) << endl;
//       // return false;
//    }
//    else {
//        clog << "scada ok" << endl;
//    }
//    if (modbus_connect(device) == -1) {
//        cerr << "device: Unable to connect " << strerror(errno) << endl;
//       // return false;
//    }
//
//    else {
//        clog << "device ok" << endl;
//    }
//
//    while (1) {
//        //Вычитываем данные
//        length = modbus_receive(scada, bufferSCADA);
//
//        cerr << "\nScada received buffer:" << endl;
//        print_buffer(bufferSCADA, BUFFER_LENGTH);
//        func = bufferSCADA[1];
//
//        if (length <= 0) {
//            continue; // данные не получали
//        }
//        //Открывааем порт назначения
//        if (modbus_connect(device) == -1) {
//            cerr << "Unable to connect " << strerror(errno) << endl;
//           // break;
//        }
//
//        if (bufferSCADA[0] == 0x00 && func == 0x2b) {
//            bufferSCADA[0] = 1; // первое устройство
//        }
//        //modbus_set_slave(mpDevicePool.find(bufferSCADA[0])->second, mpDevicePool.find(bufferSCADA[0])->first);
//
//        print_buffer(bufferSCADA, BUFFER_LENGTH);
//        if (modbus_send_raw_request(device, bufferSCADA, length - 2) == -1) {
//            /* Connection closed by the client or error */
//            cerr << "[Scada]: Connection closed by the client or error " << strerror(errno) << endl;
//            modbus_close(device);
//            //break;
//        }
//        else {
//            cerr << "\nLength after raw request to device:" << length << endl;
//        }
//        //Вычитываем данные
//        length = modbus_receive_confirmation(device, bufferDevice);
//
//        if (length <= 0) {
//            cerr << endl << "Length after receive confirmation from Device:" << length << " data: ";
//            print_buffer(bufferDevice, BUFFER_LENGTH);
//            modbus_close(device);
//            continue;
//        }
//        else {
//            cerr << "\nLength after receive confirmation from Device: " << length << endl;
//        }
//        // unlock device port
////        if (func != 0x14 || func != 0x15) {
//        modbus_close(device);
//        usleep(1000);
//        //вписываем данные в порт АСУ
//        //length-2 - срезаем crc, libmodbus сам подставит crc.
//        if (modbus_send_raw_request(scada, bufferDevice, length - 2) == -1) {
//            /* Connection closed by the client or error */
//            cerr << "Connection closed by the client or error " << strerror(errno) << endl;
//          //  break;
//        }
//
//        cerr << "[Scada::Run] buffer scada: " << endl;
//        print_buffer(bufferSCADA, BUFFER_LENGTH);
//        cerr << "[Scada::Run] buffer device: " << endl;
//        print_buffer(bufferDevice, BUFFER_LENGTH);
//
//        memset(bufferSCADA, 0, BUFFER_LENGTH);
//        memset(bufferDevice, 0, BUFFER_LENGTH);
//
//    }
//
//}
