/**
 * @file      QueryMsgI.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */

#include "QueryMsg.h"
#include <cassert>

QueryMsg::QueryMsg() :
		mReqBuf(0), mRspBuf(0), mReqSize(0), mRspSize(0), mErrCode(0), mParamAddress(
				0) {
	//std::clog << "QueryMsg" << std::endl;
}
QueryMsg::~QueryMsg() {

}

int16_t QueryMsg::getResponseSize() const {
	return mRspSize;
}

void QueryMsg::setResponseSize(const int16_t size) {
	mRspSize = size;
}

uint8_t* QueryMsg::getResponseBuffer() {
	return &mRspBuf[0];
}

int16_t QueryMsg::getErrorCode() const {
	return mErrCode;
}

int16_t QueryMsg::getRequestSize() const {
	return mReqSize;
}

uint8_t* QueryMsg::getRequestBuffer() {
	return &mReqBuf[0];
}

void QueryMsg::setResponseBuffer(const uint8_t* rsp, const int16_t size) {
	mRspSize = size;
	mRspBuf.assign(rsp, rsp + size);
}

void QueryMsg::setErrorCode(const int16_t err) {
	mErrCode = err;
}

uint8_t* QueryMsg::getDataFromResponse() {
	return &mRspBuf[0];
}

int16_t QueryMsg::getDataSize() {
	return mRspSize;
}

uint8_t QueryMsg::getRequestFunction() const {
	return mReqSize > 0 ? mReqBuf[1] : 0;
}

uint8_t QueryMsg::getResponseFunction() const {
	return mRspSize > 0 ? mRspBuf[1] : 0;
}

uint16_t QueryMsg::getParamAddress() const {
	return mParamAddress;
}

std::ostream& QueryMsg::print(std::ostream& obj) const {
	obj << "[debug] QueryMsg debug output" << std::endl;
	typedef std::vector<uint8_t>::const_iterator it;
	obj << "[debug] " << "Request buffer:" << std::endl << "[debug] ";
	for (it x = mReqBuf.begin(); x != mReqBuf.end(); ++x) {
		obj << "0x" << std::setw(2) << std::hex << std::setfill('0') << (int) *x
				<< " ";
	}
	obj << std::endl;
	obj << "[debug] " << "Response buffer:" << std::endl << "[debug] ";

	for (it x = mRspBuf.begin(); x != mRspBuf.end(); ++x) {
		obj << "0x" << std::setw(2) << std::hex << std::setfill('0') << (int) *x
				<< " ";
	}
	obj << std::endl;
	obj << "[debug] " << "Request size:" << std::dec<< mReqSize << std::endl;
	obj << "[debug] " << "Response size:" << std::dec << mRspSize << std::endl;
	obj << "[debug] " << "Error Code:" << mErrCode << std::endl;
	obj << "[debug] " << "end." << std::endl;
	return obj;
}

/////////////////////////////////////// Функции чтения /////////////////////////////////////
QueryMsgRead::QueryMsgRead(uint8_t devAddr, uint8_t funcCode, uint8_t addressHi,
		uint8_t addressLo, uint8_t quantityHi, uint8_t quantityLo) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbAddressHi(addressHi), mbAddressLo(
				addressLo), mbQuantityHi(quantityHi), mbQuantityLo(quantityLo) {

}

QueryMsgRead::QueryMsgRead() :
		mbDevAddr(0), mbFuncCode(0), mbAddressHi(0), mbAddressLo(0), mbQuantityHi(
				0), mbQuantityLo(0) {

}

/**
 uint8_t mbDevAddr;    //!< адрес модбас-устройства
 uint8_t mbFuncCode;   //!< modbus function code
 uint8_t mbAddressHi;  //!< аддресное пространство модбас
 uint8_t mbAddressLo;  //!< аддресное пространство модбас
 uint8_t mbByteLength; //!< количество байт
 */
void QueryMsgRead::makeRawRequest() {
	uint8_t req[6] = { mbDevAddr, mbFuncCode, mbAddressHi, mbAddressLo,
			mbQuantityHi, mbQuantityLo };
	mReqSize = 6;
	mParamAddress = (static_cast<uint16_t>(mbAddressHi) << 8) + mbAddressLo;
	mReqBuf.assign(req, req + mReqSize);
}

