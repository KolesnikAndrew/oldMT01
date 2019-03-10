/*
 * UMKA03utilites.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: root
 */

#include "UMKA03utilites.h"

UMKA03utilites::UMKA03utilites() :
		NumToRead { }, mN_part(0), mNum(0), mMaxAddrJrn(0), mPartsToRead(0), mEntries(
				0), mFunctionCall(0), iterator(0), SizeForSend { 1, 0 }, IsFirstReading(
				true) {
	// TODO Auto-generated constructor stub

}

UMKA03utilites::~UMKA03utilites() {
	// TODO Auto-generated destructor stub
}

int UMKA03utilites::FormLongNameFileJrn(char *NamePnt, const char *extension) {
	//time_type		*tpnt, TimeBff;

//	tpnt = &TimeBff;
	//ch_to_smhdmy(tpnt, UnixTime);	//	êóñò_8416_ñêâàæèíà_0000_16-04-2008_15-14-18.jrn
	return sprintf(NamePnt, extension);
	//êóñò_%04d_ñêâàæèíà_%04d_%02d-%02d-%04d_%02d-%02d-%02d.%s
}

int UMKA03utilites::FormHeaderFile(char *cPnt, const char *extension,
		int SzPayload, int CrcPayload, unsigned short ParamsNum,
		unsigned short UMSoftVersion) {
	int sz, tmp;
	JRNHeader_type *hPnt;
	hPnt = (JRNHeader_type *) cPnt;

	memset(cPnt, 0, sizeof(JRNHeader_type));

	sz = FormLongNameFileJrn(hPnt->filename, extension);
	//hPnt->filename			= "test.jrn";
	hPnt->TRIOL = 0;
	hPnt->SoftwareVersion = UMSoftVersion;
	hPnt->EEPROMSignature = NV_MEM_PO_SIGNATURA;
	hPnt->NumOfParams = ParamsNum;
	hPnt->SizeOfInfoPart = SzPayload;	//размер
	hPnt->JournalCRC = CrcPayload;
	hPnt->INDType = 0;
	/*
	 if (memScope.maskPuskGrfValid == 0xFFFF)
	 tmp = 16;
	 else if (memScope.LastPuskGrf == 0xFF)
	 tmp = 0;
	 else if (memScope.maskPuskGrfValid == 0)
	 tmp = 0;
	 else
	 tmp = memScope.LastPuskGrf + 1;
	 */
	hPnt->Start_count = 0;	//(memScope.maskPuskGrfValid != 0)?1:0;
	/*
	 if ((deviceUnderUmka == Ind_deviceUnderUmka)||(deviceUnderUmka == Vent_deviceUnderUmka)){
	 if(memScope.maskLogJrnValid == 0xFFFF)
	 tmp = 16;
	 else if (memScope.LastLogJrn == 0xFF)
	 tmp = 0;
	 else if (memScope.maskLogJrnValid == 0)
	 tmp = 0;
	 else
	 tmp = memScope.LastLogJrn + 1;
	 }else
	 tmp = 0;
	 */
	hPnt->Alarm_count = 0;//((memScope.maskLogJrnValid != 0)&&((deviceUnderUmka == Ind_deviceUnderUmka)||(deviceUnderUmka == Vent_deviceUnderUmka)))?1:0;
	/*
	 if (strcmp(extension, "trn")==0)
	 {
	 hPnt->ID_TMN = ID_TMN;
	 hPnt->ID_TMP = ID_TMP;
	 hPnt->VersPoTMN = VersPoTMN;
	 hPnt->VersPoTMP = VersPoTMP;
	 }
	 */
	hPnt->HeaderCRC = bigbrother::getCrc32(cPnt, SZ_HEADER_PAYLOAD);
//#ifdef DEBUG
	std::cout << hPnt->HeaderCRC << " Journal Header CRC" << std::endl;
//#endif

	return sizeof(JRNHeader_type);
}

