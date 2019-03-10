/**
 * @file      modem.cpp
 * @brief     Определение функций класса модема
 * @details
 * @note      В системе создается один объект этого класса
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include <iostream>
#include <fstream>
#include <libxml++/libxml++.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "server.h"
#include "utilites.h"
#include "modem.h"
#include "port.h"
#include "OverflowFile.h"

using namespace std;

V7Modem::V7Modem() :
		mConfigFileName(CONFIG_FILE_PATH), mThreadDatacarrier(0), mBigbrotherStartFlag(
				false), mMinPollingPeriod(0), mWorkigMode(
				modemWorkMode_type::normal), mQueryPause( { 1, 0 }), mIsZipTraffic(
				false), mpPostToServer(nullptr), mWaitConfigConfirm(150), mpServer(
				new V7Server(this)) {
	/** код ниже для отладочных нужд
	 * т.к. постоянная потеря времени не позволяет
	 * нормально тестировать модем
	 */
	bigbrother::setTime();
#ifndef DEBUG
	const std::string cmd_remove = std::string("rm ") + TMP_DIR_MT01_DYN + std::string("/*");
#endif
#ifdef DEBUG
	const std::string cmd_remove = std::string("rm -rf") + TMP_DIR_MT01;
#endif
	int r = system(cmd_remove.c_str());
	bigbrother::mkpath_p(TMP_DIR_MT01_DYN, 0755);
#ifdef DEBUG
	bigbrother::mkpath_p(MNT_ROOT, 0755);
#endif
}

V7Modem::~V7Modem() {
	while (!mvpPorts.empty()) {
		delete mvpPorts.back();
		mvpPorts.pop_back();
	}
	delete mpServer;
}

bool V7Modem::preCheckModemConfig() {
	xmlpp::DomParser tmpConfigFileParser;
	//!запускаем парсер
	try {
		tmpConfigFileParser.set_substitute_entities(true);
		tmpConfigFileParser.parse_file(mConfigFileName);

		/******* читаем режим и адрес сервера из локального конфига *****/
		const xmlpp::Element* modemElement = nullptr;
		if (!getRootElementFromConfig(modemElement)
				|| !getServerFromConfig(modemElement)
				|| !getModeFromConfig(modemElement)
				|| !getZipTrafficFlagFromConfig(modemElement)
				|| !getIPformConfig(modemElement)) {
			return false;
		}
	} catch (const std::exception& ex) {
		std::cerr << "[modem::PreCheckModemConfig] exception: " << ex.what()
				<< std::endl;
		return false;
	}
	if (!tmpConfigFileParser) {
		cerr << "[modem::PreCheckModemConfig] Parser didn't work" << endl;
		return false;
	}

	return true;
}

bool V7Modem::preCheckBaseData() {
	ifstream ifstreamBaseData(BASEDATA_FILE_PATH);
	if (!ifstreamBaseData.is_open()) {
		cerr << "Error opening file " << BASEDATA_FILE_PATH << " ("
				<< strerror(errno) << ")" << endl;
		return false;
	}

	string strTmp;
	getline(ifstreamBaseData, strTmp);
	const string::size_type pos = strTmp.find("=");
	if (pos == string::npos) {
		cerr << "Serial Number not found in file " << BASEDATA_FILE_PATH
				<< "\n";
		return false;
	}
	mSerialNumber = strTmp.substr(pos + 1, strTmp.size() - (pos + 1));
	if (mSerialNumber.empty()) {
		cerr << "Serial Number not found in file " << BASEDATA_FILE_PATH
				<< "\n";
		return false;
	}

	mFirmwareVersion = getFirmwareVersion(BASEDATA_FILE_PATH);

	ifstreamBaseData.close();

	return true;
}

