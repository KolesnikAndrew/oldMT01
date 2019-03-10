/*
 * SendData.cpp
 *
 *  Created on: 10 сент. 2014
 *      Author: rtem
 */

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include "PostToServer.h"
#include "parameter.h"
#include "globals.h"

using namespace std;

PostToServer::PostToServer(const string & serverAddress,
		const string & serverFTP, const string & proxyAddress,
		const string & proxyPort, const string & proxyUser,
		const string & proxyPassword, const string & serialNumber,
		V7Server* server) :
		mSerialNumber(serialNumber), mCurlResult(CURLE_OK), mpTm(0), mpServer(
				server), mpDataTransfer(new DataTransfer(mpServer)), mpConfigFile(
				new ConfigFile(serverAddress)), mConnect(false), mIsNeedToSendFile(
				false), mpXMLAttr(new XMLAttribute()), mCntCashRead(0), mBadParamID(
				false) {
	//CreatingFile();
	strcpy(ConnectionTime, "0000-00-00 00:00:00");
	mIsNeedAuthorize = false;
	mChoosenFlag = flagNone;
	proxySetting pr = proxySetting(proxyAddress, proxyPort, proxyUser,
			proxyPassword);
	mpCurlWrapper = new LibCurlWrapper( { serverAddress, serverFTP,
			proxySetting(proxyAddress, proxyPort, proxyUser, proxyPassword) });
	mpCurlWrapper->setCookie(std::string(F_COOKIE_PATH));
	mIsZipTraffic = server->getModemZipFlag();
	mSkipQueryImputData = false;
}

PostToServer::~PostToServer() {

	delete mpCurlWrapper;
	if (mpDataTransfer) {
		delete mpDataTransfer;
	}
	if (mpXMLAttr) {
		delete mpXMLAttr;
	}
	if (mpConfigFile) {
		delete mpConfigFile;
	}
}

string PostToServer::scanFirmwareFile() {
	string Message;
	if (ifstream(MNT_ROOT).is_open()) {
		if (ifstream(F_ROOTFS_NAND_PATH).is_open()) {
			Message += "&dt_rootfs=";
			Message += bigbrother::timeStamp();
			if (!(ifstream(F_LINUX_NAND_PATH).is_open())) { //если дирректория существует
				Message += "&dt_imx=";
				Message += bigbrother::timeStamp();
			} else {
				Message += "&dt_imx=";
				Message += "**Warning** Файл испарился!";
			}
		} else {
			Message += "&dt_rootfs=";
			Message += "**Warning** Файл испарился!";
			Message += "&dt_imx=";
			Message += "**Warning** Файл испарился!";
		}
	} else {
		Message += "&dt_rootfs=";
		Message += "**Critical** Дирректория испарилась!";
		Message += "&dt_imx=";
		Message += "**Critical** Дирректория испарилась!";
	}
	return Message;
}

bool PostToServer::queryAuthorize(bool first) {

	string PostMessage = "SerialNumber=" + mSerialNumber + scanFirmwareFile();
	if (first)
		PostMessage += "&first=1";
	CURLcode result = mpCurlWrapper->post("/Authorization.php", PostMessage);

	PRINTDEBUG2("[PostToServer::Authorized]: ",
			mpCurlWrapper->getResponseBuffer())
	//если удачно послали запрос, иначе ошибка авторизации
	if (result == CURLE_OK) {
		string Answer = mpConfigFile->ParseAnswer(
				mpCurlWrapper->getResponseBuffer(), " Authorized");
		//если авторизация на сервере удачна, иначе ошибка авторизации
		if (Answer == "OK") {
			cout << "Authorization OK!" << endl;
			mIsNeedAuthorize = false;
			if (standAloneMode) {
				switchToNormalMode = true;
			}
			return true;
		}
		PRINTDEBUG2("YOU_NOT_AUTHORIZED! ", mpCurlWrapper->getStrErrorCode());
		return false;
	}
	return false;
}

