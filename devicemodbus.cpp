/**
 * @file      devicemodbus.cpp
 * @brief     Определение функций класса Modbus-устройства
 * @details
 * @note
 * @author    Инженер-программист Пясецкий Владимир
 * @copyright © TRIOLCORP, 2014
 */

#include <modbus.h>
#include <errno.h>
#include <algorithm>    // std::sort
#include <cstring>
#include <sys/time.h>
#include "globals.h"
#include "port.h"
#include "parametermodbusint.h"
#include "parametermodbus.h"
#include "parametermodbusbool.h"
#include "parametermodbusenum.h"
#include "parametermodbusfile.h"
#include "ParameterModbusFileKN24.h"
#include "parametermodbusws.h"
#include "parametermodbusieee754.h"
#include "devicemodbus.h"
#include "EngineModbus.h"
#include "utilites.h"
#include <memory>

#include "server.h"

V7DeviceModbus::V7DeviceModbus() :
		mAddress(0), mpModbusContext(NULL), mIsConnected(false), mTimeoutAfterReading(
				0), mTimeoutAfterWriting(0), mTimeSync(false), mTimeSyncAddress(
				0), mTimeSyncPeriod(3600), mLastSync(0), mByteTimeout( { 0,
				500000 }), mResponseTimeout( { 0, 500000 }), mIsTimeSyncAvailable(
				false), mTriesNumber(2), mSmartZIP(true), mDeviceTypeID(
				deviceModbusTypeID_type::regular), mCrashFileParamIdx(-1), mIsInCrashHandleProcess(
				false), mResetTime(0) {
	PRINTDEBUG("dev modbus started");
}

V7DeviceModbus::~V7DeviceModbus() {
	PRINTDEBUG("dev modbus finished.")

}

/**
 * @fn SortParameters
 * @brief Функция сортировки параметров по адресам
 * @param param1 - первый параметр
 * @param param2 - второй параметр
 * @return true - первый меньше, false - первый больше или равен
 */
static bool SortParameters(V7Parameter* param1, V7Parameter* param2) {
	V7ParameterModbus* paramModbus1 = dynamic_cast<V7ParameterModbus*>(param1);
	V7ParameterModbus* paramModbus2 = dynamic_cast<V7ParameterModbus*>(param2);

	if (paramModbus1 && paramModbus1)
		return (paramModbus1->getAddress() < paramModbus2->getAddress());

	return false;
}

bool V7DeviceModbus::Init(const xmlpp::Node* pXMLNode, V7Port* pPort) {
	if (!V7Device::Init(pXMLNode, pPort)
			|| !initCommonDeviceModbus(pXMLNode, pPort)) {
		return false;
	}
	return true;
}

modbus_t * V7DeviceModbus::GetModbusContext() {
	return mpModbusContext;
}

bool V7DeviceModbus::Connect() {
	mIsConnected = true;

	return true;
}

unsigned int V7DeviceModbus::getDeviceAddress() {
	return mAddress;
}

void V7DeviceModbus::Disconnect() {

}

