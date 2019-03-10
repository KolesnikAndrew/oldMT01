/*
 * DataTransfer.cpp
 *
 *  Created on: 12 сент. 2014
 *      Author: rtem
 */

#include <sstream>
#include <cstdlib>
#include <iterator>
#include <string>
#include "globals.h"
#include "utilites.h"

#include "DataTransfer.h"

// redefine from 700
#define MAX_STRING_COUNT 250
#define MAX_FREE_AVAILABLE_MEMORY 20000000

DataTransfer::DataTransfer(V7Server* server) :
		tm(NULL), mMemoryPageSize(0), mFlagReadyFile(false), mpServer(server), XMLAttr(
				new XMLAttribute()), mNeedWaitResponce(false) {
	bigbrother::removeFile(CURRENT_DATA_FILE_PATH);
	bigbrother::removeFile(SETPOINT_CONFIRM_FILE_PATH);
	if ((mkfifo( CURRENT_DATA_FILE_PATH, 0777))
			|| (mkfifo( SETPOINT_CONFIRM_FILE_PATH, 0777))) {
		cerr << "Can't create named pipe " << endl;
		return;
	}
}

DataTransfer::~DataTransfer() {
	bigbrother::removeFile(CURRENT_DATA_FILE_PATH);
	bigbrother::removeFile(SETPOINT_DATA_FILE_PATH);
	if (XMLAttr) {
		delete XMLAttr;
	}
}
#ifdef DEBUG
static void copyConfirmFile(int rec_cnt) {
	static int i = 0;
	if (rec_cnt != 0) { //если ни 1 параметра не поместили в файл
		string cp_command =
				string("cp ") + string(F_CONFIRM_INPUT_DATA_PROCESS) + "  "
						+ string(F_CONFIRM_INPUT_DATA_PROCESS) + string("_")
						+ (static_cast<ostringstream*>(&(ostringstream()
								<< (++i)))->str());
		int r = system(cp_command.c_str());
	}
}
#endif

umkaFile_type DataTransfer::DTConfirmInputDataProcess() {
	const int maxLineCnt = 250;
	ofstream FileConfirmProcess;
	FileConfirmProcess.open(F_CONFIRM_INPUT_DATA_PROCESS, fstream::out); //открываем файл
	FileConfirmProcess
			<< "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
			<< endl;
	FileConfirmProcess << "<ConfirmInputDataProcess>" << endl;
	confirmData_type ConfDT; //данные для чтения/записи уставок
	umkaFile_type umkaFileStaus; //данные для чтения/записи уставок
	umkaFileStaus.umkaFileStatus = fileStatus_type::unknown;
	int rec_count = 0; //кол-во строк
	bool flgReadyFile = false;
	std::ostringstream lastFileReadState;
	std::ostringstream lastFileWriteState;
	do {
		if (mpServer->GetDataFromSetpointConfirmBuffer(ConfDT)) {
			++rec_count;
			FileConfirmProcess << "<Parameter Id = '" << ConfDT.globalID
					<< "'>";
			FileConfirmProcess << "<State DT = '" << ConfDT.dateTime << "'>";
			FileConfirmProcess << static_cast<int>(ConfDT.state)
					<< "</State></Parameter>" << endl;
		} else if (mpServer->GetDataFromFileBuffer(umkaFileStaus)) {
			lastFileWriteState.str("");
			lastFileReadState.str("");
			rec_count = rec_count > 0 ? rec_count - 1 : 0;
			if (umkaFileStaus.isWriteMode) {
				++rec_count;
				lastFileWriteState << "<DeviceConfig ";
				lastFileWriteState << "Number='" << umkaFileStaus.globalID
						<< "' ";
				lastFileWriteState << "deviceLoadState='"
						<< static_cast<int>(umkaFileStaus.umkaFileStatus)
						<< "'>";
				lastFileWriteState << "</DeviceConfig>" << endl;
			} else {
				++rec_count;
				if (umkaFileStaus.umkaFileStatus
						== fileStatus_type::ready_file) {
					flgReadyFile = true;
				}

				lastFileReadState << "<Journal ";
				lastFileReadState << "Id='" << umkaFileStaus.globalID << "' ";
				lastFileReadState << "fileSize='" << umkaFileStaus.cur_fileSize
						<< "' ";

				lastFileReadState << "journalState='"
						<< static_cast<int>(umkaFileStaus.umkaFileStatus)
						<< "' ";

				lastFileReadState << "validState='"
						<< static_cast<int>(umkaFileStaus.validState) << "'>";
				lastFileReadState << "</Journal>" << endl;

			}
			if (rec_count > MAX_STRING_COUNT) {
				break;
			}
		} else { // иначе беру из архива
			if (!bigbrother::isFileExist(F_ARCHIVES_CONFIRM)
					|| bigbrother::getFileSize(F_ARCHIVES_CONFIRM) == 0) {
				break;
			}
			ifstream fSN(F_ARCHIVES_CONFIRM);
			int ArchivesCount = 0;
			while (!fSN.eof()) {
				string str;
				getline(fSN, str);
				FileConfirmProcess.write(str.c_str(), str.size());
				FileConfirmProcess << endl;
				++ArchivesCount;
				++rec_count;
				if (rec_count >= MAX_STRING_COUNT + 300) { //если максимальное кол-во строк
					break;
				}
			}
			RemoveLines(F_ARCHIVES_CONFIRM, ArchivesCount); //удаляю строки, которые взял
			break;
		}

	} while (true);
	if (!lastFileWriteState.str().empty()) {
		FileConfirmProcess << lastFileWriteState.str();
	}
	if (!lastFileReadState.str().empty()) {
		FileConfirmProcess << lastFileReadState.str();
	}
	FileConfirmProcess << "</ConfirmInputDataProcess>";
	FileConfirmProcess.close(); //закрываем файл

	if (rec_count == 0) { //если ни 1 параметра не поместили в файл
		remove(F_CONFIRM_INPUT_DATA_PROCESS);
	}

	if (flgReadyFile) {
		umkaFileStaus.umkaFileStatus = fileStatus_type::ready_file; //принудительно устанавливаем статус готовности файла
	};

//#ifdef DEBUG
//	copyConfirmFile(rec_count);
//#endif

	return umkaFileStaus;
}

