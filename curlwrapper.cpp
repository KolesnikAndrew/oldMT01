#include <cstdlib>
#include "curlwrapper.h"
#include "globals.h"
#include <sys/stat.h>
#include "utilites.h"

//////////////////////////// cURL wrapper /////////////////////////////////////

using namespace std;

LibCurlWrapper::LibCurlWrapper(const serverSettings& server) :
		mServerSettings(server), mCurlResult(CURLE_OK), mOptVerbose(0L), mpCurl(
				0), mOptConnectTimeout(180), mOptTimeout(180), mOptCookieFilePath(
		F_COOKIE_PATH), mHttpsCertVerifyAuth(0L), mHttpsCertVerifyHost(0L) {

	curl_global_init(CURL_GLOBAL_NOTHING); //?
}

LibCurlWrapper::~LibCurlWrapper() {
	curl_global_cleanup();
}

void LibCurlWrapper::setCookie(const std::string& cookiePath) {
	mOptCookieFilePath = cookiePath;
}

bool LibCurlWrapper::compressFile(const std::string& pathToFile) {
	const std::string cmd = "gzip -f -9 < " + pathToFile + " > " + pathToFile
			+ ".gz";
	const int rez = system(cmd.c_str()); //ждем результат
	return (rez == 0);
}

CURLcode LibCurlWrapper::post(const std::string& requestURL,
		const std::string& msg) {
	PRINTDEBUG("in post...");
	mBuffer.clear();
	mpCurl = curl_easy_init();        //инициализация
	if (mpCurl) {
		struct curl_slist *headers = NULL;
		string tmp = mServerSettings.mProxy.getProxyServer();
		if (tmp.size()) {
			curl_easy_setopt(mpCurl, CURLOPT_PROXY, tmp.c_str()); //прокси настройки
			tmp = mServerSettings.mProxy.getProxyUser();
			if (tmp.size()) {
				curl_easy_setopt(mpCurl, CURLOPT_PROXYUSERPWD, tmp.c_str()); //прокси настройки
			}
		}
		mOptErrorBuffer[0] = 0;
		curl_easy_setopt(mpCurl, CURLOPT_ERRORBUFFER, mOptErrorBuffer);
		curl_easy_setopt(mpCurl, CURLOPT_VERBOSE, mOptVerbose);
		curl_easy_setopt(mpCurl, CURLOPT_TIMEOUT, mOptTimeout); //таймаут соединения
		curl_easy_setopt(mpCurl, CURLOPT_CONNECTTIMEOUT, mOptConnectTimeout); //таймаут соединения
		//////////
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYPEER, mHttpsCertVerifyAuth);
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYHOST, mHttpsCertVerifyHost);
		////////////
		curl_easy_setopt(mpCurl, CURLOPT_URL,
				(mServerSettings.mServerITA + requestURL).c_str()); //адресс запроса
		curl_easy_setopt(mpCurl, CURLOPT_WRITEFUNCTION, writer); //указываем функцию обратного вызова для записи получаемых данных
		curl_easy_setopt(mpCurl, CURLOPT_WRITEDATA, &mBuffer); //сохраняем html код cтраницы в строку content
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEFILE,
				mOptCookieFilePath.c_str()); // разрешаем использовать куки
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEJAR, mOptCookieFilePath.c_str());
		/* POST- запрос c авторизацией ( просходит получение кукисов ) */
		curl_easy_setopt(mpCurl, CURLOPT_POST, 1L);
		//curl_easy_setopt(mpCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0 );
		if (msg.size()) {
			curl_easy_setopt(mpCurl, CURLOPT_POSTFIELDS, msg.c_str());
		}
		mCurlResult = curl_easy_perform(mpCurl); //посылка запроса на сервер
		curl_easy_cleanup(mpCurl); //звершение
		if (headers) {
			curl_slist_free_all(headers); //освобождение хедера
		}
	} else {
		mCurlResult = CURLE_FAILED_INIT;
	}
	return mCurlResult;

}