void V7DeviceModbus::readUmkaFile() {

	for (int cycle_iter = 0; cycle_iter < mvpUmkaFiles.size(); cycle_iter++) {
		ParameterModbusFile* curParam =
				dynamic_cast<ParameterModbusFile*>(mvpUmkaFiles[cycle_iter]);

		if (curParam->getDeletFlag()) {
			PRINTDEBUG(">>>> NEED DELETE JOURNAL <<<<")
			curParam->DeleteFile();
		} else if (curParam->getStopFlag()) {
			PRINTDEBUG(">>>> NEED STOP JOURNAL <<<<")
			curParam->StopReadJrn();
		} else if (curParam->getReadFlag()) {
			PRINTDEBUG(">>>> NEED READ JOURNAL <<<<")
			curParam->Run();
		} else if (curParam->getWriteFlag()) {
			PRINTDEBUG(">>>> NEED WRITE CONFIG <<<<<")
			curParam->WriteConfig(); // write config into device
		}

//		} else {
//			PRINTDEBUG(">>>>> UNKNOWn STATE <<<<<<<<");
//		}

	}
}
bool V7DeviceModbus::isKN24device() {
	for (auto &x : mvpUmkaFiles) {
		if (x->getParamType() == paramType_type::p_crash_kn24) {
			return true;
		}
	}
	return false;
}
void V7DeviceModbus::Session() {

	++mSessionNumber;

	//struct timeval sessionTime;
	const int nunOfParams = mvpParameters.size();
	const bool isByteAayaysInParams = mvpByteSingleArrays.size() > 0;
	const bool isSingleMatrixInParams = mvpSingleMatrix.size() > 0;
	const bool isUmkaFileInParams = mvpUmkaFiles.size() > 0;
	const bool isWSinParams = mvpStatusWords.size() > 0;
	const bool isParamKN24device = isUmkaFileInParams ? isKN24device() : false;
	gettimeofday(&mSessionTime, NULL); 	//Берем текущее время
	mpPort->setTriesNumber(mTriesNumber);
	const uint32_t ts = bigbrother::getUnixTimeStamp();
	const uint32_t sec_in_day = 86400;
	// todo протестировать
	if ((ts - mResetTime) > 86400 || !mResetTime) {
		PRINTDEBUG("reset")
		PRINTDEBUG2("d: ", (ts - mResetTime))
		resetTimeReadFlag();
		mResetTime = ts;
	}
	//Задействуем механизм синхронизации времени, если он есть
	if (mIsTimeSyncAvailable) {
		if (mTimeSync && (mSessionTime.tv_sec - mLastSync >= mTimeSyncPeriod)) {
			WriteTimeSync(mSessionTime.tv_sec);
			mLastSync = mSessionTime.tv_sec;
		}
	}

	if (isSingleMatrixInParams) {
		readMatrix();
	}

	if (isByteAayaysInParams) {
		readByteArrays();
	}

	if (isUmkaFileInParams) {
		readUmkaFile();
	}

	PRINTDEBUG2(">>>>>>>>>>>> crash addrrrr :  ", KN24CrashRegAddr())

	if (isParamKN24device && !mIsInCrashHandleProcess && isKN24CrashEvent()) {
		sleep(5);
		bool res = KN24CrashHandler();
		if (res) {
			PRINTDEBUG("Ooooh, crash now")
		}
	}

	if (isWSinParams) {
		setStatusWordParameters();
	}

	//проходим по массивц параметров
	for (int i = 0; i < nunOfParams; ++i) {

		if (endWorkFlag) {
			break;
		}

		if (isWSinParams) {
			setStatusWordParameters();
		}

		V7ParameterModbus* curParam =
				dynamic_cast<V7ParameterModbus*>(mvpParameters[i]);
		if (!curParam) {
			continue;
		}
		// New parameter is available
		if (curParam->isNewValue()) {
			waitFreePort();
			curParam->writeParameterModbus();
			usleep(mTimeoutAfterWriting);
		} else {
			if (curParam->mSessionNumber == mSessionNumber) {
				//no need to read
				continue;
			}
		}

		if (!curParam->needToGetValue(&mSessionTime)) {
			continue;
		}

		//Какой функцией опрашивать
		unsigned int curFunction;
		// Приоритет для функции 0x04, непонятно зачем
		if (curParam->isFuncSupported(0x03)) {
			curFunction = 0x03;
		} else if (curParam->isFuncSupported(0x04)) {
			curFunction = 0x04;
		}
		unsigned int size = curParam->getSize();
		unsigned int additionalParamsCount = 0;

		V7ParameterModbus* additionalParamsAddr[MODBUS_MAX_READ_REGISTERS];

		//начинаем набирать посылку из следующих параметров
		for (int j = i + 1; j < nunOfParams; ++j) {
			V7ParameterModbus* nextParam =
					dynamic_cast<V7ParameterModbus*>(mvpParameters[j]);
			// пропускаем слово состояния
			if (!nextParam) {
				continue;
			}
			//проверяем функцию
			if (!nextParam->isFuncSupported(curFunction)) {
				continue;
			}
			//Нужно ли сейчас опрашивать параметр
			if (!nextParam->needToGetValue(&mSessionTime)) {
				continue;

			}
			//Проверяем адрес
			if (nextParam->getAddress() != curParam->getAddress() + size) {
				break;
			}
			//Проверяем не превышен ли размер посылки
			if (size + nextParam->getSize() > MODBUS_MAX_READ_REGISTERS) {
				break;
			}
			//Параметр подошел, добавляем
			size += nextParam->getSize();
			additionalParamsAddr[additionalParamsCount] = nextParam;
			++additionalParamsCount;
		}

		// uint16_t buf[MODBUS_MAX_READ_REGISTERS] = { 0 };
		uint8_t byteBuffer[MODBUS_RTU_MAX_ADU_LENGTH] = { 0 };
		validState_type validState = validState_type::valid;
		QueryMsgRead *msg;
		struct timeval readingTime;
		bool continueReading = false;

		do {

			// Читаем посылку
			continueReading = false;
			// Создаем сообщение (посылку)
			if (curFunction == 0x03 || curFunction == 0x04) {
				msg = new QueryMsgRead(mAddress, curFunction,
						static_cast<uint8_t>(curParam->getAddress() >> 8),
						static_cast<uint8_t>(curParam->getAddress() & 0x00ff),
						0x00, size);

			}
			// Ждем осовобождения порта
			waitFreePort();
			// Отпарвляем сообщение
//			cerr << "<<< send >>>" << bigbrother::timeStamp()<< endl;
//			cerr << *msg << endl;
			int mbRes = mpPort->request(msg);
			// Чтение неудачное
			validState = cvrtErrToValidState(mbRes);
			//Берем текущее время
			gettimeofday(&readingTime, NULL);

			if (validState == validState_type::invalid_function
					|| validState == validState_type::invalid_address) {
				if (additionalParamsCount > 0) { //!Запрашиваем повторно один параметр
					additionalParamsCount = 0;
					size = curParam->getSize();
					continueReading = true;
				} else {

					continueReading = false;
				}
			}
		} while (continueReading);
		//Разбираем ответ
//		cerr << "<<< receive >>>" << bigbrother::timeStamp() <<" #params: " << additionalParamsCount <<endl;
//		cerr << *msg << endl;
		uint8_t *pData = msg->getDataFromResponse();

		curParam->setModbusValueToCurrentDataPipe(&readingTime, pData,
				validState);
		curParam->setLastReading(&mSessionTime);
		curParam->mSessionNumber = mSessionNumber;
		size = curParam->getSize();
		for (int j = 0; j < additionalParamsCount; j++) {
			if (!additionalParamsAddr[j])
				break;
			additionalParamsAddr[j]->setModbusValueToCurrentDataPipe(
					&readingTime, &pData[size * 2], validState); // size * 2 - каждый модбас-регистр 2 байта
			additionalParamsAddr[j]->setLastReading(&mSessionTime);
			additionalParamsAddr[j]->mSessionNumber = mSessionNumber;
			size += additionalParamsAddr[j]->getSize();
		}
		memset(byteBuffer, 0, MODBUS_RTU_MAX_ADU_LENGTH);
		delete msg;
	}
}

