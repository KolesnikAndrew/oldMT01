/**
 * @file devicemodemstat.h
 *
 * @date Created on: 30 серп. 2018 р.
 * @author Author: v7r
 */

#ifndef SRC_DEVICEMODEMSTAT_H_
#define SRC_DEVICEMODEMSTAT_H_



#include <map>
#include <fstream>
#include <iostream>

#include "globals.h"
#include "devicemodbus.h"
/**
 * @class DeviceModemStat
 * @brief Fake modbus device for collect various statistics from MT01/MT02
 */
class DeviceModemStat: public V7DeviceModbus {
public:
	DeviceModemStat();
	virtual ~DeviceModemStat();
	virtual bool Init(const xmlpp::Node* pXMLNode, V7Port* pPort);
	virtual void Session();
private:
	unsigned long mTimeStamp; //! метка файла
	std::map<int16_t, int64_t> mData; //! актуальные данные
	std::map<int16_t, int64_t> mOldData; //! старые данные
	std::string mFname; //! файл данных
	bool mIsMotorolaOrder; //! порядок байт
	bool mIsOldDataSetupped; //! сохраняем старые данные в первую вычитку
	/**
	 * @brief Корректируем порядок следования байт
	 */
	void setByteOrder();
	/**
	 * @brief отсылаем данне
	 */
	void sendData();
	/**
	 * @brief Читаем данные из файла
	 * @return trueб если была удачная вычитка и данные новыетуально
	 */
	bool loadData();
	/**
	 * @brief Вычисляем ай-ди параметров, которые должны передаваться разницей значений
	 * @param key номер параметра
	 * @return  true,  если 99 < id < 150
	 */
	bool isNeedCalcDelta(int16_t key);
	/**
	 * @brief вычитываем метку времени из файла
	 * @return метка времени в unix time
	 */
	uint32_t getTimeStamp() const;


};

#endif /* SRC_DEVICEMODEMSTAT_H_ */
