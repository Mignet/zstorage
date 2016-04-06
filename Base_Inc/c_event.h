#pragma once

#ifndef _C_EVENT_H_
#define _C_EVENT_H_

#ifdef _MSC_VER

#define t_event		HANDLE

#else

#include <semaphore.h>
#define t_event		sem_t
	
#endif

namespace zn {

class c_event
{
public:
	c_event(void)
	{
#ifdef _MSC_VER
	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	int res = sem_init(&m_event, 0, 0);
	if (res != 0)
	{
		perror("Semaphore initialization failed");
	}
#endif
	}

	virtual ~c_event(void)
	{
#ifdef _MSC_VER
		CloseHandle(m_event);
#else
		sem_destroy(&m_event);
#endif
	}

	int32_t wait_event(int second  = 0xffff) 
	{
#ifdef _MSC_VER
	return WaitForSingleObject(m_event, second * 1000);
#else
		time_t _timout = time(0) + second;
		struct timespec timeout;
		timeout.tv_sec = _timout;
		timeout.tv_nsec = 0;
	return sem_timedwait(&m_event, &timeout);
#endif
	}

	int set_event(void)
	{
#ifdef _MSC_VER
		return SetEvent(m_event);
#else
		sem_post(&m_event);

		return 0;
#endif
	}

private:
	t_event				m_event;

#ifndef _MSC_VER
	pthread_mutex_t		m_mutex;
#endif
};
}



#endif //_C_EVENT_H_
