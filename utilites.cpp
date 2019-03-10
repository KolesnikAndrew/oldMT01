/**
 * @file      utilites.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */

#include "utilites.h"
#include <libxml++/libxml++.h>
#include <sys/time.h>
#include <sys/stat.h> // stat
#include <unistd.h>
#include <sys/io.h>  //chsize()
#include <sys/sysinfo.h>
#include <string>
#include <cstring>
#include <set>
#include <dirent.h>
#include <assert.h>
#include <libgen.h>
#include <iostream>
#include <fstream>

const uint32_t Crc32Table[256] = { 0x00000000, 0x77073096, 0xee0e612c,
		0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
		0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d,
		0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
		0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8,
		0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd,
		0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180,
		0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e,
		0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
		0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589,
		0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
		0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1,
		0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf,
		0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
		0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f,
		0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
		0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822,
		0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320,
		0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b,
		0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2, 0x1e01f268,
		0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
		0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43,
		0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
		0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c,
		0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9,
		0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
		0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82,
		0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
		0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd,
		0xf6b9265b, 0x6fb077e1, 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca,
		0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d,
		0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53,
		0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
		0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e,
		0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d };

namespace bigbrother {
std::string getStrHash(const std::string& text) {

	std::string hashed = "";
	EVP_MD_CTX* context = EVP_MD_CTX_create();

	if (context != NULL) {
		if (EVP_DigestInit_ex(context, EVP_md5(), NULL)) {
			if (EVP_DigestUpdate(context, text.c_str(), text.length())) {
				unsigned char hash[EVP_MAX_MD_SIZE];
				unsigned int lengthOfHash = 0;

				if (EVP_DigestFinal_ex(context, hash, &lengthOfHash)) {
					std::stringstream ss;
					for (unsigned int i = 0; i < lengthOfHash; ++i) {
						ss << std::hex << std::setw(2) << std::setfill('0')
								<< (int) hash[i];
					}
					hashed = ss.str();
				}
			}
		}

		EVP_MD_CTX_destroy(context);
	}
	return hashed;
}

std::string getFileHash(std::string fileName) {

	std::ifstream file(fileName.c_str());
	if (!file.is_open()) {
		std::cerr << "ConfigAT27 not found in the path." << std::endl;
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
	file.close();

	return getStrHash(content);

}
void swapBytesInWordsInBuffer(uint8_t *buf, int size) {
	for (int it = 0; it < size - 1; it += 2) {
		std::swap(buf[it], buf[it + 1]);
	}

}

bool ipValidate(const std::string& addr) {
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr));
	return result != 0;
}