/*** вычитка состояния счета с локального cgi-сервера ****/
bool PostToServer::getCashFromModem() {

	if (++mCntCashRead > 6) {
		return false;
	}
	PRINTDEBUG("Getting cash...");

	LibCurlWrapper localServer =
			LibCurlWrapper( { "http://localhost/cgi-bin/mt.cgi", "no_needed", {
					"", "", "", "" } });
	CURLcode result;
	do {
		result = localServer.get("?page=modem&data=account");
		sleep(1);
	} while (localServer.getResponseBuffer().size() == 0);
	if (result == CURLE_OK) {
		ofstream fileCash(F_CASH_SQ_XML_PATH);
		if (!fileCash) {
			PRINTDEBUG2("Error while create ", F_CASH_SQ_XML_PATH);
			return false;
		}
		fileCash << localServer.getResponseBuffer();
		fileCash.close();	//закрываем файл
		system("sync");
		return true;
	} else {
		PRINTDEBUG2("PostToServer::GetCash. Error in get!",
				localServer.getStrErrorCode());
		return false;
	}
}

void PostToServer::processCashQuery(const string& postURL) // post
		{
	CURLcode result = mpCurlWrapper->post(postURL, F_CASH_SQ_XML_PATH,
			mIsZipTraffic);
	PRINTDEBUG2("PostToServer::ReadyCash:", mpCurlWrapper->getResponseBuffer());
	if (result == CURLE_OK) {
		string Answer = mpConfigFile->ParseAnswer(
				mpCurlWrapper->getResponseBuffer(),
				" sending the Cash to the server");
		if (Answer == "5") {
			mIsNeedAuthorize = true;
		}
		mCntCashRead = 0;
	} else {
		PRINTDEBUG2("Error! Failed to send Cash on the server.",
				mpCurlWrapper->getStrErrorCode());
	}

	if (mIsNeedAuthorize)
		queryAuthorize(false);

}

bool PostToServer::sendFile(const string& postURL, const string& FilePath,
		const string& Message, unsigned int par_id) {

	CURLcode result = mpCurlWrapper->post(postURL, FilePath,
			bigbrother::isSubstrInStr(FilePath, ".gz") ? false : true, par_id);

	bool sendFileResult = false;

	if (result == CURLE_OK) {
		//проверяем успешность выполнения операции
		PRINTDEBUG("[PostToServer::SendFile]: Try to send file");
		string Answer = mpConfigFile->ParseAnswer(
				mpCurlWrapper->getResponseBuffer(), Message.c_str());
		if (Answer == "5") {
			mIsNeedAuthorize = true;
		}
		if (Answer == "OK") {
			mIsNeedAuthorize = true;
			sendFileResult = true;
		}
		if (Answer == "Error parsing file!\n") {
			mIsNeedAuthorize = true;
		}
	} else {
		//выводим сообщение об ошибке
		PRINTDEBUG2("PostToServer::SendFile:: ",
				Message.c_str() + mpCurlWrapper->getStrErrorCode());
	}
	if (mIsNeedAuthorize)
		queryAuthorize(false);
	return sendFileResult;
}

#ifdef DEBUG
/**
 * @brief Сохраняем копию файла
 * @param fileName
 */
static void saveTmpFile(const char *fileName) {

	static int i(0);
	ostringstream str;
	str << (++i);
	const std::string cmd = std::string("cp ") + fileName + " " + fileName
			+ "_";
	system(string(cmd + str.str()).c_str());
	system("sync");
}
#endif

