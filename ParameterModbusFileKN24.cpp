/**
 * @file ParameterModbusFileKN24.cpp
 * @date Created on: 8 жовт. 2018 р.
 * @author Author: v7r
 */

#include "ParameterModbusFileKN24.h"
#include "devicemodbus.h"

ParameterModbusFileKN24::ParameterModbusFileKN24() :
		mPdsFileNum { 0x01 }, mCrashLogFileNum { 0x18 }, mIsCrashLogRead { false }, mIsParamDescrFileRead {
				false }, mCrashLogFileSize { 0 }, mParamDescrFileSize { 0 }, mIsCrashNow {
				false } {

	bigbrother::mkpath_p(TMP_DIR_KN24, 0755); //make tree inj temp
	mParamType = paramType_type::p_crash_kn24; //set type
	std::clog
			<< "<<<<<<<<<<<<<<<  KN24 Без ТЗ - результат ХЗ (с) Назаренко   >>>>>>>"
			<< std::endl;
#ifdef DEBUG
	const string rm_cmd = string("rm ") + PDS_FILE_PATH;
	system (rm_cmd.c_str());
#endif
}

ParameterModbusFileKN24::~ParameterModbusFileKN24() {

}

std::string ParameterModbusFileKN24::readFile0x01() {

	if (!mParamDescrFileSize) {
		setParamDescHeaderData();
		mParamDescrFileSize = getParamDescrFileSize() >> 1;
		if (!mParamDescrFileSize) {
			return "";
		}
	}
	const uint8_t devAddr =
			static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
	const int8_t maxAttempts = 3;
	const uint8_t fileNumber = 0x01;
	const uint8_t shift_to_data = 8;
	int8_t cntErrorConnection(0);
	std::string fileBuffer = "";
	int32_t curSize(0);
	do {

		uint8_t recCnt =
				curSize + MAX_REGISTER_COUNT < mParamDescrFileSize ?
						static_cast<uint8_t>(MAX_REGISTER_COUNT) :
						mParamDescrFileSize - curSize;
		QueryMsgJrnAT27 msg = request0x68(fileNumber, devAddr, curSize, recCnt);
		waitForFreePort();
		int mbRes = mpPort->request(&msg);
		if (mbRes == 0) {
			uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
			uint8_t rspSize = msg.getResponseBuffer()[7] << 1;
			memcpy(rspBuf, msg.getResponseBuffer() + shift_to_data, rspSize);
			fileBuffer.append(rspBuf, rspBuf + rspSize);

		} else {
			++cntErrorConnection;
		}
		curSize = fileBuffer.length() / 2;
	} while (curSize < mParamDescrFileSize && cntErrorConnection != maxAttempts);
	if (cntErrorConnection == maxAttempts) {
		PRINTDEBUG("error")
		return "";
	}
	std::ofstream tmp(PDS_FILE_PATH, std::ios_base::binary);
	if (tmp.is_open()) {
		tmp.write(fileBuffer.c_str(), fileBuffer.size());
		tmp.close();
	}

	return fileBuffer;
}

std::string ParameterModbusFileKN24::readFile0x18() {

	if (!mCrashLogFileSize) {
		setCrashLogHeaderData();

		mCrashLogFileSize = getCrashLogFileSize() >> 1;
		PRINTDEBUG2("mCrashLogFileSize:  ", mCrashLogFileSize)
		if (!mCrashLogFileSize) {
			return "";
		}
	}
	const int8_t maxAttempts = 3;
	const uint8_t fileNumber = 0x18;
	const uint8_t shift_to_data = 8;
	int8_t cntErrorConnection(0);
	std::string fileBuffer = "";
	int32_t curSize(0);
	do {

		uint8_t recCnt =
				curSize + MAX_REGISTER_COUNT < mCrashLogFileSize ?
						static_cast<uint8_t>(MAX_REGISTER_COUNT) :
						mCrashLogFileSize - curSize;
		QueryMsgJrnAT27 msg = request0x68(fileNumber,
				static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress()),
				curSize, recCnt);
		waitForFreePort();
		int mbRes = mpPort->request(&msg);
		if (mbRes == 0) {
			uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
			uint8_t rspSize = msg.getResponseBuffer()[7] << 1;
			memcpy(rspBuf, msg.getResponseBuffer() + shift_to_data, rspSize);
			bigbrother::swapBytesInWordsInBuffer(rspBuf, rspSize); //!< @warning device and !!!file!!! in triol device specific
			fileBuffer.append(rspBuf, rspBuf + rspSize);

		} else {
			++cntErrorConnection;
		}
		curSize = fileBuffer.length() / 2;
	} while (curSize < mCrashLogFileSize && cntErrorConnection != maxAttempts);
	if (cntErrorConnection == maxAttempts) {
		PRINTDEBUG("readFile0x18() : error")
		return "";
	}
	std::ofstream tmp(KN24_CRASH_FILE, std::ios_base::binary);
	if (tmp.is_open()) {
		tmp.write(fileBuffer.c_str(), fileBuffer.size());
		tmp.flush();
		tmp.close();
	}
	return fileBuffer;
}

