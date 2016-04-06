#pragma once

#ifndef _NETTIMER_H_
#define _NETTIMER_H_

class CNetTimer 
	: public mw_net_timer
	, public CMPool
{
public:
	CNetTimer(mw_net_timer_sink* sink);
	~CNetTimer(void);
	event* _event();
	timeval* _timeval();
	int on_timer();
private:
	virtual void _schedule(uint32_t usec);
	virtual void _cancel();
	virtual void _release();

private:
	static void timeout_cb(int fd, short event, void *arg);
	
private:
	struct event		m_ev;
	struct timeval		m_tv;
	mw_net_timer_sink*	m_sink;
};

#endif //_NETTIMER_H_