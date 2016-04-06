#ifndef c_thread_chatroom_h
#define c_thread_chatroom_h

#pragma once
#include "stdafx.h"
#include "c_server.h"

class c_thread_refresh
	: public c_singleton<c_thread_refresh>
	, public c_thread
{
public:
	c_thread_refresh(void) {}
	~c_thread_refresh(void) {}
public:
	void setEvent() {
		m_event.set_event();
	}

	void waitEvent()
	{
		m_event.wait_event();
	}
private:
	virtual void Execute()
	{
		uint32 tick = 0;
		setEvent();
		while (Get_Running())
		{
			_sleep(1000);
		}
	}

	c_event		m_event;

};

#endif //