void ParameterModbusFileKN24::Run() {
	PRINTDEBUG("ParameterModbusFileKN24::Run()")
	if (mPdsFileBuffer.empty()) {
		if (bigbrother::isFileExist(PDS_FILE_PATH)) {
			int64_t bufSize = bigbrother::getFileSize(PDS_FILE_PATH);
//			PRINTDEBUG2("file size 1: ", bufSize)
//			PRINTDEBUG2("Buffer size 1: ", mPdsFileBuffer.size())
			std::ifstream ifile(PDS_FILE_PATH, std::ios_base::binary);
			if (!ifile.is_open()) {
				mPdsFileBuffer = readFile0x01();
				return;
			}
			std::unique_ptr<uint8_t> fileBuffer(new uint8_t[bufSize]);
			ifile.read((char*) fileBuffer.get(), bufSize);

			mPdsFileBuffer.assign(fileBuffer.get(), fileBuffer.get() + bufSize);
//			PRINTDEBUG2("Buffer size 2: ", mPdsFileBuffer.size())
		} else {
			mPdsFileBuffer = readFile0x01();
		}
	}

	if (mIsCrashNow) {
		mCrashLogBuffer = readFile0x18();
		mIsCrashNow = false;
		bool res = makeTdriveFormatLogFile();
	}
//	PRINTDEBUG("makeTdriveFormatLogFile EXIT")
}

void ParameterModbusFileKN24::setCrashHandleProcess() {
	PRINTDEBUG("TRUE")
	mIsCrashNow = true;
}

uint32_t ParameterModbusFileKN24::getCrashLogRegAddress() const {
	const headOfParamDescFile_type* pdsHeader =
			(headOfParamDescFile_type*) mPdsFileBuffer.data();
	PRINTDEBUG2(">>>>>>>>>  pdsHeader->SpecialPrm:   ", pdsHeader->SpecialPrm)
	PRINTDEBUG2(">>>>>>>>>  pdsHeader->AdrTDrivePrm: ", pdsHeader->AdrTDrivePrm)
	PRINTDEBUG2(">>>>>>>>>  pdsHeader->regAddr     : ",
			((TSpezPrm* )(mPdsFileBuffer.data() + pdsHeader->SpecialPrm * 2))->AdrStatLog)
	PRINTDEBUG2(">>>>>>>>>  pdsHeader->BitMask: ",
			((TAdrTDrivePrm* )((uint8_t* )mPdsFileBuffer.data()
					+ (pdsHeader->AdrTDrivePrm << 1)))->KodBeginEndLog)

//PRINTDEBUG2(">>>>>>>>>> CRASH LOG AdDR:          ", *(mPdsFileBuffer.data() + pdsHeader->SpecialPrm << 1  + 15 * 2))
	return ((TSpezPrm*) (mPdsFileBuffer.data() + pdsHeader->SpecialPrm * 2))->AdrStatLog;
}

uint16_t ParameterModbusFileKN24::getCrashLogMask() const {
	const headOfParamDescFile_type* pdsHeader =
			(headOfParamDescFile_type*) mPdsFileBuffer.data();
	return (((TAdrTDrivePrm*) ((uint8_t*) mPdsFileBuffer.data()
			+ (pdsHeader->AdrTDrivePrm << 1)))->KodBeginEndLog);
}