#ifdef DEBUG_UM
uint8_t* UMKA03utilites::CheckForSectionFilling(EngineModbusRTU *mpModbusEngine, unsigned short NumOfUMKAPArams)
{
	QueryMsgReadFileRecords *qmFile = new QueryMsgReadFileRecords();
	int UMKAParameterBaseAddr;
	int headerAddr = 0;
	int MaxAddrJrn = 0;

	switch (NumOfUMKAPArams)
	{
		case 1024:
		{
			UMKAParameterBaseAddr = 2 * 528;
			break;
		}
		case 2048:
		{
			UMKAParameterBaseAddr = (int)trunc(((NumOfUMKAPArams*2+112+4+4+4+2+2)/2));//это корявое число взято из кода СПО
			break;
		}
		default:
		{
			std::cout<<"This Number of UMKA params is not permit" << std::endl;
			break;
		}
	}

	qmFile->mbDevAddr = 0x01;
	qmFile->mbRecordNumberLo = 0x05; //тип журнала - основной
	qmFile->mbRecordLengthHi = 0x00;
	qmFile->mbRecordLengthLo = 0x20;

	// = 0x00000420;
	for (int i = 0; i < 32; ++i)
	{
		headerAddr = i*128*528;
		headerAddr+=(UMKAParameterBaseAddr)-32;
		qmFile->mbFileNumberHi = (uint8_t)(headerAddr & 0xFF);
		qmFile->mbFileNumberLo = (uint8_t)((headerAddr>>8) & 0xFF);
		qmFile->mbRecordNumberHi = (uint8_t)((headerAddr>>16) & 0xFF);
		qmFile->makeRawRequest();
		int out = UMKA03utilites::requestPort(qmFile, mpModbusEngine);
#ifdef DEBUG
//		std::cout << "ERR Code:" << out << std::endl;
//		std::cout << *qmFile << std::endl;
#endif
		if (qmFile->checkUM03JournalFilling())
		NumToRead[i] = 1;
	}
	delete(qmFile);
	return &NumToRead[0];
}
#else

unsigned short UMKA03utilites::getUMKA03SoftVersion(V7Port* mpPort,
		uint8_t devAddr) {
	QueryMsg0x2b *qm2b = new QueryMsg0x2b();
	qm2b->mbDevAddr = devAddr;
	qm2b->mbObjectId = 0x00;
	qm2b->makeRawRequest();
	mpPort->request(qm2b);
	unsigned short SoftVersion = qm2b->getUMKA03Id();
	delete (qm2b);
	return SoftVersion;
}

fileReadStatus_type UMKA03utilites::CheckForSectionFilling(V7Port* mpPort,
		unsigned short NumOfUMKAPArams, uint8_t devAddr) {
	QueryMsgReadFileRecords *qmFile = new QueryMsgReadFileRecords();
	int UMKAParameterBaseAddr;
	int headerAddr = 0;
	int MaxAddrJrn = 0;
	uint8_t FilledSections = 0;

	switch (NumOfUMKAPArams) {
	case 1024: {
		UMKAParameterBaseAddr = 2 * 528;
		break;
	}
	case 2048: {
		UMKAParameterBaseAddr = (int) trunc(
				((NumOfUMKAPArams * 2 + 112 + 4 + 4 + 4 + 2 + 2) / 2));	//это корявое число взято из кода СПО
		break;
	}
	default: {
		std::cout << "This Number of UMKA params is not permit" << std::endl;
		break;
	}
	}

	qmFile->mbDevAddr = devAddr;
	qmFile->mbRecordNumberLo = 0x05; //тип журнала - основной
	qmFile->mbRecordLengthHi = 0x00;
	qmFile->mbRecordLengthLo = 0x20;

	// = 0x00000420;
	for (int i = 0; i < 32; ++i) {
		headerAddr = i * 128 * 528;
		headerAddr += (UMKAParameterBaseAddr) - 32;
		qmFile->mbFileNumberHi = (uint8_t) (headerAddr & 0xFF);
		qmFile->mbFileNumberLo = (uint8_t) ((headerAddr >> 8) & 0xFF);
		qmFile->mbRecordNumberHi = (uint8_t) ((headerAddr >> 16) & 0xFF);
		qmFile->makeRawRequest();
		int out = mpPort->request(qmFile);
		//  PRINTDEBUG2("Modbus Status ", out);
		if (out != 0) {
			for (uint8_t ds = 0; i < 32; i++) {
				NumToRead[ds] = 0;
			}
			return (fileReadStatus_type::filling_check_error);
		}
#ifdef DEBUG
		//std::cout << "ERR Code:" << out << std::endl;
		//std::cout << *qmFile << std::endl;
#endif
		if (qmFile->checkUM03JournalFilling()) {
			NumToRead[i] = 1;
			FilledSections++;
		}
	}
//	SizeForSend.calcsize = (FilledSections*128*528)+67584+135168;
	//PRINTDEBUG2("\n\n\n CALC JRN SIZE", SizeForSend.calcsize);
	//PRINTDEBUG2("\n\n\n FilledSections ", (int)FilledSections);
	FilledSections = 0;
	delete (qmFile);
	return (fileReadStatus_type::success);
}
#endif

