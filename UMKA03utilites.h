/*
 * UMKA03utilites.h
 *
 *  Created on: Dec 15, 2017
 *      Author: Михаил Ткаченко
 */
#include <stdint.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include "utilites.h"
#include "QueryMsg.h"
#include "globals.h"
#include "EngineModbus.h"
#include "port.h"
#include <tgmath.h>
//#include "parametermodbus.h"


#ifndef SRC_UMKA03UTILITES_H_
#define SRC_UMKA03UTILITES_H_

#define	NV_MEM_PO_SIGNATURA		6
#define SZ_HEADER_PAYLOAD       120


enum class fileReadStatus_type{
	    success = 0,
		success_section_reading,
		undefined_error,
		filling_check_error = 4,
		modbus_exchange_error,
		error
	} ;


struct UMKA03utilites {
	struct UM03JRN_Size
	{
		long calc_size;
		long cur_size;
	};
	enum class UM3Journal_type
	{
	    main_journal = 5, /**<  5 -  основной журнал */
	    alarm = 7, /**< 7 - журнал аварий */
	    start_stop /**< 8 - журнал пусков  */
	} ;

	struct JRNHeader_type
			{
				char filename[64];
				char Start_count;
				char Alarm_count;
				char Reserved1[37];
				char TRIOL;
				unsigned int Reserved2;
				unsigned char VSDType;
				unsigned char INDType;
				unsigned short EEPROMSignature;
				unsigned short SoftwareVersion;
				unsigned short NumOfParams;
				unsigned int SizeOfInfoPart;
				unsigned int HeaderCRC;
				unsigned int JournalCRC;
			};

	UMKA03utilites();
	virtual ~UMKA03utilites();
	private:
	int requestPort(QueryMsg * msg, EngineModbusRTU *mpModbusEngine);
	protected:
	UM03JRN_Size SizeForSend;
	uint8_t NumToRead[32];
	unsigned char mN_part;
	unsigned int  mNum;
	unsigned int  mMaxAddrJrn;
	unsigned char mPartsToRead;
	int mEntries;
	uint8_t mFunctionCall;
	unsigned char iterator;
	bool IsFirstReading;
	public:
#ifdef DEBUG_UM
	int FormLongNameFileJrn(char *NamePnt, const char *extension);
	int FormHeaderFile(char *cPnt, const char *extension, int SzPayload, int CrcPayload);
	uint8_t* CheckForSectionFilling(EngineModbusRTU *mpModbusEngine, unsigned short NumOfUMKAPArams);
	void initJournalHeader(const char *Path);
	uint8_t ReadJournal(EngineModbusRTU *mpModbusEngine, const char *Path,volatile UM3Journal_type JournalType, unsigned short NumOfUMKAPArams, unsigned int iteratorMax);
	uint8_t ReadWholeJournal(EngineModbusRTU *mpModbusEngine, const char *Path, unsigned short NumOfUMKAPArams,unsigned int iterator);
	uint8_t WriteJournalHeader(char *Path);
#else
	int FormLongNameFileJrn(char *NamePnt, const char *extension);
	int FormHeaderFile( char *cPnt, const char *extension, int SzPayload, int CrcPayload, unsigned short ParamsNum, unsigned short UMSoftVersion );
	fileReadStatus_type CheckForSectionFilling( V7Port* mpPort, unsigned short NumOfUMKAPArams, uint8_t devAddr);
	void initJournalHeader(const char *Path);
	fileReadStatus_type ReadJournal(V7Port* mpPort, const char *Path, const UM3Journal_type& JournalType, unsigned short NumOfUMKAPArams, unsigned int iteratorMax, uint8_t devAddr);
	fileReadStatus_type ReadWholeJournal(V7Port* mpPort, const char *Path, unsigned short NumOfUMKAPArams, unsigned int iterator, uint8_t devAddr);
	uint8_t WriteJournalHeader(const char *Path, unsigned short NumOfParams, unsigned short UMSoftVersion);
	unsigned short getUMKA03SoftVersion(V7Port* mpPort, uint8_t devAddr);
	long journalSize();
	long currentJournalSize();
	void resetFileSize();
	void setIsFirstReading(bool newvalue);

#endif
};

#endif /* SRC_UMKA03UTILITES_H_ */
