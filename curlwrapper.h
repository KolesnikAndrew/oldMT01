#ifndef CURLWRAPPER_H_
#define CURLWRAPPER_H_

#include <iostream>
#include <cstdio>
#include <cassert>
#include <sstream>
#include "curl/curl.h"


struct FtpFile
{
    const char *filename;
    FILE *stream;
};

//ServerAddress, mServerFTP, mProxyAddress,
//            mProxyPort, mProxyUser, mProxyPassword, mSerialNumber, mpServer

/**
 * @brief Структура для хранения настроек прокси-сервера
 * @class proxySetting
 */
struct proxySetting
{
    proxySetting(const std::string& prxAddr = "", const std::string& prxPort =
            "", const std::string& prxUser = "", const std::string& prxPaaswd =
            "") :
            mProxyAddress(prxAddr), mProxyPort(prxPort), mProxyUser(prxUser), mProxyPassword(
                    prxPaaswd)
    {
    }
    /**
     * Если не заданы адрес  - нет прокси
     * @return
     */
    bool isEnpty()
    {
        return (mProxyAddress.size() == 0);
    }
    std::string getProxyServer() const
    {
        return mProxyAddress + ":" + mProxyPort;
    }
    std::string getProxyUser() const
    {
        return mProxyUser + ":" + mProxyPassword;
    }

    std::string mProxyAddress;
    std::string mProxyPort;
    std::string mProxyUser;
    std::string mProxyPassword;

};

/**
 * @brief Настройки подключения к серверу
 * @class serverSettings
 */
struct serverSettings
{
    serverSettings(const std::string& server, const std::string& ftp,
            const proxySetting& proxy) :
            mServerITA(server), mServerFtp(ftp), mProxy(proxy)
    {
#ifdef DEBUG
        assert(server.size() != 0);
        assert(ftp.size() != 0);
#endif
    }
    std::string mServerITA;
    std::string mServerFtp;
    proxySetting mProxy; // по умолчанию нет прокси
};

/**
 * @class LibCurlWrapper
 */
class LibCurlWrapper
{
public:
    LibCurlWrapper(const serverSettings& server);
    ~LibCurlWrapper();
    /**
     * @brief записываем куки в файл
     * @param cookiePath
     */
    void setCookie(const std::string& cookiePath);
    /**
     * @brief берем куки из файла
     * @return
     */
    std::string getCookie() const;
    /**
     * @brief Отсылка POST-запроса
     * @param requestURL
     * @param msg
     * @return
     */
    CURLcode post(const std::string& requestURL, const std::string& msg);
    /**
     * @brief Отсылка файла через POST
     * @param requstURL
     * @param pathToFile
     * @param gz - флаг сжатия
     * @return
     */
    CURLcode post(const std::string& requstURL, const std::string& pathToFile,
            bool gz, int par_id = -1, long dt = 0,  const std::string& formName="XmlFile");

    /**
     * @brief Получаем с сервера ответ и пишем во внутренний буфер
     * @param requestURL
     * @return Код выполнения запроса
     */
    /**
     * @brief Забрать данные с ftp c автоизацией
     * @param requestURL
     * @param ftpUser
     * @param ftpPass
     * @return
     */
    CURLcode get(const std::string& requestURL, const std::string& fileName,
            const std::string& ftpUser, const std::string& ftpPass,
            bool flagResumeFrom);
    /**
     *
     * @param data
     * @param size
     * @param nmemb
     * @param buffer
     * @return
     */
    CURLcode get(const std::string& requestURL);
    std::string getResponseBuffer() const;
    bool compressFile(const std::string& pathToFile);

    serverSettings getServerSettings() const;
    void getServerSettings(const serverSettings& value);
    std::string getStrErrorCode() const;
    long getResponseCode() const;
    // writer для libcurl
    static int writer(char *data, size_t size, size_t nmemb, std::string *buffer);
    static size_t my_fwrite(void *buffer, size_t size, size_t nmemb,
            void* stream);
    static size_t my_fwrite_resume(void *buffer, size_t size, size_t nmemb,
            void *stream);
    friend std::ostream& operator<< (std::ostream&stream, const LibCurlWrapper& obj);
    friend std::ostream& operator<< (std::ostream&stream, const LibCurlWrapper* obj);

private:
    serverSettings mServerSettings; //!< настройки сервера
    CURL *mpCurl; //!< указатель на структуру для выполнения запроса
    std::string mBuffer;
    // настройки соединения
    long mOptTimeout; //!<
    long mOptConnectTimeout; //!<
    char mOptErrorBuffer[CURL_ERROR_SIZE];
    std::string mOptCookieFilePath;
    std::string mOptURL;
    std::string mOptPostMsg;
    long mOptVerbose;
    CURLcode mCurlResult;
    long mHttpsCertVerifyAuth; //! 0 - no; 2 - yes
    long mHttpsCertVerifyHost;
    //  char mErrorBuffer[CURL_ERROR_SIZE];
};

#endif
