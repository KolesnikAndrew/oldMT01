/**
 * @file      parametermodbusjournal.cpp
 * @brief     Определение функций класса Modbus-параметра
 * @details
 * @note
 * @author    Инженер-программист Зозуля Артем
 * @copyright © TRIOLCORP, 2016
 */

#include <modbus.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <parametermodbusfile.h>
#include "globals.h"
#include "port.h"
#include "modem.h"
#include "devicemodbus.h"
#include "sys/stat.h"
#include <iostream>

using namespace std;

ParameterModbusFile::ParameterModbusFile() :
		mAddressInJournal(0), mFileSize(0), mDevConfigNumber(0), mProcessState(
				fileStatus_type::rw_error), mFileNumber(0), mRequestCount(1), mFlagReadingJrn(
				false), mpUM03utl(nullptr), mFileClassType(
				umkaFileClass_type::undefined), mIsNeedDeleteFile(false), mIsNeedStopReadFile(
				false), mIsNeedWriteFile(false), mIsNeedReadFile(false) {
	mParamType = paramType_type::p_file;
}

ParameterModbusFile::~ParameterModbusFile() {

}

bool ParameterModbusFile::initParam(const xmlpp::Node* pXMLNode,
		V7Device* pDevice) {

	mpDevice = pDevice;
	if (!mpDevice) {
		return false;
	}
	mpDeviceModbus = dynamic_cast<V7DeviceModbus*>(mpDevice);
	mpPort = mpDevice->getPort();
	if (!mpPort) {
		return false;
	}
	mpModem = mpPort->GetModem();
	if (!mpModem)
		return false;
	mpServer = mpModem->getServerPtr();
	const xmlpp::Element* paramElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!paramElement) {
		cerr << "Element \"ParameterModbusFile\" not found" << endl;
		return false;
	}

	const int line = paramElement->get_line();

	//!Забираем атрибуты
	const int8_t paramsCount = 9;
	const xmlpp::Attribute* attributes[paramsCount];
	const Glib::ustring attrNames[paramsCount] = { "serverID", "name", "number",
			"file_number", "functions", "file_size", "file_extension",
			"requestCount", "journal_type" };
	Glib::ustring attrValues[paramsCount];
	for (int i = 0; i < paramsCount; ++i) {
		attributes[i] = paramElement->get_attribute(attrNames[i]);
		if (!attributes[i] && i != 8) {
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i]) {
			attrValues[i] = attributes[i]->get_value();
		}
	}

	//!Преобразуем атрибуты
	// serverID -----------------------------------------------------------------------------------
	istringstream(attrValues[0]) >> mServerID;
	//name ---------------------------------------------------------------------------------------
	mName = attrValues[1];
	if (mName.empty()) {
		cerr << "Error in the value of the attribute \"file_number\" (line="
				<< line << ") " << "Default value will be used." << endl;
		return false;
	}
	//number -------------------------------------------------------------------------------------
	istringstream(attrValues[2]) >> mNumber;
	if (mNumber == 0) {
		cerr << "Error in the value of the attribute \"file_number\" (line="
				<< line << ") " << "Default value will be used." << endl;
		mNumber = 1000;
	}
	//file_number -------------------------------------------------------------------------------------
	istringstream(attrValues[3]) >> mFileNumber;
	if (mFileNumber == 0) {
		cerr << "Error in the value of the attribute \"file_number\" (line="
				<< line << ") " << "Default value will be used." << endl;
		mFileNumber = 247;
	}
	//mbFunctions ----------------------------------------------------------------------------------
	istringstream tmpStream(attrValues[4]);
	do {
		unsigned int function = 0;
		tmpStream >> function;
		if (function > 0)
			mvModbusFunctions.push_back(function);
		else
			break;
	} while (1);
	if (mvModbusFunctions.empty()) {
		cerr << "Error in the value of the attribute \"file_number\" (line="
				<< line << ") " << "Default value will be used." << endl;
		mvModbusFunctions.push_back((0x03));
	}
	//file_size -------------------------------------------------------------------------------------
	istringstream(attrValues[5]) >> mFileSize;
	if (mFileSize == 0) {
		cerr << "Error in the value of the attribute \"file_number\" (line="
				<< line << ") " << "Default value will be used." << endl;
		mFileSize = 1;
	}
	//file_extension -------------------------------------------------------------------------------------
	mFileExtension = attrValues[6];
	const std::string fileClassName = bigbrother::toUpperCase(attrValues[8]);
	if (fileClassName == "UM03_TRIOL") {
		mFileClassType = umkaFileClass_type::umka03;
		mpUM03utl = std::unique_ptr<UMKA03utilites>(new UMKA03utilites);
	} else if (fileClassName == "UM27" || fileClassName == "") {
		mFileClassType = umkaFileClass_type::umka27;
	} else if (fileClassName == "KN24") {
		mFileClassType = umkaFileClass_type::kn24;
	}
	mFilePath = std::string(JOURNAL_PATH) + "/" + attrValues[0] + "."
			+ mFileExtension;

	if (!attrValues[7].empty()) {
		istringstream(attrValues[7]) >> mRequestCount;
		if (mRequestCount <= 0) {
			mRequestCount = 1; // процесс вычитки будет очень долгим
			clog
					<< "[ParameterModbusFile::Init()] ]Set default value of mRequestCount."
					<< endl;
		}
	} else {
		mRequestCount = 1; // процесс вычитки будет очень долгим
		clog
				<< "[ParameterModbusFile::Init()] Set default value of mRequestCount."
				<< endl;
	}

	return true;
}