void PostToServer::sendValuesArchive() {
	mpDataTransfer->DTPrepareSendArchiveValues();
	if (bigbrother::isFileExist(F_SEND_VALUES_ARCHIVE)) {
		const std::string postURL = "/SendValues.php";
#ifdef DEBUG
		saveTmpFile(F_SEND_VALUES_ARCHIVE);
#endif
		mCurlResult = mpCurlWrapper->post(postURL, F_SEND_VALUES_ARCHIVE,
				mIsZipTraffic);
		if (mCurlResult == CURLE_OK) {
			mCommentConnect = "mConnect yes";
			string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(),
					" send values ​​to the server");
			if (Answer == "5") {
				mIsNeedAuthorize = true;
			}
			mConnect = true;
			bigbrother::removeFile(F_SEND_VALUES_ARCHIVE);
		} else {
			mCommentConnect = "No connect";
			mConnect = false;
		}
	}
	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	}

}
void PostToServer::sendValuesBinaryArray() {
	std::set<std::string> ls = bigbrother::getDirList(TMP_DIR_MT01_DYN);
	if (ls.empty()) {
		//PRINTDEBUG("No data will be sent");
		return;
	} else {
		const std::string postURL = "/SendBinaryArray";
		const std::string fileName = *(ls.begin());
		const std::size_t foundUnderline = fileName.find("_");
		const std::string timeStamp = fileName.substr(0, foundUnderline);
		const std::string tmpId = fileName.substr(foundUnderline + 1);

		const int paramId = std::stoi(tmpId);
		const std::string p2file = std::string(TMP_DIR_MT01_DYN) + "/"
				+ fileName;

		mCurlResult = mpCurlWrapper->post(postURL, p2file.c_str(), false,
				std::stoi(tmpId), std::stol(timeStamp), "data"); //посылка запроса на сервер

		PRINTDEBUG2("Dynagraph file: ", fileName);
		PRINTDEBUG2("Dynagraph file path: ", p2file.c_str());
		PRINTDEBUG2("CurlResult: ", mCurlResult);

		if (mCurlResult == CURLE_OK) { //проверяем успешность выполнения операции
			remove(p2file.c_str());
			mCommentConnect = "mConnect yes";
			string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(),
					" send values ​​to the server");
			if (Answer == "5") {
				mIsNeedAuthorize = true;
			}
			mConnect = true;
		} else {
			mCommentConnect = "No connect";
			mConnect = false;
		}
	}
	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	};
}

void PostToServer::sendValuesKN24CrashLog() {

	std::set<std::string> ls = bigbrother::getDirList(TMP_DIR_KN24);
	if (ls.empty()) {
		//PRINTDEBUG("No data will be sent");
		return;
	} else {
		PRINTDEBUG("\n\n\n in post \n\n\n")
		const std::string postURL = "/SendBinaryKnArray";
		const std::string fileName = *(ls.begin());
		PRINTDEBUG2("KN24 crash file: ", fileName);
		const std::size_t foundUnderline = fileName.find("_");
		const std::string timeStamp = fileName.substr(0, foundUnderline);
		PRINTDEBUG2("KN24 crash file: ", timeStamp);
		const std::string tmpId = fileName.substr(foundUnderline + 1);

		const int paramId = std::stoi(tmpId);
		const std::string p2file = std::string(TMP_DIR_KN24) + "/" + fileName;

		mCurlResult = mpCurlWrapper->post(postURL, p2file.c_str(), false,
				std::stoi(tmpId), std::stol(timeStamp), "data");

		PRINTDEBUG2("KN24 crash file: ", fileName);
		PRINTDEBUG2("KN24 crash path: ", p2file.c_str());
		PRINTDEBUG2("CurlResult: ", mCurlResult);

		if (mCurlResult == CURLE_OK) {
			remove(p2file.c_str());
			mCommentConnect = "mConnect yes";
			string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(),
					" send values ​​to the server");
			if (Answer == "5") {
				mIsNeedAuthorize = true;
			}
			mConnect = true;
		} else {
			mCommentConnect = "No connect";
			mConnect = false;
		}
	}
	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	};

}

void PostToServer::sendValues() {
	using namespace bigbrother;
	mpDataTransfer->DTPrepareSendValues();
	if (isFileExist(F_SEND_VALUES)) {
		const std::string postURL = "/SendValues.php";
#ifdef DEBUG
		saveTmpFile(F_SEND_VALUES);
#endif
		mCurlResult = mpCurlWrapper->post(postURL, F_SEND_VALUES,
				mIsZipTraffic);
		if (mCurlResult == CURLE_OK) {
			mCommentConnect = "mConnect yes";
			const string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(),
					" send values ​​to the server");
			if (Answer == "5") {
				mIsNeedAuthorize = true;
			}
			mConnect = true;
			if (Answer == "OK") {
				removeFile(F_SEND_VALUES);
			}
		} else {
			mCommentConnect = "No connect";
			mpDataTransfer->writeArchiveFile();
			removeFile(F_SEND_VALUES);
			removeFile(string(F_SEND_VALUES) + ".gz");
			mConnect = false;
		}
	}
	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	}
}