CURLcode LibCurlWrapper::post(const std::string& requestURL,
		const std::string& pathToFile, bool gz, int par_id, long dt,
		const std::string& formName) {

	mBuffer.clear();
	mpCurl = curl_easy_init();        //инициализация
	if (mpCurl) {
		struct curl_httppost* post = NULL;
		struct curl_httppost *last = NULL;
		struct curl_slist *headers = NULL;
		const string tmp = mServerSettings.mProxy.getProxyServer();
		if (tmp.size()) {
			curl_easy_setopt(mpCurl, CURLOPT_PROXY, tmp.c_str()); //прокси настройки
			string tmp = mServerSettings.mProxy.getProxyUser();
			if (tmp.size()) {
				curl_easy_setopt(mpCurl, CURLOPT_PROXYUSERPWD, tmp.c_str()); //прокси настройки
			}
		}

		if (par_id != -1) {
			PRINTDEBUG2("paramID: ", par_id);
			curl_formadd(&post, &last, CURLFORM_COPYNAME, "par_id",
					CURLFORM_COPYCONTENTS, std::to_string(par_id).c_str(),
					CURLFORM_END); //устанавливаем формат для передачи файла, и прикрепляем файл
		}

		if (dt) {
			PRINTDEBUG2("datestamp:  ", dt)
			curl_formadd(&post, &last, CURLFORM_COPYNAME, "dt",
					CURLFORM_COPYCONTENTS, std::to_string(dt).c_str(),
					CURLFORM_END); //устанавливаем формат для передачи файла, и прикрепляем файл
		}

		if (formName != "XmlFile") {
			PRINTDEBUG2("XmlFile will be sent: ", formName);
			curl_formadd(&post, &last, CURLFORM_COPYNAME, formName.c_str(),
					CURLFORM_FILE, pathToFile.c_str(), CURLFORM_CONTENTTYPE,
					"application/octet-stream", CURLFORM_END);

		} else {
			if (gz) {
				if (!compressFile(pathToFile)) {
					clog << "Error while compress file!" << endl;
				}
				curl_formadd(&post, &last, CURLFORM_COPYNAME, formName.c_str(),
						CURLFORM_FILE, (pathToFile + ".gz").c_str(),
						CURLFORM_CONTENTTYPE, "application/gzip", CURLFORM_END);

			} else {
				PRINTDEBUG2("Path to file: ", pathToFile);
				curl_formadd(&post, &last, CURLFORM_COPYNAME, formName.c_str(),
						CURLFORM_FILE, (pathToFile).c_str(),
						CURLFORM_CONTENTTYPE, "text/xml", CURLFORM_END);
			}
		}

		headers = curl_slist_append(headers,
				"Content-Type: multipart/form-data"); //для посылки файла выставляем Content-Type
		curl_easy_setopt(mpCurl, CURLOPT_POST, 1L);
		mOptErrorBuffer[0] = 0;
		curl_easy_setopt(mpCurl, CURLOPT_ERRORBUFFER, mOptErrorBuffer);
		curl_easy_setopt(mpCurl, CURLOPT_VERBOSE, mOptVerbose);
		curl_easy_setopt(mpCurl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(mpCurl, CURLOPT_HTTPPOST, post); //формируем POST с нашим файлом
		//////////
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYPEER, mHttpsCertVerifyAuth);
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYHOST, mHttpsCertVerifyHost);
		////////////
		curl_easy_setopt(mpCurl, CURLOPT_URL,
				(mServerSettings.mServerITA + requestURL).c_str());
		curl_easy_setopt(mpCurl, CURLOPT_TIMEOUT, mOptTimeout);
		curl_easy_setopt(mpCurl, CURLOPT_CONNECTTIMEOUT, mOptConnectTimeout);
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEFILE,
				mOptCookieFilePath.c_str()); //используем куки файлы
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEJAR, mOptCookieFilePath.c_str());
		curl_easy_setopt(mpCurl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(mpCurl, CURLOPT_WRITEDATA, &mBuffer); //строка, в которую пишутся ответ

		mCurlResult = curl_easy_perform(mpCurl); //посылка запроса на сервер
		curl_easy_cleanup(mpCurl); //звершение
		if (headers) {
			curl_slist_free_all(headers); //освобождение хедера
		}
		if (post) {
			curl_formfree(post); //освобождение посылки
		}
	} else {
		mCurlResult = CURLE_FAILED_INIT;
	}
	return mCurlResult;
}

CURLcode LibCurlWrapper::get(const std::string& requestURL) {
	mBuffer.clear();
	mpCurl = curl_easy_init();   //инициализация
	string Address = mServerSettings.mServerITA + requestURL;

	if (mpCurl) {
		char errbuf[CURL_ERROR_SIZE];
		string tmp = mServerSettings.mProxy.getProxyServer();
		if (tmp.size()) {
			curl_easy_setopt(mpCurl, CURLOPT_PROXY, tmp.c_str()); //прокси настройки
			tmp = mServerSettings.mProxy.getProxyUser();
			if (tmp.size()) {
				curl_easy_setopt(mpCurl, CURLOPT_PROXYUSERPWD, tmp.c_str()); //прокси настройки
			}
		}
		mOptErrorBuffer[0] = 0;
		curl_easy_setopt(mpCurl, CURLOPT_ERRORBUFFER, mOptErrorBuffer);
		curl_easy_setopt(mpCurl, CURLOPT_VERBOSE, mOptVerbose);
		curl_easy_setopt(mpCurl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(mpCurl, CURLOPT_TIMEOUT, mOptTimeout);
		curl_easy_setopt(mpCurl, CURLOPT_CONNECTTIMEOUT, mOptConnectTimeout);
		//////////
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYPEER, mHttpsCertVerifyAuth);
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYHOST, mHttpsCertVerifyHost);
		////////////
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEFILE,
				mOptCookieFilePath.c_str()); //чтение
		curl_easy_setopt(mpCurl, CURLOPT_COOKIEJAR, mOptCookieFilePath.c_str()); //запись
		curl_easy_setopt(mpCurl, CURLOPT_URL,
				(mServerSettings.mServerITA + requestURL).c_str());
		curl_easy_setopt(mpCurl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(mpCurl, CURLOPT_WRITEDATA, &mBuffer); // ответ сервера
		mCurlResult = curl_easy_perform(mpCurl);
		curl_easy_cleanup(mpCurl);
	} else {
		mCurlResult = CURLE_FAILED_INIT;
	}
	return mCurlResult;
}