void ParameterModbusFile::SetProcessState(const fileStatus_type& status) {
	mProcessState = status;
}

void ParameterModbusFile::setModbusValueToCurrentDataPipe(struct timeval *tp,
		const std::string& data, const validState_type& validState) {
	cerr << "Not implemented!" << endl;
}

void ParameterModbusFile::readFile0x68() {

	if (mFileClassType == umkaFileClass_type::umka27
			|| mFileClassType == umkaFileClass_type::kn24) {
		//PRINTDEBUG("HERE")
		uint8_t buffer[MODBUS_TCP_MAX_ADU_LENGTH];
		umkaFile_type* journalStatus_local = new umkaFile_type;
		FILE *pFile = 0;
		journalStatus_local->cur_filePath = mFilePath.c_str();
		journalStatus_local->isWriteMode = false;
		mIsReadingUmkaFile = true;

		pFile = fopen(mFilePath.c_str(), "ab");
		if (!pFile) {
			journalStatus_local->umkaFileStatus = fileStatus_type::rw_error;
			SetDataToFileBuffer(journalStatus_local);
			remove(mFilePath.c_str());
			mIsReadingUmkaFile = false;
			return;
		} else {
			//                cout << "In process" << endl;
			journalStatus_local->umkaFileStatus = fileStatus_type::in_process;
		}

		journalStatus_local->globalID = mServerID;
		V7DeviceModbus* pDeviceModbus = dynamic_cast<V7DeviceModbus*>(mpDevice);
		for (int i = 0; i < mRequestCount; ++i) {
			sleep(0);
			long cur_sizeFile = ftell(pFile); //смотрим если файл есть, и начинаем читатьс его размера
			// clog << "FileSize:" << cur_sizeFile << endl;
			journalStatus_local->cur_fileSize = cur_sizeFile;// шлем на сервер вычитанный размер файла.ы
			journalStatus_local->validState = validState_type::valid;
			journalStatus_local->cur_filePath = mFilePath.c_str();

			if (mFileSize == cur_sizeFile) {
				fclose(pFile);

				string str_id = static_cast<ostringstream*>(&(ostringstream()
						<< journalStatus_local->globalID))->str();
				journalStatus_local->umkaFileStatus =
						fileStatus_type::ready_file; //последняя вычитка, добавляем сообщение об успешности.
				const string Gzip = string(GZIP_SCRIPT_PATH)
						+ string(JOURNAL_PATH) + "/" + " -f " + str_id + "."
						+ mFileExtension;
				PRINTDEBUG2("gzip path    ################", Gzip)
				system(Gzip.c_str());					//сжимаю
				SetDataToFileBuffer(journalStatus_local);
				mIsReadingUmkaFile = false;
				return;
			}
			mAddressInJournal = cur_sizeFile / 2;

			QueryMsgJrnAT27 *msg = new QueryMsgJrnAT27;
			msg->mbDevAddr =
					static_cast<uint8_t>(pDeviceModbus->getDeviceAddress());
			msg->mbFuncCode = 0x68;
			msg->mbFileNumber = static_cast<uint8_t>(mFileNumber);
			msg->mbStartAddrHiWordHi = static_cast<uint8_t>(mAddressInJournal
					>> 24);
			msg->mbStartAddrHiWordLo = static_cast<uint8_t>(mAddressInJournal
					>> 16);
			msg->mbStartAddrLoWordHi = static_cast<uint8_t>(mAddressInJournal
					>> 8);
			msg->mbStartAddrLoWordLo = static_cast<uint8_t>(mAddressInJournal);
			if ((cur_sizeFile + (MAX_REGISTER_COUNT * 2)) < mFileSize) {
				msg->mbCntRecords = static_cast<uint8_t>(MAX_REGISTER_COUNT); //122 регистра просим вычитать
			} else {
				msg->mbCntRecords = static_cast<uint8_t>((mFileSize
						- cur_sizeFile) / 2);
			}
			//! запрос
			waitForFreePort();
			int mbRes = mpPort->request(msg);

			if (mbRes != 0) {
#ifdef DEBUG
				cout << "Error read file. Error No ( " << mbRes << " )" << endl;
#endif
				journalStatus_local->umkaFileStatus =
						fileStatus_type::modbus_error;
				journalStatus_local->validState = cvrtErrToValidState(mbRes);
				SetDataToFileBuffer(journalStatus_local);
				fclose(pFile);
				return;

			} else {
				uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
				memcpy(rspBuf, msg->getResponseBuffer(),
						msg->getResponseSize());
				if (mFileClassType == umkaFileClass_type::umka27) {
					bigbrother::swapBytesInWordsInBuffer(rspBuf + 8,
							rspBuf[7] * 2 + 8);
				}
				fwrite(rspBuf + 8, 2, rspBuf[7], pFile);
			}
			umkaFile_type* tmpStatus = new umkaFile_type;
			tmpStatus->globalID = journalStatus_local->globalID;
			tmpStatus->cur_fileSize = journalStatus_local->cur_fileSize;
			tmpStatus->cur_filePath = journalStatus_local->cur_filePath;
			tmpStatus->umkaFileStatus = fileStatus_type::in_process;
			tmpStatus->validState = validState_type::valid;
			SetDataToFileBuffer(tmpStatus);
			delete msg;
		}
		SetDataToFileBuffer(journalStatus_local);
		fclose(pFile);
	}

}