void V7DeviceModbus::WriteTimeSync(__time_t sec) {

	if (!mpPort) {
		PRINTDEBUG(
				"[DeviceModbus::WriteTimeSync] Fatal error port not found in system!");
		return;
	}

	uint8_t byteBuffer[4] = { static_cast<uint8_t>(sec >> 24),
			static_cast<uint8_t>(sec >> 16), static_cast<uint8_t>(sec >> 8),
			static_cast<uint8_t>(sec & 0xff) };

	QueryMsgWriteMulti * msg = new QueryMsgWriteMulti(
			static_cast<uint8_t>(getDeviceAddress()), 0x06,
			static_cast<uint8_t>(mTimeSyncAddress >> 8),
			static_cast<uint8_t>((mTimeSyncAddress >> 8) & 0xff), 0x00, 0x02, 4,
			byteBuffer);

	waitFreePort();
	int mbRes = mpPort->request(msg);
	if (mbRes) {
		mIsTimeSyncAvailable = false;
		PRINTDEBUG2("Time synchronization error. Error code: ", mbRes);
	}
	delete msg;

}

void V7DeviceModbus::waitFreePort() {
	while (mpPort->isBusy()) {
		sched_yield();
	}
}

bool V7DeviceModbus::initCommonDeviceModbus(const xmlpp::Node* pXMLNode,
		V7Port* pPort) {
	const xmlpp::Element* deviceElement =
			dynamic_cast<const xmlpp::Element*>(pXMLNode);
	if (!deviceElement) {
		cerr << "Element \"DeviceModbus\" not found" << endl;
		return false;
	}

	const int line = deviceElement->get_line();

	//!Забираем атрибуты
	const short unsigned numOfParams = 9;
	const xmlpp::Attribute* attributes[numOfParams];
	const Glib::ustring attrNames[numOfParams] = { "address", "byteTimeout",
			"responseTimeout", "timeoutAfterReading", "timeoutAfterWriting",
			"timeSyncAddress", "timeSyncPeriod", "triesNumber", "smartZIP" };
	Glib::ustring attrValues[numOfParams];
	for (int i = 0; i < numOfParams; i++) {
		attributes[i] = deviceElement->get_attribute(attrNames[i]);

		if ((i == 0) && (attributes[i] == NULL)) { //byteTimeout, responseTimeout, timeoutAfterReading, timeoutAfterWriting  timeSyncAddress timeSyncPeriod необязательные
			cerr << "Attribute \"" << attrNames[i] << "\" not found (line="
					<< line << ") " << endl;
			return false;
		}
		if (attributes[i])
			attrValues[i] = attributes[i]->get_value();
	}

	//!Преобразуем атрибуты
	//address---------------------------------------------------------------------------------------
	istringstream(attrValues[0]) >> mAddress;
	PRINTDEBUG2("Modbus address", mAddress);
	if ((mAddress < 1) || (mAddress > 247)) {
		cerr << "Error in the value of the attribute \"address\" (line=" << line
				<< ") " << endl;
		return false;
	}

	//byteTimeout-----------------------------------------------------------------------------------
	if (attributes[1]) {
		unsigned int byteTimeout;
		istringstream(attrValues[1]) >> byteTimeout;
		if (/*(byteTimeout == 0) ||*/(byteTimeout > 5000)) { //!Должен быть больше 0 и меньше 5с
			cerr
					<< "[DeviceModbus]: Error in the value of the attribute \"byteTimeout\" (line="
					<< line << ") " << endl;
			mByteTimeout.tv_sec = 0 / 1000;
			mByteTimeout.tv_usec = (5000 % 1000) * 1000;
		}

		mByteTimeout.tv_sec = byteTimeout / 1000;
		mByteTimeout.tv_usec = (byteTimeout % 1000) * 1000;
	}
	//responseTimeout-------------------------------------------------------------------------------
	if (attributes[2]) {
		unsigned int responseTimeout;
		istringstream(attrValues[2]) >> responseTimeout;
		if ((responseTimeout == 0) || (responseTimeout > 5000)) { //!Должен быть больше 0 и меньше 5с
			cerr << "Error in the value of the attribute \"byteTimeout\" (line="
					<< line << ") " << endl;
			mResponseTimeout.tv_sec = 0 / 1000;
			mResponseTimeout.tv_usec = (5000 % 1000) * 1000;
		}
		mResponseTimeout.tv_sec = responseTimeout / 1000;
		mResponseTimeout.tv_usec = (responseTimeout % 1000) * 1000;
	}
	//!< Установка параметров тайм-аутов
	if (mpPort->getBackendType() == RTU) {
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->responseTimeout =
				mResponseTimeout;
		dynamic_cast<RtuPortSettings*>(mpPort->getPortSettings())->byteTimeout =
				mByteTimeout;
	}
	//timeoutAfterReading---------------------------------------------------------------------------
	if (attributes[3]) {
		istringstream(attrValues[3]) >> mTimeoutAfterReading;
		if ((mTimeoutAfterReading < 0) || (mTimeoutAfterReading > 5000)) { //!Должен быть больше 0 и меньше 5с
			cerr
					<< "Error in the value of the attribute \"timeoutAfterReading\" (line="
					<< line << ") " << endl;
			mTimeoutAfterReading = 500 * 1000; // 0,5 сек
		}
		mTimeoutAfterReading *= 1000;   //!Переводим в микросекунды
	}
	//timeoutAfterWriting---------------------------------------------------------------------------
	if (attributes[4]) {
		istringstream(attrValues[4]) >> mTimeoutAfterWriting;
		if ((mTimeoutAfterWriting < 0) || (mTimeoutAfterWriting > 5000)) { //!Должен быть больше 0 и меньше 5с
			cerr
					<< "Error in the value of the attribute \"timeoutAfterWriting\" (line="
					<< line << ") " << endl;
			mTimeoutAfterWriting = 500 * 1000;   // 0,5 сек
		}
		mTimeoutAfterWriting *= 1000;   //!Переводим в микросекунды
	}
	//timeSyncAddress-------------------------------------------------------------------------------
	if (attributes[5]) {
		istringstream(attrValues[5]) >> mTimeSyncAddress;
		if (mTimeSyncAddress > 0xFFFF) {
			cerr
					<< "Error in the value of the attribute \"timeSyncAddress\" (line="
					<< line << ") " << endl;
			return false;
		}
		mTimeSync = true;
		//timeSyncPeriod--------------------------------------------------------------------------------
		if (attributes[6]) {
			istringstream(attrValues[6]) >> mTimeSyncPeriod;
			if (mTimeSyncPeriod == 0) { //!Должен быть больше 0
				cerr
						<< "Error in the value of the attribute \"timeSyncPeriod\" (line="
						<< line << ") " << endl;
				mTimeSyncPeriod = 500 * 1000; // 0,5 сек
			}
		}
	}
	// triesNumber
	if (attributes[7]) {
		istringstream(attrValues[7]) >> mTriesNumber;
		if (mTriesNumber <= 0) {
			mTriesNumber = 1;
		} else if (mTriesNumber > 3) {
			mTriesNumber = 3;
		}
	}

	if (attributes[8]) {
		if (bigbrother::toUpperCase(attrValues[8]) == "NO")
			mSmartZIP = false;
		else
			mSmartZIP = true;
	}

	//////////// !Берем параметры устройства
	xmlpp::Node::NodeList parametersIntegerList = deviceElement->get_children(
			"ParameterModbusInteger");
	if (!parametersIntegerList.empty()) {
		for (xmlpp::Node::NodeList::iterator iter =
				parametersIntegerList.begin();
				iter != parametersIntegerList.end(); ++iter) {
			V7ParameterModbusInt* mbParamInt = new V7ParameterModbusInt;
			mbParamInt->setParameterSmartZip(mSmartZIP);
			mvpParameters.push_back(mbParamInt);
			if (!mbParamInt->initParam(*iter, this)) {
				return false;
			}
		}
	}
	xmlpp::Node::NodeList parametersBooleanList = deviceElement->get_children(
			"ParameterModbusBoolean");
	if (!parametersBooleanList.empty()) {
		for (xmlpp::Node::NodeList::iterator iter =
				parametersBooleanList.begin();
				iter != parametersBooleanList.end(); ++iter) {
			V7ParameterModbusBool* mbParamBool = new V7ParameterModbusBool;
			mbParamBool->setParameterSmartZip(mSmartZIP);
			mvpParameters.push_back(mbParamBool);
			if (!mbParamBool->initParam(*iter, this))
				return false;
		}
	}
	xmlpp::Node::NodeList parametersEnumList = deviceElement->get_children(
			"ParameterModbusEnum");
	if (!parametersEnumList.empty()) {
		for (xmlpp::Node::NodeList::iterator iter = parametersEnumList.begin();
				iter != parametersEnumList.end(); ++iter) {
			V7ParameterModbusEnum* mbParamEnum = new V7ParameterModbusEnum;
			mbParamEnum->setParameterSmartZip(mSmartZIP);
			mvpParameters.push_back(mbParamEnum);
			if (!mbParamEnum->initParam(*iter, this))
				return false;
		}
	}

	xmlpp::Node::NodeList parametersJournalList = deviceElement->get_children(
			"ParameterModbusFile");
	if (!parametersJournalList.empty()) {
		Glib::ustring jt = "journal_type";
		Glib::ustring jt_name = "kn24";
		Glib::ustring jt_value;
		for (xmlpp::Node::NodeList::iterator iter =
				parametersJournalList.begin();
				iter != parametersJournalList.end(); ++iter) {
			PRINTDEBUG("====================================")
			jt_value =
					dynamic_cast<const xmlpp::Element*>(*iter)->get_attribute(
							"journal_type")->get_value();
			PRINTDEBUG2("jrn_type: ", jt_value)
			ParameterModbusFile* mbJournal;
			if (bigbrother::toLowerCase(jt_value) == "kn24"
					&& mDeviceTypeID != deviceModbusTypeID_type::kn24) {
				mbJournal = new ParameterModbusFileKN24;
				mDeviceTypeID = deviceModbusTypeID_type::kn24;
			} else {
				mbJournal = new ParameterModbusFile;
			}
			mvpUmkaFiles.push_back(mbJournal);
			if (!mbJournal->initParam(*iter, this)) {
				return false;
			}
		}
	}

	auto parametersSingleMatrix = deviceElement->get_children(
			"ParameterModbusSingleMatrix");
	if (!parametersSingleMatrix.empty()) {
		for (auto iter : parametersSingleMatrix) {
			auto * mbS = new ParameterModbusIEEE754SingleMatrix;
			mbS->setParameterSmartZip(mSmartZIP);
			mvpSingleMatrix.push_back(mbS);
			if (!mbS->initParam(iter, this)) {
				return false;
			}
		}
	}

	xmlpp::Node::NodeList parametersWSList = deviceElement->get_children(
			"ParameterModbusWordStateEnum");
	if (!parametersWSList.empty()) {
#ifdef DEBUG
		cout << "not empty" << endl;
#endif
		for (xmlpp::Node::NodeList::iterator iter = parametersWSList.begin();
				iter != parametersWSList.end(); ++iter) {
			ParameterModbusWordState* mbWS = new ParameterModbusWordState;
			mbWS->setParameterSmartZip(mSmartZIP);
			mvpStatusWords.push_back(mbWS);
			if (!mbWS->initParam(*iter, this)) {
				return false;
			}
		}
	}
	xmlpp::Node::NodeList parametersSingleList = deviceElement->get_children(
			"ParameterModbusSingle");
	if (!parametersSingleList.empty()) {
		for (xmlpp::Node::NodeList::iterator iter =
				parametersSingleList.begin();
				iter != parametersSingleList.end(); ++iter) {
			ParameterModbusIEEE754Single* mbS = new ParameterModbusIEEE754Single;
			mbS->setParameterSmartZip(mSmartZIP);
			mvpParameters.push_back(mbS);
			if (!mbS->initParam(*iter, this)) {
				return false;
			}
		}
	}
	auto parametersSingleArrayList = deviceElement->get_children(
			"ParameterModbusSingleArray");
	if (!parametersSingleArrayList.empty()) {
		for (auto iter : parametersSingleArrayList) {
			auto * mbS = new ParameterModbusIEEE754SingleArray;
			mbS->setParameterSmartZip(mSmartZIP);
			mvpByteSingleArrays.push_back(mbS);
			if (!mbS->initParam(iter, this)) {
				return false;
			}
		}
	}

	//Сортируем параметры по адресам
	sort(mvpParameters.begin(), mvpParameters.end(), SortParameters);
	return true;
}

