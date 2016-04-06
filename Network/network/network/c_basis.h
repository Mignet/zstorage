#pragma once

#include "stdafx.h"
#include "c_pipe.h"
#include "socketbase.h"


typedef struct event_close
{
	CSocketBase* sock;
}event_close;


class CRawTcpConnection;

class c_basis
	: public mw_net_pipe_sink
	, public mw_net_timer_sink
	, public pool::CMPool
{
	typedef queue<int32> L_CLOSE;
	typedef map<int32, CRawTcpConnection*>	MSocket;
	typedef list<int32>				LFD;
	typedef queue<CRawTcpConnection*>		QReBind;			//需要重先绑定线程连接
public:
	c_basis(void) : m_base(event_init()), m_pipe(NULL), m_timer(NULL), m_thread_timer(NULL) {}
	~c_basis(void) {}
public:
	void p_close(int32 fd)
	{
		//c_wlock lock(&m_rwlock);
		m_qclose.push(fd);

		m_pipe->_notify("2", 1);

	}

	int32 get_close()
	{
		//c_wlock lock(&m_rwlock);
		int32 result = 0;
		if (m_qclose.empty())
		{
			return result;
		}

		result = m_qclose.front();
		m_qclose.pop();

		return result;
	}

public:
	event_base* base() {return m_base;}
	void dispatch(bool bmthread = false);
	void loopbreak();
	void push_socket(int32 fd, CRawTcpConnection* con);
	void erase_socket(int32 fd);
	void push_fd(int32 fd);
	int32 pop_fd();
	void del_fd(int32 fd);
	int  notify(const char* data, int size);
	//重先绑定
	void push_rebind(CRawTcpConnection* con);
	CRawTcpConnection* pop_rebind();
private:
	virtual void on_process(void* data, int size, mw_net_pipe* pipe);
	virtual int on_timer(mw_net_timer *timer);
private:
	event_base*				m_base;
	mw_net_pipe*			m_pipe;				//通信层定时器
	mw_net_timer*			m_timer;			//定时器
	mw_net_timer*			m_thread_timer;		//线程定时器
	
	L_CLOSE					m_qclose;
	MSocket					m_mSocket;
	rw_lock					m_lock;
	LFD						m_fds;
	QReBind					m_rebind;				
};