void ParameterModbusFile::readFile0x14() {
	if (mFileClassType == umkaFileClass_type::umka03) {
		//PRINTDEBUG2("\n\n\nin 0x14:::::::", FunctionIsSupported(0x14));
		//PRINTDEBUG2("\n\n\n CURRENT JRN SIZE", mpUM03utl->currentJournalSize());
		//PRINTDEBUG2("\n\n\n CALC JRN SIZE", mpUM03utl->journalSize());
		if (mIsNeedReadFile) {
			//PRINTDEBUG("\n\n\n INTO JRN READING");
			mIsReadingUmkaFile = true;
			umkaFile_type* journalStatus_UM3 = new umkaFile_type;
			waitForFreePort();
			fileReadStatus_type status = mpUM03utl->ReadWholeJournal(mpPort,
					mFilePath.c_str(),
					mFileClassType == umkaFileClass_type::umka03 ? 1024 : 2048,
					mRequestCount,
					static_cast<uint8_t>(mpDeviceModbus->getDeviceAddress()));
			journalStatus_UM3->globalID = mServerID;
			journalStatus_UM3->umkaFileStatus = fileStatus_type::in_process;
			journalStatus_UM3->validState = validState_type::valid;
//			journalStatus_UM3->isNeedReadFile = true;

			umkaFile_type* journalStatus_UM31 = new umkaFile_type;
			*journalStatus_UM31 = *journalStatus_UM3;

			SetDataToFileBuffer(journalStatus_UM31);

			switch (status) {
			case fileReadStatus_type::success: {
				string str_id = static_cast<ostringstream*>(&(ostringstream()
						<< journalStatus_UM3->globalID))->str();
				journalStatus_UM3->cur_fileSize = bigbrother::getFileSize(
						mFilePath);
#ifdef DEBUG
				cout << journalStatus_UM3->cur_fileSize << " FILESIZE" << endl;
#endif
				journalStatus_UM3->umkaFileStatus = fileStatus_type::ready_file; //последняя вычитка, добавляем сообщение об успешности.
				journalStatus_UM3->validState = validState_type::valid;
				journalStatus_UM3->cur_filePath = mFilePath;
				// mNeedToRead = false;
				const string Gzip = string(GZIP_SCRIPT_PATH)
						+ string(JOURNAL_PATH) + "/" + " -f " + str_id + "."
						+ mFileExtension;
#ifdef DEBUG
				cerr << "gzip path    ################" << Gzip << endl;
#endif
				system(Gzip.c_str()); //сжимаю
				mUmkaFileStatus = fileStatus_type::ready_file;
				mpUM03utl.reset();
				mpUM03utl = std::unique_ptr<UMKA03utilites>(new UMKA03utilites);
//				journalStatus_UM3->isNeedReadFile = false;
				mIsReadingUmkaFile = false;
				mIsNeedReadFile = false;
				SetDataToFileBuffer(journalStatus_UM3);
				return;

			}
			case fileReadStatus_type::success_section_reading: {
				journalStatus_UM3->cur_fileSize =
						mpUM03utl->currentJournalSize();
//				journalStatus_UM3->isNeedReadFile = true;
				SetDataToFileBuffer(journalStatus_UM3);
				mIsNeedReadFile = true;
				return;
			}
			case fileReadStatus_type::modbus_exchange_error: {
				journalStatus_UM3->cur_fileSize =
						mpUM03utl->currentJournalSize();
				journalStatus_UM3->umkaFileStatus =
						fileStatus_type::modbus_error;
				journalStatus_UM3->validState = validState_type::not_responding;
//				journalStatus_UM3->isNeedReadFile = true;
				mIsNeedReadFile = true;
				SetDataToFileBuffer(journalStatus_UM3);
				return;
			}
			case fileReadStatus_type::error: {
				journalStatus_UM3->umkaFileStatus =
						fileStatus_type::modbus_error;
//				journalStatus_UM3->isNeedReadFile = true;
				mIsNeedReadFile = true;
				SetDataToFileBuffer(journalStatus_UM3);
				return;
			}
			}

		}

	}

}