void PostToServer::queryInputData() {

	CURLcode result = mpCurlWrapper->get("/QueryInputData.php");

	if (result == CURLE_OK) {
		if (mpCurlWrapper->getResponseCode() != 204) {

			const string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(), " query input data"); //проверяем отв
			if (Answer == "OK") {
				parseQueryInputData(mpCurlWrapper->getResponseBuffer()); //парсинг полученных данных с сервера
#ifdef DEBUG
				PRINTDEBUG2("QiD::  ", mpCurlWrapper->getResponseBuffer());
#endif
				switchToNormalMode = true;
			} else if (Answer == "5") {
				mIsNeedAuthorize = true;
				switchToNormalMode = true;
			}
			mConnect = true;
		}
	} else {
		PRINTDEBUG2("QueryInputData error!", mpCurlWrapper->getResponseBuffer()); //выводим сообщение об ошибке
		mConnect = false;
	}
//////////////////////////////
	confirmInputDataProcess(true);
//////////////////////////////
	manageFlags();

	if (mIsNeedAuthorize)
		queryAuthorize(false);

}

void PostToServer::GetNewFile(const std::string& fileName, bool ResumeFrom) {

#ifdef DEBUG
	const char* newFileName = "/tmp/new_file.tar.gz";
#else
	const char* newFileName = "/mnt/sd/tmp/new_file.tar.gz";
#endif

	mCurlResult = mpCurlWrapper->get("/new_file.tar.gz", "", "ck4tdnoFTP",
			"killFTP13", false);
	if (mCurlResult != CURLE_OK) { //если удачно пsослали запрос, иначе ошибка авторизации
		cerr << "Connection error! Trying to continue downloading ..." << endl;
		if (0 != bigbrother::getFileSize(newFileName)) {
			GetNewFile(fileName, true);
		}
	}
}

void PostToServer::ReadyNewFile() {
	const std::string postURL = "/ReadyNewFile.php";
	mCurlResult = mpCurlWrapper->get(postURL);
	if (mCurlResult == CURLE_OK) { //если удачно послали запрос, иначе ошибка авторизации
		const string Answer = mpConfigFile->ParseAnswer(
				mpCurlWrapper->getResponseBuffer(),
				mpCurlWrapper->getServerSettings().mServerITA + postURL);
		if (Answer == "OK") { //если авторизация на сервере удачна, иначе ошибка авторизации
			PRINTDEBUG("PostToServer::ReadyNewFile():: 'OK'")
			mIsNeedAuthorize = true;
		} else if (Answer == "5") {
			mIsNeedAuthorize = true;
		}
	} else {
		cerr << "Error in "
				<< mpCurlWrapper->getServerSettings().mServerITA + postURL
				<< mpCurlWrapper->getStrErrorCode() << endl;
	}

	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	}
}

void PostToServer::processRebootModem(const string& Address) {

	mCurlResult = mpCurlWrapper->get(Address);
	if (mCurlResult == CURLE_OK) { //если удачно послали запрос, иначе ошибка авторизации
		PRINTDEBUG2("ReadyModemReboot", mpCurlWrapper->getResponseBuffer());
		const string Answer = mpConfigFile->ParseAnswer(
				mpCurlWrapper->getResponseBuffer(), " modem reboot");
		if (Answer == "OK") { //если авторизация на сервере удачна, иначе ошибка авторизации
			PRINTDEBUG(
					"PostToServer::ReadyModemReboot: Reboot flag was obtained.");
#ifdef DEBUG
			PRINTDEBUG("sync && reboot");
			sleep(10);
#else
			system("sync && reboot");	//перезагружаю модем
#endif
		} else if (Answer == "5") {
			mIsNeedAuthorize = true;
		}
	} else {
		PRINTDEBUG2("Error modem reboot ", mpCurlWrapper->getStrErrorCode());
	}
	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	}
}

