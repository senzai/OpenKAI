/*
 * ThreadBase.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#include "_ThreadBase.h"

namespace kai
{

_ThreadBase::_ThreadBase()
{
	m_bThreadON = false;
	m_threadID = 0;
	m_timeStamp = 0;
	m_dTime = 0.0;
	m_dTimeAvr = 0.0;
	m_tSleep = 0;
	pthread_mutex_init(&m_wakeupMutex, NULL);
	pthread_cond_init(&m_wakeupSignal, NULL);

	for(int i=0;i<NUM_MUTEX;i++)
	{
		pthread_mutex_init(&m_mutex[i], NULL);
	}

}

_ThreadBase::~_ThreadBase()
{
	pthread_mutex_destroy(&m_wakeupMutex);
	pthread_cond_destroy(&m_wakeupSignal);
}

void _ThreadBase::sleepThread(int32_t sec, int32_t usec)
{
	struct timeval now;
	struct timespec timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sec;
	timeout.tv_nsec = (now.tv_usec + usec) * 1000;
	pthread_mutex_lock(&m_wakeupMutex);
	pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &timeout);
	pthread_mutex_unlock(&m_wakeupMutex);
}

void _ThreadBase::wakeupThread(void)
{
	m_tSleep = 0;
	pthread_cond_signal(&m_wakeupSignal);
}

bool _ThreadBase::mutexTrylock(int mutex)
{
	if(mutex >= NUM_MUTEX)return false;
	if(pthread_mutex_trylock(&m_mutex[mutex])!=0)return false;

	return true;
}

void _ThreadBase::mutexLock(int mutex)
{
	if(mutex >= NUM_MUTEX)return;
	pthread_mutex_lock(&m_mutex[mutex]);
}

void _ThreadBase::mutexUnlock(int mutex)
{
	if(mutex >= NUM_MUTEX)return;
	pthread_mutex_unlock(&m_mutex[mutex]);
}

void _ThreadBase::updateTime(void)
{
	uint64_t newTime = get_time_usec();
	m_dTime = newTime - m_timeStamp;
	m_timeStamp = newTime;

	m_dTimeAvr *= 0.5;
	m_dTimeAvr += m_dTime*0.5;
}

double _ThreadBase::getFrameRate(void)
{
	return 	1000000.0/m_dTimeAvr;
}

bool _ThreadBase::complete(void)
{
	return true;
}

void _ThreadBase::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

}

void _ThreadBase::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

}