QueryMsgJrnAT27 ParameterModbusFileKN24::request0x68(uint8_t fileNumber,
		uint8_t deviceAddress, uint32_t recAddress, uint8_t recCnt) {
	return QueryMsgJrnAT27(deviceAddress, 0x68, fileNumber,
			(uint8_t) (recAddress >> 24), (uint8_t) (recAddress >> 16),
			(uint8_t) (recAddress >> 8), (uint8_t) recAddress, recCnt);
}

bool ParameterModbusFileKN24::setCrashLogHeaderData() {

	const uint8_t devAddr =
			static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());

	QueryMsgJrnAT27 msg = request0x68(mCrashLogFileNum, devAddr, 0,
			mCrashLogHeaderSize / 2);
	waitForFreePort();
	int mbRes = mpPort->request(&msg);
	const uint8_t shift_to_data = 8;
	if (mbRes == 0) {
		//PRINTDEBUG2("mCrashLogHeaderSize in q", mCrashLogHeaderSize)
		memcpy(reinterpret_cast<uint8_t*>(&mCrashLogHeader),
				msg.getDataFromResponse() + shift_to_data, mCrashLogHeaderSize);
		bigbrother::swapBytesInWordsInBuffer(
				reinterpret_cast<uint8_t*>(&mCrashLogHeader),
				mCrashLogHeaderSize);
	} else {
		PRINTDEBUG("Error read file")
		return false;
	}
	return true;

}

bool ParameterModbusFileKN24::setParamDescHeaderData() {
	const uint8_t devAddr =
			static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress());
	QueryMsgJrnAT27 msg = request0x68(mPdsFileNum, devAddr, 0, 64);
	waitForFreePort();
	int mbRes = mpPort->request(&msg);
	const uint8_t shift_to_data = 8;
	if (mbRes == 0) {
		memcpy(reinterpret_cast<uint8_t*>(&mParamDescFileHeader),
				msg.getDataFromResponse() + shift_to_data, mPdsHeaderSize);

	} else {
		PRINTDEBUG("Error read file")
		return false;
	}
	return true;
}

int32_t ParameterModbusFileKN24::getParamDescrFileSize() {
	return (mParamDescFileHeader.SizeOfFileDescr << 1) + mPdsHeaderSize;
}

int32_t ParameterModbusFileKN24::getCrashLogFileSize() {
	return (mCrashLogHeader.Nvar * mCrashLogHeader.Sz << 1)
			+ mCrashLogHeaderSize;
}

DescriptorPrm_type ParameterModbusFileKN24::getParamDescr(
		headOfParamDescFile_type* pdsHeader, uint16_t param_num) {
// see protocol
	uint8_t id_gr = param_num >> 7;
	uint8_t id_pr = param_num - (id_gr << 7);
	DescriptorPrm_type* pds = (((DescriptorPrm_type *) ((size_t) (pdsHeader)
			+ (size_t) ((pdsHeader->PrmStart << 1)))));

	const size_t finish = (size_t) (pdsHeader)
			+ (size_t) (pdsHeader->SizeOfFileDescr << 1);
	while ((size_t) pds < (size_t) finish && pds->IDGrp != id_gr) {
		++pds;
	}
	return *(pds + id_pr);
}

uint16_t ParameterModbusFileKN24::getUnit(const DescriptorPrm_type& param) {
	return param.Unit;
}

const char* ParameterModbusFileKN24::getUnitName(uint16_t nameIdx) {
	return kn24::units_ansi_name[nameIdx].c_str();
}

uint16_t ParameterModbusFileKN24::getPower(const DescriptorPrm_type& param) {
	return param.FlgPrm.Power;
}

std::vector<uint16_t> ParameterModbusFileKN24::getPntPrm(
		const headOfLogFile_type &log_header) {
	const int n_params = sizeof(log_header.pntPrm) / sizeof(*log_header.pntPrm);
	std::vector<uint16_t> res(n_params);
	for (int8_t i = 0; i < n_params; ++i) {
		res[i] = log_header.pntPrm[i];
	}
	return res;
}

std::vector<uint32_t> ParameterModbusFileKN24::getKAmp(
		const headOfLogFile_type &log_header) {
	const int n_params = sizeof(log_header.kAmp) / sizeof(*log_header.kAmp);
	std::vector<uint32_t> res(n_params);
	for (int8_t i = 0; i < n_params; ++i) {
		res[i] = log_header.kAmp[i];
	}
	return res;
}

