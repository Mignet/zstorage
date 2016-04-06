
#ifndef rw_lock_h
#define rw_lock_h

#ifdef _MSC_VER
#pragma	warning(disable: 4127) 
#endif


#include "df_macro.h"

//#define TEST_LOCK

namespace zn
{
	//读写锁
	class rw_lock
	{
	public:
		rw_lock(void) 
		{
#ifdef _MSC_VER
			InitializeCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_init(&m_rw_lock, NULL);
#else
			pthread_rwlock_init(&m_rw_lock, NULL);
#endif
		}
		~rw_lock(void)
		{
#ifdef _MSC_VER
			DeleteCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_destroy(&m_rw_lock);
#else
			pthread_rwlock_destroy(&m_rw_lock);
#endif
		}
	public:
		//读锁进入
		void r_enter(void)
		{
#ifdef _MSC_VER
			EnterCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_lock(&m_rw_lock);
#else
			pthread_rwlock_rdlock(&m_rw_lock);
#endif
		}
		//读锁离开
		void r_leave(void)
		{
#ifdef _MSC_VER
			LeaveCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_unlock(&m_rw_lock);
#else
			 pthread_rwlock_unlock(&m_rw_lock);
#endif
		}
		//写锁保护进入
		void w_enter(void)
		{
#ifdef _MSC_VER
			EnterCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_lock(&m_rw_lock);
			
#else
			pthread_rwlock_wrlock(&m_rw_lock);
#endif
		}
		//写锁保护离开
		void w_leave(void)
		{
#ifdef _MSC_VER
			LeaveCriticalSection(&m_rw_lock);
#elif _ANDROID
			pthread_mutex_unlock(&m_rw_lock);
#else
			pthread_rwlock_unlock(&m_rw_lock);
#endif
		}
	private:
#ifdef _MSC_VER
		CRITICAL_SECTION		m_rw_lock;
#elif _ANDROID
		pthread_mutex_t 		m_rw_lock;
#else
		pthread_rwlock_t 		m_rw_lock;
#endif
	};


	class c_wlock
	{
	public:
		c_wlock(rw_lock* lock): m_wlock(lock) 
		{
#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " wlock enter in:" << lock);
#endif
			m_wlock->w_enter(); 

#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " wlock enter out:" << lock);
#endif
		}

		~c_wlock() 
		{
#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " wlock leave in:" << m_wlock);
#endif
			m_wlock->w_leave();

#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " wlock leave out:" << m_wlock);
#endif
		}
	private:
		rw_lock*	m_wlock;
	};

	class c_rlock
	{
	public:
		c_rlock(rw_lock* lock): m_rlock(lock) 
		{
#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " rlock enter in:" << lock);
#endif
			m_rlock->r_enter();


#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " rlock enter out:" << lock);
#endif
		}

		~c_rlock() 
		{
#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " rlock leave in:" << m_rlock);
#endif

			m_rlock->r_leave();

#ifdef TEST_LOCK
			LOG4_INFO("THREAD:" << _tid() << " rlock leave out:" << m_rlock);
#endif
		}
	private:
		rw_lock*	m_rlock;
	};

}

#endif //rw_lock_h