long UMKA03utilites::journalSize() {
	return SizeForSend.calc_size;
}
long UMKA03utilites::currentJournalSize() {
	return SizeForSend.cur_size;
}

void UMKA03utilites::resetFileSize() {
	SizeForSend.cur_size = 0;
	SizeForSend.calc_size = 0;
}

void UMKA03utilites::initJournalHeader(const char *Path) //create empty header
		{
	unsigned char zero[128] = { 0x00 };
	unsigned char *ze = &zero[0];
	std::ofstream f(Path, std::ios_base::binary);
	f.write(reinterpret_cast<char*>(ze),
			sizeof(UMKA03utilites::JRNHeader_type));
	f.close();
	return;
}

void UMKA03utilites::setIsFirstReading(bool newvalue) {
	IsFirstReading = newvalue;
}
#ifdef DEBUG_UM
uint8_t UMKA03utilites::ReadJournal(EngineModbusRTU *mpModbusEngine, const char *Path, volatile UM3Journal_type JournalType, unsigned short NumOfUMKAPArams, unsigned int iteratorMax)
{
	/****************************************************************************************************/ // MAIN Journal Reading As SPO Algo
	//Journal Reading As SPO Algo
	//for (int i = 0; i < 32; ++i)
	QueryMsgReadFileRecords *qmFile = new QueryMsgReadFileRecords();
	std::ofstream f(Path, std::ios::binary|std::ios::app);
	unsigned char DI;
	const unsigned char MaxByteOne = 110;
	unsigned char NumOfBlocks = 0;
	unsigned char BlockSize = 0;

	//uint8_t* NumToRead = 0;
	qmFile->mbDevAddr = 0x01;
	switch(JournalType)
	{
		case UM3Journal_type::main_journal:
		{
			qmFile->mbRecordNumberLo = main_journal;
			NumOfBlocks = 32;
			BlockSize = 128;
			break;
		}
		case UM3Journal_type::start_stop:
		{
			qmFile->mbRecordNumberLo = start_stop;
			NumOfBlocks = 16;
			BlockSize = 4;
			break;
		}
		case UM3Journal_type::alarm:
		{
			qmFile->mbRecordNumberLo = alarm;
			NumOfBlocks = 16;
			BlockSize = 8;
			break;
		}
	}
	//cout<<JournalType<<endl;
	if (JournalType == UM3Journal_type::main_journal)
	{
		UMKA03utilites::CheckForSectionFilling(mpModbusEngine, NumOfUMKAPArams);
		while (mN_part < NumOfBlocks)
		{
			if (NumToRead[mN_part] !=0)
			{
				if (mPartsToRead == 0)
				{
					mNum = mN_part *BlockSize *528;
					mMaxAddrJrn = mNum + BlockSize * 528;
					if (iteratorMax > mMaxAddrJrn) iteratorMax = mMaxAddrJrn;
				}

				while (mNum < mMaxAddrJrn)
				{
					if (iterator == iteratorMax)
					{
						iterator = 0;
						return 1; //have smth to read
					}
					mPartsToRead = 1;
					iterator++;
					if ((mMaxAddrJrn - mNum) > MaxByteOne)
					{
						DI = MaxByteOne;
					}
					else
					{
						DI = mMaxAddrJrn - mNum;
					}

					//send request
					/**************************************************************************************/
					qmFile->mbRecordLengthLo = DI; // number of bytes to read
					qmFile->mbFileNumberHi = (uint8_t)(mNum & 0xFF);
					qmFile->mbFileNumberLo = (uint8_t)((mNum>>8) & 0xFF);
					qmFile->mbRecordNumberHi = (uint8_t)((mNum>>16) & 0xFF);
					qmFile->makeRawRequest();
					int out = UMKA03utilites::requestPort(qmFile, mpModbusEngine);

//					cout << "ERR Code:" << out << endl;
//					cout << *qmFile << endl;

					uint8_t *pRetBuf = qmFile->getDataFromResponse();

					f.write(reinterpret_cast<char*>(pRetBuf), qmFile->getDataSize());
					/****************************************************************************************/

					mNum +=DI;
				}

				mPartsToRead = 0;
				mN_part++;
			}
			else
			{
				//iterator = 0;
				mN_part++;
			}
		}
		//JournalType = 5; // костыль. Без него не работает. входит в елс.
	}
	else
	{

		{
			while (mN_part < NumOfBlocks)
			{
				if (mPartsToRead == 0)
				{
					mNum = mN_part *BlockSize *528;
					mMaxAddrJrn = mNum + BlockSize * 528;
					if (iteratorMax > mMaxAddrJrn) iteratorMax = mMaxAddrJrn;
				}
				while (mNum < mMaxAddrJrn)
				{
					if (iterator == iteratorMax)
					{
						iterator = 0;
						return 1; //have smth to read
					}
					mPartsToRead = 1;
					iterator++;
					if ((mMaxAddrJrn - mNum) > MaxByteOne)
					{
						DI = MaxByteOne;
					}
					else
					{
						DI = mMaxAddrJrn - mNum;
					}

					//send request
					/**************************************************************************************/
					qmFile->mbRecordLengthLo = DI; // number of bytes to read
					qmFile->mbFileNumberHi = (uint8_t)(mNum & 0xFF);
					qmFile->mbFileNumberLo = (uint8_t)((mNum>>8) & 0xFF);
					qmFile->mbRecordNumberHi = (uint8_t)((mNum>>16) & 0xFF);
					qmFile->makeRawRequest();
					int out = UMKA03utilites::requestPort(qmFile, mpModbusEngine);

//					cout << "ERR Code:" << out << endl;
//					cout << *qmFile << endl;

					uint8_t *pRetBuf = qmFile->getDataFromResponse();

					f.write(reinterpret_cast<char*>(pRetBuf), qmFile->getDataSize());
					/****************************************************************************************/

					mNum +=DI;
				}
				//iterator = 0;
				mPartsToRead = 0;
				mN_part++;
			}
		}
	}

	f.close();
	if (JournalType == UM3Journal_type::alarm || JournalType == UM3Journal_type::start_stop)
	{
		if (mN_part >=16)
		{
			mN_part = 0;
			mPartsToRead = 0;
			///iterator = 0;
			delete(qmFile);
			return 0;
		}
	}
	else
	{
		if (mN_part >=32)
		{
			mN_part = 0;
			mPartsToRead = 0;
			///iterator = 0;
			delete(qmFile);
			return 0;
		}
	}

	/****************************************************************************************************/ //MAIN Journal Reading As SPO Algo
	return 2;
}
#else
/**
 * @brief
 * возвращает 0 в случае успешного окончания вычитки
 * возвращает 1 в случае успешного окончания вычитки секции журнала, но при этом есть еще данные для чтения
 * возвращает 2 в случае возникновения любой непредвиденной и необработанной ошибки
 * возвращает 4 в случае ошибки проверки заполненности журнала
 * возвращает 5 при ошибке чтения данных по модбас
 */
