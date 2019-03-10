/**
 * @file KN24defs.h
 * @brief KN24(TDrive) definitions( Tdrive_9.013)
 *
 * Created on: 20 жовт. 2018 р.
 *      Author: v7r
 */

#ifndef SRC_KN24DEFS_H_
#define SRC_KN24DEFS_H_
#include <iostream>
#include <cstdint>
#include "units_ansi.h"

////////////////////////////////////////////////////////
/// Parameter Descriptor file foe KN24
////////////////////////////////////////////////////////

/**
 * @struct FlagOpisParam_type 4 bytes
 */
#pragma pack(push , 2)
struct FlagOpisParam_type {
	uint32_t TypeVar :5;
	uint32_t MasterVis :1;
	uint32_t MasterChange :1;
	uint32_t TexnVis :1;
	uint32_t TexnChange :1;
	uint32_t UserVis :1;
	uint32_t UserChange :1;
	uint32_t Power :3;
	uint32_t EditType :3;
	uint32_t nonVolatile :1;
	uint32_t LimChange :1;
	uint32_t FormatChange :1;
	uint32_t NumBase :4;
	uint32_t NumFunc :4;
	uint32_t FieldType :1; // 0- Prm, // 1- JPrm
	uint32_t Visible :1;
};
#pragma pack(pop)

/**
 * @struct headOfParamDescFile_type
 * @brief KN24 128-byte header
 */
#pragma pack(push, 2)
struct headOfParamDescFile_type {
	uint32_t FileCrc;
	uint32_t Sign;
	uint16_t SizeOfFileDescr;
	uint16_t MenuStart;
	uint16_t PrmStart;
	uint16_t QuantityMenu;
	uint16_t QuantityPrm;
	uint8_t VendorName[32];
	uint8_t DeviceName[32];
	uint32_t DeviceType;
	uint16_t TopOfJrnFlt;
	uint16_t NumOfRecFlt;
	uint16_t TopOfJrnAl;
	uint16_t NumOfRecAl;
	uint16_t AdrListPrmJrn;
	uint16_t NumPrmInJrn;
	uint16_t QuantityUst;
	uint16_t JrnStart;
	uint16_t QuantityJrn;
	uint8_t Res[8];
	uint16_t NomProtocol; //Номер протокола 0-старый,1- старый +спец.параметры,2-мультиязычный, 3-мультиязычный +спец.параметры
	uint32_t SpecialPrm; //Ссылка на начало списка специальных параметров пульта
	uint16_t QuantitySpecialPrm; //Количество элементов в списке специальных параметров
	uint32_t AdrTDrivePrm; // Адрес начала параметров TDrive   ссылка на TDrivePrm
	uint32_t HeaderCrc;
};
#pragma pack(pop)

/**
 * @struct HeadOfFileMultyLang_type (UMCA07, UMKA03)
 */
#pragma pack(push, 2)
struct HeadOfFileMultyLang_type {
	uint32_t FileCrc;
	uint32_t Sign;
	uint32_t SizeOfFileDescr;
	uint8_t MaxLengthName;
	uint8_t MaxLengthMess;
	uint8_t MaxLengthHelp;
	uint8_t MaxLengthEdIzm;
	uint32_t MenuStart;
	uint32_t PrmStart;
	uint16_t QuantityMenu;
	uint16_t QuantityPrm;
	uint8_t VendorName[32];
	uint8_t DeviceName[32];
	uint32_t DeviceType;
	uint16_t TopOfJrnFlt;
	uint16_t NumOfRecFlt;
	uint16_t TopOfJrnAl;
	uint16_t NumOfRecAl;
	uint16_t AdrListPrmJrn;
	uint16_t NumPrmInJrn;
	uint16_t QuantityUst;
	uint32_t JrnStart;
	uint16_t QuantityJrn;
	uint16_t QuantityFildJrn;
	uint32_t MessPultStart;
	uint16_t QuantityMessPult;
	uint32_t Pnt1LngTxt;
	uint32_t QuantityLng;
	uint8_t Res[2];
	uint16_t NomProtocol; //Номер протокола 0-старый,1- старый +спец.параметры,2-мультиязычный, 3-мультиязычный +спец.параметры
	uint32_t SpecialPrm; //Ссылка на начало списка специальных параметров пульта
	uint16_t QuantitySpecialPrm; //Количество элементов в списке специальных параметров
	uint32_t AdrTDrivePrm; // Адрес начала параметров TDrive   ссылка на TDrivePrm
	uint32_t HeaderCrc;
};
//150 bytes
#pragma pack(pop)