void V7DeviceModbus::setStatusWordParameters() {
	// адрес статусного слова
	if (mvpStatusWords.size() == 0) {
		return;
	}

	auto *pSW = dynamic_cast<V7ParameterModbus*>(mvpStatusWords[0]);

	if (!pSW->needToGetValue(&mSessionTime)) {
		return;
	}

	uint8_t curFunction;
	// приоритет для функции 0х04, непонятно зачем.
	//TODO доработать на веб-интерфейсе добавление только одной функции
	if (pSW->isFuncSupported(0x03)) {
		curFunction = 0x03;
	} else if (pSW->isFuncSupported(0x04)) {
		curFunction = 0x04;
	}
	uint8_t byteBuffer[MODBUS_RTU_MAX_ADU_LENGTH];
	validState_type validState = validState_type::valid;

	std::unique_ptr<QueryMsgRead> msg(
			new QueryMsgRead(mAddress, curFunction,
					static_cast<uint8_t>(pSW->getAddress() >> 8),
					static_cast<uint8_t>(pSW->getAddress() & 0x00ff), 0x00,
					pSW->getSize()));

	// Отпарвляем сообщение
	int mbRes = mpPort->request(msg.get());
	// Чтение неудачное
	validState = cvrtErrToValidState(mbRes);
	// Слово состояния
	uint8_t *pData = msg->getDataFromResponse();
	struct timeval readingTime;
	gettimeofday(&readingTime, NULL);
	// записываем статусы
	for (auto x : mvpStatusWords) {
		auto p = dynamic_cast<V7ParameterModbus*>(x);
		p->setModbusValueToCurrentDataPipe(&readingTime, pData, validState);
		p->setLastReading(&mSessionTime);
		p->mSessionNumber = mSessionNumber;
	}

}

