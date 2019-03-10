/*
 * ConfigFile.cpp
 *
 *  Created on: 26 нояб. 2014
 *      Author: rtem
 */

#include "ConfigFile.h"
#include "globals.h"
#include "utilites.h"
#include <libxml++/libxml++.h>

using namespace std;

ConfigFile::ConfigFile(const string& serverAddress) :
		/*tm(NULL), */mResult(CURLE_OK), mpCurl(NULL), mpServerInstance(0), mServerAddress(
				serverAddress) {

	mpXMLAttr = new XMLAttribute();
	mpServerITA = new LibCurlWrapper( { mServerAddress, "no_needed", { "", "",
			"", "" } });

}

ConfigFile::ConfigFile(const string& serverAddress, V7Server *serverInstance) :
		/*tm(NULL), */mResult(CURLE_OK), mpCurl(NULL), mpServerInstance(
				serverInstance), mServerAddress(serverAddress) {

	mpXMLAttr = new XMLAttribute();
	mpServerITA = new LibCurlWrapper( { mServerAddress, "no_needed", { "", "",
			"", "" } });
}

ConfigFile::~ConfigFile() {
	delete mpXMLAttr;
	delete mpServerITA;
}

bool ConfigFile::GetConfigFile(bool isZipped) {

	PRINTDEBUG("GetConfigGile...");

	bool flagAnswer = false;
	bool flagReady = false;
	std::string postURL = "";
	std::string fileName = CONFIG_FILE_PATH;

	if (!isZipped) {
		postURL = "/GetConfigFile.php";
	} else {
		postURL = "/GetArchiveConfigFile.php";
		fileName += ".gz";
	}
	const CURLcode result = mpServerITA->get(postURL);
	//если удачно послали запрос, иначе ошибка авторизации
	if (result == CURLE_OK && mpServerITA->getResponseCode() == 200) {
		mpServerInstance->setNormalMode();
		std::string tmpCfg = mpServerITA->getResponseBuffer();
		if (isZipped && bigbrother::isGzipArchive(tmpCfg)) {
			bigbrother::saveBufferInFile(fileName, tmpCfg);
			if (bigbrother::unzipConfig(fileName) != 0) {
				PRINTDEBUG("Error: can't unpack file");
				return false;
			}
			tmpCfg = bigbrother::readFileInString(CONFIG_FILE_PATH);
		}
		const string Answer = ParseAnswer(tmpCfg, " get config file");
		//если авторизация на сервере удачна, иначе ошибка авторизации
		if (Answer == "OK") {
			bigbrother::saveBufferInFile(CONFIG_FILE_PATH, tmpCfg);
			flagReady = flagAnswer = true;
		} else if (Answer == "8") {
			flagAnswer = true;
		} else {
			PRINTDEBUG2(
					"[GetConfigFile]: Error answer for server in get config file. ",
					mpServerITA->getStrErrorCode())
			flagAnswer = false;
		}
	} else {
		PRINTDEBUG2("[GetConfigFile]: Error in get config file. ",
				mpServerITA->getStrErrorCode());
	}

	if (flagReady) {
		ReadyConfigFile();
	}
	PRINTDEBUG("\n====>>>   GetConfig Ok    <<<<=========\n");
	return flagAnswer;
}

void ConfigFile::ReadyConfigFile() {

	const std::string postURL = "/ReadyConfigFile.php";
	const CURLcode result = mpServerITA->get(postURL);
	if (result == CURLE_OK) { //если удачно послали запрос, иначе ошибка авторизации
		const string Answer = ParseAnswer(mpServerITA->getResponseBuffer(),
				" ready config file");
		if (Answer == "OK") { //если авторизация на сервере удачна, иначе ошибка авторизации
			PRINTDEBUG("ready return\n");
		}
	} else {
		PRINTDEBUG2("Error readiness config file",
				mpServerITA->getStrErrorCode());
	}
}

static void copyConfigs() {
	using namespace bigbrother;

	const std::string cmd = std::string("cp -f ") + CONFIG_FILE_PATH + " "
	+ CONFIG_FILE_MINIMAL_PATH;
	if (bigbrother::chkXmlStruct(CONFIG_FILE_PATH)) {
		const int r = system(cmd.c_str());
		sync();
	}
}

