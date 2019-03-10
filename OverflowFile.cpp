/*
 * OverflowFile.cpp
 *
 *  Created on: 31 окт. 2014
 *      Author: rtem
 */

#include "OverflowFile.h"
#include <iostream>
#include "utilites.h"

using namespace std;

#define MAX_SIZE_FILE 10000000

const char *bb_log = "/var/log/MT01/BigBrother.log";

OverflowFile::OverflowFile() {

}

OverflowFile::~OverflowFile() {

}

void OverflowFile::Overflow() {
	using namespace bigbrother;
	long SizeLogFile = 0;
	if (getFileSize(bb_log) >= MAX_SIZE_FILE)
		system("logrotate /etc/logrotate.conf");
	SizeLogFile = getFileSize(F_ARCHIVE_SEND_VALUES)
			+ getFileSize(F_ARCHIVES_CONFIRM); // + getFileSize(F_ARCHIVES_STATUS);
#ifdef DEBUG
	cout <<"SizeLogFile: "<< SizeLogFile << endl;
	cout << "getFreeMem: " << getFreeSystemMemory() << endl;
#endif
	if (SizeLogFile >= (getFreeSystemMemory() * 0.45)) { //если размер файлов больше 45% свободной памяти модема
		RemoveLinesFromFile(F_ARCHIVE_SEND_VALUES);
		RemoveLinesFromFile(F_ARCHIVES_CONFIRM);
		RemoveLinesFromFile(F_ARCHIVES_STATUS);
	}
	sleep (10);
}