void V7DeviceModbus::readByteArrays() {
	if (mvpByteSingleArrays.size() == 0) {
		//PRINTDEBUG("Vector is empty");
		return;
	}
	auto *pSW =
			dynamic_cast<ParameterModbusIEEE754SingleArray*>(mvpByteSingleArrays[0]);
	if (!pSW->needToGetValue(&mSessionTime)) {
		//PRINTDEBUG("in array: ooh, not time yet");
		return;
	}
	uint8_t curFunction;
	// приоритет на 0х03
	if (pSW->isFuncSupported(0x04)) {
		curFunction = 0x04;
	} else if (pSW->isFuncSupported(0x03)) {
		curFunction = 0x03;
	}

	validState_type validState = validState_type::valid;

	for (auto x : mvpByteSingleArrays) {
		//cout << "id: " << x->getGlobalServerId() << endl;
		auto it = dynamic_cast<ParameterModbusIEEE754SingleArray*>(x);
		const uint8_t reqSize = it->getArraySize() / 2 & 0x00ff;
		const uint16_t reqAddr = it->getAddress();
		//cout << hex << reqAddr << dec << endl;
		struct timeval readingTime;
		gettimeofday(&readingTime, NULL);
		std::unique_ptr<QueryMsgRead> msg0(
				new QueryMsgRead(mAddress, curFunction,
						static_cast<uint8_t>(reqAddr >> 8),
						static_cast<uint8_t>(reqAddr & 0x00ff), 0x00, reqSize));
		std::unique_ptr<QueryMsgRead> msg1(
				new QueryMsgRead(mAddress, curFunction,
						static_cast<uint8_t>((reqAddr + 64) >> 8),
						static_cast<uint8_t>((reqAddr + 64) & 0x00ff), 0x00,
						reqSize));
		std::unique_ptr<QueryMsgRead> msg2(
				new QueryMsgRead(mAddress, curFunction,
						static_cast<uint8_t>((reqAddr + 128) >> 8),
						static_cast<uint8_t>((reqAddr + 128) & 0x00ff), 0x00,
						reqSize));
		std::unique_ptr<QueryMsgRead> msg3(
				new QueryMsgRead(mAddress, curFunction,
						static_cast<uint8_t>((reqAddr + 192) >> 8),
						static_cast<uint8_t>((reqAddr + 192) & 0x00ff), 0x00,
						reqSize));
		int res0 = mpPort->request(msg0.get());
		int res1 = mpPort->request(msg1.get());
		int res2 = mpPort->request(msg2.get());
		int res3 = mpPort->request(msg3.get());

		validState = cvrtErrToValidState(res0 | res1 | res2 | res3);
		uint8_t data[it->getArraySize() * 2 * 2];

		memcpy(data, msg0->getDataFromResponse(), 128);
		memcpy((data + 128), msg1->getDataFromResponse(), 128);
		memcpy((data + 256), msg2->getDataFromResponse(), 128);
		memcpy((data + 384), msg3->getDataFromResponse(), 128);

		it->setModbusValueToCurrentDataPipe(&readingTime, data, validState);
		it->setLastReading(&mSessionTime);
		it->mSessionNumber = mSessionNumber;

	}

}