/**
 * @brief Parameter descriptor for KN24
 * @struct Descriptor_prm_type
 */
#pragma pack(push, 1)
struct DescriptorPrm_type {
	uint8_t IDGrp;
	uint8_t IDPrm;
	uint16_t Unit;
	FlagOpisParam_type FlgPrm; //?
	int16_t LowLim;
	int16_t HighLim;
	uint16_t OfsTxt;
	uint16_t DefaultSet;
	uint8_t Name[16];
	uint16_t LenHlp;
	uint16_t OfsHlp;
	uint16_t Functional;
	uint16_t AdrVal;
	uint16_t Visible;
	uint16_t tmp;
};
// 44 bytes
#pragma pack(pop) //Вернули предыдущую настройку

/**
 * @brief parameter descriptor file
 * @struct DescriptorPrmMultyLang_type
 */
#pragma pack(push, 1)
struct DescriptorPrmMultyLang_type {
	uint8_t IDGrp;
	uint8_t IDPrm;
	uint16_t Unit;
	FlagOpisParam_type FlgPrm;
	int16_t LowLim;
	int16_t HighLim;
	uint32_t OfsTxt;
	uint16_t DefaultSet;
	uint32_t OfsName;
	uint16_t OfsHlp;
	uint16_t Functional;
	uint16_t AdrVal;
	uint16_t Visible;
	uint8_t res[2];
};
#pragma pack(pop)

////////////////////////////////////////////////////////
/// KN24 logfile definitions
/////////////////////////////////////////////////////////
#define szExtRAMmem       8192
#define szExtRAMmemData   (szExtRAMmem - 64)
/**
 * @union headOfLogFile_type
 * @brief head of log
 */
#pragma pack(push, 1)
struct headOfLogFile_type {
	headOfLogFile_type();
	uint16_t Ntic;        // Количество тиков в прореживалке.
	uint16_t Nvar;        // Количество переменных в логе
	uint32_t LogInd;      // Указатель на текущую запись
	uint32_t Sz;          // Количество отсчетов.
	uint16_t TimeOneTick; //
	uint16_t rez2;        // Резерв
	uint16_t pntPrm[9];   // Ссылки на параметры логера задаются в ручную
	uint16_t rez3;        // Резерв
	uint32_t kAmp[9];     // Коэффициенты коррекции для правильного отображения  параметров логера
	uint16_t res[28];
};
//128 bytes
#pragma pack(pop)

///////////////////////////////////////////////////////
/// KN24 RDrive logfile definitions
///////////////////////////////////////////////////////
/**
 * @struct TPrm
 */
#pragma pack(push, 1)
struct TPrm {
	TPrm();
	uint8_t Name[16];     //наименование параметра
	uint8_t EdIzm[8];     //единицы измерения
	uint8_t IDgrp;        //номер группы
	uint8_t IDPrm;        //номер в группе
	uint8_t unused[4]; //
	uint32_t Power;    //степень
	uint32_t kAmp; //Коэффициент коррекции для правильного отображения параметров логера
};
//38 байта
#pragma pack(pop)

/**
 * @brief запись данных логера в файл
 * @union
 */
#pragma pack(push, 1)
struct headOfLogFileSave_type {
	headOfLogFileSave_type();
	uint16_t CRC;
	uint16_t Ntic;            // Количество тиков в прореживалке.
	uint16_t Nvar;            // Количество переменных в логе
	uint16_t TimeOneTick;     // Период вызова логгера (квант времени).
	uint32_t LogInd;          // Указатель на текущую запись
	uint32_t Sz;              // Количество отсчетов.
	uint32_t DeviceType;
	uint32_t UnixTime;        //время записи журнала
	uint8_t DeviceName[32];  //	наименование изделия
	uint8_t VendorName[32];  //	производитель
	TPrm PrmLog[20];    //Stepanov => описание параметров лога   20*38 байта
	uint8_t ErrStatLor[32];  //причина останова при формировании лога
};
#pragma pack(pop)
/**
 * @struct
 * @brief
 */
#pragma pack(push, 1)
struct logFileSave_type {
	headOfLogFileSave_type hof;                     // Заголовок файла
	int16_t *data; // Массив с данными
};
#pragma pack(pop)