// извлекаем данные
void DataTransfer::PrepareRegularData(std::ostringstream& data) {
	writeSendValueFile(data);
}

void DataTransfer::PrepareByteArrayData(outputData_type& data) {
	writeByteArrayFile(data.globalID, data.dateTime, data.value);

}
//вычитываем архив и отдаем данные
void DataTransfer::PrepareArchiveRegularData(std::ostringstream& data) {
	ifstream fSN(F_ARCHIVE_SEND_VALUES);
	int rec_count(0);
	string str;
	while (getline(fSN, str) && ++rec_count < MAX_STRING_COUNT + 1) {
		data.write(str.c_str(), str.size());
		data << endl;
	}
	RemoveLines(F_ARCHIVE_SEND_VALUES, rec_count); //удаляю строки, которые взял
}

void DataTransfer::writeByteArrayFile(const int paramId,
		const std::string& tSatamp, const std::string& data) {
	const std::string bFName = std::string(TMP_DIR_MT01_DYN) + "/"
			+ std::to_string(bigbrother::cvrtStrToUnixTime(tSatamp)) + "_"
			+ std::to_string(paramId);
	std::ofstream bFile(bFName, std::ios_base::binary | std::ios_base::app);
	if (!bFile.is_open()) {
		cerr << "can't open " << bFName << endl;
	}
	//long ts = atol(to_string(bigbrother::cvrtStrToUnixTime(tSatamp)).c_str());
	//bFile.write(reinterpret_cast<char*>(&ts), sizeof(long));
	bFile.write(data.c_str(), data.length());
	bFile.close();
}

void DataTransfer::DTPrepareSendArchiveValues() {
	std::ostringstream data;
	if (bigbrother::getFileSize(F_ARCHIVE_SEND_VALUES)
			&& !bigbrother::isFileExist(F_SEND_VALUES_ARCHIVE)) {
		PrepareArchiveRegularData(data);
		writeSendValueFile(data, F_SEND_VALUES_ARCHIVE);
	}

}

