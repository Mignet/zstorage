#pragma once

#if !defined(__RWLOCK_H__)
#define __RWLOCK_H__

#ifndef _MSC_VER
#include<pthread.h>
#endif

namespace zn {
	//��д��
	class CRWLock
	{
	public:
		CRWLock(void)
		{
#ifdef _MSC_VER
			InitializeCriticalSection(&m_rw_lock);
#else
			pthread_mutex_init(&m_rw_lock, NULL);
#endif
		}
		~CRWLock(void)
		{
#ifdef _MSC_VER
			DeleteCriticalSection(&m_rw_lock);
#else
			pthread_mutex_destroy(&m_rw_lock);
#endif
		}
	public:
		//��������
		void REnter(void)
		{
#ifdef _MSC_VER
			EnterCriticalSection(&m_rw_lock);
#else
			pthread_mutex_lock(&m_rw_lock);
#endif
		}
		//�����뿪
		void RLeave(void)
		{
#ifdef _MSC_VER
			LeaveCriticalSection(&m_rw_lock);
#else
			pthread_mutex_unlock(&m_rw_lock);
#endif
		}
		//д����������
		void WEnter(void)
		{

#ifdef _MSC_VER
			EnterCriticalSection(&m_rw_lock);
#else
			pthread_mutex_lock(&m_rw_lock);
#endif
		}
		//д�������뿪
		void WLeave(void)
		{
#ifdef _MSC_VER
			LeaveCriticalSection(&m_rw_lock);
#else
			pthread_mutex_unlock(&m_rw_lock);
#endif
		}
	private:
#ifdef _MSC_VER
		CRITICAL_SECTION		m_rw_lock;
#else
		pthread_mutex_t			m_rw_lock;
#endif

	};

	class CWLock
	{
	public:
		CWLock(CRWLock* lock): m_Lock(lock) 
		{
			m_Lock->WEnter(); 
		}

		~CWLock() 
		{
			m_Lock->WLeave();
		}
	private:
		CRWLock*	m_Lock;
	};

	class CRLock
	{
	public:
		CRLock(CRWLock* lock): m_Lock(lock) 
		{
			m_Lock->REnter(); 
		}

		~CRLock() 
		{
			m_Lock->RLeave();
		}
	private:
		CRWLock*	m_Lock;
	};
} //end namespace zn {

#endif //__RWLOCK_H__