uint8_t* QueryMsgRead::getDataFromResponse() {
	return &mRspBuf[3]; // для функции чтения, данные начинаются с 4 байта
}

int16_t QueryMsgRead::getDataSize() {
	return mRspSize - 3; // 3 байта - заголовок
}

std::ostream& QueryMsgRead::print(std::ostream& stream) const {

	stream << "[debug] " << "QueryMsgRead" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

///////////////////////////////// Запись одной пары регистров /////////////////////////////////
QueryMsgWriteSingle::QueryMsgWriteSingle(uint8_t devAddr, uint8_t funcCode,
		uint8_t addressHi, uint8_t addressLo, uint8_t valueHi, uint8_t valueLo) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbAddressHi(addressHi), mbAddressLo(
				addressLo), mbValueHi(valueHi), mbValueLo(valueLo) {
}

QueryMsgWriteSingle::QueryMsgWriteSingle() :
		mbDevAddr(0), mbFuncCode(0), mbAddressHi(0), mbAddressLo(0), mbValueHi(
				0), mbValueLo(0) {
}

/**
 uint8_t mbDevAddr;    //!< адрес модбас-устройства
 uint8_t mbFuncCode;   //!< modbus function code
 uint8_t mbAddressHi;  //!< старший байт
 uint8_t mbAddressLo;  //!< младший байт
 uint8_t mbValeHi;     //!< старший байт
 uint8_t mbValeLo;     //!< младший байт
 */
void QueryMsgWriteSingle::makeRawRequest() {
	uint8_t req[6] = { mbDevAddr, mbFuncCode, mbAddressHi, mbAddressLo,
			mbValueHi, mbValueLo };
	mReqSize = 6;
	mParamAddress = (static_cast<uint16_t>(mbAddressHi) << 8) + mbAddressLo;
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgWriteSingle::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgWriteSingle" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

////////////////////////////////// Запись группы регистров  ////////////////////////////////////
QueryMsgWriteMulti::QueryMsgWriteMulti(uint8_t devAddr, uint8_t funcCode,
		uint8_t addressHi, uint8_t addressLo, uint8_t quantityRegHi,
		uint8_t quantityRegLo, uint8_t byteCnt, uint8_t* values) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbAddressHi(addressHi), mbAddressLo(
				addressLo), mbQuantityRegHi(quantityRegHi), mbQuantityRegLo(
				quantityRegLo), mbByteCnt(byteCnt), mbValues(values) {
}

QueryMsgWriteMulti::QueryMsgWriteMulti() :
		mbDevAddr(0), mbFuncCode(0), mbAddressHi(0), mbAddressLo(0), mbQuantityRegHi(
				0), mbQuantityRegLo(0), mbByteCnt(0), mbValues(0) {
}

/**
 uint8_t mbDevAddr;       //!< адрес модбас-устройства
 uint8_t mbFuncCode;      //!< modbus function code
 uint8_t mbAddressHi;     //!< старший байт
 uint8_t mbAddressLo;     //!< младший байт
 uint8_t mbQuantityRegHi; //!< кол-во байт - старший байт (всегда 0)?
 uint8_t mbQuantityRegLo; //!< младший байт
 uint8_t mbByteCnt;       //!< количество байт
 uint8_t* mbValues;       //!< данные
 */
void QueryMsgWriteMulti::makeRawRequest() {

	assert(mbValues != 0); // если ноль, нет данных, критическая ошибка
	mbByteCnt = 2 * mbQuantityRegLo; //!< 2* (0x01 .. 0x7b)
	uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH] = { mbDevAddr, mbFuncCode,
			mbAddressHi, mbAddressLo, mbQuantityRegHi, mbQuantityRegLo,
			mbByteCnt };
	memcpy(req + 7, mbValues, mbByteCnt); //!< 7 - размер посылки без данных
	mReqSize = 7 + mbByteCnt;
	mParamAddress = (static_cast<uint16_t>(mbAddressHi) << 8) + mbAddressLo;
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgWriteMulti::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgWriteMulti" << std::endl;
	QueryMsg::print(stream);
	return stream;

}

////////////////////////////////// Журнал АТ27 /////////////////////////////////////////////////////
QueryMsgJrnAT27::QueryMsgJrnAT27(uint8_t devAddr, uint8_t funcCode,
		uint8_t fileNumber, uint8_t startAddrHiWordHi,
		uint8_t startAddrHiWordLo, uint8_t startAddrLoWordHi,
		uint8_t startAddrLoWordLo, uint8_t cntRecords) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbFileNumber(fileNumber), mbStartAddrHiWordHi(
				startAddrHiWordHi), mbStartAddrHiWordLo(startAddrHiWordLo), mbStartAddrLoWordHi(
				startAddrLoWordHi), mbStartAddrLoWordLo(startAddrLoWordLo), mbCntRecords(
				cntRecords) {
}

QueryMsgJrnAT27::QueryMsgJrnAT27() :
		mbDevAddr(0), mbFuncCode(0), mbFileNumber(0), mbStartAddrHiWordHi(0), mbStartAddrHiWordLo(
				0), mbStartAddrLoWordHi(0), mbStartAddrLoWordLo(0), mbCntRecords(
				0) {
}

/**
 uint8_t mbDevAddr;            //!< адрес модбас-устройства
 uint8_t mbFuncCode;           //!< modbus function code
 uint8_t mbFileNumber;         //!< аддресное пространство модбас
 uint8_t mbStartAddrHiWordHi;  //!< аддрес старший байт
 uint8_t mbStartAddrHiWordLo;  //!< аддрес
 uint8_t mbStartAddrLoWordHi;  //!< аддрес
 uint8_t mbStartAddrLoWordLo;  //!< аддрес младший байт
 uint8_t mbCntRecords;         //!< количество записей (1..122))
 */
void QueryMsgJrnAT27::makeRawRequest() {
	uint8_t req[8] = { mbDevAddr, mbFuncCode, mbFileNumber, mbStartAddrHiWordHi,
			mbStartAddrHiWordLo, mbStartAddrLoWordHi, mbStartAddrLoWordLo,
			mbCntRecords };
	mReqSize = 8;
	mParamAddress = mbFileNumber; //! @attention адрес - это имя файла
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgJrnAT27::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgJrnAT27" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

////////////////////////////////// Рабоота с сырым запросом ////////////////////////////////////
QueryMsgRaw::QueryMsgRaw(uint8_t reqLength, uint8_t* reqBuf) :
		mbReqLength(reqLength), req(reqBuf) {

}

QueryMsgRaw::QueryMsgRaw() :
		mbReqLength(0), req(0) {
}

/**
 uint8_t mbReqLength; //!< кол-во байт в посылке или ответе (CRC не учитывается)
 uint8_t* req;       //!< данные запроса - массив байт
 */
void QueryMsgRaw::makeRawRequest() {
	mReqSize = mbReqLength;
	mParamAddress = (static_cast<uint16_t>(req[2]) << 8) + req[3];
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgRaw::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgRaw" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

///////////////////////////////////// Конфиги АТ27 ///////////////////////////////////////////////
QueryMsgCfgAT27::QueryMsgCfgAT27(uint8_t devAddr, uint8_t funcCode,
		uint8_t fileNumber, uint8_t startAddrHiWordHi,
		uint8_t startAddrHiWordLo, uint8_t startAddrLoWordHi,
		uint8_t startAddrLoWordLo, uint8_t cntRecords, uint8_t* values) :
		QueryMsgJrnAT27(devAddr, funcCode, fileNumber, startAddrHiWordHi,
				startAddrHiWordLo, startAddrLoWordHi, startAddrLoWordLo,
				cntRecords), mbValues(values) {
}

QueryMsgCfgAT27::QueryMsgCfgAT27() :
		QueryMsgJrnAT27(0, 0, 0, 0, 0, 0, 0, 0), mbValues(0) {
}

/**
 uint8_t mbDevAddr;            //!< адрес модбас-устройства
 uint8_t mbFuncCode;           //!< modbus function code
 uint8_t mbFileNumber;         //!< аддресное пространство модбас
 uint8_t mbStartAddrHiWordHi;  //!< аддрес старший байт
 uint8_t mbStartAddrHiWordLo;  //!< аддрес
 uint8_t mbStartAddrLoWordHi;  //!< аддрес
 uint8_t mbStartAddrLoWordLo;  //!< аддрес младший байт
 uint8_t mbCntRecords;         //!< количество записей (1..122))
 uint8_t *mbValues;            //!< массив данных (только для записи)
 */
void QueryMsgCfgAT27::makeRawRequest() {
	assert(mbValues != 0); // если ноль, нет данныхб критическая ошибка
	uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH] = { mbDevAddr, mbFuncCode,
			mbFileNumber, mbStartAddrHiWordHi, mbStartAddrHiWordLo,
			mbStartAddrLoWordHi, mbStartAddrLoWordLo, mbCntRecords };
	memcpy(req + 8, mbValues, 2 * req[7]); //!< 8 - размер заголовка
	mReqSize = 8 + (2 * req[7]);
	mParamAddress = mbFileNumber; //!< @attention адрес - это имя файла
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgCfgAT27::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgCfgAT27" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

/////////////////////////////////////////  Запись ячейки //////////////////////////////////////////
QueryMsgWriteSingleCoil::QueryMsgWriteSingleCoil(uint8_t devAddr,
		uint8_t funcCode, uint8_t addressHi, uint8_t addressLo, uint8_t valueHi,
		uint8_t valueLo) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbAddressHi(addressHi), mbAddressLo(
				addressLo), mbValueHi(valueHi), mbValueLo(valueLo) {
}

QueryMsgWriteSingleCoil::QueryMsgWriteSingleCoil() :
		mbDevAddr(0), mbFuncCode(0), mbAddressHi(0), mbAddressLo(0), mbValueHi(
				0), mbValueLo(0) {
}

void QueryMsgWriteSingleCoil::makeRawRequest() {
	uint8_t req[6] = { mbDevAddr, mbFuncCode, mbAddressHi, mbAddressLo,
			(uint8_t) (mbValueHi > 0 ? 0xff : 0x00, mbValueLo & 0x00) };
	mReqSize = 6;
	mParamAddress = (static_cast<uint16_t>(mbAddressHi) << 8) + mbAddressLo;
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsgWriteSingleCoil::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgWriteSingleCoil" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

//////////////////////////////////  Печать отладочной инфы
std::ostream& operator<<(std::ostream& stream, const QueryMsg& instance) {
	return instance.print(stream);
}

////////////////////////////////// ФАйловое чтение
/**
 uint8_t mbDevAddr;        //!< адрес модбас-устройства
 uint8_t mbFuncCode;       //!< modbus function code
 uint8_t mbByteCnt;        //!< количество байт 0x07..0xF5
 uint8_t mbRefType;        //!< тип - всегда 0x06
 uint8_t mbFileNumberHi;   //!< номер файла
 uint8_t mbFileNumberLo;   //!< номер файла
 uint8_t mbRecordNumberHi; //!< номер записи
 uint8_t mbRecordNumberLo; //!< номер записи
 uint8_t mbRecordLengthHi; //!< размер записи
 uint8_t mbRecordLengthLo; //!< разсер записи
 */

QueryMsgReadFileRecords::QueryMsgReadFileRecords(uint8_t devAddr,
		uint8_t funcCode, uint8_t byteCnt, uint8_t refType,
		uint8_t fileNumberHi, uint8_t fileNumberLo, uint8_t recAddressHi,
		uint8_t recAaddressLo, uint8_t recLenHi, uint8_t recLenLo) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbByteCnt(byteCnt), mbRefType(
				refType), mbFileNumberHi(fileNumberHi), mbFileNumberLo(
				fileNumberLo), mbRecordNumberHi(recAddressHi), mbRecordNumberLo(
				recAaddressLo), mbRecordLengthHi(recLenHi), mbRecordLengthLo(
				recLenLo) {
}

void QueryMsgReadFileRecords::makeRawRequest() {
	const int8_t bufSize = 10;
	uint8_t req[bufSize] = { mbDevAddr, mbFuncCode, mbByteCnt, mbRefType,
			mbFileNumberHi, mbFileNumberLo, mbRecordNumberHi, mbRecordNumberLo,
			mbRecordLengthHi, mbRecordLengthLo };
	mReqSize = bufSize;
	mParamAddress = (static_cast<uint16_t>(mbFileNumberHi) << 8) + mbFileNumberLo; //!< @attention адрес - имя файла
	mReqBuf.assign(req, req + mReqSize);

}

QueryMsgReadFileRecords::QueryMsgReadFileRecords() :
		mbDevAddr(0), mbFuncCode(0x14), mbByteCnt(0x07), mbRefType(0x06), mbFileNumberHi(
				0), mbFileNumberLo(0x01), mbRecordNumberHi(0), mbRecordNumberLo(
				0), mbRecordLengthHi(0), mbRecordLengthLo(0) {
}

int16_t QueryMsgReadFileRecords::getDataSize() {
	return mRspSize - 5; // 3 байта - заголовок
}

uint8_t* QueryMsgReadFileRecords::getDataFromResponse() {
	return &mRspBuf[5];
}

std::ostream& QueryMsgReadFileRecords::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsgReadFileRecords" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

bool QueryMsgReadFileRecords::checkUM03JournalFilling() {
	if (mRspBuf[61] != 0xFF)
		return true;
	return false;
}
///////////////////////
QueryMsg0x42::QueryMsg0x42() :
		mbDevAddr(0), mbFuncCode(0), mbDataHi(0), mbDataLo(0) {
}

QueryMsg0x42::QueryMsg0x42(uint8_t devAddr, uint8_t funcCode, uint8_t dataHi,
		uint8_t dataLo) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbDataHi(dataHi), mbDataLo(
				dataLo) {
}

void QueryMsg0x42::makeRawRequest() {
	uint8_t req[4] = { mbDevAddr, mbFuncCode, mbDataHi, mbDataLo };
	mReqSize = 4;
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsg0x42::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsg0x42" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

/**
 *  uint8_t mbDevAddr;        //!< адрес модбас-устройства
 uint8_t mbFuncCode;       //!< modbus function code
 uint8_t mMEIType;         //!< 0x0e
 uint8_t mReadDeviceIDcode; //!< 0x01, 0x02, 0x03, 0x04
 */
QueryMsg0x2b::QueryMsg0x2b() :
		mbDevAddr(0), mbFuncCode(0x2b), mbMEIType(0x0e), mbReadDeviceIDcode(
				0x01), mbObjectId(0) {
}

QueryMsg0x2b::QueryMsg0x2b(int8_t devAddr, uint8_t funcCode, uint8_t MEIType,
		uint8_t mReadDeviceIDcode, uint8_t ObjectId) :
		mbDevAddr(devAddr), mbFuncCode(funcCode), mbMEIType(MEIType), mbReadDeviceIDcode(
				mReadDeviceIDcode), mbObjectId(ObjectId) {
}

void QueryMsg0x2b::makeRawRequest() {
	uint8_t req[5] = { mbDevAddr, mbFuncCode, mbMEIType, mbReadDeviceIDcode,
			mbObjectId };
	mReqSize = 5;
	mReqBuf.assign(req, req + mReqSize);

}

std::ostream& QueryMsg0x2b::print(std::ostream& stream) const {
	stream << "[debug] " << "QueryMsg0x2b" << std::endl;
	QueryMsg::print(stream);
	return stream;
}

uint8_t* QueryMsg0x2b::getDataFromResponse() {
	//TODO проверитьь
	return &mRspBuf[9]; // для функции чтения, данные начинаются с 4 байта
}

int16_t QueryMsg0x2b::getDataSize() {
	return mRspSize - 9; // 3 байта - заголовок
}

unsigned short QueryMsg0x2b::getUMKA03Id() {
	//TODO проверить, поменял на uint8_t
	uint8_t resp[4] = { mRspBuf[mRspBuf.size() - 5],
			mRspBuf[mRspBuf.size() - 4], mRspBuf[mRspBuf.size() - 2],
			mRspBuf[mRspBuf.size() - 1] };
	return (unsigned short) (atoi(reinterpret_cast<char*>(resp)));
}

int QueryMsg0x2b::getUMKA27Id() {
	uint8_t resp[5] = { mRspBuf[mRspBuf.size() - 5],
			mRspBuf[mRspBuf.size() - 4], mRspBuf[mRspBuf.size() - 3],
			mRspBuf[mRspBuf.size() - 2], mRspBuf[mRspBuf.size() - 1] };
	return (unsigned short) (atoi(reinterpret_cast<char*>(resp)));
}