void ParameterModbusFile::Run() {
//    cout
//            << (mProcessState == READY_FILE ?
//                    "ready file in run" : "now file read") << endl;
//TODO Доделать синхронизацию статусов, при отправке файлов < 30К не успевает пролететь статус от сервера и подтвердить вычитку файла
	if (isFuncSupported(0x68)) {
		readFile0x68();
	}

	if (isFuncSupported(0x14)) {
		readFile0x14();
	}

}

void ParameterModbusFile::setModbusValueToCurrentDataPipe(struct timeval *tp,
		uint16_t* data, const validState_type& validState) {
	cerr << "You shall not pass! It's virtual method..." << endl;
	return;
}

void ParameterModbusFile::writeParameterModbus() {

//    PRINTDEBUG2(
//            "======================> writefilwintodevice: <=============================# ",
//            mServerID);
//    umkaFile_type *fileWriteStatus = new umkaFile_type;
//    fileWriteStatus->globalID = mDevConfigNumber;
//    fileWriteStatus->cur_filePath = mConfigAT27Buffer.size();
//    fileWriteStatus->isWriteMode = false;
//    fileWriteStatus->umkaFileStatus = WRITE_SUCCESS_STATE;
//    fileWriteStatus->validState = VALID_STATE;
//    SetDataToFileBuffer(fileWriteStatus);

	/**
	 * Ограничение на кол-во записываемых байт
	 * Согласно протоколу согласования
	 */
	umkaFile_type* writeState = new umkaFile_type;
	writeState->globalID = mDevConfigNumber; ///! передается на сервер номер девайса, для которого пишется конфиг
	writeState->isWriteMode = true;
	const int maxRecToWrite = 0x58; //!< адреса должны находиться в пределах одной страницы 128 байт
	std::string cfgAT27Buffer = mConfigAT27Buffer;
	if (cfgAT27Buffer.size() != mFileSize) {
		cerr << "[ParameterNModbusFile]:: Error write config! " << dec
				<< "File size is: " << mFileSize << " , buffer size is "
				<< cfgAT27Buffer.size() << endl;
		writeState->cur_fileSize = 0;
		writeState->cur_filePath = "";
		writeState->umkaFileStatus = fileStatus_type::rw_error;
		writeState->validState = validState_type::invalid_unknown;
		SetDataToFileBuffer(writeState);
		return;
	}

	if (isFuncSupported(FN_WRITE_CONFIG_AT27)) {
		int curReadWritePos = 0; //!< текущая позиция чтения из файла, записи в УМКА
		for (int i = 0; i < mRequestCount; ++i) {

			int leftRecordsToRead = cfgAT27Buffer.size() / 2; //!< общее количество записей
			const uint8_t *dataToWrite =
					reinterpret_cast<const uint8_t *>(cfgAT27Buffer.c_str()); //!< буфер записываемых данных
			while (curReadWritePos * 2 <= cfgAT27Buffer.size()) {

				uint8_t recToRead =
						(maxRecToWrite < leftRecordsToRead) ?
								static_cast<uint8_t>(maxRecToWrite) :
								static_cast<uint8_t>(leftRecordsToRead);
				uint8_t *tmpValuesBuffer = new uint8_t[recToRead * 2] { 0 }; // buf
				memcpy(tmpValuesBuffer, (dataToWrite + curReadWritePos * 2),
						recToRead * 2);
				bigbrother::swapBytesInWordsInBuffer(tmpValuesBuffer,
						recToRead * 2);
				QueryMsgCfgAT27 *msg = new QueryMsgCfgAT27(
						static_cast<uint8_t>(getAddress()),
						FN_WRITE_CONFIG_AT27, static_cast<uint8_t>(mFileNumber),
						static_cast<uint8_t>(curReadWritePos >> 24),
						static_cast<uint8_t>(curReadWritePos >> 16),
						static_cast<uint8_t>(curReadWritePos >> 8),
						static_cast<uint8_t>(curReadWritePos), recToRead,
						tmpValuesBuffer);

				waitForFreePort();
				int mbRes = mpPort->request(msg);

				if (mbRes != 0) {
					writeState->validState = cvrtErrToValidState(mbRes); //except
					writeState->umkaFileStatus = fileStatus_type::write_fail;
					SetDataToFileBuffer(writeState);
					delete[] tmpValuesBuffer;
					delete msg;
					return;

				} else {
					curReadWritePos += maxRecToWrite; // А.П. УМКА
					leftRecordsToRead = cfgAT27Buffer.size() / 2
							- curReadWritePos;
//                if (leftRecordsToRead < maxRecToWrite) {
//                    numOfReadBytes = leftRecordsToRead * 2; // число байт для чтения из файла
//                }

				}
				delete[] tmpValuesBuffer;
				delete msg;
			}
			PRINTDEBUG("[ConfigAT27] Config was writed into device ");
			writeState->validState = validState_type::valid;
			writeState->umkaFileStatus = fileStatus_type::write_success;
			mIsNeedWriteFile = false;
			SetDataToFileBuffer(writeState);
			return;
		}
	} else {
		PRINTDEBUG("[ConfigAT27] Error while Config was writed into device ");
		writeState->validState = validState_type::invalid_function;
		writeState->umkaFileStatus = fileStatus_type::write_fail;
		SetDataToFileBuffer(writeState);
	}
}

