/**
 * @file ParameterModbusFileKN24.h
 * @date Created on: 8 жовт. 2018 р.
 * @author: v7r
 * @warning In current release checking of the PDS type is not realized!
 * @details Протокол стыковки Р-24Е и НВСА изм 24
 */

#ifndef SRC_PARAMETERMODBUSFILEKN24_H_
#define SRC_PARAMETERMODBUSFILEKN24_H_

#include "parametermodbusfile.h"
#include "KN24defs.h"

/**
 * @class ParameterModbusFileKN24
 * @brief Class for read files from KN24
 *
 */
class ParameterModbusFileKN24: public ParameterModbusFile {

public:
	ParameterModbusFileKN24();
	virtual ~ParameterModbusFileKN24();
	virtual void Run();
	void setCrashHandleProcess();
	bool getCrashHandleResult() const;
	/**
	 * @brief
	 * @return
	 */
	uint16_t getCrashLogMask() const;
	/**
	 * @brief Read address of the status register
	 * @return address
	 */
	uint32_t getCrashLogRegAddress() const;
	/**
	 * @brief Extract mask from PDS file
	 * @param pdsHeader
	 * @return
	 */

private:
	/**
	 * @brief Return param from PDS file
	 * @param fileBuffer - file buffer
	 * @param pdsHeader - pointer to the PDS file header
	 * @param param_mun - parameter number (absolute)
	 * @return structure with parameter data
	 */
	DescriptorPrm_type getParamDescr(headOfParamDescFile_type* pdsHeader,
			uint16_t param_num);
	/**
	 * @brief Extract unit code from param
	 * @param param
	 * @return
	 */
	uint16_t getUnit(const DescriptorPrm_type& param);
	/**
	 * @brief Convert units index to ansi name
	 * @warning string is not human readable in *nix with utf8 locales
	 * @param nameIdx
	 * @return
	 */
	const char* getUnitName(uint16_t nameIdx);
	/**
	 * @brief extract Power muultiplyer (0..7)
	 * @param param
	 * @return
	 */
	uint16_t getPower(const DescriptorPrm_type& param);
	/**
	 * @brief getPntPrm
	 * @param log_header
	 * @return
	 */
	std::vector<uint16_t> getPntPrm(const headOfLogFile_type &log_header);
	/**
	 * @brief getKAmp
	 * @param log_header
	 * @return
	 */
	std::vector<uint32_t> getKAmp(const headOfLogFile_type &log_header);
	/**
	 * @brief convert kn24 to Tdrive log
	 * @return
	 */
	bool makeTdriveFormatLogFile();

	uint8_t mCrashLogFileNum;
	uint8_t mPdsFileNum;
	std::string mPdsFileBuffer;
	std::string mCrashLogBuffer;
	bool mIsCrashNow;
	/**
	 * @brief Read file #1 (parameters descriptor file))
	 * @return fileBuffer
	 */
	std::string readFile0x01();
	/**
	 * @brief Read file #24 (еmergency log file)
	 * @return fileBuffer
	 */
	std::string readFile0x18();
	/**
	 *
	 * @return
	 */
	bool setCrashLogHeaderData();
	/**
	 *
	 * @return
	 */
	bool setParamDescHeaderData();
	/**
	 * @fn getFileSizeFromParamDescrFile
	 * @param offset
	 * @return
	 */
	int32_t getParamDescrFileSize();
	/**
	 * @fn getFileSizeFromEmergencyFile
	 * @param offset
	 * @return
	 */
	int32_t getCrashLogFileSize();

	const int32_t mCrashLogHeaderSize = sizeof(headOfLogFile_type);
	const int32_t mPdsHeaderSize = sizeof(headOfParamDescFile_type);

	// variables
	bool mIsParamDescrFileRead; //!< флаг вычитки файла описателя параметров (0x01)
	bool mIsCrashLogRead; //
	int32_t mParamDescrFileSize; //!<  размер ФОП
	int32_t mCrashLogFileSize; //!<  размер лога
	headOfLogFile_type mCrashLogHeader;
	headOfParamDescFile_type mParamDescFileHeader;

	QueryMsgJrnAT27 request0x68(uint8_t fileNumber, uint8_t deviceAddress,
			uint32_t recAddress, uint8_t recCnt);

};

#endif /* SRC_PARAMETERMODBUSFILEKN24_H_ */
