/*
 * semaphors.cpp
 *
 *  Created on: 2 квіт. 2018 р.
 *      Author: v7r
 */
#include "semaphors.h"
#include <time.h>
#include <stdexcept>

namespace IPC {
AbstarctSemaphore::AbstarctSemaphore(unsigned int nVal) :
		m_nVal(nVal), m_pSemId(NULL) {

}

void AbstarctSemaphore::Wait() {
	int nRet = sem_wait(m_pSemId);
	if (nRet < 0) {
		switch (errno) {
		case EINVAL:
			throw CSemaphoreException("InValid semaphore");
			break;
		case EOVERFLOW:
			throw CSemaphoreException(
					"The maximum allowable value for a semaphore would be exceeded");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore ");
			break;
		}
	}
}

void AbstarctSemaphore::Post() {
	int nRet = sem_post(m_pSemId);
	if (nRet < 0) {
		switch (errno) {
		case EINTR:
			throw CSemaphoreException(
					" The call was interrupted by a signal handler");
			break;
		case EINVAL:
			throw CSemaphoreException("InValid semaphore");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore Post");
			break;
		}
	}
}

void AbstarctSemaphore::TryWait() {
	int nRet = sem_trywait(m_pSemId);
	if (nRet < 0) {
		switch (errno) {
		case EINTR:
			throw CSemaphoreException(
					"TryWait Operation was interrupted by a signal handler");
			break;
		case EINVAL:
			throw CSemaphoreException("TryWait on InValid semaphore");
			break;
		case EAGAIN:
			throw CSemaphoreException(
					"TryWait operation could not be performed without blocking");
			break;
		default:
			throw CSemaphoreException("Invalid exception occurred in TryWait");
			break;
		}
	}
}

void AbstarctSemaphore::TimedWait(int nTimeMilliseconds) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += nTimeMilliseconds / 1000;
	int nRet = sem_timedwait(m_pSemId, &ts);
	if (nRet < 0) {
		switch (errno) {
		case EINTR:
			throw CSemaphoreException(
					"TimedWait Operation was interrupted by a signal handler");
			break;
		case EINVAL:
			throw CSemaphoreException("InValid semaphore on TimedWait");
			break;
		case ETIMEDOUT:
			throw CSemaphoreException(
					"TryWait operation he call has timed out before the semaphore could be locked");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in TimedWait");
			break;
		}
	}
}

CNamedSemaphore::CNamedSemaphore(const std::string& sName, unsigned int nVal,
		CreationMode mode, Permission per) :
		AbstarctSemaphore(nVal), m_sName(sName), m_Mode(mode), m_Per(per), m_bCloseOnExit(
				false) {
	Open();
}

void CNamedSemaphore::Open() {
	m_pSemId = sem_open(m_sName.c_str(), m_Mode, m_Per, m_nVal);
	if (m_pSemId < 0) {
		switch (errno) {
		case EACCES:
			throw CSemaphoreException("Permission Exception in Open");
			break;
		case EEXIST:
			throw CSemaphoreException(
					"Semaphore object specified by name already exists.");
			break;
		case EINTR:
			throw CSemaphoreException(
					"The operation was interrupted by a signal");
			break;
		case EINVAL:
			throw CSemaphoreException(
					"Invalid Semaphore memory name passed or semaphore value exceeds ");
			break;
		case EMFILE:
			throw CSemaphoreException(
					"The process already has the maximum number of files open.");
			break;
		case ENAMETOOLONG:
			throw CSemaphoreException(
					"The length of name exceeds SEM_NAME_LEN.");
			break;
		case ENFILE:
			throw CSemaphoreException(
					"The limit on the total number of files open on the system has been reached");
			break;
		case ENOSPC:
			throw CSemaphoreException("There is insufficient space");
			break;
		case ENOENT:
			throw CSemaphoreException(" named semaphore does not exist");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore creation");
			break;
		}
	}
}

void CNamedSemaphore::Close() {
	int nRet = sem_close(m_pSemId);
	if (nRet < 0) {
		switch (errno) {
		case EINVAL:
			throw CSemaphoreException("InValid semaphore");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore Close");
			break;
		}
	}

	m_bCloseOnExit = true;
}

void CNamedSemaphore::Remove() {
	int nRet = sem_unlink(m_sName.c_str());
	if (nRet < 0) {
		switch (errno) {
		case EACCES:
			throw CSemaphoreException(
					"The caller does not have permission to remove this semaphore.");
			break;
		case ENAMETOOLONG:
			throw CSemaphoreException("Semaphore name was too long");
			break;
		case ENOENT:
			throw CSemaphoreException(
					"There is no semaphore with the given name.");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore Remove");
			break;
		}
	}
}

CNamedSemaphore::~CNamedSemaphore() {
	if (!m_bCloseOnExit) {
		Close();
	}
}

CAnoymousSemaphore::CAnoymousSemaphore(unsigned int nVal,
		Accessbility accessbility) :
		AbstarctSemaphore(nVal), m_nAccessbility(accessbility) {
	Open();
}

void CAnoymousSemaphore::Close() {
	Remove();
}

void CAnoymousSemaphore::Remove() {
	int nRet = sem_destroy(m_pSemId);
	if (nRet != 0) {
		switch (errno) {
		case EINVAL:
			throw CSemaphoreException("Invalid Semaphore");
			break;
		case EBUSY:
			throw CSemaphoreException(
					"There are currently processes blocked on the semaphore.");
			break;
		case ENOSYS:
			throw CSemaphoreException(
					"POSIX sem_destroy() is not supported by this implementation");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore Remove");
			break;

		}
	}
}

void CAnoymousSemaphore::Open() {
	m_pSemId = new sem_t;
	int nRet = sem_init(m_pSemId, m_nAccessbility, m_nVal);
	if (nRet != 0) {
		printf("\nError Inside Open");
		switch (errno) {
		case EINVAL:
			throw CSemaphoreException("semaphore value exceeds ");
			break;
		case ENOSPC:
			throw CSemaphoreException(
					"The limit on semaphores(SEM_NSEMS_MAX) has been reached");
			break;
		case ENOSYS:
			throw CSemaphoreException(
					"POSIX sem_init() is not supported by this implementation");
			break;
		case EPERM:
			throw CSemaphoreException(
					"The process lacks the appropriate privileges to initialize the semaphore.");
			break;
		default:
			throw CSemaphoreException(
					"Invalid exception occurred in semaphore creation");
			break;

		}
	}

}

CAnoymousSemaphore::~CAnoymousSemaphore() {
	printf("deleting sem id");
	delete m_pSemId;
}

}