void DataTransfer::DTPrepareSendValues() {
	int rec_count = 0;
	outputData_type tmp;
	std::ostringstream tmpValues;
	bool isDataAvailable = false;
	do {
		isDataAvailable = mpServer->GetDataFromCurrentDataBuffer(tmp);
		// заглушка на пустые файлы, нужно найти, где и при каких условиях в очередь попадает пустое значение
		if (isDataAvailable && (tmp.value.length() > 10)
				&& (tmp.paramType == paramType_type::p_byte_array_single
						|| tmp.paramType == paramType_type::p_single_matrix)) {
			PrepareByteArrayData(tmp);
		}

		if (isDataAvailable
				&& (tmp.paramType != paramType_type::p_byte_array_single
						&& tmp.paramType != paramType_type::p_single_matrix)) {

			tmpValues << "<Parameter Id='" << tmp.globalID << "' "
					<< "DateTime='" << tmp.dateTime << "' " << "ms='" << tmp.ms
					<< "' valid_state='" << static_cast<int>(tmp.validState)
					<< "'> " << tmp.value << "</Parameter>" << endl;
			++rec_count;
		}

		sleep(0);
	} while (isDataAvailable && rec_count <= MAX_STRING_COUNT);
	PrepareRegularData(tmpValues);

}

void DataTransfer::RemoveLines(const char *filename, int StrCount) {

	if (!bigbrother::isFileExist(filename)) {
		return;
	}
	ifstream ifs(filename);
	ofstream ofs(F_TEMP_ARCHIVE_FILE); //Файловый поток для записи
	if (!ifs.is_open() || !ofs.is_open()) {
		cerr << "Error open file to read/write " << filename << endl;
		return;
	}
	string tmp;		//Буфер чтения
	for (int j = 0; j <= StrCount; ++j) {
		getline(ifs, tmp);
	}
	while (getline(ifs, tmp)) {
		if (tmp.empty()) {
			continue; // пустые строки
		}
		ofs << tmp << endl;		//пишем строку в ofs
	}
	ifs.close();
	ofs.close();
	rename(F_TEMP_ARCHIVE_FILE, filename);

}

// создаем архивный файл (archives_send_values.xml)
void DataTransfer::writeArchiveFile() {
	ofstream fArchives(F_ARCHIVE_SEND_VALUES, std::ios_base::app);
	ifstream FileSendValues(F_SEND_VALUES);
	if (!fArchives.is_open() || !FileSendValues.is_open()) {
		cerr << "Error opening files. (" << F_ARCHIVE_SEND_VALUES << " or "
				<< F_SEND_VALUES << endl;
		return;
	}
	string str;
	getline(FileSendValues, str);
	getline(FileSendValues, str);
	while (getline(FileSendValues, str)) {
		if ((str == "</SendValues>") || (str == "")) {
			break;      //если закрывающий тег или пустая строка = выйти
		}
		fArchives.write(str.c_str(), str.size());       //пишем в архив
		fArchives << endl;
	}
	fArchives.close();
	FileSendValues.close();

}

void DataTransfer::writeConfirmDataArchive() {
	//Процесс архивирования данных
	ofstream fArchives(F_ARCHIVES_CONFIRM, std::ios_base::app);
	ifstream FileConfirmProcess(F_CONFIRM_INPUT_DATA_PROCESS);
	if (!fArchives.is_open() || !FileConfirmProcess.is_open()) {
		return;
	}
	string str;
	getline(FileConfirmProcess, str);
	getline(FileConfirmProcess, str);
	while (getline(FileConfirmProcess, str)) {
		if (str == "</ConfirmInputDataProcess>" || str.empty()) {
			break;  //если закрывающий тег или пустая строка = выйти
		}
		fArchives.write(str.c_str(), str.size());
		fArchives << endl;
	}
	fArchives.close();
	FileConfirmProcess.close();
	//remove(fConfirmInputDataProcess);
}

bool DataTransfer::writeSendValueFile(const std::ostringstream& data,
		const std::string& fname) {
	//отбрасываем пустые или битые файлы
	if (data.str().length() < 1) {
		return false;
	}
	ofstream FileSendValues(fname); //открываем файл
#ifdef DEBUG
	cout << fname + " created." << endl;
#endif
	if (!FileSendValues.is_open()) {
		cerr << "Error open file to write " << fname << endl;
		return false;
	}
	FileSendValues
			<< "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
			<< endl;
	FileSendValues << "<SendValues>" << endl;
	FileSendValues << data.str();
	FileSendValues << "</SendValues>";
	FileSendValues.close();
	return true;
}

bool DataTransfer::isNeedWaitResponce() {
	return mNeedWaitResponce;
}