bool V7Modem::initModem() {

	using namespace xmlpp;
	using namespace bigbrother;

	if (!preInitCheck()) {
		clog
				<< "[V7Modem::Init()] :: critical error, config file corrupt and cannot be repaired."
				<< endl;
		return false;
	}

	/*** задерживаем инит до запуска обмена ***/
	if (mWorkigMode == modemWorkMode_type::normal
			&& xchangeConfigWithServer()) {
		clog << "[modem:] xchange configs is  ok." << endl;
	}
	// запускаем офлайновый и стандэлон режимы
	const Element* modemElement;
	if (!getRootElementFromConfig(modemElement)
			|| !getServerFromConfig(modemElement)
			|| !getModeFromConfig(modemElement)
			|| !getAndInitPortsFromConfig(modemElement)
			|| !getZipTrafficFlagFromConfig(modemElement)
			|| !getQueryLatencyFromConfig(modemElement)
			|| !getGsmModeFromConfig(modemElement)) {
		return false;
	}

	clog << ">>> ITA server address: " << mServerAddress << flush << endl;

	if (mWorkigMode != modemWorkMode_type::offline) {
		mpPostToServer->setZipTrafficFlag(mIsZipTraffic);
	}

	switchGsmMode();

	clog << "[modem::Init()] : successful." << endl;
	clog << "[modem::Init()] Starting  BigBrother. Ver. " << mFirmwareVersion
			<< endl;
	cout << "Exchange between the server and the device is running!" << endl;

	mBigbrotherStartFlag = true;

#ifndef DEBUG
	int r = system("mount /dev/mmcblk0p3 /mnt/sd");
	if ((mWorkigMode == modemWorkMode_type::normal)) {
		r = system("/etc/init.d/S49ntp restart");
	}
	r = system("rm -f /var/lib/logrotate.status");
	r = system("/etc/init.d/S01rsyslog restart");
#endif

	// проверяем и корректируем системное время на время, полученное на сервере
	if (mWorkigMode == modemWorkMode_type::normal
			&& (getSysYear() == 70 || getSysYear() == 117)) {
		std::string ts = mpPostToServer->getServerTime();
		if (!ts.empty())
			setSysTimeUTC(ts);
		else {
			clog << "[modem] Unable get time from server";
		}
	}

	return true;
}

void V7Modem::runModem() {
// стартуем потоки
	startDatacarrier();
	switch (mWorkigMode) {
	case modemWorkMode_type::offline:
		runModemOffline();
		break;
	case modemWorkMode_type::normal:
		runModemOnline();
		break;
	case modemWorkMode_type::standalone:
		runModemStandAlone();
		break;
	default:
		runModemOnline();
		break;
	}

}

void V7Modem::runModemStandAlone() {

	PRINTDEBUG("[modem::Run()] started in standalone mode.");

	switchGreenLed(true);

	for (int i = 0; i < mvpPorts.size(); i++) {
		mvpPorts[i]->Start();
	}

	mpServer->InputDataReceivingStart();

	mpServer->InputDataReceivingWait();

	for (int i = 0; i < mvpPorts.size(); i++) {
		mvpPorts[i]->Wait();
	}

	waitDatacarrier();

	switchGreenLed(false);

}

void V7Modem::runModemOnline() {
	PRINTDEBUG("[modem::Run()] started in normal mode");

	switchGreenLed(true);

	for (int i = 0; i < mvpPorts.size(); i++) {
		mvpPorts[i]->Start();
	}
	mpServer->InputDataReceivingStart();
	mpServer->InputDataReceivingWait();
	for (int i = 0; i < mvpPorts.size(); i++) {
		mvpPorts[i]->Wait();
	}

	waitDatacarrier();

	switchGreenLed(false);

}

void V7Modem::runModemOffline() {

	PRINTDEBUG("[modem::Run()] started");

	switchGreenLed(true);

	offlineMode = true;

	for (int i = 0; i < mvpPorts.size(); i++) {
		mvpPorts[i]->Start();
	}

	waitDatacarrier();

	switchGreenLed(false);

}

V7Server* V7Modem::getServerPtr() {
	return mpServer;
}

bool V7Modem::setDataToDevice(inputData_type* inputData) {
	if (!inputData)
		return false;

	for (int i = 0; i < mvpPorts.size(); i++) {
		if (mvpPorts[i]->setDataToDevice(inputData))
			return true;
	}
	return false;
}

