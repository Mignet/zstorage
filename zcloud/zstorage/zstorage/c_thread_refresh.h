#ifndef c_thread_chatroom_h
#define c_thread_chatroom_h

#pragma once
#include "stdafx.h"
#include "c_server.h"
#include "c_timeout.h"
#include "c_list.h"
#include "c_mlist.h"
#include "c_seq.h"

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
			c_timeout::get_instance()->timeout();

			if (time(0) - tick > 300)
			{
				tick = time(0);
				c_list_admin::get_instance()->check();
				c_mlist_admin::get_instance()->check();
				c_seq_admin::get_instance()->check();
			}

			_sleep(1000);
		}
	}

	c_event		m_event;

};

#endif //
