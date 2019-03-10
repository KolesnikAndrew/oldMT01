/*
 * parametermodbusIEEE754.h
 *
 *  Created on: 11 квітня. 2018 р.
 *      Author: v7r
 */

#include "globals.h"
#include "parametermodbus.h"
#include "utilites.h"
#include "parametermodbusws.h"

#ifndef SRC_PARAMETERMODBUSIEEE754_H_
#define SRC_PARAMETERMODBUSIEEE754_H_
/**
 * @class ParameterModbusIEEE754
 * @brief Support IEEE 754 floating point arithmetic
 */

class ParameterModbusIEEE754: public V7ParameterModbus {

public:
	ParameterModbusIEEE754();
	virtual ~ParameterModbusIEEE754();
	virtual bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState) = 0;
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) {};
	virtual void writeParameterModbus() = 0;

protected:
	string mUnit; /**< Единицы измерения */
	double mMaxValue; /**< Значение мин. */
	double mMinValue; /**< Значение макс. */
	double mDelta; /**< Отклонение значения от текущего, которое инициирует запись нового значения в БД. */
	double mOldValue; /**< Старое значение */
	double mScaleFactor;
};

/**
 * @class ParameterModbusIEEE754Single
 * @brief ParameterModbusIEEE754
 */
class ParameterModbusIEEE754Single: public ParameterModbusIEEE754 {
public:
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) {};
	virtual void writeParameterModbus();
};

class ParameterModbusIEEE754SingleArray: public ParameterModbusIEEE754 {
public:
	ParameterModbusIEEE754SingleArray();
	virtual ~ParameterModbusIEEE754SingleArray();
	virtual bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) {};
	virtual void writeParameterModbus();
	int getArraySize() const;
	std::string getArrayData() const;
	validState_type getValidState() const;
	void setVlaues(struct timeval *tp, uint8_t* data,
			const validState_type& validState);
private:
	int mArraySize; /**< */
	struct timeval mTimeStamp; /**< */
	validState_type mState; /**< */
	std::string mData; /**< */

};

class ParameterModbusIEEE754SingleMatrix: public V7ParameterModbus {
public:
	ParameterModbusIEEE754SingleMatrix();
	~ParameterModbusIEEE754SingleMatrix();
	virtual bool initParam(const xmlpp::Node* pXMLNode, V7Device* pDevice);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint8_t* data, const validState_type& validState);
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
			uint16_t* data, const validState_type& validState) {
	}
	virtual void setModbusValueToCurrentDataPipe(struct timeval *tp,
				const std::string& data, const validState_type& validState) ;
	virtual void writeParameterModbus();
	std::vector<ParameterModbusIEEE754SingleArray*> getArray() const;
	void addData(const std::string& data);
	std::string getData() const;
	size_t getDataSize() const;
	void resetData();
	bool isDataReady();
	void setDataReadyState(const ParameterModbusWordState& state);
	ParameterModbusWordState getDataReadyState() const;
private:
	std::vector<ParameterModbusIEEE754SingleArray*> mArrays;
	struct timeval mTimeStamp;
	std::string mData; /**< */
	ParameterModbusWordState mState;
	std::string mOldData;


};

#endif /* SRC_PARAMETERMODBUSIEEE754_H_ */