bool V7Modem::setDataToDevice(umkaFile_type* inputData) {
	//PRINTDEBUG2("V7Modem::SetDataToFile::: inputData SET DATA TO FILE  ", (int)inputData->umkaFileStatus);
	if (!inputData)
		return false;

	for (int i = 0; i < mvpPorts.size(); i++) {
		if (mvpPorts[i]->setDataToDevice(inputData))
			return true;
	}
	return false;
}

void V7Modem::switchGreenLed(bool on) {

#ifndef DEBUG
	const std::string script = BB_LED_BLINK;
	cout << (on ? "####### LED is on" : "LED is off") << endl;
	if (on) {
		const std::string cmd (script + " 1");
		system (cmd.c_str());
	} else {
		const std::string cmd (script + " 0");
		system (cmd.c_str());
	}
#else
	PRINTDEBUG(on ? "####### LED is on" : "LED is off");
#endif

}

//==========================================================================================
//                                   Datacarrier
bool V7Modem::startDatacarrier() {
	if (pthread_create(&mThreadDatacarrier, NULL, V7Modem::threadFunc,
			(void*) this) == 0)
		return true;
	else
		return false;
}

bool V7Modem::waitDatacarrier() {
	if (pthread_join(mThreadDatacarrier, NULL) == 0)
		return true;
	else
		return false;
}

/**************************************************************************
 * @fn      mtime
 * @brief   Функция замера времени
 * @param   (void)
 **************************************************************************/
static long mtime() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return (long) t.tv_sec * 1000 + t.tv_usec / 1000;
}

typedef void *(*pthread_startroutine_t)__P((void *)); /**< Для потока */
static pthread_t ledSOSThread; /**< Переменная потока */

void LedSOSThread(int arg) {
	const std::string script(BB_LED_BLINK);
	system((script + " 2").c_str());
}

int ledSOS() {
	pthread_create(&ledSOSThread, NULL, (pthread_startroutine_t) LedSOSThread,
			(void *) 0);
	return 1;
}
void killSOS() {
	const std::string cmd = std::string("killall ") + BB_LED_BLINK;
	int r = system(cmd.c_str());
	r = system((std::string(BB_LED_BLINK) + " 0").c_str());
}


unsigned int V7Modem::getPoolingPeriod() const {
	return mMinPollingPeriod;
}

void V7Modem::setPoolingPeriod(const unsigned int pr) {
	mMinPollingPeriod = pr;
}

time_t V7Modem::qtime() const {
	struct timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec;
}

bool V7Modem::getZipFlag() const {
	return mIsZipTraffic;
}

bool V7Modem::xchangeConfigWithServer() {

	ConfigFile Config = ConfigFile(mServerAddress, mpServer);
	std::unique_ptr<PostToServer> PostMess(
			new PostToServer(mServerAddress, mServerFTP, mProxyAddress,
					mProxyPort, mProxyUser, mProxyPassword, mSerialNumber,
					mpServer));
	// авторизация
	while (!endWorkFlag) {
		if (PostMess->queryAuthorize(true))
			break;
		sleep(mWaitConfigConfirm / 6);
	}
	// получение конфига
	while (!endWorkFlag) {
		if (Config.GetConfigFile(mIsZipTraffic))
			break;
		sleep(mWaitConfigConfirm);
	}
	// отправка конфига
	//TODO усовершенствовать протокол обмена конфигами
	if (!Config.SendModemConfig(mIsZipTraffic)) {
#ifndef DEBUG
		ledSOS();
#endif
		while (!endWorkFlag) {
			if (Config.SendModemConfig(mIsZipTraffic)) {
#ifndef DEBUG
				killSOS();
#endif
				break;
			}
			sleep(mWaitConfigConfirm); // спим долго, сервер может быть уставшим
		}
	}
	mpPostToServer = std::move(PostMess);
	return true;

}