void ParameterModbusFile::SetDataToFileBuffer(umkaFile_type* journalData) {

	mUmkaFileStatus = journalData->umkaFileStatus;

//	if ((mUmkaFileStatus == fileStatus_type::unknown
//
//	|| journalData->umkaFileStatus == fileStatus_type::unknown)
//
//	&& mIsNeedReadFilejournalData->isNeedReadFile) {
//
//		mUmkaFileStatus = fileStatus_type::in_process;
//	}

	mpServer->SetDataToFileBuffer(journalData);
}

void ParameterModbusFile::DeleteFile() {
	PRINTDEBUG(">>>>> Remove call <<<<<")
	if (bigbrother::isFileExist(mFilePath.c_str())) {
		int r = remove(mFilePath.c_str());
		if (!r) {
			cerr << "Failed to delete file: " << mFilePath.c_str() << endl;
		}
	}
	umkaFile_type* jrnState = new umkaFile_type;
	jrnState->globalID = mServerID;
	jrnState->cur_fileSize = 0;
	jrnState->umkaFileStatus = fileStatus_type::delete_success;
	jrnState->validState = validState_type::valid;

	if (mFileClassType == umkaFileClass_type::umka03) {
		mpUM03utl.reset();
		mpUM03utl = std::unique_ptr<UMKA03utilites>(new UMKA03utilites);
	}
	SetDataToFileBuffer(jrnState);
	mIsNeedDeleteFile = false;
	mIsNeedReadFile = false;
}