std::string readFileInString(const std::string& fileName) {
	std::ifstream file(fileName.c_str());
	if (!file.is_open()) {
		std::cerr << "File not found in the path." << std::endl;
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
	file.close();
	return content;
}
/*
 *  Member header and trailer
 ID1 (IDentification 1)
 ID2 (IDentification 2)
 These have the fixed values ID1 = 31 (0x1f, \037), ID2 = 139 (0x8b, \213), to identify the file as being in gzip format.
 http://www.zlib.org/rfc-gzip.html#file-format
 */

bool isGzipArchive(const std::string& fileName, bool& flagNotExists) {
	std::ifstream file(fileName.c_str(), std::ios_base::binary);
	if (!file.is_open()) {
		std::cerr << "File not found in the path." << std::endl;
		flagNotExists = true;
		return false;
	}
	const unsigned char gzipId[2] = { 0x1f, 0x8b };
	char id[2] = { 0, 0 };
	file.read(id, 2);
	file.close();
	return (gzipId[1] != static_cast<unsigned char>(id[1])
			|| gzipId[0] != static_cast<unsigned char>(id[0])) ? false : true;
}

bool isGzipArchive(const std::string& buffer) {
	if (buffer.size() == 0) {
		return false;
	}
	return (static_cast<unsigned char>(buffer[0]) == 0x1f
			&& static_cast<unsigned char>(buffer[1]) == 0x8b);
}

int unzipConfig(const std::string& fileName) {
	// проверка на gzip
	bool flagNotExist = false;
	if (!isGzipArchive(fileName, flagNotExist)) {
		return -1;
	}
	std::string cmd = std::string("gzip -d -f ") + std::string(fileName);
	int r = system(cmd.c_str());
	return r;
}

std::string toUpperCase(const std::string& str) {
	std::string tmp(str);
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	return tmp;
}

std::string toLowerCase(const std::string& str) {
	std::string tmp(str);
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	return tmp;
}

void setTime() {
	time_t now = time(NULL);
	tm *curTime = localtime(&now);
	if (curTime->tm_year == 70) {
		system("date -s '2017-01-01 00:00'");
	}
}

int getSysYear() {
	time_t now = time(NULL);
	tm *curTime = localtime(&now);
	return curTime->tm_year;
}

std::string timeStamp() {
	//!Берем текущее время
	const uint8_t stampSizeInBytes = 20;
	struct timeval readingTime;
	gettimeofday(&readingTime, NULL);
	char strTmp[stampSizeInBytes];
	struct tm* tm = gmtime(&readingTime.tv_sec);
	sprintf(strTmp, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return std::string(strTmp);
}

bool isSubstrInStr(const std::string& str, const std::string& sub_str) {

	return (str.find(sub_str) == std::string::npos ? false : true);
}

uint32_t crc32Pice(const char *vPnt, int sz, unsigned int CrcPrev) {
	int c;
	const char *cPnt;

	cPnt = vPnt;
	while (sz--) {
		c = *cPnt++;
		CrcPrev = ((CrcPrev >> 8) & 0x00FFFFFF)
				^ Crc32Table[(CrcPrev ^ c) & 0xFF];
	}
	return CrcPrev;
}

uint32_t getCrc32(const char *vPnt, int sz) {
	return crc32Pice(vPnt, sz, 0xFFFFFFFF) ^ 0xFFFFFFFF;
}

//long getFileSize(const std::string &fileName)
//{
//    std::ifstream file(fileName.c_str(),
//            std::ifstream::in | std::ifstream::binary);
//
//    if (!file.is_open()) {
//        return -1;
//    }
//
//    file.seekg(0, std::ios::end);
//    long fileSize = file.tellg();
//    file.close();
//
//    return fileSize;
//}

long getFileSize(const std::string &fileName) {
	struct stat buf = { };
	int r = stat(fileName.c_str(), &buf);
	return r == 0 ? buf.st_size : -1;
}

bool isFileExist(const char* fname) {
	struct stat buf = { };
	int r = stat(fname, &buf);
	return r == 0 ? true : false;
}

bool chkXmlStruct(const char* fname) {
	xmlpp::DomParser tmpConfigFileParser;
	try {
		tmpConfigFileParser.set_substitute_entities(true);
		tmpConfigFileParser.parse_file(fname);

	} catch (const std::exception& ex) {
		std::cerr << "chkXmlStruct exception: " << ex.what() << std::endl;
		return false;
	}

	if (!tmpConfigFileParser) {
		std::cerr << "chkXmlStruct exception: Parser didn't work" << std::endl;
		return false;
	}

	return true;
}
// mt01 date command date -s "2018-04-03 11:23:55"

bool setSysTimeUTC(const std::string& tm) {
	int r = system(std::string("date -s \"" + tm + "\"").c_str());
	return r = 0 ? true : false;
}

void saveBufferInFile(const std::string& fname, const std::string& buf) {
	std::ofstream ConfigFile(fname.c_str()); //открываем файл
	if (ConfigFile.is_open()) {
		ConfigFile << buf;
		ConfigFile.flush();
		ConfigFile.close();
	}
	sync();
}

void print_buf(uint8_t *buffer, int length) {
	for (int i = 0; i < length; ++i) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex
				<< (int) (buffer[i]) << " ";
	}
	std::cout << std::dec << std::endl;
}

void makeLocalMode(const std::string& srvAddress) {

	int r;
	// rename scripts
	r =
			system(
					"if [ -f /etc/init.d/S60openvpn ] ; then mv /etc/init.d/S60openvpn /etc/init.d/_S60openvpn ; fi");
	r =
			system(
					"if [ -f /etc/init.d/S80tftpd-hpa ] ; then mv /etc/init.d/S80tftpd-hpa /etc/init.d/_S80tftpd-hpa ; fi");
	r =
			system(
					"if [ -f /etc/init.d/S95modem_connect ] ; then mv /etc/init.d/S95modem_connect /etc/init.d/_S95modem_connect ; fi");
	r =
			system(
					"if [ -f /etc/init.d/S98restart_network ] ; then mv /etc/init.d/S98restart_network /etc/init.d/_S98restart_network ;  fi");
	// change scripts
	r =
			system(
					"if [ -f /etc/root.local ] ; then cp -f /etc/root.local /etc/root ; fi");
	r =
			system(
					"if [ -f /etc/hosts.standard ] ; then cp -f /etc/hosts.standard /etc/hosts && sync ; fi");

	std::fstream hosts("/etc/hosts", std::ios_base::out | std::ios_base::app);
	if (hosts.is_open()) {
		hosts << srvAddress << "\tpool.ntp.org" << std::flush << std::endl;
		hosts.close();
	}
	system("sync");
}

void makeStandardMode() {
	int r;
	r =
			system(
					"if [ -f /etc/init.d/_S60openvpn ] ; then mv /etc/init.d/_S60openvpn /etc/init.d/S60openvpn ; fi");
	r =
			system(
					"if [ -f /etc/init.d/_S80tftpd-hpa ] ; then mv /etc/init.d/_S80tftpd-hpa /etc/init.d/S80tftpd-hpa ; fi");
	r =
			system(
					"if [ -f /etc/init.d/_S95modem_connect ] ; then mv /etc/init.d/_S95modem_connect /etc/init.d/S95modem_connect ; fi");
	r =
			system(
					"if [ -f /etc/init.d/_S98restart_network ] ; then mv /etc/init.d/_S98restart_network /etc/init.d/S98restart_network ; fi");
	////////
	r =
			system(
					"if [ -f /etc/root.standard ] ;  then cp -f /etc/root.standard /etc/root ; fi");
	r =
			system(
					"if [ -f /etc/hosts.standard ] ; then cp -f /etc/hosts.standard /etc/hosts && sync ; fi");
}

bool chkLocalMode() {
	return (isFileExist("/etc/init.d/_S95modem_connect"));
}

std::string getIpFromHostname(const std::string& hostName) {
	std::string ip_addr(hostName);
	if (isSubstrInStr(hostName, "http://")) {
		ip_addr.erase(ip_addr.begin(), ip_addr.begin() + 7);
	} else if (isSubstrInStr(hostName, "https://")) {
		ip_addr.erase(ip_addr.begin(), ip_addr.begin() + 8);
	}
	size_t pos = 0;
	if ((pos = ip_addr.find(":")) != std::string::npos) {
		ip_addr.erase(ip_addr.begin() + pos, ip_addr.end());
	}
	if ((pos = ip_addr.find("/")) != std::string::npos) {
		ip_addr.erase(ip_addr.begin() + pos, ip_addr.end());
	}
	return ip_addr;
}

bool isCommaSeparator(const std::string & num) {
	return num.find(",") == std::string::npos ? false : true;
}

bool isDotSeparator(const std::string & num) {
	return num.find(".") == std::string::npos ? false : true;
}

std::string replaceCommaWithDot(const std::string & num) {
	// comma is separator
	std::string tmp(num);
	if (isCommaSeparator(num)) {
		size_t pos = tmp.find(",");
		if (pos != std::string::npos) {
			tmp.replace(pos, 1, ".");
		}
	}
	return tmp;
}

void swapBytesInIeeeSingleRead(uint8_t* word) {
	std::swap(*word, *(word + 3));
	std::swap(*(word + 1), *(word + 2));
}

void swapBytesInIeeeSingleWrite(uint8_t* word) {
	std::swap(*word, *(word + 1));
	std::swap(*(word + 2), *(word + 3));
}

void printFloatAsChar(float f) {
	union {
		unsigned char c[sizeof(float)];
		float f;
	} tmp;
	tmp.f = f;
	for (int i = 0; i < sizeof(float); ++i) {
		std::cout << (i == 0 ? "[debug]: " : "") << std::hex << std::setw(2)
				<< std::setfill('0') << (int16_t) tmp.c[i]
				<< (i < sizeof(float) - 1 ? ":" : "\n");
	}

}
//
time_t cvrtStrToUnixTime(const std::string& strTimr) {

	struct tm tm = { 0 };
	int n = 0;
	sscanf(strTimr.c_str(), "%d-%d-%d %d:%d:%d %n", &tm.tm_year, &tm.tm_mon,
			&tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &n);
	if (n == 0 || strTimr.c_str()[n]) {
		return (time_t) -1;
	}
	tm.tm_isdst = -1;
	tm.tm_mon--;
	if (tm.tm_year >= 0 && tm.tm_year < 100) {
		tm.tm_year += 2000;
	}
	tm.tm_year -= 1900; // Years since 1900
	time_t t = mktime(&tm);
	return t;
}

std::string cvrtUnixTimeToStr(const time_t tm_) {
	struct tm *tm = gmtime(&tm_);
	const uint8_t stampSizeInBytes = 20;
	char strTmp[stampSizeInBytes];
	sprintf(strTmp, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return strTmp;
}

std::string getLastestFileNameInDir(const std::string& dirName) {
	struct dirent *de;
	DIR *dr = opendir(dirName.c_str());
	if (dr == NULL) {
		//std::cerr << "[debug] " << dirName << " is empty." << std::endl;
		return "";
	}
	std::set<std::string> ls;
	while ((de = readdir(dr)) != NULL) {
		ls.insert(de->d_name);
	}
	closedir(dr);
	return *(ls.begin());
}

std::set<std::string> getDirList(const std::string& dirName) {
	struct dirent *de;
	DIR *dr = opendir(dirName.c_str());
	std::set<std::string> ls;
	if (dr == NULL) {
		return ls;
	}
	while ((de = readdir(dr)) != NULL) {
		std::string tmp = de->d_name;
		if (tmp != "." && tmp != "..")
			ls.insert(tmp);
	}
	closedir(dr);
	return ls;
}

void makeDir(const std::string& dirName) {
	mkdir(dirName.c_str(), ACCESSPERMS);
}

void configNetworkInterface(const std::string& iface, const std::string& ip,
		const std::string& nm) {

	const std::string cmdDown = "ifconfig " + iface + " down";
	const std::string cmdUP = "ifconfig -a " + iface + " add " + ip
			+ " netmask " + nm + " up ";
	int r = system(cmdUP.c_str());
	std::cout << "Secondary IP was setupped." << std::endl;
}

int mkpath(char* file_path, mode_t mode) {
	assert(file_path && *file_path);
	char* p;
	for (p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		if (mkdir(file_path, mode) == -1) {
			if (errno != EEXIST) {
				*p = '/';
				return -1;
			}
		}
		*p = '/';
	}
	return 0;
}

int mkpath_p(const char *dir, mode_t mode) {
	struct stat sb;
	if (!dir) {
		errno = EINVAL;
		return 1;
	}
	if (!stat(dir, &sb))
		return 0;
	mkpath_p(dirname(strdupa(dir)), mode); // память чистится автоматически
	return mkdir(dir, mode);
}

std::string ip_set(const std::string &ip, const std::string &def_ip) {
	if (!ip.empty() && ipValidate(ip))
		return ip;
	else
		return def_ip;
}

int removeFile(const std::string& filename) {
	int r = remove(filename.c_str());
	return r;
}

bool isBigEndianPlatform() {
	union {
		uint32_t i;
		char c[4];
	} data = { 0x01020304 };

	return data.c[0] == 0x01;
}

bool isLittleEndianPlatform() {
	union {
		uint32_t i;
		char c[4];
	} data = { 0x01020304 };

	return data.c[0] == 0x04;
}

void RemoveLinesFromFile(const std::string& filename) {
	const char *tmpFile = "/tmp/MT01/TempArchiveFile.xml";
	const size_t sz = bigbrother::getFileSize(filename);
	if (sz > 1000) { //если файл больше 1kб
		std::ifstream ifs(filename); //Файловый поток для чтения
		std::ofstream ofs(tmpFile); //Файловый поток для записи
		if (!ifs.is_open() || !ofs.is_open()) {
			std::cerr << "Error while reduce files" << std::endl;
			return;
		}
		ifs.seekg(sz * 0.2);
		std::string sLine;
		getline(ifs, sLine);		//исключаем первую строку. Она битая.
		sLine.clear();
		while (!ifs.eof()) {	//Читаем поток пока не достигнем конца
			getline(ifs, sLine);
			if (sLine == "") {
				break;
			}
			ofs << sLine;
			ofs << std::endl;
		}
		ifs.close();
		ofs.close();
		int r = rename("/tmp/MT01/TempArchiveFile.xml", filename.c_str());
	}
}
//TODO исправить код?
size_t getFreeSystemMemory() {
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if (error == 0) {
		return (s_info.freeram);
	} else {
		return 0; //?
	}
}

int64_t getUnixTimeStamp() {
	return static_cast<int64_t>(std::time(nullptr));
}


uint16_t CalcCRC16_tdrive(const uint8_t* buf, size_t size) {
	uint16_t CRC1 = 0xffff;
	uint16_t temp;
	uint16_t rr;
	for (; size > 0; size--) {
		rr = *buf++;
		CRC1 ^= rr;
		for (uint8_t j = 0; j < 8; j++) {
			if (CRC1 & 0x0001) {
				temp = 1;
			} else {
				temp = 0;
			}
			CRC1 = CRC1 >> 1;
			if (temp) {
				CRC1 ^= 0xA001;
			}
		}
	}
	temp = CRC1;
	CRC1 = CRC1 << 8;
	temp = temp >> 8;
	temp &= 0x00FF;
	CRC1 |= temp;
	return CRC1 & 0xFFFF;
}


//std::string utf8ToString(const char *utf8str, const std::locale& loc) {
//	using namespace std;
//	wstring_convert<codecvt_utf8<wchar_t>> wconv;
//	wstring wstr = wconv.from_bytes(utf8str);
//	vector<char> buf(wstr.size());
//	use_facet<ctype<wchar_t>>(loc).narrow(wstr.data(),
//			wstr.data() + wstr.size(), '?', buf.data());
//	return string(buf.data(), buf.size());
//}

} // end of bigbrother namespace

