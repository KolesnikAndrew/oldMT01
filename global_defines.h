/*
 * global_defines.h
 *
 *  Created on: 24 вер. 2018 р.
 *      Author: v7r
 */

#ifndef SRC_GLOBAL_DEFINES_H_
#define SRC_GLOBAL_DEFINES_H_

#ifndef DEBUG
/**
 * @def PREFIX_PATH
 * @brief префикс к файлам конфигурации и логам, которые нужно сохранить
 */
#define PREFIX_PATH ""
#else
#define PREFIX_PATH "/home/v7r/projects/MT01/bigbrother/run"
#endif

#define LOG PREFIX_PATH"/logfile"
/**
 * @def TMP_DIR
 */
#define TMP_DIR PREFIX_PATH"/tmp"
/**
 * @def TMP_DIR_MT01
 */
#define TMP_DIR_MT01 TMP_DIR"/MT01"
/**
 * @def CONFIG_FILE_PATH
 */
#define CONFIG_FILE_PATH          PREFIX_PATH"/etc/MT01/ModemConfig.xml"
/**
 * @def CONFIG_MINIMAL_FILE_PATH
 */
#define CONFIG_FILE_MINIMAL_PATH  PREFIX_PATH"/etc/MT01/ModemConfigMinimal.xml"
/**
 * @def  BASEDATA_FILE_PATH
 */
#define BASEDATA_FILE_PATH        PREFIX_PATH"/etc/MT01/BaseData"
/**
 * @def  TIMESTAMP_FILE_PATH
 */
#define TIMESTAMP_FILE_PATH       PREFIX_PATH"/etc/MT01/TimeStamp"
/**
 * @def REBOOTSTAMP_FILE_PATH
 */
#define MODBUS_ERR_FILE_PATH     PREFIX_PATH"/etc/MT01/ModbusErrStamp"
/**
 * @def CNT_PACKETS__FILE_PATH
 */
#define CNT_PACKETS__FILE_PATH    PREFIX_PATH"/etc/MT01/CntPackets"
/**
 * @def CONFIG_FILE_AT27_PATH
 */
#define CONFIG_FILE_AT27_PATH     PREFIX_PATH"/etc/MT01/ConfigAT27"
/**
 * @def NEED_SAVELOG_IN_MODEM
 */
#define NEED_SCADA_SAVELOG_IN_MODEM
/**
 * @def CURRENT_DATA_FILE_PATH
 */
#define CURRENT_DATA_FILE_PATH      TMP_DIR_MT01"/current_data_pipe"
/**
 * @def SETPOINT_DATA_FILE_PATH
 */
#define SETPOINT_DATA_FILE_PATH     TMP_DIR_MT01"/setpoint_data_pipe"
/**
 * @def SETPOINT_CONFIRM_FILE_PATH
 */
#define SETPOINT_CONFIRM_FILE_PATH  TMP_DIR_MT01"/setpoint_confirm_pipe"
/**
 * @def NLINE_STATUS_PATH
 */
#define F_ONLINE_STATUS TMP_DIR_MT01"/modem_online"

/**
 * @def TMP_DIR_MT01_DYN
 */
#define TMP_DIR_MT01_DYN  TMP_DIR_MT01"/dyn"
/**
 * @def JOURNAL_PATH
 */
#define JOURNAL_PATH TMP_DIR_MT01
/**
 * @def GZIP_SCRIPT_PATH
 */
#define GZIP_SCRIPT_PATH PREFIX_PATH"/usr/local/bin/gzip_journal -p "
/**
 * @def MNT_ROOT
 */
#define MNT_ROOT PREFIX_PATH"/root"
/**
 * @def fLINUX_NAND_PATH
 */
#define F_LINUX_NAND_PATH PREFIX_PATH"/root/imx28_ivt_linux_nand.sb"
/**
 * @def fROOTFS_NAND_PATH
 */
#define F_ROOTFS_NAND_PATH PREFIX_PATH"/root/rootfs_nand.tar.gz"
/**
 * @def fNEW_FILE_PATH PREFIX_PATH
 */
#define F_NEW_FILE_PATH PREFIX_PATH"/root/new_file.tar.gz"
/**
 * @def XML_FILE
 */