bool V7Modem::xchangeConfigWithServerStandalone() {

	ConfigFile Config = ConfigFile(mServerAddress, mpServer);
	std::unique_ptr<PostToServer> PostMess(
			new PostToServer(mServerAddress, mServerFTP, mProxyAddress,
					mProxyPort, mProxyUser, mProxyPassword, mSerialNumber,
					mpServer));
	while (!endWorkFlag) {
		//пока не авторизуемся
		if (PostMess->queryAuthorize(true)) {
			standAloneMode = false;
			mpServer->setNormalMode();
			break;
		}
		// wait for server response
		sleep(mWaitConfigConfirm);
		if (mpServer->Mode() == modemWorkMode_type::normal) {
			mpServer->setStandaloneMode();
			standAloneMode = true;
		}
#ifdef DEBUG
		clog << "[modem]: Standalone mode will be activated." << endl;
		clog << "[modem]: Server mode is "
				<< (mpServer->Mode() == modemWorkMode_type::normal ?
						"normal" : "standalone") << endl;
#endif
		break;
	}
	//  если в онлайне
	if (mpServer->Mode() == modemWorkMode_type::normal) {
		while (!endWorkFlag) {
			if (Config.GetConfigFile(mIsZipTraffic)) {
				break;
			}
			sleep(mWaitConfigConfirm);
		}
		if (!Config.SendModemConfig(mIsZipTraffic))   // пока не отправим
				{
#ifndef DEBUG
			ledSOS();
#endif
			while (!endWorkFlag) {
				if (Config.SendModemConfig(mIsZipTraffic))   //пока не отправим
						{
#ifndef DEBUG
					killSOS();
#endif
					break;
				}
				sleep(mWaitConfigConfirm);
			}
		}
	}
	mpPostToServer = std::move(PostMess);
	return true;
}

bool V7Modem::xchangeConfigWithServerStandaloneTmp() {

	ConfigFile Config = ConfigFile(mServerAddress, mpServer);
	std::unique_ptr<PostToServer> PostMessStandalone(
			new PostToServer(mServerAddress, mServerFTP, mProxyAddress,
					mProxyPort, mProxyUser, mProxyPassword, mSerialNumber,
					mpServer));
	//пока не авторизуемся
	while (!endWorkFlag) {
		if (PostMessStandalone->queryAuthorize(true)) {
			mpServer->setNormalMode();
			standAloneMode = false;
			break;
		}
		sleep(mWaitConfigConfirm / 6);
	}
	mpPostToServer = std::move(PostMessStandalone);
	// обработка конфига
	// здесь возможны 2 перезагрузки модема
	const string old_hash = bigbrother::getFileHash(mConfigFileName);
	while (!endWorkFlag) {
		if (Config.GetConfigFile(mIsZipTraffic)) {
			switchToNormalMode = false;
			break;
		}
		sleep(mWaitConfigConfirm);
	}

	if (!Config.SendModemConfig(mIsZipTraffic))   // пока не отправим
			{
#ifndef DEBUG
		ledSOS();
#endif
		while (!endWorkFlag) {
			if (Config.SendModemConfig(mIsZipTraffic)) //пока не отправим
					{
#ifndef DEBUG
				killSOS();
#endif
				break;
			}
			sleep(mWaitConfigConfirm);
		}
	}
	if (old_hash != bigbrother::getFileHash(mConfigFileName)) {
		PRINTDEBUG(
				">>>>>>>>>>>> Reboot. New config file from server. <<<<<<<<<<<<<<<<");
#ifndef DEBUG
		system ("sync && reboot");
#endif
	}
	return true;
}

void V7Modem::runDataCarrierOffile() {

#ifndef DEBUG
	OverflowFile *Full = new OverflowFile();
#endif

#ifdef DEBUG
	clog << "Offline mode is runnig!" << endl;
#endif
	mBigbrotherStartFlag = true;
	standAloneMode = true;
	mkfifo(F_ONLINE_STATUS, S_IRWXU);

	while (!endWorkFlag) {
#ifndef DEBUG
		Full->Overflow();
#endif
		sleep(mWaitConfigConfirm / 5);
	}

#ifndef DEBUG
	delete Full;
#endif
}