void V7DeviceModbus::readMatrix() {

	if (mvpSingleMatrix.size() == 0) {
		//PRINTDEBUG("Vector is empty");
		return;
	}

	for (auto row : mvpSingleMatrix) {
		if (!row->needToGetValue(&mSessionTime)) {
			//PRINTDEBUG("in matrix: ooh, not time yet");
			continue;
		}
		auto data = dynamic_cast<ParameterModbusIEEE754SingleMatrix*>(row);
		data->resetData();
		validState_type validState;
		uint8_t tmpData[data[0].getArray()[0]->getArraySize() * 2 * 2];
		std::string dynData[2];
		for (int i = 0; i < 2; ++i) {
			for (auto arr : data->getArray()) {
				uint8_t curFunction;
				if (arr->isFuncSupported(0x04)) {
					curFunction = 0x04;
				} else if (arr->isFuncSupported(0x03)) {
					curFunction = 0x03;
				}
				const uint8_t reqSize = arr->getArraySize() / 2 & 0x00ff;
				const uint16_t reqAddr = arr->getAddress();
				struct timeval readingTime;
				gettimeofday(&readingTime, NULL);

				std::unique_ptr<QueryMsgRead> msg0(
						new QueryMsgRead(mAddress, curFunction,
								static_cast<uint8_t>(reqAddr >> 8),
								static_cast<uint8_t>(reqAddr & 0x00ff), 0x00,
								reqSize));
				std::unique_ptr<QueryMsgRead> msg1(
						new QueryMsgRead(mAddress, curFunction,
								static_cast<uint8_t>((reqAddr + 64) >> 8),
								static_cast<uint8_t>((reqAddr + 64) & 0x00ff),
								0x00, reqSize));
				std::unique_ptr<QueryMsgRead> msg2(
						new QueryMsgRead(mAddress, curFunction,
								static_cast<uint8_t>((reqAddr + 128) >> 8),
								static_cast<uint8_t>((reqAddr + 128) & 0x00ff),
								0x00, reqSize));
				std::unique_ptr<QueryMsgRead> msg3(
						new QueryMsgRead(mAddress, curFunction,
								static_cast<uint8_t>((reqAddr + 192) >> 8),
								static_cast<uint8_t>((reqAddr + 192) & 0x00ff),
								0x00, reqSize));

				int res0 = mpPort->request(msg0.get());
				int res1 = mpPort->request(msg1.get());
				int res2 = mpPort->request(msg2.get());
				int res3 = mpPort->request(msg3.get());

				validState = cvrtErrToValidState(res0 | res1 | res2 | res3);

				// !
				if (validState != validState_type::valid) {
					break;
				}

				uint8_t data[arr->getArraySize() * 2 * 2];

				memcpy(tmpData, msg0->getDataFromResponse(), 128);
				memcpy((tmpData + 128), msg1->getDataFromResponse(), 128);
				memcpy((tmpData + 256), msg2->getDataFromResponse(), 128);
				memcpy((tmpData + 384), msg3->getDataFromResponse(), 128);
				arr->setVlaues(&readingTime, tmpData, validState);
				reinterpret_cast<ParameterModbusIEEE754SingleMatrix*>(row)->addData(
						arr->getArrayData());

			}
			dynData[i] = data->getData();
			data->resetData();
		}
		if (validState == validState_type::valid && dynData[0] == dynData[1]) {
			data->setModbusValueToCurrentDataPipe(&mSessionTime, dynData[0],
					validState);
		} else if (validState == validState_type::valid) {
			data->setModbusValueToCurrentDataPipe(&mSessionTime, "",
					validState);
		}
		row->setLastReading(&mSessionTime);
	}
}