void PostToServer::parseQueryInputData(const string& Message) {
	xmlNodePtr NodePtrLvl1 = 0;
	xmlDocPtr DocPtr = 0;

	DocPtr = xmlParseMemory(Message.c_str(), Message.size());//пытаемся получить данные для парсинга из памятu
	if (DocPtr == NULL) {
		cerr << "Error parsing file in QueryInputData\n" << stderr;
		xmlKeepBlanksDefault(0);
		xmlCleanupCharEncodingHandlers();
		xmlCleanupParser();
		xmlFreeDoc(DocPtr);
		return;
	}
	NodePtrLvl1 = xmlDocGetRootElement(DocPtr);	//получаем самый верхний елемент
	if (xmlStrcmp(NodePtrLvl1->name, (const xmlChar *) "QueryInputData")) {
		xmlKeepBlanksDefault(0);
		xmlCleanupCharEncodingHandlers();
		xmlCleanupParser();
		xmlFreeDoc(DocPtr);
		return;
	}
	NodePtrLvl1 = NodePtrLvl1->xmlChildrenNode;	// двигаемся к дочернему элементу
	ofstream FileConfirmProcess(F_CONFIRM_INPUT_DATA_PROCESS);
	FileConfirmProcess
			<< "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
			<< endl;
	FileConfirmProcess << "<ConfirmInputDataProcess>" << endl;
	int rec_count = 0;
	int i = 0;
	while (NodePtrLvl1 != NULL) {	//перебираем все дочерние элементы
		if ((!xmlStrcmp(NodePtrLvl1->name, (const xmlChar *) "Parameter"))) { //проверяем наш элемент?

			inputData_type* data = new inputData_type;
			data->globalID = stol(
					(char*) xmlGetProp(NodePtrLvl1, (const xmlChar*) "Id"));
			data->value = string(
					(char*) xmlNodeGetContent(NodePtrLvl1->xmlChildrenNode));

			if (mpServer->SetDataToSetpointDataBuffer(data)) { //кладу данные пайп! Если удачно формирую файл
				FileConfirmProcess << "<Parameter Id = '" << data->globalID
						<< "'>";
				FileConfirmProcess << "<State DT = '" << bigbrother::timeStamp()
						<< "'>"
						<< static_cast<int>(inDataState_type::processing)
						<< "</State>";
				FileConfirmProcess << "</Parameter>" << endl;
				++rec_count; //счетчик строк
			}
		}
		// работа с файлами
		if ((!xmlStrcmp(NodePtrLvl1->name, (const xmlChar *) "Journal"))) {
			umkaFile_type* journal_data = new umkaFile_type;
			long paramId = std::stol(
					(const char*) xmlGetProp(NodePtrLvl1,
							(const xmlChar*) "Id"));
			fileStatus_type parsed_state =
					static_cast<fileStatus_type>(std::stoi(
							(const char*) xmlGetProp(NodePtrLvl1,
									(const xmlChar*) "journalState")));

			journal_data->isWriteMode = false;
			journal_data->globalID = paramId;
			journal_data->cur_fileSize = 0;
			journal_data->validState = validState_type::valid;

			if (parsed_state == fileStatus_type::ready_file) {
				mIsNeedToSendFile = true;
			}

			switch (parsed_state) {
			case fileStatus_type::start_process:
			case fileStatus_type::modbus_error:
			case fileStatus_type::need_restart:
				journal_data->umkaFileStatus = parsed_state;
				break;
			case fileStatus_type::need_delete:
				journal_data->umkaFileStatus = parsed_state;
				break;
			case fileStatus_type::need_stop:
				journal_data->umkaFileStatus = parsed_state;
				break;
			default:
				break;
			}
			if ((journal_data->umkaFileStatus != fileStatus_type::unknown
					|| journal_data->umkaFileStatus
							!= fileStatus_type::ready_file)
					&& mpServer->SetDataToFile(journal_data)) {
				FileConfirmProcess << "<Journal ";
				FileConfirmProcess << "Id='" << paramId << "' ";
				FileConfirmProcess << "journalState='"
						<< static_cast<int>(journal_data->umkaFileStatus)
						<< "'>";
				FileConfirmProcess << "</Journal>" << endl;
				++rec_count; //счетчик строк
			}
		}

		if ((!xmlStrcmp(NodePtrLvl1->name, (const xmlChar *) "DeviceConfig"))) { //проверяем наш элемент?
			// PRINTDEBUG("<<=== DeviceConfig ===>>");
			unsigned int uiTmp_id, uiTmp_status;
			umkaFile_type* config_data = new umkaFile_type;
			istringstream(
					(const char*) xmlGetProp(NodePtrLvl1,
							(const xmlChar*) "Id")) >> uiTmp_id;
			config_data->globalID = 0;
			config_data->isWriteMode = false;
			config_data->config_buffer = GetNewAT27Config(uiTmp_id);
			//PRINTDEBUG2(" DeviceConfig ===>>:", config_data->config_buffer.size());
			if (config_data->config_buffer.empty()) {
				config_data->isWriteMode = false;
				config_data->deviceNumber = uiTmp_id;
				config_data->umkaFileStatus = fileStatus_type::rw_error;
				config_data->cur_fileSize = config_data->config_buffer.size();
				config_data->validState = validState_type::valid;
			} else {
				config_data->isWriteMode = true;
				config_data->deviceNumber = uiTmp_id;
				config_data->umkaFileStatus = fileStatus_type::need_write;
				config_data->cur_fileSize = config_data->config_buffer.size();
				config_data->validState = validState_type::valid;
			}
			//   PRINTDEBUG2("config_data->isWriteMode: ", config_data->isWriteMode);
			if (mpServer->SetDataToFile(config_data)) { //кладу данные пайп! Если удачно формирую файл
				FileConfirmProcess << "<DeviceConfig ";
				FileConfirmProcess << "Number='" << uiTmp_id << "' ";
				FileConfirmProcess << "deviceLoadState='"
						<< static_cast<int>(fileStatus_type::in_process)
						<< "'>";
				FileConfirmProcess << "</DeviceConfig>" << endl;
				rec_count++; //счетчик строк
			} else {
				//  PRINTDEBUG("==>> Error setDataToJournal");
			}
		}
		checkNewFlagProcess("reboot", NodePtrLvl1, flagReboot);
		checkNewFlagProcess("modemLog", NodePtrLvl1, flagLogFile);
		checkNewFlagProcess("check_cash", NodePtrLvl1, flagCash);
		checkNewFlagProcess("new_file", NodePtrLvl1, flagNewFile);
		NodePtrLvl1 = NodePtrLvl1->next; //двигаемся к следующему элементу
	}
	FileConfirmProcess << "</ConfirmInputDataProcess>";
	FileConfirmProcess.close();

	if (rec_count == 0) {	//если нет строк
		bigbrother::removeFile(F_CONFIRM_INPUT_DATA_PROCESS);	//удаляю файл
	}
	xmlKeepBlanksDefault(0);
	xmlCleanupCharEncodingHandlers();
	xmlCleanupParser();
	xmlFreeDoc(DocPtr);
}