void V7Modem::runDataCarrierOnline() {

	standAloneMode = false;

#ifndef DEBUG
	OverflowFile *Full = new OverflowFile();
	system("mount /dev/mmcblk0p3 /mnt/sd");
#endif

	time_t queryTime = qtime();

	mkfifo(F_ONLINE_STATUS, S_IRWXU);

	while (!endWorkFlag) {
		double currTime = mtime(); //первая отметка времени
		if ((qtime() - queryTime) >= mQueryPause.tv_sec) {
			mpPostToServer->queryInputData();
			queryTime = qtime();
		}
#ifndef DEBUG
		Full->Overflow();
#endif
		currTime = mtime() - currTime; //разница времени
		double sendValuesTime(0.0);
		sendValuesTime += currTime / 1000;

		if (sendValuesTime >= (mMinPollingPeriod / 1000.)) {
			mpPostToServer->sendValues();
			mpPostToServer->sendValuesArchive();
			mpPostToServer->sendValuesBinaryArray();
			mpPostToServer->sendValuesKN24CrashLog();
			sendValuesTime = 0.0;
		}

		sched_yield();
	}

#ifndef DEBUG
	delete Full;
#endif

}

void V7Modem::runDataCarrierStandalone() {

#ifndef DEBUG
	system("mount /dev/mmcblk0p3 /mnt/sd");
	OverflowFile *Full = new OverflowFile();
#endif
	standAloneMode = true;
	xchangeConfigWithServerStandalone();
	PRINTDEBUG("[modem]: Waiting for server response.");
	if (mpServer->Mode() != modemWorkMode_type::standalone)
		cout
				<< "[modem]: Exchange between the server and the device is running!"
				<< endl;
	else
		cout << "[modem]: Waiting for the server connection (standalone)."
				<< endl;
	mBigbrotherStartFlag = true;

	time_t queryTime = qtime();

	const bool needToSendConfigFile =
			mpServer->Mode() == modemWorkMode_type::standalone ? true : false;

	mkfifo(F_ONLINE_STATUS, S_IRWXU);

	while (!endWorkFlag) {
		if (switchToNormalMode && needToSendConfigFile) {
			PRINTDEBUG2("[>>>>>>>> modem <<<<<<<<]: Server mode is ",
					(mpServer->Mode() == modemWorkMode_type::normal ?
							"normal" : "standalone"));

#ifndef DEBUG
			// перешли в онлайн, синхронизация времени
			int r = system ("/etc/init.d/S49ntp restart");
			// проверяем и корректируем системное время на время, полученное на сервере
			if (mWorkigMode == modemWorkMode_type::normal && (bigbrother::getSysYear() == 70 || bigbrother::getSysYear() == 117)) {
				std::string ts = mpPostToServer->getServerTime();
				if (!ts.empty())
				bigbrother::setSysTimeUTC(ts);
				else {
					clog << "[modem] Unable get time from server";
				}
			}
#endif
			// перешли в онлайн, авторизуемся
			xchangeConfigWithServerStandaloneTmp();

		}
		double currTime = mtime(); //первая отметка времени
		if (qtime() - queryTime >= mQueryPause.tv_sec) {
			PRINTDEBUG2("In querry cycle:: ", *mpPostToServer);
			mpPostToServer->queryInputData();
			queryTime = qtime();
		}
#ifndef DEBUG
		Full->Overflow();
#endif
		double sendValuesTime(0.0);
		currTime = mtime() - currTime; //разница времени
		sendValuesTime += currTime / 1000;

		if (sendValuesTime >= (mMinPollingPeriod / 1000)) {
			mpPostToServer->sendValues();
			mpPostToServer->sendValuesArchive();
			mpPostToServer->sendValuesBinaryArray();
			mpPostToServer->sendValuesKN24CrashLog();
			sendValuesTime = 0;
		}
		sched_yield();
	}
#ifndef DEBUG
	delete Full;
#endif

}

void V7Modem::runDatacarrier() {

	switch (mWorkigMode) {
	case modemWorkMode_type::normal:
		runDataCarrierOnline();
		break;
	case modemWorkMode_type::standalone:
		runDataCarrierStandalone();
		break;
	case modemWorkMode_type::offline:
		runDataCarrierOffile();
		break;
	}

}