bool ParameterModbusFileKN24::makeTdriveFormatLogFile() {
//PRINTDEBUG("i AM HERE   1")
	if (mCrashLogBuffer.empty() || mPdsFileBuffer.empty()) {
		PRINTDEBUG("makeTdriveFormatLogFile(): File buffers is empty.")
		return false;
	}
	headOfLogFile_type *pLog = (headOfLogFile_type *) mCrashLogBuffer.data();
	headOfParamDescFile_type *pHof =
			(headOfParamDescFile_type *) mPdsFileBuffer.data();
	if (pLog == nullptr || pHof == nullptr) {
		PRINTDEBUG("makeTdriveFormatLogFile(): Error, ")
		return false;
	}
//	PRINTDEBUG("i AM HERE   2")
	logFileSave_type log_tdrive;
	headOfLogFileSave_type logSaveHead;
	logSaveHead.CRC = 0;
	logSaveHead.Ntic = pLog->Ntic;
	logSaveHead.Nvar = pLog->Nvar;
	logSaveHead.TimeOneTick = pLog->TimeOneTick;
	logSaveHead.LogInd = pLog->LogInd;
	logSaveHead.Sz = pLog->Sz;
	logSaveHead.DeviceType = pHof->DeviceType;
	logSaveHead.UnixTime = bigbrother::getUnixTimeStamp(); //UTC !!!
	memcpy(logSaveHead.DeviceName, pHof->DeviceName, sizeof(pHof->DeviceName));
	memcpy(logSaveHead.VendorName, pHof->VendorName, sizeof(pHof->VendorName));
	std::vector<uint16_t> pntPrm = getPntPrm(*pLog);
	std::vector<uint32_t> kAmp = getKAmp(*pLog);
//	PRINTDEBUG("i AM HERE   3")
	for (int i = 0; i < pLog->Nvar; ++i) {
		TPrm par;
		DescriptorPrm_type param = getParamDescr(pHof, pntPrm[i]);
//		cout << param << endl;
		memcpy(par.Name, param.Name, sizeof(param.Name));
		memcpy(par.EdIzm, getUnitName(param.Unit), sizeof(param.Unit));
		par.IDgrp = param.IDGrp;
		par.IDPrm = param.IDPrm;
		par.Power = getPower(param);
		par.kAmp = kAmp[i];
		logSaveHead.PrmLog[i] = par;
//		PRINTDEBUG("i AM HERE   4")
	}

	const size_t logDataSz = (pLog->Nvar * pLog->Sz) << 1;
	const size_t logSaveSz = sizeof(headOfLogFileSave_type) + logDataSz;

//	PRINTDEBUG2("makeTdriveFormatLogFile(): logDataSz: ", logDataSz)
//	PRINTDEBUG2("makeTdriveFormatLogFile(): logSaveSz: ", logSaveSz)

	std::unique_ptr<uint8_t> saveLogFileBuffer(new uint8_t[logSaveSz]);

	memcpy(saveLogFileBuffer.get(), reinterpret_cast<const char*>(&logSaveHead),
			sizeof(headOfLogFileSave_type));

	memcpy(saveLogFileBuffer.get() + sizeof(headOfLogFileSave_type),
			mCrashLogBuffer.data() + sizeof(headOfLogFile_type), logDataSz);
	const uint8_t crc_size = 2;
	uint16_t temp_crc = bigbrother::CalcCRC16_tdrive(
			saveLogFileBuffer.get() + crc_size, logSaveSz - crc_size);

	memcpy(saveLogFileBuffer.get(), reinterpret_cast<char*>(&temp_crc), 2);

	const std::string fname = string(TMP_DIR_KN24) + "/"
			+ to_string(bigbrother::getUnixTimeStamp()) + "_"
			+ to_string(getGlobalServerId());

	ofstream logTdrive(fname, std::ios::binary);

	if (logTdrive.is_open()) {
		logTdrive.write((char*) saveLogFileBuffer.get(), logSaveSz);
		logTdrive.flush();
		logTdrive.close();
		PRINTDEBUG("makeTdriveFormatLogFile(): success")
//		sleep(5); // wait for data process sync
	} else {
		PRINTDEBUG("makeTdriveFormatLogFile(): Error while open file")
		return false;
	}
	return true;
}

bool ParameterModbusFileKN24::getCrashHandleResult() const {
	return mIsCrashNow ? false : true;
}

