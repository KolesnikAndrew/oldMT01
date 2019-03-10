/**
 * @file      utilites.h
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */
#ifndef UTILITES_H_
#define UTILITES_H_

#include <openssl/evp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <stdint.h>
#include <algorithm>
#include <locale>
#include <arpa/inet.h>
#include <semaphore.h>
#include <set>
#include <sys/stat.h>
#include <map>
#include <locale>
//#include <codecvt>

#include "csv_reader.h"

/**
 * @namespace bigbrother
 * @details Специфичные для проекта функции
 */
namespace bigbrother {
/**
 * @fn GetStrHash
 * @brief Возвращает md5 сумму содержимого строки
 * @param text - строка
 * @return MD5 сумму
 */
std::string getStrHash(const std::string& text);
/**
 * @fn GetFileHash
 * @brief Возвращает md5 сумму содержимого файлы
 * @param fileName имя файла
 * @return MD5 сумму
 */
std::string getFileHash(std::string fileName);
/**
 * @fn swapBytesInWordsInBuffer
 * @details менят местами соседние байты
 * @param buf буфер (массив)
 * @param size размер буфера
 */
void swapBytesInWordsInBuffer(uint8_t *buf, int size);

/**
 * @fn ip_validate
 * @brief Валидация ip-адреса
 * ( https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c)
 * @param addr
 * @return true, если адрес соотвествует шаблону
 */
bool ipValidate(const std::string& addr);

/**
 * @brief Считывает файл в строку
 * @param fileName имя файла
 * @return
 */
std::string readFileInString(const std::string & fileName);
/**
 *
 * @param fileName
 * @return
 */
bool isGzipArchive(const std::string& fileName, bool& flagNotExists);
/**
 *
 * @param buffer
 * @return
 */
bool isGzipArchive(const std::string& buffer);
/**
 *
 * @param fileName
 * @return
 */
int unzipConfig(const std::string& fileName);
/**
 *
 * @param str
 * @return
 */
std::string toUpperCase(const std::string& str);
/**
 *
 * @param str
 * @return
 */
std::string toLowerCase(const std::string& str);
/**
 * @brief Задает время, если 1970 -> 2017.01.01
 */
void setTime();
/**
 * @brief Временной штамп в формате
 * @return
 */
std::string timeStamp();
/**
 * @brief
 * @param str
 * @param sub_str
 * @return
 */
bool isSubstrInStr(const std::string& str, const std::string& sub_str);
/**
 * @brief Расчет CRC для фрагмента
 * @param vPnt
 * @param sz
 * @param CrcPrev
 * @return
 */
uint32_t crc32Pice(const char *vPnt, int sz, unsigned int CrcPrev);
/**
 * @brief
 * @param vPnt
 * @param sz
 * @return
 */
uint32_t getCrc32(const char *vPnt, int sz);
/**
 * @brief Вычисление размера файла
 * @param fileName
 */
long getFileSize(const std::string &fileName);
/**
 * @brief Проверка существования файла
 * @param fname
 * @return
 */
bool isFileExist(const char* fname);
/**
 * @brief Проверка валидности XML-структуры
 * @return
 */
bool chkXmlStruct(const char* fname);
/**
 * @brief установка системного времени по UTC
 * @param tm строка в формате "YYYY-MM-DD HH:MM:SS"
 */
bool setSysTimeUTC(const std::string &tm);
/**
 * @brief Возвращает текущий год
 */
int getSysYear();

/**
 * @brief Save buffer in file
 * @param fname - file name
 * @paran buf - string buffer
 */
void saveBufferInFile(const std::string& fname, const std::string& buf);
/**
 * @fn print_buf
 */
void print_buf(uint8_t *buffer, int length);

/**
 * @fn makeLocalMode()
 * @brief корректировка скриптов запуска для работы в режиме лакльной АРМ
 */
void makeLocalMode(const std::string &srvAddress);
/**
 * @fn makeStandardMode()
 * @brief корректировка скриптов запуска для работы в обычном режиме
 */
void makeStandardMode();
/**
 * @fn chkLocalMode()
 */
bool chkLocalMode();
/**
 * @fn getIpFromHostname
 * @brief Выделям адрес (имя) хоста из URL
 */
std::string getIpFromHostname(const std::string & hostName);
/**
 * @fn replaceCommaWithDot
 * @brief replace a comma in a number with a dot
 */
std::string replaceCommaWithDot(const std::string & num);
/**
 * @fn isCommaSeparator
 * @brief check if a comma is a separator
 */
bool isCommaSeparator(const std::string & num);
/**
 * @fn isDotSeparator
 * @brief check if a dot is a separator
 */
bool isDotSeparator(const std::string & num);
/**
 * @fn isStrIsNumber
 * @brief check if a str is a number
 */
template<typename T>
bool isStrIsNumber(const std::string & num) {
	T value = T();
	return !((std::stringstream(num) >> std::noskipws >> value).rdstate()
			^ std::ios_base::eofbit);
}
/**
 * @fn convert string to number
 * @brief
 */
template<typename T>
T cvrtStrToNumber(const std::string & num) {
	T value = T();
	if (isStrIsNumber<T>(num)) {
		std::stringstream(num) >> value;
		return value;
	}
	return 0;
}

/**
 * @brief заменяет порядок следования байт в фрпмате ИЕЕЕ754 Сингл (ШЛБ): 3412 -> 1234
 * @fn swapBytesInSingle
 * @param pointer to the byte array
 */
void swapBytesInIeeeSingleRead(uint8_t* word);
/**
 *
 * @param word
 */
void swapBytesInIeeeSingleWrite(uint8_t* word);
/**
 *
 * @param data
 */
void printFloatAsChar(float data);
/***
 *
 * @param strTimr
 * @return
 */
time_t cvrtStrToUnixTime(const std::string& strTimr);
/**
 *
 * @param tm_
 * @return
 */
std::string cvrtUnixTimeToStr(const time_t tm_);
/**
 *
 * @param dirname
 * @return
 */
std::string getLastestFileNameInDir(const std::string&);
/**
 *
 * @param dirname
 * @return
 */
std::set<std::string> getDirList(const std::string&);
/**
 * @fn makeDir
 * @param dirName
 * @brief Создает катклог, если его нет
 */
void makeDir(const std::string& dirName);
/**
 *
 * @param iface
 * @param ip
 * @param nm
 */
void configNetworkInterface(const std::string& iface, const std::string& ip,
		const std::string& nm);
/**
 * @brief mkdir -p implementation ()
 * @details see on https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
 */
int mkpath(char* file_path, mode_t mode);
/**
 * @brief Рекурсивное создание дерева каталогов
 * @details см. https://github.com/troglobit/libite/blob/master/src/makepath.c
 * @param dir
 * @param mode
 * @return
 */
int mkpath_p(const char *dir, mode_t mode);
/**
 * @brief ip_set
 * @param ip
 * @param def_ip
 * @return string ip
 */
std::string ip_set(const std::string &ip, const std::string &def_ip);
/**
 * @fn removeFile
 * @param filename
 * @return
 */
int removeFile(const std::string& filename);

/**
 * @fn loadCSV
 * @param fname
 * @param res_data
 * @return true when procees is ok
 */
template<typename T, typename U>
bool loadCSV(const std::string fname, std::map<T, U>& res_data) {
	using namespace aria::csv;
	try {
		std::ifstream csv(fname);
		CsvParser parser(csv);
		parser.delimiter(';');
		for (const auto &row : parser) {
			if (!isStrIsNumber<long>(row[0])) {
				continue;
			}
			res_data.insert(
					std::pair<T, U>(std::stol(row[0]), std::stol(row[1])));
		}
		csv.close();
	} catch (...) {
		std::cerr << "Exception while load CSV" << std::endl;
		res_data.clear();
		return false;
	}
	return true;
}

/**
 * @fn convertAnyToUCharArray
 * @param value
 * @return any type converted to byte array
 */
template<typename T>
std::vector<uint8_t> convertAnyToUCharArray(T value) {
	union {
		uint8_t tmp[sizeof(T)];
		T value;
	} cvrt;
	cvrt.value = value;
	return std::vector<uint8_t>(cvrt.tmp, cvrt.tmp + sizeof(T));
}

/**
 * @fn convertAnyToUCharArrayReverse
 * @param value
 * @return any type converted to byte array
 */
template<typename T>
std::vector<uint8_t> convertAnyToUCharArrayReverse(T value) {
	union {
		uint8_t tmp[sizeof(T)];
		T value;
	} cvrt;
	cvrt.value = value;
	std::vector<uint8_t> v(cvrt.tmp, cvrt.tmp + sizeof(T));
	std::reverse(v.begin(), v.end());
	return v;
}
/**
 * @fn isBigEndianPlatform
 * @return return
 */
bool isBigEndianPlatform(void);
/**
 * @fn emoveLinesFromFile
 * @param filename
 */
void RemoveLinesFromFile(const std::string& filename);

/**
 * @fn getFreeSystemMemory
 * @return volue of the system memory
 */
size_t getFreeSystemMemory();
/**
 * @brief Get Unix time stamp
 * @return unix time stamp
 */
int64_t getUnixTimeStamp();
///**
// * @brief only with compiler with full C++11 support (gcc > 5)
// * @fn utf8_to_string
// * @param utf8str
// * @param loc, default set to ru_RU.1251
// * @return
// */
//std::string utf8ToString(const char *utf8str, const std::locale& loc=std::locale("ru_RU.1251"));

template<typename T>
void debug_print_vector(const std::vector<T>& v, std::ostream& stream =	std::cout) {
	for (auto& elem : v) {
		stream << elem << " ";
	}
	stream << std::endl;
}
/**
 * "
 * @param buf
 * @param size
 * @return
 */
uint16_t CalcCRC16_tdrive(const uint8_t* buf, size_t size);


/**
 * @fn convertAnyToUCharArrayReverse
 * @param value
 * @return any type converted to byte array
 */
template<typename T>
T convertUCharArrayToNumber(uint8_t* buf) {
	union {
		uint8_t tmp[sizeof(T)];
		T value;
	} cvrt;
	memcpy(cvrt.tmp, buf, sizeof(T));
	return cvrt.value;
}

//////////////////////////////////

}//end of bigbrother namespace

#endif /* UTILITES_H_ */