fileReadStatus_type UMKA03utilites::ReadJournal(V7Port* mpPort,
		const char *Path, const UM3Journal_type& JournalType,
		unsigned short NumOfUMKAPArams, unsigned int iteratorMax,
		uint8_t devAddr) {
	/****************************************************************************************************/ // MAIN Journal Reading As SPO Algo
	//Journal Reading As SPO Algo
	//for (int i = 0; i < 32; ++i)
	QueryMsgReadFileRecords *qmFile = new QueryMsgReadFileRecords();
	std::ofstream f(Path, std::ios::binary | std::ios::app);
	unsigned char DI;
	int out = 0;
	const unsigned char MaxByteOne = 110;
	unsigned char NumOfBlocks = 0;
	unsigned char BlockSize = 0;

	//uint8_t* NumToRead = 0;
	qmFile->mbDevAddr = devAddr;
	switch (JournalType) {
	case UM3Journal_type::main_journal: {
		qmFile->mbRecordNumberLo = static_cast<int>(UM3Journal_type::main_journal);
		NumOfBlocks = 32;
		BlockSize = 128;
		break;
	}
	case UM3Journal_type::start_stop: {
		qmFile->mbRecordNumberLo =
				static_cast<int>(UM3Journal_type::start_stop);
		NumOfBlocks = 16;
		BlockSize = 4;
		break;
	}
	case UM3Journal_type::alarm: {
		qmFile->mbRecordNumberLo = static_cast<int>(UM3Journal_type::alarm);
		NumOfBlocks = 16;
		BlockSize = 8;
		break;
	}
	}
	//cout<<JournalType<<endl;
	if (JournalType == UM3Journal_type::main_journal) {
		if (IsFirstReading) {
			fileReadStatus_type state = UMKA03utilites::CheckForSectionFilling(
					mpPort, NumOfUMKAPArams, devAddr);
			if (state == fileReadStatus_type::filling_check_error) {
				return (fileReadStatus_type::filling_check_error); // ошибка проверки заполненности секций журнала.
			}
			IsFirstReading = false;
		}
		while (mN_part < NumOfBlocks) {
			if (NumToRead[mN_part] != 0) {
				if (mPartsToRead == 0) {
					mNum = mN_part * BlockSize * 528;
					mMaxAddrJrn = mNum + BlockSize * 528;
					if (iteratorMax > mMaxAddrJrn)
						iteratorMax = mMaxAddrJrn;
				}

				while (mNum < mMaxAddrJrn) {
					if (iterator == iteratorMax) {
						iterator = 0;
						return (fileReadStatus_type::success_section_reading); //have smth to read
					}
					mPartsToRead = 1;
					iterator++;
					if ((mMaxAddrJrn - mNum) > MaxByteOne) {
						DI = MaxByteOne;
					} else {
						DI = mMaxAddrJrn - mNum;
					}

					//send request
					/**************************************************************************************/
					qmFile->mbRecordLengthLo = DI; // number of bytes to read
					qmFile->mbFileNumberHi = (uint8_t) (mNum & 0xFF);
					qmFile->mbFileNumberLo = (uint8_t) ((mNum >> 8) & 0xFF);
					qmFile->mbRecordNumberHi = (uint8_t) ((mNum >> 16) & 0xFF);
					qmFile->makeRawRequest();
					out = mpPort->request(qmFile);
					//	 PRINTDEBUG2("Modbus Status ", out);
					if (out == 0) {
#ifdef DEBUG
					//	cout << "ERR Code:" << out << endl;
					//	cout << *qmFile << endl;
#endif
						uint8_t *pRetBuf = qmFile->getDataFromResponse();
						f.write(reinterpret_cast<char*>(pRetBuf),
								qmFile->getDataSize());
						/****************************************************************************************/
						SizeForSend.cur_size += DI * 2;
						mNum += DI;
					} else {
						f.close();
						return (fileReadStatus_type::modbus_exchange_error); // ошибка запроса по модбас
					}

				}
				if (out == 0) {

					mPartsToRead = 0;
					mN_part++;
				} else {
					f.close();
					return (fileReadStatus_type::modbus_exchange_error); // ошибка запроса по модбас
				}

			} else {
				if (out == 0)
					//iterator = 0;
					mN_part++;
				else {
					f.close();
					return (fileReadStatus_type::modbus_exchange_error); // ошибка запроса по модбас
				}

			}
		}
		//	JournalType = 5; // костыль. Без него не работает. входит в елс.
	} else {

		{
			while (mN_part < NumOfBlocks) {
				if (mPartsToRead == 0) {
					mNum = mN_part * BlockSize * 528;
					mMaxAddrJrn = mNum + BlockSize * 528;
					if (iteratorMax > mMaxAddrJrn)
						iteratorMax = mMaxAddrJrn;
				}
				while (mNum < mMaxAddrJrn) {
					if (iterator == iteratorMax) {
						iterator = 0;
						return (fileReadStatus_type::success_section_reading); //have smth to read
					}
					mPartsToRead = 1;
					iterator++;
					if ((mMaxAddrJrn - mNum) > MaxByteOne) {
						DI = MaxByteOne;
					} else {
						DI = mMaxAddrJrn - mNum;
					}

					//send request
					/**************************************************************************************/
					qmFile->mbRecordLengthLo = DI; // number of bytes to read
					qmFile->mbFileNumberHi = (uint8_t) (mNum & 0xFF);
					qmFile->mbFileNumberLo = (uint8_t) ((mNum >> 8) & 0xFF);
					qmFile->mbRecordNumberHi = (uint8_t) ((mNum >> 16) & 0xFF);
					qmFile->makeRawRequest();
					out = mpPort->request(qmFile);
					// PRINTDEBUG2("Modbus Status ", out);
					if (out == 0) {
#ifdef DEBUG
//						cout << "ERR Code:" << out << endl;
//						cout << *qmFile << endl;
#endif
						uint8_t *pRetBuf = qmFile->getDataFromResponse();
						f.write(reinterpret_cast<char*>(pRetBuf),
								qmFile->getDataSize());

						SizeForSend.cur_size += DI * 2;
						mNum += DI;
					}

					else {
						f.close();
						return (fileReadStatus_type::modbus_exchange_error); // ошибка запроса по модбас
					}

				}
				if (out == 0) {									//iterator = 0;
					mPartsToRead = 0;
					mN_part++;
				} else {
					f.close();
					return (fileReadStatus_type::modbus_exchange_error); // ошибка запроса по модбас
				}
			}
		}
	}

	f.close();
	if (JournalType == UM3Journal_type::alarm
			|| JournalType == UM3Journal_type::start_stop) {
		if (mN_part >= 16) {
			mN_part = 0;
			mPartsToRead = 0;
			///iterator = 0;
			delete (qmFile);
			return (fileReadStatus_type::success);
		}
	} else {
		if (mN_part >= 32) {
			mN_part = 0;
			mPartsToRead = 0;
			///iterator = 0;
			delete (qmFile);
			return (fileReadStatus_type::success);
		}
	}

	/****************************************************************************************************/ //MAIN Journal Reading As SPO Algo
	return fileReadStatus_type::undefined_error;
}
#endif