bool ConfigFile::SendModemConfig(bool isZipped) {
	//TODO После реализации на сервере доделать отправку на zip-URL
	PRINTDEBUG("Sending config...");
	const std::string postURL = "/SendModemConfig.php";
	bool flagAnswer = false;
	mResult = mpServerITA->post(postURL, string(CONFIG_FILE_PATH), isZipped);
	if (mResult == CURLE_OK) {
		mpServerInstance->setNormalMode();
		const string Answer = ParseAnswer(mpServerITA->getResponseBuffer(),
				" sending the configuration file to the server");
		if (Answer == "OK") {
			ofstream ConfigFile(CONFIG_FILE_PATH); //открываем файл
			ConfigFile << mpServerITA->getResponseBuffer();
			ConfigFile.flush();
			ConfigFile.close();
			sync();
			copyConfigs();
			flagAnswer = true;
		} else {
			flagAnswer = false;
		}
	} else {
		PRINTDEBUG2(
				"Error! Failed to send the configuration file on the server.:\n",
				mpServerITA->getStrErrorCode())
		flagAnswer = false;	//если не удалось послать запрос
	}

	return flagAnswer;        //если нулевой хендл запроса
}

//void ConfigFile::WriteLocalFileConfig()
//{
//    xmlNodePtr Modem;
//    xmlNodePtr Port;
//    xmlNodePtr DeviceModbus;
//    xmlDocPtr DocPtr;
////пытаемся получить данные для парсинга из памяти
//    DocPtr = xmlParseFile(CONFIG_FILE_PATH);
//    if (DocPtr == NULL) {
//        cerr << "Error detection in write local configuration file\n" << stderr;
//        xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//        xmlCleanupCharEncodingHandlers();
//        xmlCleanupParser();
//        xmlFreeDoc(DocPtr);
//        return;
//    }
////получаем самый верхний елемент
//    Modem = xmlDocGetRootElement(DocPtr);
////проверяем наш элемент или нет, если нет выходим.
//    if (xmlStrcmp(Modem->name, (const xmlChar *) "Modem")) {
//        xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//        xmlCleanupCharEncodingHandlers();
//        xmlCleanupParser();
//        xmlFreeDoc(DocPtr);
//        return;
//    }
//// двигаемся к дочернему елементу
//    Modem = Modem->xmlChildrenNode;
////перебираем все дочерние елементы
//    while (Modem != NULL) {
//        if ((!xmlStrcmp(Modem->name, (const xmlChar *) "Port"))) { //проверяем наш элемент?
//            Port = Modem->xmlChildrenNode;        //двигаемся к дочернему
//            while (Port != NULL) {      //перебираем все порты
//                if ((!xmlStrcmp(Port->name, (const xmlChar *) "DeviceModbus"))
//                        || (!xmlStrcmp(Port->name,
//                                (const xmlChar *) "DeviceRecorder"))) {
//                    DeviceModbus = Port->xmlChildrenNode;
//                    while (DeviceModbus != NULL) { //перебираем все теги в DeviceModbus
//                        //далее если тег совпал вставляем атрибуты из ответа
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterModbusInteger"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterModbusBoolean"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterModbusFloat"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterModbusEnum"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterModbusString"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterSizeFull"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        if ((!xmlStrcmp(DeviceModbus->name,
//                                (const xmlChar *) "ParameterSizeAvailable"))) {
//                            xmlSetProp(DeviceModbus,
//                                    (const xmlChar *) "serverID",
//                                    (const xmlChar *) DeleteParams().c_str());
//                        }
//                        DeviceModbus = DeviceModbus->next;   //cледующий элемент
//                    }
//                }
//                Port = Port->next;        //cледующий элемент
//            }
//        }
//        Modem = Modem->next;        //cледующий элемент
//    }
//    xmlSaveFileEnc(CONFIG_FILE_PATH, DocPtr, "UTF-8"); //сохраняем измененный файл
//    xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//    xmlCleanupCharEncodingHandlers();
//    xmlCleanupParser();
//    xmlFreeDoc(DocPtr);
//}

//string ConfigFile::DeleteParams() {
//	string ParamsId;
//	for (int i = 0; i < IdFromServerToModem.length(); i++) {
//		if (IdFromServerToModem[i] == ' ') {      //если пробел
//			IdFromServerToModem.erase(0, i + 1); //удаляем кол-во занков + пробел
//			return ParamsId;        //возвращаем ID
//		}
//		ParamsId += IdFromServerToModem[i];
//	}
//	return ParamsId;
//}

//void ConfigFile::ParseConfigFile(const string& Answer)
//{
//    xmlNodePtr SendModemConfig = 0;
//    xmlNodePtr Device = 0;
//    xmlDocPtr DocPtr = 0;
//    DocPtr = xmlParseMemory(Answer.c_str(), Answer.size());	//пытаемся получить данные для парсинга из памяти
//    if (DocPtr == NULL) {
//        cerr << "Error detection for parsing configuration file\n" << stderr;
//        xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//        xmlCleanupCharEncodingHandlers();
//        xmlCleanupParser();
//        xmlFreeDoc(DocPtr);
//        return;
//    }
//    SendModemConfig = xmlDocGetRootElement(DocPtr);	//получаем самый верхний елемент
////проверяем наш элемент или нет, если нет выходим.
//    if (xmlStrcmp(SendModemConfig->name, (const xmlChar *) "SendModemConfig")) {
//        xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//        xmlCleanupCharEncodingHandlers();
//        xmlCleanupParser();
//        xmlFreeDoc(DocPtr);
//        return;
//    }
//    SendModemConfig = SendModemConfig->xmlChildrenNode;	// двигаемся к дочернему элементу
//    while (SendModemConfig != NULL) {	//перебираем все дочерние элементы
//        //проверяем наш элемент?
//        if ((!xmlStrcmp(SendModemConfig->name, (const xmlChar *) "Device"))) {
//            Device = SendModemConfig->xmlChildrenNode->xmlChildrenNode;
//            while (Device != NULL) {
//                //проверяем наш элемент?
//                if ((!xmlStrcmp(Device->name, (const xmlChar *) "Parameter"))) {
//                    //пишем в строку ID параметров
//                    mpXMLAttr->CXMLAttr(Device, "Id");
//                    if (mpXMLAttr->IsValid())
//                        IdFromServerToModem += mpXMLAttr->Value() + " ";
//                }
//                Device = Device->next;	//следующий параметр
//            }
//        }
//        SendModemConfig = SendModemConfig->next;//двигаемся к следующему элементу
//    }
//    xmlKeepBlanksDefault(0);
////	xmlDocDumpFormatMemory(DocPtr, &mem, &size, 1);
//    xmlCleanupCharEncodingHandlers();
//    xmlCleanupParser();
//    xmlFreeDoc(DocPtr);
//    WriteLocalFileConfig();
//}

string ConfigFile::ParseAnswer(const string& Answer, const string& Function) {
	xmlNodePtr fail = 0;
	xmlDocPtr DocPtr = 0;
	string Code = "";
	DocPtr = xmlParseMemory(Answer.c_str(), Answer.size());	//пытаемся получить данные для парсинга из памяти
	if (DocPtr == NULL) {
		cerr << "Error parse answer at " << Function << endl << "Answer: "
				<< Answer.c_str() << endl;
		xmlFreeDoc(DocPtr);
		xmlCleanupParser();
		xmlKeepBlanksDefault(0);
		xmlCleanupCharEncodingHandlers();
		return "Error parsing file!\n";
	}
	fail = xmlDocGetRootElement(DocPtr);	//получаем самый верхний елемент
	if ((!xmlStrcmp(fail->name, (const xmlChar *) "fail"))) { //проверяем наш элемент? иначе все ОК
		mpXMLAttr->CXMLAttr(fail, "code");
		Code += "Code = ";
		Code += mpXMLAttr->Value();
		Code += " COntent = ";
		mpXMLAttr->CXMLNode(fail->xmlChildrenNode);
		Code += mpXMLAttr->Value();
		Code += " In ";
		Code += Function;
		cerr << Code << endl;
		Code.clear();
		mpXMLAttr->CXMLAttr(fail, "code");
		xmlFreeDoc(DocPtr);
		xmlKeepBlanksDefault(0);
		xmlCleanupCharEncodingHandlers();
		xmlCleanupParser();
		if (mpXMLAttr->IsValid()) {
			return mpXMLAttr->Value();
		} else
			return "Unknow Error!";
	}
	xmlKeepBlanksDefault(0);
	xmlCleanupCharEncodingHandlers();
	xmlCleanupParser();
	xmlFreeDoc(DocPtr);
	return "OK";
}