void V7DeviceModbus::setTypeID(const deviceModbusTypeID_type &id) {
	mDeviceTypeID = id;

}

void V7DeviceModbus::setTypeID(const int id) {
	switch (id) {
	case 3:
		mDeviceTypeID = deviceModbusTypeID_type::regular;
		break;
	case 1001:
		mDeviceTypeID = deviceModbusTypeID_type::din16v1;
		break;
	case 1002:
		mDeviceTypeID = deviceModbusTypeID_type::din16v2;
		break;
	case 1003:
		mDeviceTypeID = deviceModbusTypeID_type::stat;
		break;
	case 1004:
		mDeviceTypeID = deviceModbusTypeID_type::kn24;
		break;
	default:
		mDeviceTypeID = deviceModbusTypeID_type::regular;
		break;
	}
}

deviceModbusTypeID_type V7DeviceModbus::getTypeID() const {
	return mDeviceTypeID;
}

bool V7DeviceModbus::isKN24CrashEvent() {
	if (mCrashFileParamIdx == -1) {
		setKN24CrashFileID();
		PRINTDEBUG2("CRASH ID: ", mCrashFileParamIdx)
	}

	uint8_t byteBuffer[MODBUS_RTU_MAX_ADU_LENGTH];
	//todo replace hardcoded address with call getCrashLogRegAddress
	uint16_t addr = KN24CrashRegAddr(); // error mask
	uint16_t mask = KN24CrashMask();
	uint16_t data;

	PRINTDEBUG2("************************>>>> ", addr)
	PRINTDEBUG2("************************>>>> ", mask)

	std::unique_ptr<QueryMsgRead> msg(
			new QueryMsgRead(mAddress, 0x03, static_cast<uint8_t>(addr >> 8),
					static_cast<uint8_t>(addr & 0x00ff), 0x00, 1));

	int mbRes = mpPort->request(msg.get());
	if (mbRes != 0) {
		PRINTDEBUG("Modbus error")
		cerr << "Modbus error while crash log handle" << endl;
		return false;
	} else {
		data = msg->getDataFromResponse()[0] << 8 | msg->getDataFromResponse()[1];
		PRINTDEBUG2("CRASH FILE STATE: ",
				data & mask);
		data = msg->getDataFromResponse()[0] << 8 | msg->getDataFromResponse()[1];
		return (data & mask);
	}

}