#ifdef DEBUG_UM
uint8_t UMKA03utilites::ReadWholeJournal(EngineModbusRTU *mpModbusEngine, const char *Path, unsigned short NumOfUMKAPArams, unsigned int iterator)
{

	volatile uint8_t status = 0;

	if (mEntries == 0)
	{
		UMKA03utilites::initJournalHeader(Path);
		mFunctionCall = 1;
	}

	while(mFunctionCall == 1)
	{
		status = (UMKA03utilites::ReadJournal(mpModbusEngine, Path, UM3Journal_type::main_journal, NumOfUMKAPArams, iterator));
		mEntries++;
		if (status == 0)
		{
			mFunctionCall = 2;
			break;
		}
		else
		{
			mFunctionCall = 1;
			return 1;
		}
	}
	while(mFunctionCall == 2)
	{
		status = UMKA03utilites::ReadJournal(mpModbusEngine, Path, UM3Journal_type::start_stop, NumOfUMKAPArams, iterator);
		mEntries++;
		if (status == 0)
		{
			mFunctionCall = 3;
			break;
		}
		else
		{
			mFunctionCall = 2;
			return 2;
		}
	}
	while(mFunctionCall == 3)
	{
		mEntries++;
		status = UMKA03utilites::ReadJournal(mpModbusEngine, Path, UM3Journal_type::alarm, NumOfUMKAPArams, iterator);

		if (status == 0)
		{
			mFunctionCall = 0;
			break;
		}
		else
		{
			mFunctionCall = 3;
			return 3;
		}
	}

	UMKA03utilites::WriteJournalHeader(Path);
	mEntries=0;
	return 0;

}
#else
/**
 * @brief
 * возвращает 0 в случае успешного окончания чтения журнала
 * возвращает 1 в случае успешного окончания чтения секции, но нужно продолжать читать эту секцию
 * возвращает 2 в случае успешного окончания чтения секции, но нужно продолжать читать эту секцию
 * возвращает 3 в случае успешного окончания чтения секции, но нужно продолжать читать эту секцию
 * возвращает 5 в случае ошибки чтения данных
 * возвращает 6 в случае непредвидененой и необрработанной ошибки
 *
 */