void ParameterModbusFile::StopReadJrn() {
	PRINTDEBUG("\n>>>>> STOP CALL <<<<<\n")
	umkaFile_type *jrnState = new umkaFile_type;
	jrnState->umkaFileStatus = fileStatus_type::stop_success;
	jrnState->globalID = mServerID;
	jrnState->validState = validState_type::valid;
	jrnState->cur_fileSize = bigbrother::getFileSize(mFilePath.c_str());
	mIsNeedStopReadFile = false;
	mIsNeedReadFile = false;
	SetDataToFileBuffer(jrnState);
}

bool ParameterModbusFile::isConfigFile() const {

	return mFileNumber == CONFIG_AT27_NUMBER ? true : false;

}

void ParameterModbusFile::setConfigFileAT27Buffer(const std::string &str) {
	mConfigAT27Buffer = str;
}

void ParameterModbusFile::setDeviceConfigNumber(unsigned devNumber) {
	mDevConfigNumber = devNumber;
}

void ParameterModbusFile::WriteConfig() {
	umkaFile_type* writeState = new umkaFile_type;
	writeState->globalID = mDevConfigNumber; ///! передается на сервер номер девайса, для которого пишется конфиг
	writeState->isWriteMode = true;
	writeParameterModbus();
	std::string response = readConfigAT27();
//    PRINTDEBUG("//////////////////////////////////////")
//    PRINTDEBUG2("mConfigAT27Buffer size", mConfigAT27Buffer.size());
//    PRINTDEBUG2("response",response.size());
//    PRINTDEBUG2("resp is empty? ", response.empty());
//    PRINTDEBUG("//////////////////////////////////////")
//    cout << "mConfigAT27Buffer size" << mConfigAT27Buffer.size();
//    cout << "response" << response.size() << endl;

	if (mConfigAT27Buffer != response) {
		writeState->validState = validState_type::invalid_function; //except
		writeState->umkaFileStatus = fileStatus_type::write_confirm_fail;
		SetDataToFileBuffer(writeState);
	} else {
		writeState->validState = validState_type::valid; //except
		writeState->umkaFileStatus = fileStatus_type::write_confirm_success;
		SetDataToFileBuffer(writeState);
		mIsNeedWriteFile = false;
	}
}

void ParameterModbusFile::setModbusValueToCurrentDataPipe(struct timeval* tp,
		uint8_t* data, const validState_type& validState) {
	cerr << "You shall not pass! It's virtual method..." << endl;
	return;
}