#define XML_FILE_ATTRIBUTE                "XmlFile"
/**
 * @def  fCOOKIE_PATH
 */
#define F_COOKIE_PATH             TMP_DIR_MT01"/cookiefile.txt"
/**
 * @def fModemConfigToServ
 */
#define F_MODEM_CONFIG_TO_SRV       TMP_DIR_MT01"/ModemConfigToServ.xml"
/**
 * @def ARCHIVES_SEND_VALUES
 */
#define F_ARCHIVE_SEND_VALUES      TMP_DIR_MT01"/archives_send_values.xml"
/**
 * @def ARCHIVES_CONFIRM
 */
#define F_ARCHIVES_CONFIRM         TMP_DIR_MT01"/archives_confirm.xml"
/**
 * @def ARCHIVES_STATUS
 */
#define F_ARCHIVES_STATUS          TMP_DIR_MT01"/archives_status.xml"
/**
 * @def fCASH_SQ_PATH
 */
#define F_CASH_SQ_PATH            TMP_DIR_MT01"/cash_SQ"
/**
 * @def fCASH_SQ_XML_PATH
 */
#define F_CASH_SQ_XML_PATH        TMP_DIR_MT01"/cash_SQ.xml"
/**
 * @def fSendValues
 */
#define F_SEND_VALUES              TMP_DIR_MT01"/SendValues.xml"
/**
 * @def fSendValuesArchive
 */
#define F_SEND_VALUES_ARCHIVE       TMP_DIR_MT01"/SendValuesArchive.xml"
/**
 * @def fConfirmInputDataProcess
 */
#define F_CONFIRM_INPUT_DATA_PROCESS TMP_DIR_MT01"/ConfirmInputDataProcess.xml"
/**
 * @def fJournalProcess
 */
#define F_JOURNAL_PROCESS          TMP_DIR_MT01"/JournalProcess.xml"
/**
 * @def fConnectionStatus
 */
#define F_CONNECTION_STATUS        TMP_DIR_MT01"/ConnectionStatus.xml"
/**
 * @def fTempArchiveFile
 */
#define F_TEMP_ARCHIVE_FILE         TMP_DIR_MT01"/TempArchiveFile.xml"
/**
 * @def F_MODEM_STAT_FILE
 */
#define F_MODEM_STAT_FILE TMP_DIR_MT01"/mt01.csv"

#define F_FIFO_STAT_LOCK TMP_DIR_MT01"/mt01.csv.lock"
/**
 * @def
 */
#define BB_LED_BLINK PREFIX_PATH"/usr/local/bin/bb_led_blink.sh"

// debug messages
#ifdef DEBUG_MSG
/**
 * @def PRINTDEBUG
 */
#define PRINTDEBUG(debugString) std::clog << "[debug]: "<<(debugString) << std::endl;
/**
 * @def PRINTDEBUG2
 */
#define PRINTDEBUG2(debugString, debugValue) std::clog << "[debug]: " << (debugString) << (debugValue) << std::endl;
#else
#define PRINTDEBUG(debugString)
#define PRINTDEBUG2(debugString, debugValue)
#endif



/**
 * @def DATE_TIME_SIZE
 * @brief Размер строки под временной штамп
 */
#define DATE_TIME_SIZE  20
/// ppp ///
#define F_PPP_DEF                 PREFIX_PATH"/etc/ppp/chat_dialog.def"
#define F_PPP_2G                  PREFIX_PATH"/etc/ppp/chat_dialog.def_2g"
#define F_PPP_3G                  PREFIX_PATH"/etc/ppp/chat_dialog.def_3g"


/**
 * @def PDS_FILE_PATH
 */
#define PDS_FILE_PATH     TMP_DIR_MT01"/0x01.mem"
/**
 * @def KN24_CRASH_FILE
 */
#define TMP_DIR_KN24      TMP_DIR_MT01"/kn24"
#define KN24_CRASH_FILE   TMP_DIR_MT01"/0x18.mem"
#define KN24_LOG_TDRIVE   TMP_DIR_MT01"/kn24_crash"

#endif /* SRC_GLOBAL_DEFINES_H_ */