bool V7Modem::getServerFromConfig(const xmlpp::Element *modemElement) {

	using namespace xmlpp;

	if (!modemElement) {
		return false;
	}

	const int line = modemElement->get_line();

	//!Забираем атрибуты
	const Attribute* attributes[6];
	const Glib::ustring attrNames[6] = { "serverAddress", "serverFTP",
			"proxyAddress", "proxyPort", "proxyUser", "proxyPassword" };
	Glib::ustring attrValues[6];
	for (int i = 0; i < 6; i++) {
		attributes[i] = modemElement->get_attribute(attrNames[i]);
		if ((i == 0) && (attributes[i] == NULL)) //"proxyAddress","proxyPort","proxyUser","proxyPassword" необязательные
				{
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i])
			attrValues[i] = attributes[i]->get_value();
	}

	//!Преобразуем атрибуты
	//serverAddress-----------------------------------------
	mServerAddress = attrValues[0];
	//serverAddress-----------------------------------------
	if (attributes[1])
		mServerFTP = attrValues[1];
	//proxyAddress------------------------------------------
	if (attributes[2])
		mProxyAddress = attrValues[2];
	//proxyPort---------------------------------------------
	if (attributes[3])
		mProxyPort = attrValues[3];
	//proxyUser---------------------------------------------
	if (attributes[4])
		mProxyUser = attrValues[4];
	//proxyPassword-----------------------------------------
	if (attributes[5])
		mProxyPassword = attrValues[5];
	return true;

}

bool V7Modem::getZipTrafficFlagFromConfig(const xmlpp::Element *modemElement) {
	using namespace xmlpp;
	using namespace bigbrother;
	// вычитка режима работы
	if (!modemElement) {
		return false;
	}

	const Attribute *attr = modemElement->get_attribute("zip");

	const std::string modeValue = attr ? attr->get_value() : "no";

	if (toUpperCase(modeValue) == "YES") {
		mIsZipTraffic = true; // старый конфиг без "Mode"
	} else {
		mIsZipTraffic = false;
	}
	return true;
}

bool V7Modem::getModeFromConfig(const xmlpp::Element *modemElement) {
	using namespace xmlpp;
	using namespace bigbrother;
	// вычитка режима работы
	if (!modemElement) {
		return false;
	}
	const Attribute *attr = modemElement->get_attribute("mode");

	const std::string modeValue = attr ? attr->get_value() : "modeOnline"; // старый конфиг без "Mode" атвоматом онлайн

	if (toUpperCase(modeValue) == "MODEONLINE") {
		mWorkigMode = modemWorkMode_type::normal;
	} else if (toUpperCase(modeValue) == "MODESTANDALONE") {
		mWorkigMode = modemWorkMode_type::standalone;
	} else if (toUpperCase(modeValue) == "MODEOFFLINE") {
		mWorkigMode = modemWorkMode_type::offline;
	} else if (toUpperCase(modeValue) == "MODELOCAL") {
		mWorkigMode = modemWorkMode_type::local;
	} else {
		mWorkigMode = modemWorkMode_type::normal;
	}
	if (mWorkigMode == modemWorkMode_type::local) {
		if (!chkLocalMode()) {
			makeLocalMode(getIpFromHostname(mServerAddress));
			int r = system("sync && reboot");
		}
		mWorkigMode = modemWorkMode_type::normal;
	} else {
		if (chkLocalMode()) {
			makeStandardMode();
			int r = system("sync && reboot");
		}
	}
	return true;
}

bool V7Modem::getQueryLatencyFromConfig(const xmlpp::Element* modemElement) {
	using namespace xmlpp;
	// вычитка режима работы
	if (!modemElement) {
		return false;
	}
	std::string latValue = "";
	Attribute *attr = modemElement->get_attribute("queryLatency");

	if (attr) {
		latValue = attr->get_value();
	}
	//нет аттрибута
	if (latValue == "") {
		mQueryPause = {1, 0}; //1 секунда
	}
	else {
		int tmp;
		if (bigbrother::isStrIsNumber<int>(latValue)) {
			istringstream (latValue) >> tmp;
			mQueryPause = {abs(tmp) /1000, (abs(tmp) % 1000) * 1000}; //1 секунда
		} else {
			mQueryPause = {1, 0}; //1 секунда
		}
		PRINTDEBUG2("Lat_sec :   ", mQueryPause.tv_sec);
		PRINTDEBUG2("Lat_usec:   ", mQueryPause.tv_usec);
	}
	return true;
}

