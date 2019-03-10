/*
 * devicemodemstat.cpp
 *
 *  Created on: 30 серп. 2018 р.
 *      Author: v7r
 */

#include "devicemodemstat.h"
//#include <sys/types.h>
#include <sys/stat.h>
#include "parametermodbusint.h"
#include <algorithm>

using namespace std;

const int64_t fixedTimeStamp = 1535760000; //September, 1, 2018

DeviceModemStat::DeviceModemStat() :
		mTimeStamp(0), mFname(F_MODEM_STAT_FILE), mIsMotorolaOrder(
				bigbrother::isBigEndianPlatform()), mIsOldDataSetupped(false) {

}

DeviceModemStat::~DeviceModemStat() {

}

bool DeviceModemStat::Init(const xmlpp::Node* pXMLNode, V7Port* pPort) {
	if (!V7Device::Init(pXMLNode, pPort)
			|| !V7DeviceModbus::Init(pXMLNode, pPort)) {
		return false;
	}
	setByteOrder();
	return true;
}

bool DeviceModemStat::loadData() {

	using namespace bigbrother;
	if (!isFileExist(F_FIFO_STAT_LOCK)) {
		mkfifo(F_FIFO_STAT_LOCK, 0444);
		if (isFileExist(mFname.c_str())) {
			bool status = loadCSV<int16_t, int64_t>(mFname, mData);
			if (!status) {
				std::cerr << "Can't read CSV file!!!" << std::endl;
			}
			if (!mIsOldDataSetupped) {
				mOldData = mData;
				mIsOldDataSetupped = true;
			}
			int r = remove(F_FIFO_STAT_LOCK);
			sync();
			if (mTimeStamp == getTimeStamp()) {
				return false;
			} else {
				mTimeStamp = getTimeStamp();
				return !static_cast<bool>(r);
			}
		} else {
			int r = remove(F_FIFO_STAT_LOCK);
			sync();
			return static_cast<bool>(r);
		}
	} else {
		return false; //файл занят, запись новых данных
	}

}

uint32_t DeviceModemStat::getTimeStamp() const {
	auto pos = mData.find(99);
	return pos != mData.end() ? pos->second : 0;
}

void DeviceModemStat::sendData() {
	using namespace bigbrother;
	using namespace std;
	gettimeofday(&mSessionTime, NULL);

	for (auto &x : mvpParameters) {
		auto param = dynamic_cast<V7ParameterModbus*>(x);

		auto id = param->getAddress();
		auto pos = mData.find(id);
		auto pos_old = mOldData.find(id);
		if (pos == mData.end() || pos_old == mOldData.end()) {
			continue;
		}
		auto data = pos->second;
		auto old_data = pos_old->second;
		std::vector<uint8_t> t;
		if (isNeedCalcDelta(id)) {
			t = convertAnyToUCharArrayReverse<int64_t>(abs(old_data - data));
#ifdef DEBUG
			cerr << old_data << "  ::  " << data << endl;
#endif
			pos_old->second = data;
		} else {
			t = convertAnyToUCharArrayReverse<int64_t>(data);
		}
		uint8_t *pData = t.data();
		const short delta = sizeof(int64_t) - param->getSize() * 2;
		param->setModbusValueToCurrentDataPipe(&mSessionTime, pData + delta,
				validState_type::valid);
	}

}

void DeviceModemStat::setByteOrder() {
	if (!mIsMotorolaOrder) {
		for (auto &x : mvpParameters) {
			dynamic_cast<V7ParameterModbus*>(x)->setParamByteOrder(
					byteOrder_type::motorola);
		}
	} else {
		for (auto &x : mvpParameters) {
			dynamic_cast<V7ParameterModbus*>(x)->setParamByteOrder(
					byteOrder_type::intel);
		}
	}
}

void DeviceModemStat::Session() {

	const uint32_t ts = bigbrother::getUnixTimeStamp() - mResetTime;
	const uint32_t sec_in_day = 86400;
	if (ts - mResetTime > 86400 || !mResetTime) {
		PRINTDEBUG("reset")
		resetTimeReadFlag();
		mResetTime = ts;
		sleep(10);
	}
	mData.clear();
	if (!loadData()) {
		return;
	}
	if (getTimeStamp() < fixedTimeStamp) {
		return;
	}
	sendData();
}

bool DeviceModemStat::isNeedCalcDelta(int16_t key) {
	return (key > 99 && key < 150); // see /usr/local/bin/main_stat.py
}

