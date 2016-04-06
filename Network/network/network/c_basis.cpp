#include "c_basis.h"
#include "rawtcpconnect.h"
#include "libevent.h"

void c_basis::on_process(void* data, int size, mw_net_pipe* pipe)
{
	if (size > 0 && ((char*)data)[0] == 'Q')
	{
		s_log()->info("loopbreak(0x%x)\r\n", ctid());
		loopbreak();
		return;
	}

	//1.优先处理重先绑定的
	while (true)
	{
		CRawTcpConnection* con = pop_rebind();
		if (con == NULL)
			break;

		con->on_rebind();	
	}

	//判断是否有连接关闭
	while (true)
	{
		int32 fd = get_close();
		if (fd == 0)
			break;

		MSocket::iterator pos = m_mSocket.find(fd);
		if (pos != m_mSocket.end())
		{
			pos->second->on_close(EVUTIL_SOCKET_ERROR());
		}
	}

	//判断是否有新连接到来
	accept_fd fd;
	while ( libevent::get_instance()->g_fd(fd) )
	{	

		CRawTcpConnection* pcon = new CRawTcpConnection(fd.fd, fd.remote_ip);
		fd.sink->on_accept(pcon, fd.port);
		
		m_mSocket.insert(make_pair(fd.fd, pcon));
	}

	//判断是否有新的连接
	connect_fd cd;
	while ( libevent::get_instance()->g_cd(cd) )
	{
		cd.sink->asyn_connect(cd.addr, cd.port);
	}

	//信息本线程所有连接信号
	while (true)
	{
		int fd = pop_fd();
		if (fd == 0)
			break;

		MSocket::iterator pos = m_mSocket.find(fd);
		if (pos != m_mSocket.end())
		{
			CRawTcpConnection* con = pos->second;
			pos->second->on_signal();
		}
	}
	
}

void c_basis::dispatch(bool bmthread)
{
	m_timer = create_net_timer(this);
	//30定时器
	m_timer->_schedule(1000*1000*60);
	m_pipe = create_net_pipe(this);
	m_pipe->_create();

	if ((!bmthread || libevent::get_instance()->threads() == 1) && 
		libevent::get_instance()->thread_cb())
	{
		mw_thread_cb* cb = libevent::get_instance()->thread_cb();
		cb->thread_cb();
	}

	libevent::get_instance()->push_basis(this);

	if (bmthread)
	{
		libevent::get_instance()->set_thread_pipe(m_pipe);
	}

	event_base_dispatch(m_base);

	s_log()->err("event_dispatch exit(0x%x)\r\n", ctid());
}

void c_basis::erase_socket( int32 fd )
{
	m_mSocket.erase(fd);

	del_fd(fd);
}

void c_basis::push_fd( int32 fd )
{
	c_wlock lock(&m_lock);
	m_fds.push_back(fd);
}

int32 c_basis::pop_fd()
{
	int32 result = 0;

	c_wlock lock(&m_lock);
	if (!m_fds.empty())
	{
		result = m_fds.front();
		m_fds.pop_front();
	}

	return result;
}

void c_basis::del_fd( int32 fd )
{
	c_wlock lock(&m_lock);
	for (LFD::iterator pos = m_fds.begin(); pos != m_fds.end();)
	{
		if ((*pos) == fd)
			m_fds.erase(pos++);
		else
			++pos;
	}
}


int c_basis::on_timer( mw_net_timer *timer )
{
	if (timer == m_timer)
	{
		for (MSocket::iterator iter = m_mSocket.begin(); 
			iter != m_mSocket.end(); ++iter)
		{
			if (iter->second->is_timeout())
			{
				((mw_connect*)iter->second)->_disconnect();
				s_log()->info("timeout disconnect.\r\n", iter->second);
			}
		}
	}

	return 0 ;
}

int c_basis::notify( const char* data, int size )
{
	return m_pipe->_notify(data, size);
}

void c_basis::push_socket( int32 fd, CRawTcpConnection* con )
{
	m_mSocket.insert(make_pair(fd, con));
}

void c_basis::push_rebind( CRawTcpConnection* con )
{
	c_wlock lock(&m_lock);
	m_rebind.push(con);
}

CRawTcpConnection* c_basis::pop_rebind()
{
	c_wlock lock(&m_lock);
	if (m_rebind.empty())
		return NULL;

	CRawTcpConnection* con = m_rebind.front();
	m_rebind.pop();

	return con;
}

void c_basis::loopbreak()
{
	event_loopbreak();
}