std::string ParameterModbusFile::readConfigAT27() {
//    while(!endWorkFlag){
	std::string tmpCfgBuffer;
	mRequestCount = 100;

	if (isFuncSupported(FN_READ_CONFIG_AT27)) {

		for (int i = 0; i < mRequestCount; ++i) {

			long cur_sizeFile = tmpCfgBuffer.size(); //смотрим если файл есть, и начинаем читатьс его размера
			if (mFileSize == cur_sizeFile) {
				return tmpCfgBuffer;
			}
			mAddressInJournal = cur_sizeFile / 2;
			QueryMsgJrnAT27 *msg =
					new QueryMsgJrnAT27(
							(uint8_t) dynamic_cast<V7DeviceModbus*>(mpDevice)->getDeviceAddress(),
							FN_READ_CONFIG_AT27, mFileNumber,
							(uint8_t) (mAddressInJournal >> 24),
							(uint8_t) (mAddressInJournal >> 16),
							(uint8_t) (mAddressInJournal >> 8),
							(uint8_t) mAddressInJournal,
							((cur_sizeFile + (MAX_REGISTER_COUNT * 2))
									< mFileSize) ?
									(uint8_t) MAX_REGISTER_COUNT :
									(uint8_t) (mFileSize - cur_sizeFile) / 2);

			waitForFreePort();
			int mbRes = mpPort->request(msg);

			if (mbRes != 0) {
				PRINTDEBUG("[ConfigAT27] empty row!");
				delete msg;
				return "";
			} else { /**< пишем ответ в переменную*/

				uint8_t rspBuf[MODBUS_TCP_MAX_ADU_LENGTH] = { 0 };
				/// пишем ответ
				memcpy(rspBuf, msg->getResponseBuffer(),
						msg->getResponseSize());
				bigbrother::swapBytesInWordsInBuffer(rspBuf + 8,
						rspBuf[7] * 2 + 8);
				// заполняем буффер ответаж
				ostringstream tmpStr;
				tmpStr.write(reinterpret_cast<const char*>(rspBuf + 8),
						2 * rspBuf[7]);
				tmpCfgBuffer.append(tmpStr.str());
				delete msg;
			}

		}
	}
	return tmpCfgBuffer;
}

void ParameterModbusFile::setDeleteFlag() {
	mIsNeedDeleteFile = true;

}
void ParameterModbusFile::setStopFlag() {
	mIsNeedStopReadFile = true;

}
void ParameterModbusFile::setReadFlag() {
	mIsNeedReadFile = true;
}

void ParameterModbusFile::setWriteFlag() {
	mIsNeedWriteFile = true;
}

bool ParameterModbusFile::getDeletFlag() const {
	return mIsNeedDeleteFile;

}
bool ParameterModbusFile::getStopFlag() const {
	return mIsNeedStopReadFile;
}

bool ParameterModbusFile::getReadFlag() const {
	return mIsNeedReadFile;
}

bool ParameterModbusFile::getWriteFlag() const {
	return mIsNeedWriteFile;
}
void ParameterModbusFile::resetReadFlag() {
	mIsNeedReadFile = false;
}

void ParameterModbusFile::resetDeleteFlag() {
	mIsNeedDeleteFile = false;
}

void ParameterModbusFile::resetStopFlag() {
	mIsNeedStopReadFile = false;
}

void ParameterModbusFile::resetWriteFlag() {
	mIsNeedWriteFile = false;
}

void ParameterModbusFile::resetAllStateFlags() {
	resetReadFlag();
	resetStopFlag();
	resetDeleteFlag();
	resetWriteFlag();
}

void ParameterModbusFile::badIdAnswer(uint32_t bad_id) {
	umkaFile_type* jrnState = new umkaFile_type;
	jrnState->globalID = bad_id;
	jrnState->umkaFileStatus = fileStatus_type::bad_id;
	jrnState->validState = validState_type::valid;
	SetDataToFileBuffer(jrnState);
}