bool V7Modem::getGsmModeFromConfig(const xmlpp::Element *modemElement) {
	using namespace xmlpp;
	// вычитка режима работы
	if (!modemElement) {
		return false;
	}
	string gsmModeStr = "2g";
	const Attribute *attr = modemElement->get_attribute("gsmMode");

	if (attr) {
		gsmModeStr = attr->get_value();
	}
	gsmModeStr = bigbrother::toUpperCase(gsmModeStr);
	if (gsmModeStr == "2G" || gsmModeStr == "3G") {
		mGsmMode = gsmModeStr;
	} else if (gsmModeStr == "AUTO") {
		mGsmMode = "3G";
	} else {
		mGsmMode = "2G";
	}
	return true;

}

bool V7Modem::getRootElementFromConfig(const xmlpp::Element *&modemElement) {
//!запускаем парсер
	using namespace xmlpp;

	try {
		mConfigFileParser.set_substitute_entities(true);
		mConfigFileParser.parse_file(mConfigFileName);
	} catch (const std::exception& ex) {
		std::cerr << "[modem::Init] XML parser exception: " << ex.what()
				<< std::endl;
		return false;
	}

	if (!mConfigFileParser) {
		cerr << "[modem::Init] Parser didn't work" << endl;
		return false;
	}

//!Берем и проверяем корневой элемент <Modem>
	modemElement =
			dynamic_cast<const Element*>(mConfigFileParser.get_document()->get_root_node());
	if ((!modemElement) || (modemElement->get_name() != "Modem")) {
		cerr << "Element \"Modem\" not found" << endl;
		return false;
	}
	return true;
}

bool V7Modem::getAndInitPortsFromConfig(const xmlpp::Element* modemElement) {
//!Берем порты системы
	xmlpp::Node::NodeList portsList = modemElement->get_children("Port");
	if (portsList.empty()) {
		cout << "Ports not found (line: " << modemElement->get_line() << ") "
				<< endl;
		return false;
	}

	for (xmlpp::Node::NodeList::iterator iter = portsList.begin();
			iter != portsList.end(); ++iter) {
		V7Port* mbPort = new V7Port;
		mvpPorts.push_back(mbPort);
		if (!mbPort->Init(*iter, this))
			return false;
	}

	return true;
}

bool V7Modem::preInitCheck() {
	using namespace bigbrother;
	if (!preCheckBaseData()) {
		cerr << "Basedata error!!!" << endl;
		return false;
	}
	if (!chkXmlStruct(CONFIG_FILE_PATH)
			&& !chkXmlStruct(CONFIG_FILE_MINIMAL_PATH)) {
		std::clog << "[Attention] Unrecoverable config file error" << std::endl;
		return false;
	} else if (!chkXmlStruct(CONFIG_FILE_PATH)) {
		cerr << "[modem::Init] Config file in modem is corrupted!!!" << endl;
		const std::string cmdCopy = std::string("cp ") + CONFIG_FILE_PATH + "  "
				+ CONFIG_FILE_PATH + ".err";
		const std::string cmdRemoveConfig = std::string(
				"sync && rm ") + CONFIG_FILE_PATH;
		const std::string cmdRestoreConfig = std::string(
				"sync && cp ") + CONFIG_FILE_MINIMAL_PATH
				+ " " + CONFIG_FILE_PATH;
		int r = system(cmdCopy.c_str());
		r = system(cmdRemoveConfig.c_str());
		r = system(cmdRestoreConfig.c_str());
		r = system("sync");
		clog << "[Warning] Minimal config file will be used" << endl;
	}
	clog << mConfigFileName << endl;

	if (!preCheckModemConfig()) {
		std::clog << "[Attention] Unrecoverable config file error" << std::endl;
	}
	return true;

}