fileReadStatus_type UMKA03utilites::ReadWholeJournal(V7Port* mpPort,
		const char *Path, unsigned short NumOfUMKAPArams, unsigned int iterator,
		uint8_t devAddr) {

	volatile fileReadStatus_type status = fileReadStatus_type::success;

	if (mEntries == 0) {
		IsFirstReading = true;
		UMKA03utilites::initJournalHeader(Path);
		mFunctionCall = 1;
	}

	while (mFunctionCall == 1) {
		status = (UMKA03utilites::ReadJournal(mpPort, Path,
				UM3Journal_type::main_journal, NumOfUMKAPArams, iterator, devAddr));
		mEntries++;
		if (status == fileReadStatus_type::undefined_error) {
			mEntries = 0;
			mFunctionCall = 0;
			return fileReadStatus_type::error;
		}
		if (status == fileReadStatus_type::success) {
			mFunctionCall = 2;
			break;
		}
		if (status == fileReadStatus_type::filling_check_error) {
			mEntries = 0;
			SizeForSend.cur_size = 0;
		}
		if (status == fileReadStatus_type::modbus_exchange_error) {
			mFunctionCall = 1;
			return fileReadStatus_type::modbus_exchange_error;
		} else {
			mFunctionCall = 1;
			return fileReadStatus_type::success_section_reading;
		}
	}
	while (mFunctionCall == 2) {
		status = UMKA03utilites::ReadJournal(mpPort, Path,
				UM3Journal_type::start_stop, NumOfUMKAPArams, iterator,
				devAddr);
		mEntries++;
		if (status == fileReadStatus_type::undefined_error) {
			mEntries = 0;
			mFunctionCall = 0;
			return fileReadStatus_type::error;
		}
		if (status == fileReadStatus_type::success) {
			mFunctionCall = 3;
			break;
		}
		if (status == fileReadStatus_type::modbus_exchange_error) {
			mFunctionCall = 2;
			return fileReadStatus_type::modbus_exchange_error;
		} else {
			mFunctionCall = 2;
			return fileReadStatus_type::success_section_reading;
		}
	}
	while (mFunctionCall == 3) {
		mEntries++;
		status = UMKA03utilites::ReadJournal(mpPort, Path,
				UM3Journal_type::alarm, NumOfUMKAPArams, iterator, devAddr);
		if (status == fileReadStatus_type::undefined_error) {
			mEntries = 0;
			mFunctionCall = 0;
			return fileReadStatus_type::error;
		}
		if (status == fileReadStatus_type::success) {
			mFunctionCall = 0;
			break;
		}
		if (status == fileReadStatus_type::modbus_exchange_error) {
			mFunctionCall = 3;
			return fileReadStatus_type::modbus_exchange_error;
		} else {
			mFunctionCall = 3;
			return fileReadStatus_type::success_section_reading;
		}
	}
	unsigned short UMSoftVersion = UMKA03utilites::getUMKA03SoftVersion(mpPort,
			devAddr);
#ifdef DEBUG
	PRINTDEBUG2("UMSoft   ", UMSoftVersion)
#endif
	UMKA03utilites::WriteJournalHeader(Path, NumOfUMKAPArams, UMSoftVersion);
	mEntries = 0;
	SizeForSend.cur_size = SizeForSend.cur_size - 1;
	//SizeForSend.calcsize = 0;
	return fileReadStatus_type::success;
}
#endif