bool V7DeviceModbus::KN24CrashHandler() {

	mIsInCrashHandleProcess = true;
	PRINTDEBUG("IN CRASH BRANCH: ")
	auto pFile =
			dynamic_cast<ParameterModbusFileKN24*>(mvpUmkaFiles[mCrashFileParamIdx]);
	pFile->setCrashHandleProcess();
	pFile->Run();
	bool res = pFile->getCrashHandleResult();
	if (!res) {
		cerr << "Can't handle crash log!" << endl;
		return false;
	} else {
		clog << "Successful handle crash log!" << endl;
		PRINTDEBUG("IN CRASH BRANCH: EXIT")
		mIsInCrashHandleProcess = false;
		sleep(5); //синхронизация данных
		return true;
	}
	return false;

}

void V7DeviceModbus::setKN24CrashFileID() {
	for (auto idx = 0; idx < mvpUmkaFiles.size(); ++idx) {
		if (mvpUmkaFiles[idx]->getParamType() == paramType_type::p_crash_kn24) {
			mCrashFileParamIdx = idx;
			break;
		}
	}
}

uint16_t V7DeviceModbus::KN24CrashRegAddr() {
	setKN24CrashFileID();
	auto pFile =
			dynamic_cast<ParameterModbusFileKN24*>(mvpUmkaFiles[mCrashFileParamIdx]);
	pFile->Run();
	PRINTDEBUG2(">>>>> CRASh log:", pFile->getCrashLogRegAddress())
	return pFile->getCrashLogRegAddress();

}

uint16_t V7DeviceModbus::KN24CrashMask() {
	setKN24CrashFileID();
	auto pFile =
			dynamic_cast<ParameterModbusFileKN24*>(mvpUmkaFiles[mCrashFileParamIdx]);
	pFile->Run();
	PRINTDEBUG2(">>>>> CRASh log:", pFile->getCrashLogMask())
	return pFile->getCrashLogMask();
}

void V7DeviceModbus::resetTimeReadFlag() {
	for (auto& param : mvpParameters) {
		param->resetLastReadingTime();
	}
}