//данные  TDrive
#pragma pack(push, 2)
/**
 * @brief
 */
struct TAdrTDrivePrm {
	uint16_t countByte;                 //длина секции в байтах
	uint16_t SwapDan;                   // 0- нет, 1- да
	uint16_t countLang;                 //количество поддерживаемых языков
	uint16_t numLogFile;                //Номер файла  команды  содержащего лог
	uint16_t KodPuskStop;               // код пуска команды 5
	uint16_t KodBeginEndLog;          // код начала-конца  чтения лога команды 5
	uint16_t countEditPrm;              //количество параметров редактирования
	uint16_t countMonitorPrm;           //количество параметров мониторинга
};
#pragma pack(pop)

#pragma pack(push, 2)
struct TSpezPrm {
	uint16_t AdrStat1;               //адрес параметра слова состояния 1
	uint16_t AdrStat2;               //адрес параметра слова состояния 2
	uint16_t AdrPassUser;            //адрес параметра пароля пользователя
	uint16_t AdrEPassUser;           //адрес параметра эталона пароля пользователя
	uint16_t AdrPassTexn;            //адрес параметра пароля технолога
	uint16_t AdrEPassTexn;           //адрес параметра эталона пароля технолога
	uint16_t AdrPassMaster;          //адрес параметра пароля производителя
	uint16_t AdrEPassMaster;         //адрес параметра эталона пароля производителя
	uint16_t AdrMultilang;           //адрес параметра переключения языка
	uint16_t AdrErrorTek;            //адрес параметра текущей аварии
	uint16_t AdrErrorFict;           //адрес параметра фиксированной аварии
	uint16_t NabParam;               //Набор параметров - Заводских настроек АТ24
	uint16_t SaveNab1;               //Сохр.наб 1. пульт - Заводских настроек АТ24
	uint16_t SaveNab2;               //Сохр.наб 2. пульт - Заводских настроек АТ24
	uint16_t RecovNab1;              //Восст. наб 1.- Заводских настроек АТ24
	uint16_t RecovNab2;              //Восст. наб 2.- Заводских настроек АТ24
	uint16_t AdrStatLog;             //адрес словасостояния лога
};
#pragma pack(pop)

////////////////////////////////////////////////////////////////////////
/// Units array
///////////////////////////////////////////////////////////////////////

/**
 * @brief use only with full c++11 compliant compiler
 */
const std::string unitsName[] = { u8" ", u8"мВ", u8"В", u8"кВ", u8"мА", u8"А", u8"кА", u8"Гц", u8"кГц", u8"ВА",
u8"кВА", u8"Вт", u8"кВт", u8"мОм", u8"Ом", u8"кОм", u8"МОм", u8"мкГн", u8"мГн", u8"мкс",
u8"мс", u8"сек", u8"мин", u8"ч ", u8"мм", u8"см", u8"дм", u8"м", u8"об/м", u8"°С",
u8"%", u8"м2", u8"м3", u8"м2/м", u8"м3/ч", u8"опис", u8"атм", u8"л", u8"g", u8"Гц/с",
u8"В/с", u8"%/с", u8"км", u8"м/с2", u8"кПа", u8"МПа", u8"кг/см2", u8"psi", u8"°F", u8"куб/с",
u8"кВт/ч", u8"МВт/ч", u8"В/км", u8"Мвар/ч", u8"рад", u8"рад/с", u8"рад/с2", u8"кг/м2", u8"Вб", u8"Нм",
u8"град", u8"Гн", u8"1/с", u8"кг", u8" "
};

////////////////////////////////////////////////////////////////////////
/// menu descriptors
////////////////////////////////////////////////////////////////////////
/**
 * @brief DescriptorMenuMultyLang_type
 */
#pragma pack(push, 2)
struct DescriptorMenuMultyLang_type {
	uint8_t IDGrp;
	uint8_t QuantityPrmMenu;
	uint32_t AdrNprm;
	uint16_t FlgMenu;
	uint32_t OfsName;
	uint16_t OfsHlp;
	uint8_t res[6];
};
#pragma pack(pop)

std::ostream& operator<<(std::ostream& stream, const DescriptorPrm_type& prm);
std::ostream& operator<<(std::ostream& stream, const headOfLogFile_type& head);
std::ostream& operator <<(std::ostream&stream,
		const headOfParamDescFile_type& fl);

#endif /* SRC_KN24DEFS_H_ */