void PostToServer::checkNewFlagProcess(string name, xmlNodePtr NodePtrLvl1,
		flagProcess set_flag) {
	if ((!xmlStrcmp(NodePtrLvl1->name, (const xmlChar *) name.c_str()))) { //проверяем наш элемент?
		mpXMLAttr->CXMLNode(NodePtrLvl1->xmlChildrenNode);
		if (mpXMLAttr->Value() == "1") {  //если с сервера 1 пришла
			mChoosenFlag = set_flag;
		}
	}
}

void PostToServer::confirmInputDataProcess(bool JustNOw) {
// Фиксируем факт готовности файла к отправке
	const umkaFile_type sTmp = mpDataTransfer->DTConfirmInputDataProcess(); //формируем файл из прихода от устройства

	if (sTmp.umkaFileStatus == fileStatus_type::ready_file) {
		mSavedData = sTmp;
	}

	if (bigbrother::isFileExist(F_CONFIRM_INPUT_DATA_PROCESS)) { //если в файле есть строки
		const string postURL = "/ConfirmInputDataProcess.php";
		mCurlResult = mpCurlWrapper->post(postURL, F_CONFIRM_INPUT_DATA_PROCESS,
				mIsZipTraffic); //посылка запроса на сервер

		if (mCurlResult == CURLE_OK) { //проверяем успешность выполнения операции
			mCommentConnect = "Connect yes";
			const string Answer = mpConfigFile->ParseAnswer(
					mpCurlWrapper->getResponseBuffer(),
					" confirmation processing of the input data"); //проверяем ответ
			mIsNeedAuthorize = false;
			if (Answer == "5") {
				mIsNeedAuthorize = true;
			}
			// curl_easy_getinfo(mpCurl, CURLINFO_SPEED_DOWNLOAD, &download_speed);
			mConnect = true;
		} else {
			mCommentConnect = "No connect";
			//выводим сообщение об ошибке
			cerr << "Error confirm input data processing!"
					<< mpCurlWrapper->getStrErrorCode() << endl;
			mpDataTransfer->writeConfirmDataArchive();
			mConnect = false;
		}
	}

	if (mIsNeedAuthorize) {
		queryAuthorize(false);
	}

// PRINTDEBUG2("\nmSavedData.umkaFileStatus    ",mSavedData.umkaFileStatus )
// PRINTDEBUG2("\nmIsNeedToSendFile    ",mIsNeedToSendFile )

	if (mSavedData.umkaFileStatus == fileStatus_type::ready_file
			&& mIsNeedToSendFile) {
		clog << "mSavedData.cur_filePath" << mSavedData.cur_filePath << endl;

		if (sendFile("/SendJournalFile", mSavedData.cur_filePath + ".tar.gz",
				" error send journal file", mSavedData.globalID)) {

			bigbrother::removeFile(mSavedData.cur_filePath);
			bigbrother::removeFile(mSavedData.cur_filePath + ".tar.gz");
			//mSavedData.umkaFileStatus = fileStatus_type::unknown;
			umkaFile_type *tmp = new umkaFile_type();
			tmp->umkaFileStatus = fileStatus_type::send_success;
			tmp->globalID = mSavedData.globalID;
			tmp->validState = validState_type::invalid_unknown;
			mpServer->SetDataToFile(tmp);
			mIsNeedToSendFile = false;
		}
	}
}

