#include "libevent.h"

void libevent::p_fd( int fd, short ev, mw_acceptor_sink* sink, unsigned short port)
{
	accept_fd accept;
	accept.fd = fd;
	accept.ev = ev;
	accept.sink = sink;
	accept.port = port;

	sockaddr_in sa;
	socklen_t len = sizeof(sa);
	getpeername(fd, (sockaddr *)&sa, &len);
	//#ifdef _MSC_VER
	accept.remote_ip = inet_ntoa(sa.sin_addr);


	if (true) 
	{
		c_wlock lock(&m_rwlock);
		m_qfd.push(accept);
	}

	//通知线程工作
	if (true)
	{
		notify_basis();
	}
}

void libevent::p_connect_fd(IConnectSink* sink, uint32 addr, uint16 port )
{
	connect_fd connect;
	connect.addr = addr;
	connect.port = port;
	connect.sink = sink;

	if (true) 
	{
		c_wlock lock(&m_rwlock);
		m_qcd.push(connect);
	}

	//通知线程工作
	if (true)
	{
		notify_basis();
	}
}


bool libevent::g_fd( accept_fd& accept )
{
	c_wlock lock(&m_rwlock);
	if (m_qfd.empty())
		return false;
	accept = m_qfd.front();
	m_qfd.pop();
	return true;
}


bool libevent::g_cd( connect_fd& connect )
{
	c_wlock lock(&m_rwlock);
	if (m_qcd.empty())
		return false;
	connect = m_qcd.front();
	m_qcd.pop();
	return true;
}


int libevent::init( int threads )
{
	if (threads <= 0)
		threads = 1;

	if (threads > 64)
		threads = 8;

	m_threads = threads;

	m_man_threadid = _tid();

	for (int i = 0; i < threads - 1; i++)
	{
		evthread* thread = new evthread;
		thread->Start();

		m_qthread.push_back(thread);
	}

	for (vector<evthread*>::iterator iter = m_qthread.begin(); 
		iter != m_qthread.end();)
	{
		if (!(*iter)->m_reading)
		{
			_sleep(1);
			continue;
		}

		++iter;
	}

	TlsSingelton<c_basis>::tlsInstance();

	s_log()->info("network Initialization!\r\n");

	return 0;
}

void libevent::close_socket( int fd, struct event* ev_read, struct event* ev_write, CSocketBase* sock )
{
	
	if (sock->is_close())
		return;

	//增加引用记数
	sock->add();
	sock->set_close();
	if (ev_read) event_del(ev_read);
	if (ev_write) event_del(ev_write);

	TlsSingelton<c_basis>::tlsInstance()->p_close(fd);
}	

void libevent::push_basis( c_basis* basis )
{
	if (m_threads > 1 && _tid() == m_man_threadid)
	{
		//多线程操作时不把主线程做业务操作
		return;
	}
	c_wlock lock(&m_base_rwlock);
	m_basis.insert(make_pair(_tid(), basis));
}

void libevent::notify_basis()
{
	c_rlock lock(&m_base_rwlock);
	
	for (M_BASIS::iterator iter = m_basis.begin(); iter != m_basis.end(); ++iter)
	{
		iter->second->notify("1", 1);
	}
}

void libevent::exit_basis()
{
	c_rlock lock(&m_base_rwlock);

	for (M_BASIS::iterator iter = m_basis.begin(); iter != m_basis.end(); ++iter)
	{
		iter->second->notify("Q", 1);
	}
}

void libevent::signal(thread_t tid, int32 fd)
{
	c_rlock lock(&m_base_rwlock);

	M_BASIS::iterator pos = m_basis.find(tid);

	if (pos != m_basis.end())
	{
		pos->second->push_fd(fd);
		int rt = pos->second->notify("1", 1);
	}
}

int libevent::rebind( thread_t tid, CRawTcpConnection* con )
{
	c_rlock lock(&m_base_rwlock);

	M_BASIS::iterator pos = m_basis.find(tid);
	if (pos == m_basis.end())
		return -1;

	pos->second->push_rebind(con);
	pos->second->notify("1", 1);
	return 0;
}

void libevent::set_thread_cb( mw_thread_cb* cb )
{
	m_thread_cb = cb;
}

