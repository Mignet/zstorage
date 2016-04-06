#include "stdafx.h"
#include "nettimer.h"
#include "c_basis.h"

CNetTimer::CNetTimer(mw_net_timer_sink* sink)
: m_sink(sink)
{
	evutil_timerclear(&m_tv);
}

CNetTimer::~CNetTimer(void)
{
}

void CNetTimer::_schedule( uint32_t usec )
{
	evtimer_set(&m_ev, timeout_cb, this);
	m_tv.tv_sec = usec / 1000000;
	m_tv.tv_usec = usec % 1000000;
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_ev);
	event_add(&m_ev, &m_tv);
	//CBProcess::Interface()->throw_event(network_event_add, &m_ev, &m_tv);
}

void CNetTimer::_cancel()
{
	event_del(&m_ev);
	//CBProcess::Interface()->throw_event(network_event_del, &m_ev);
}

void CNetTimer::_release()
{
	delete this;
}

void CNetTimer::timeout_cb( int fd, short event, void *arg )
{
	CNetTimer* timer = (CNetTimer*)arg;
	if (timer->on_timer() == 0)
	{
		assert (timer->_event()->ev_base == TlsSingelton<c_basis>::tlsInstance()->base());
		event_add(timer->_event(), timer->_timeval());
	}

}

event* CNetTimer::_event()
{
	return &m_ev;
}

int CNetTimer::on_timer()
{
	return m_sink->on_timer(this);
}

timeval* CNetTimer::_timeval()
{
	return &m_tv;
}