uint8_t UMKA03utilites::WriteJournalHeader(const char *Path,
		unsigned short NumOfParams, unsigned short UMSoftVersion) {
	std::ifstream file(Path);
	if (!file.is_open()) {
		std::cerr << "Error opening journal file." << std::endl;
		return 1;
	} else {
		std::string content((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
		file.close();
		char *buf = &content[0];
		FormHeaderFile(buf, Path, (content.length() - 128), 0,
				NumOfParams, UMSoftVersion);
		std::ofstream fh(Path, std::ios_base::binary);
		//  std::ios::binary|std::ios::app
		fh.write(buf, content.length());
		fh.close();
		return 0;

	}
}

#ifdef DEBUG_UM
int UMKA03utilites::requestPort(QueryMsg * msg, EngineModbusRTU *mpModbusEngine)
{

	//flagBusy = true;
	const useconds_t pauseInUs = 300;
	int exitCode(0);//нет ошибки
	// подключение к движку
	if (!mpModbusEngine->getModbusContext())
	mpModbusEngine->open();
	// если подключились
	if (mpModbusEngine->connect()) {
		PRINTDEBUG2("\n[Port::request()] Modbus ctx is: ",
				mpModbusEngine->getModbusContext());
		msg->makeRawRequest();
		int retCode = mpModbusEngine->sendRawRequest(msg->getRequestBuffer(),
				msg->getRequestSize());
		cout << "reqtCode" << (retCode);
		uint8_t reqFunc = msg->getRequestFunction();
		cout << "reqFunc" << (int)reqFunc<<endl;;
		//int addr = msg->getRequestBuffer()[0];

		if (retCode != 0) {
			// error & exit
			PRINTDEBUG("[Port::request()] error & exit");
			usleep(pauseInUs);
			msg->setErrorCode(retCode);
		}
		else {

			uint8_t tmp[MODBUS_TCP_MAX_ADU_LENGTH];
			uint8_t* pTmp = tmp;
			int tmpSize(0);
			retCode = mpModbusEngine->receiveRawConfirmation(&pTmp, tmpSize);

			if (retCode != 0) {
				cout << "Error!!!!!!\n";
				msg->setErrorCode(retCode); // exit
			}
			else {
				engine_backend_type mBackendType = RTU;
				if (mBackendType == RTU) {
					if (reqFunc != tmp[1]) {
						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[2]));
					}
					else {

						msg->setErrorCode(retCode);
						msg->setResponseBuffer(tmp, tmpSize);
					}

				}
				else if (mBackendType == TCP) {

					if (reqFunc != tmp[7]) {
						msg->setErrorCode(mpModbusEngine->exceptHadler(tmp[8]));
					}
					else {
						msg->setErrorCode(retCode);
						msg->setResponseBuffer(tmp + 6, tmpSize); ///
					}
				}

			}

		}
//        PRINTDEBUG2("\n[Port::request()] Request result:\n", *msg);
		exitCode = msg->getErrorCode();
		mpModbusEngine->close();
	}
	else {
		exitCode = 0x04; // slave device failure
	}
	// flagBusy = false;
	return exitCode;
}
#endif