void V7Modem::switchGsmMode() {
	const string cmd = "cp -f ";
	const string chatdef = F_PPP_DEF;
	const string chat2g = F_PPP_2G;
	const string chat3g = F_PPP_3G;
	const string space = " ";
	const string cmd2G = cmd + chat2g + space + chatdef;
	const string cmd3G = cmd + chat3g + space + chatdef;
	const string cmdSync = "sync";
#ifndef DEBUG
	const string cmdRestartPPPD =
	"kill -9 $(ps aux | grep -v grep | grep \"connect_sim\" | awk '{print $1}')";
#else
	const string cmdRestartPPPD = "echo \"Ok, kill pppd\"";
#endif

	cout << mGsmMode << endl;
	if (testGsmMode() == 0 && mGsmMode == "3G") {
		system(cmd3G.c_str());
		system(cmdSync.c_str());
		system(cmdRestartPPPD.c_str());

	} else if (testGsmMode() == 1 && mGsmMode == "2G") {
		system(cmd2G.c_str());
		system(cmdSync.c_str());
		system(cmdRestartPPPD.c_str());
	}
#ifdef DEBUG
	else if ((testGsmMode() == 0 && mGsmMode == "2G")
			|| (testGsmMode() == 1 && mGsmMode == "3G")) {
		PRINTDEBUG("chat_dialog.def ok");
	} else {
		PRINTDEBUG("chat_dialog.def not found");
	}
#endif

}

uint8_t V7Modem::testGsmMode() const {
	using namespace bigbrother;

	if (isFileExist(F_PPP_DEF)) {
		const std::string pppChatDef = readFileInString(F_PPP_DEF);
		// "CNMP=13" - режим 2g
		if (isSubstrInStr(pppChatDef, "CNMP=13")) {
			return 0;
		} else {
			return 1;
		}
	} else {
		PRINTDEBUG("test GSM error!")
		return 2;
	}

}

bool V7Modem::getIPformConfig(const xmlpp::Element *modemElement) {
	using namespace bigbrother;
	using namespace xmlpp;

	if (!modemElement) {
		return false;
	}
	const short numOfParams = 4;
	const Glib::ustring attrNames[numOfParams] = { "primaryIP",
			"primaryNetmask", "secondaryIP", "secondaryNetmask" };
	Glib::ustring attrValues[numOfParams];
	for (short i = 0; i < numOfParams; ++i) {
		Attribute *tmp = modemElement->get_attribute(attrNames[i]);
		if (tmp) {
			attrValues[i] = tmp->get_value();
		}

	}
	mPrimaryIP = ip_set(attrValues[0], "10.0.0.1");
	mPrimaryNetmask = ip_set(attrValues[1], "255.0.0.0");
	const string ver = getFirmwareVersion(BASEDATA_FILE_PATH);
	if (ver == "117.306") {
		mSecondaryIP = ip_set(attrValues[2], "10.10.181.253");
		mSecondaryNetmask = ip_set(attrValues[3], "255.255.255.0");
	} else if (ver != "117.306"
			&& (!attrValues[2].empty() && !attrValues[3].empty())) {
		mSecondaryIP = ip_set(attrValues[2], "");
		mSecondaryNetmask = ip_set(attrValues[3], "");
	} else {
		mSecondaryIP = "";
		mSecondaryNetmask = "";
	}
	PRINTDEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	PRINTDEBUG2("mPrimaryIP        : ", mPrimaryIP);
	PRINTDEBUG2("mPrimaryNetmask   : ", mPrimaryNetmask);
	PRINTDEBUG2("mSecondaryIP      : ", mSecondaryIP);
	PRINTDEBUG2("mSecondaryNetmask : ", mSecondaryNetmask);
	PRINTDEBUG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
	reconfNetworkInterfaces();
	return true;
}

void V7Modem::reconfNetworkInterfaces() {
#ifndef DEBUG_PC
	if(!mSecondaryIP.empty() && !mSecondaryNetmask.empty())
	bigbrother::configNetworkInterface("eth0:1", mSecondaryIP,
			mSecondaryNetmask);
#endif
}

std::string V7Modem::getFirmwareVersion(std::string basedata) {
	const std::string bd = bigbrother::readFileInString(basedata);
	const std::string::size_type n = bd.find("Firmware=");
	if (n == std::string::npos) {
		return "";
	} else {
		return bd.substr(n + 9, 7);
	}

}

