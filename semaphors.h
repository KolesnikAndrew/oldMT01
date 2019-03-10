/*
 * semaphors.h
 *
 *  Created on: 2 квіт. 2018 р.
 *      Author: v7r
 */

#ifndef SEMAPHORS_H_
#define SEMAPHORS_H_

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include<string>

namespace IPC {
/**
 *   Signals a problem with the execution of a Semaphore call.
 */

class CSemaphoreException: public std::exception {
public:
	/**
	 *   Construct a SharedMemoryException with a explanatory message.
	 *   @param message explanatory message
	 *   @param bSysMsg true if system message (from strerror(errno))
	 *   should be postfixed to the user provided message
	 */
	CSemaphoreException(const std::string &message,
			bool bSysMsg = false) throw () {
	}
	;

	/** Destructor.
	 * Virtual to allow for subclassing.
	 */
	virtual ~CSemaphoreException() throw () {
	}
	;

	/** Returns a pointer to the (constant) error description.
	 *  @return A pointer to a \c const \c char*. The underlying memory
	 *          is in posession of the \c Exception object. Callers \a must
	 *          not attempt to free the memory.
	 */
	virtual const char* what() const throw () {
		return m_sMsg.c_str();
	}

protected:
	/** Error message.
	 */
	std::string m_sMsg;
};

enum CreationMode {
	CREATE_IF_NOT_EXISTS = O_CREAT, CREATE_ERROR_IF_EXISTS = O_CREAT | O_EXCL
};

enum Accessbility {
	INSIDE_PROCESS = 0, ACROSS_PROCESS = 1
};

enum Permission {
	READ = S_IRUSR, WRITE = S_IWUSR, READ_WRITE = READ | WRITE
};

class ISemaphore {
public:
	ISemaphore() {
	}
	virtual ~ISemaphore() {
	}
	virtual void Wait() = 0;
	virtual void Post() = 0;
	virtual void TryWait() = 0;
	virtual void TimedWait(int nTimeMilliseconds) = 0;
	virtual void Close() = 0;
	virtual void Remove() = 0;
};

class AbstarctSemaphore: public ISemaphore {

public:
	AbstarctSemaphore(unsigned int nVal);
	virtual void Wait();
	virtual void Post();
	virtual void TryWait();
	virtual void TimedWait(int nTimeMilliseconds);
	virtual void Close() = 0;
	virtual void Remove() = 0;
protected:
	sem_t * m_pSemId;
	unsigned int m_nVal;
};

class CNamedSemaphore: public AbstarctSemaphore {

public:
	CNamedSemaphore(const std::string& sName, unsigned int nVal,
			CreationMode mode = CREATE_IF_NOT_EXISTS, Permission per =
					READ_WRITE);
	virtual ~CNamedSemaphore();
	virtual void Close();
	virtual void Remove();
private:
	void Open();
	const std::string& m_sName;
	bool m_bCloseOnExit;
	CreationMode m_Mode;
	Permission m_Per;
};

class CAnoymousSemaphore: public AbstarctSemaphore {

public:
	CAnoymousSemaphore(unsigned int nVal, Accessbility access = INSIDE_PROCESS);
	virtual ~CAnoymousSemaphore();
	virtual void Close();
	virtual void Remove();
private:
	void Open();
	Accessbility m_nAccessbility;
};
}


#endif /* SEMAPHORS_H_ */