void PostToServer::setZipTrafficFlag(bool flag) {
	mIsZipTraffic = flag;
}

std::string PostToServer::getServerTime() {
	const string postURL = std::string("/GetTime");
	mCurlResult = mpCurlWrapper->get(postURL);
	if (mCurlResult == CURLE_OK) {
		clog << "[getServerTime] Time was gotten from server!" << endl;
		return mpCurlWrapper->getResponseBuffer();
	} else {
		clog << "[getServerTime] Unable to get time from server!" << endl;
		return "";
	}

}

std::string PostToServer::GetNewAT27Config(unsigned int id) {

	const string postURL = std::string("/DeviceLoadConfig?device_number=")
			+ static_cast<ostringstream*>(&(ostringstream() << id))->str();

	mCurlResult = mpCurlWrapper->get(postURL);

	if (mCurlResult == CURLE_OK) {
		clog << "[ConfigAT27::get] New file was gotten!" << endl;
		return mpCurlWrapper->getResponseBuffer();
	} else {
		clog << "[ConfigAT27::get] File not found on server" << endl;
		return "";
	}

}

void PostToServer::manageFlags() {
//PRINTDEBUG2("flag obtained   ", mChoosenFlag);
	switch (mChoosenFlag) {
	case flagLogFile:
		//SendLogFile("/SendLogFile.php");
		break;
	case flagCash:
		if (!getCashFromModem()) {
			break;
		}
		processCashQuery("/ReadyCash.php");
		break;
	case flagNewFile:
		system("mount /dev/mmcblk0p3 /mnt/sd");
		GetNewFile(F_NEW_FILE_PATH, false); //#warnign //поменять на falsе //зачем?
		ReadyNewFile();
		system("sync");
		//SendLogFile("/SendLogFile.php");
		sleep(1);
		system("/usr/local/bin/new_file");   //сжимаю
		//        system("tar -xvf /mnt/sd/tmp/new_file.tar");
		//        system("cp -fRv /mnt/sd/tmp/new_file/* /");
		//        system("/usr/local/bin/replace_firmware -r /mnt/sd/tmp/rootfs_nand.tar.gz");
		//        system("replace_firmware -r /mnt/sd/tmp/rootfs_nand.tar.gz");
		//        return true;
		break;
	case flagReboot:
		PRINTDEBUG("====> Reboot Modem! <=====")
		processRebootModem("/ReadyModemReboot.php");
		break;
	default:
		break;
	}
	mChoosenFlag = flagNone;
}

std::ostream &operator<<(std::ostream& stream, const PostToServer& obj) {

	return stream << obj.mpCurlWrapper;
}