std::string LibCurlWrapper::getResponseBuffer() const {
	return mBuffer;
}

serverSettings LibCurlWrapper::getServerSettings() const {
	return mServerSettings;
}

void LibCurlWrapper::getServerSettings(const serverSettings& value) {
	mServerSettings = value;

}
std::string LibCurlWrapper::getStrErrorCode() const {
	return curl_easy_strerror(mCurlResult);
}

int LibCurlWrapper::writer(char* data, size_t size, size_t nmemb,
		string* buffer) {
	int result = 0; //переменная - результат, по умолчанию нулевая
	if (buffer != NULL) {   //проверяем буфер
		buffer->append(data, size * nmemb); //добавляем к буферу строки из data, в количестве nmemb
		result = size * nmemb;   //вычисляем объем принятых данных
	}
	return result;  //вовзращаем результат
}

size_t LibCurlWrapper::my_fwrite(void *buffer, size_t size, size_t nmemb,
		void *stream) {
	struct FtpFile *out = (struct FtpFile *) stream;
	if (out && !out->stream) {
		/* open file for writing */
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; /* failure, can't open file to write */
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

size_t LibCurlWrapper::my_fwrite_resume(void *buffer, size_t size, size_t nmemb,
		void *stream) {
	struct FtpFile *out = (struct FtpFile *) stream;
	if (out && !out->stream) {
		/* open file for writing */
		out->stream = fopen(out->filename, "ab");
		if (!out->stream) {
			return -1; /* failure, can't open file to write */
		}
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

CURLcode LibCurlWrapper::get(const std::string& requestURL,
		const std::string& fileName, const std::string& ftpUser,
		const std::string& ftpPass, bool flagResumeFrom) {
	struct FtpFile ftpfileTAR = { fileName.c_str(), NULL };
	mpCurl = curl_easy_init();
	if (mpCurl) {
		mOptErrorBuffer[0] = 0;
		curl_easy_setopt(mpCurl, CURLOPT_URL,
				(mServerSettings.mServerFtp + requestURL).c_str());
		curl_easy_setopt(mpCurl, CURLOPT_WRITEDATA, &ftpfileTAR);
		//ERROR: тут хардкор логин и пароль для FTP подключения!!!
		curl_easy_setopt(mpCurl, CURLOPT_USERNAME, ftpUser.c_str());
		curl_easy_setopt(mpCurl, CURLOPT_PASSWORD, ftpPass.c_str());
		curl_easy_setopt(mpCurl, CURLOPT_TIMEOUT, mOptTimeout); //таймаут соединения
		curl_easy_setopt(mpCurl, CURLOPT_CONNECTTIMEOUT, mOptConnectTimeout); //таймаут соединения
		curl_easy_setopt(mpCurl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(mpCurl, CURLOPT_VERBOSE, mOptVerbose);
		//////////
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYPEER, mHttpsCertVerifyAuth);
//		curl_easy_setopt(mpCurl, CURLOPT_SSL_VERIFYHOST, mHttpsCertVerifyHost);
		////////////
		if (flagResumeFrom) {
			curl_easy_setopt(mpCurl, CURLOPT_WRITEFUNCTION, my_fwrite_resume);
			curl_easy_setopt(mpCurl, CURLOPT_RESUME_FROM,
					bigbrother::getFileSize(fileName));
		} else {
			curl_easy_setopt(mpCurl, CURLOPT_WRITEFUNCTION, my_fwrite);
		}
		mCurlResult = curl_easy_perform(mpCurl);
		curl_easy_cleanup(mpCurl);

	} else {
		mCurlResult = CURLE_FAILED_INIT;
	}
	if (ftpfileTAR.stream)
		fclose(ftpfileTAR.stream); /* close the local file */
	return mCurlResult;
}

long LibCurlWrapper::getResponseCode() const {

	long ret_code;
	curl_easy_getinfo(mpCurl, CURLINFO_RESPONSE_CODE, &ret_code);
	return ret_code;
}

std::ostream& operator<<(std::ostream&stream, const LibCurlWrapper& obj) {
	return stream << "[debug]:: " << obj.mServerSettings.mServerITA << ", "
			<< obj.mServerSettings.mServerFtp;
}

std::ostream& operator<<(std::ostream&stream, const LibCurlWrapper* obj) {
	return stream << "[debug]:: " << obj->mServerSettings.mServerITA << ", "
			<< obj->mServerSettings.mServerFtp;
}

