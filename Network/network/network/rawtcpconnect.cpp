#include "stdafx.h"
#include "rawtcpconnect.h"
#include "c_basis.h"
#include "libevent.h"


#define BP_CLOSE()

CRawTcpConnection::CRawTcpConnection( mw_connect_sink* psink )
	: m_sink(psink)
	, m_bConnection(false)
	, m_max_send(DEFAULT_MAX_SEND_SIZE)
	, m_max_recv(DEFAULT_MAX_RECV_SIZE)
	, m_network_packet_size(DEFAULT_NETWORK_NETWORK_PACKET)
	, m_active(time(0))
	, m_network_timeout(0)
	, m_original_threadid(0)
{
	memset(&m_client, 0, sizeof(m_client));
	m_threadid = _tid();
	m_original_threadid = _tid();

	_atom_add_one(&libevent::get_instance()->m_handle_count);
}

CRawTcpConnection::CRawTcpConnection(int32_t fd, const string& remote_ip)
	: m_sink(NULL)
	, m_bConnection(true)
	, m_max_send(DEFAULT_MAX_SEND_SIZE)
	, m_max_recv(DEFAULT_MAX_RECV_SIZE)
	, m_network_packet_size(DEFAULT_NETWORK_NETWORK_PACKET)
	, m_active(time(0))
	, m_original_threadid(0)
	, m_remote_ip(remote_ip)
{
	m_fd = fd;
	
	m_threadid = _tid();
	m_original_threadid = _tid();

	memset(&m_client, 0, sizeof(m_client));
	event_set(&m_client.ev_write, m_fd, EV_WRITE , _on_write, this );
	event_set(&m_client.ev_read, m_fd, EV_READ | EV_PERSIST, _on_read, this );

	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_read);
	/* Setting up the event does not activate, add the event so it becomes active. */
	event_add(&m_client.ev_read, NULL);
	//CBProcess::Interface()->throw_event(network_event_add, &m_client.ev_read);

	_atom_add_one(&libevent::get_instance()->m_handle_count);
}


CRawTcpConnection::~CRawTcpConnection(void)
{
}

void CRawTcpConnection::_on_connect( int32_t fd, short ev, void *arg )
{	
	CRawTcpConnection* pCon = (CRawTcpConnection*)arg;

#ifndef _MSC_VER
	int keepAlive		= 1; // 开启keepalive属性
	int keepIdle		= 60; // 如该连接在60秒内没有任何数据往来,则进行探测 
	int keepInterval	= 5; // 探测时发包的时间间隔为5 秒
	int keepCount		= 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
#endif

	pCon->on_connect(fd, ev);
}

void CRawTcpConnection::_on_read( int32_t fd, short ev, void *arg )
{

	CRawTcpConnection* pCon = (CRawTcpConnection*)arg;

	pCon->on_recv(fd, ev);
}

void CRawTcpConnection::_on_write( int32_t fd, short ev, void *arg )
{
	CRawTcpConnection* pCon = (CRawTcpConnection*)arg;

	pCon->on_send(fd, ev);
}

void CRawTcpConnection::on_connect( int32_t fd, short ev )
{
	m_threadid = _tid();
	m_original_threadid = _tid();
	int error = -1;
	_get_opt(OPT_NETWORK_GET_SO_ERROR, &error, sizeof(int));

	if (error == 0 && ev == EV_WRITE)
	{
		m_bConnection = true;

		event_set(&m_client.ev_write, m_fd, EV_WRITE, _on_write, this );
		event_set(&m_client.ev_read, m_fd, EV_READ | EV_PERSIST, CRawTcpConnection::_on_read, this );

		event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_read);
		event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);
		event_add(&m_client.ev_read, NULL);

		//压入句柄
		TlsSingelton<c_basis>::tlsInstance()->push_socket(m_fd, this);

		m_sink->on_connect(0, this);

	}
	else
	{
		//bug socket not free
		EVUTIL_CLOSESOCKET(m_fd);
		m_sink->on_connect(error == 0 ? ERROR_NEWORK_TIMEOUT : error, this);
	}
}

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

void CRawTcpConnection::on_recv( int32_t fd, short ev )
{
	//线程不同步时不处理
	if (m_threadid != _tid())
		return;

	assert(m_threadid == _tid());

	int len;
	len = recv(fd, m_client.buffer, sizeof( m_client.buffer ), 0);
	if (len == 0) 
	{
		
		libevent::get_instance()->close_socket(m_fd, &m_client.ev_read, &m_client.ev_write, this);
		//CBProcess::Interface()->close_socket(m_fd, &m_client.ev_read, &m_client.ev_write, this);
		return;
	}

	else if (len < 0
		&& (EVUTIL_SOCKET_ERROR() != EINTR)
		&& (EVUTIL_SOCKET_ERROR() != EWOULDBLOCK)
		&& !IS_TRY_AGAIN )
	{
		s_log()->err("on_recv error close. fd:%d m_fd:%d\r\n", fd, m_fd);
		libevent::get_instance()->close_socket(m_fd, &m_client.ev_read, &m_client.ev_write, this);
		return;
	}

	if( len > 0 ) 
	{
		m_active = time(0);
		//c_wlock lock(&m_recv_lock);

		m_recv_ring.Append((unsigned char*)m_client.buffer, len);
		//回调数据
		m_sink->on_read(this, m_recv_ring.Count());
	}

	int n = 0;
#ifdef _MSC_VER
	u_long lng = n;
	if (ioctlsocket(fd, FIONREAD, &lng) == -1) {
		s_log()->err("ioctlsocket\r\n");
	}
	else
	{
		n = lng;
	}
#else
	if (ioctl(fd, FIONREAD, &n) == -1) {
		s_log()->err("ioctl\r\n");
		n = 0;
	}
#endif

	if (n > 0) //还有数据可以接收
	{
		on_recv(fd, ev);
	}
}

void CRawTcpConnection::on_send( int32_t fd, short ev )
{
	assert(m_threadid == _tid());

	if (fd < 0)
		return;

	if (is_close())
		return;

	//发送数据
	uint32_t nCount = do_send();

	if (nCount > 0)
	{
		
	}

	m_active = time(0);

	//没有关闭连接，回调
	m_sink->on_write(this, nCount);
}

void CRawTcpConnection::on_close(int32_t error )
{
	assert(m_threadid == _tid());

	m_send_ring.clear();
	m_recv_ring.clear();

	TlsSingelton<c_basis>::tlsInstance()->erase_socket(m_fd);
	EVUTIL_CLOSESOCKET(m_fd);

	if (m_bConnection)
	{
		m_bConnection = false;
	}

	m_sink->on_disconnect(error, this);
	
	dec();
}


int CRawTcpConnection::_connect(const char* addr, uint16_t port, int asyn)
{
	if (asyn) 
	{
		libevent::get_instance()->p_connect_fd(this, ntohl(inet_addr(addr)), port);
	}
	else
	{
		return syn_connect(ntohl(inet_addr(addr)), port);
	}
	return 0;
}

int CRawTcpConnection::_send( unsigned char* date, int32_t size )
{
	if (!m_bConnection)
		return ERROR_NEWORK_UNCONNECT;

	if (date != NULL)
	{
		//c_wlock lock(&m_send_lock);
		if (m_send_ring.Count() > m_max_send)
		{
			return ERROR_NEWORK_LOCAL_GET_SENDBUFFER;
		}
		m_send_ring.Append(date, size);
	}

	//发送数据
	int32_t count = do_send();

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpConnection::_disconnect( void )
{
	m_send_ring.clear();
	m_recv_ring.clear();

	m_bConnection = false;

	libevent::get_instance()->close_socket(m_fd, 
		&m_client.ev_read, &m_client.ev_write, this);

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpConnection::_release( void )
{
	dec();

	_atom_sub_one(&libevent::get_instance()->m_handle_count);
	return ERROR_NEWORK_SUCCESS;
}

void CRawTcpConnection::_set_sink( mw_connect_sink* psink )
{
	m_sink = psink;
}

void CRawTcpConnection::on_release()
{
	assert(m_threadid == _tid());

	m_sink->on_release(this);
	delete this;
}

int CRawTcpConnection::_isconnect( void )
{
	return m_bConnection;
}

int CRawTcpConnection::_set_opt( int32_t type, void* arg , int32_t size)
{
	//OPT_NETWORK_LOCAL_SET_SENDBUFFER,			//int					设置本地的发送最大缓存
	//OPT_NETWORK_LOCAL_SET_RECVDBUFFER,		//int					设置本地的接收最大缓存
	//OPT_NETWORK_SET_RECV
	switch (type)
	{
	case OPT_NETWORK_LOCAL_SET_SENDBUFFER:
		m_max_send = *(int32_t*)arg;
		break;

	case OPT_NETWORK_LOCAL_SET_RECVDBUFFER:
		m_max_recv = *(int32_t*)arg;
		break;

	case OPT_NETWORK_SET_PACKET_SIZE:
		m_network_packet_size = *(int32_t*)arg;
		break;

	case OPT_NETWORK_SET_RECV:
		m_recv_ring.Advance(*(int32_t*)arg);
		break;

	case OPT_NETWORK_SET_KEEPLIVE_TIMEOUT:
		m_network_timeout = *(int32_t*)arg;
		break;

	default:
		{
			return  CSocketBase::_set_opt(type, arg, size);
		}

		break; 
	}

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpConnection::_get_opt( int32_t type, void* arg , int32_t size)
{
	switch (type)
	{
	case OPT_NETWORK_LOCAL_GET_SENDBUFFER:
		*(int*)arg = m_max_send;
		break;

	case OPT_NETWORK_LOCAL_GET_RECVBUFFER:
		*(int*)arg = m_max_recv;
		break;

	case OPT_NETWORK_GET_RECV_SIZE:
		{
			*(int*)arg = m_recv_ring.Count();
		}
		break;

	case OPT_NETWORK_GET_RECV:
		{
			unsigned char* buffer = NULL;
			m_recv_ring.Attach(&buffer, size);
			*(size_t*)arg = (size_t)buffer;
		}
		break;

	case OPT_NETWORK_LOCAL_GET_SEND_SIZE:
		{
			//c_rlock lock(&m_send_lock);

			*(int32_t*)arg = m_send_ring.Count();
		}
		break;

	case OPT_NETWORK_GET_PACKET_SIZE:
		*(int32_t*)arg = m_network_packet_size;
		break;

	case OPT_NETWORK_GET_KEEPLIVE_TIMEOUT:
		*(int32_t*)arg = m_network_timeout;
		break;
	case OPT_NETWORK_REMOTE_ADDR:
		memcpy(arg, m_remote_ip.c_str(), min(size, m_remote_ip.length() + 1));
		break;
	default:
		{
			return CSocketBase::_get_opt(type, arg, size);
		}
		break;
	}

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpConnection::do_send()
{
	//c_wlock lock(&m_send_lock);
	if (is_close())
		return 0;

	if (m_send_ring.Count() > 0)
	{
		event_base* base = TlsSingelton<c_basis>::tlsInstance()->base();
		assert (m_client.ev_write.ev_base == TlsSingelton<c_basis>::tlsInstance()->base());
		//event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);

		event_add(&m_client.ev_write, NULL);

		//CBProcess::Interface()->throw_event(network_event_add, &m_client.ev_write, NULL);
	}

	while (m_send_ring.Count() > 0)
	{
		unsigned char* buffer = NULL;

		int32_t n = m_send_ring.Attach(&buffer);

		int32_t nLen = send(m_fd, (const char*)buffer, min(n, m_network_packet_size), 0);

		if (nLen > 0)
		{
			m_send_ring.Advance(nLen);
		}
		else if ( nLen < 0 )
		{
#ifndef _MSC_VER
			if (errno == EAGAIN ||
				errno == EINTR ||
				errno == EINPROGRESS)
			{
			}
			else
			{
				libevent::get_instance()->close_socket(m_fd, &m_client.ev_read, &m_client.ev_write, this);
				return 0;
			}
#endif
			break;
		}	 
	}

	return m_send_ring.Count();
}

bool CRawTcpConnection::is_timeout()
{
	if (m_network_timeout <= 0)
		return false;

#ifdef _CLIENT
	if (time(0) - m_active > m_network_timeout)
		return true;
#else
	if (time(0) - m_active > m_network_timeout * 60)
		return true;
#endif

	return false;
}

int CRawTcpConnection::_signal( void )
{
	libevent::get_instance()->signal(m_threadid, m_fd);
	return 0;
}

int CRawTcpConnection::_rebind( thread_t bind_tid )
{
	assert(m_threadid == _tid());
	if (bind_tid != m_threadid)
	{
		//解除现有的绑定关系
		event_del(&m_client.ev_read);
		event_del(&m_client.ev_write);
		
		//Socket解绑
		TlsSingelton<c_basis>::tlsInstance()->erase_socket(m_fd);

		//开始发起线程重先绑定
		return 	libevent::get_instance()->rebind(bind_tid, this);
	}
	else
	{
		m_sink->on_rebind(this);
	}

	return 0;
}


void CRawTcpConnection::on_signal()
{
	if (m_threadid != _tid())
		return;

	assert(m_threadid == _tid());
	m_sink->on_signal(this);
}

void CRawTcpConnection::on_rebind()
{
	//开始重先绑定
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_read);
	/* Setting up the event does not activate, add the event so it becomes active. */
	event_add(&m_client.ev_read, NULL);
	//Socket绑定
	TlsSingelton<c_basis>::tlsInstance()->push_socket(m_fd, this);
	m_threadid = _tid();
	m_sink->on_rebind(this);
}

int CRawTcpConnection::syn_connect( uint32 ipaddr, uint16 port )
{
	int rt = open_sock();

	if (rt != 0)
	{
		return rt;
	}	

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ipaddr);
	listen_addr.sin_port = htons(port);

	/* Set the socket to non-blocking, this is essential in event
	* based programming with libevent. */
	if (evutil_make_socket_nonblocking(m_fd) < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("failed to set server socket to non-blocking fun:%s\r\n", __func__);
		//event_err(1, "failed to set server socket to non-blocking");
		EVUTIL_CLOSESOCKET(m_fd);
		m_sink->on_connect(error, this);
		return error;
	}

	rt = connect(m_fd, (sockaddr*)&listen_addr, sizeof(listen_addr));
	if (SOCKET_ERROR == rt 
		&& (!IS_TRY_AGAIN) && !IS_EINPROGRESS)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		EVUTIL_CLOSESOCKET(m_fd);
		m_sink->on_connect(error, this);
		return error;
	}

	event_set(&m_client.ev_write, m_fd, EV_WRITE, CRawTcpConnection::_on_connect, this);

	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_read);


	m_timeout.tv_sec = 5;		//15秒
	m_timeout.tv_usec = 0;

	event_add(&m_client.ev_write, &m_timeout);

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpConnection::asyn_connect( uint32 ipaddr, uint16 port )
{
	m_threadid = _tid();
	int rt = open_sock();

	if (rt != 0)
	{
		m_sink->on_connect(rt, this);
		return rt;
	}	

	struct sockaddr_in listen_addr;
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ipaddr);
	listen_addr.sin_port = htons(port);

	/* Set the socket to non-blocking, this is essential in event
	* based programming with libevent. */
	if (evutil_make_socket_nonblocking(m_fd) < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("failed to set server socket to non-blocking fun:%s\r\n", __func__);
		//event_err(1, "failed to set server socket to non-blocking");
		EVUTIL_CLOSESOCKET(m_fd);
		m_sink->on_connect(error, this);
		return error;
	}


	if ( SOCKET_ERROR == connect( m_fd, (sockaddr*)&listen_addr, sizeof(listen_addr) ) 
		&& (!IS_TRY_AGAIN) && !IS_EINPROGRESS)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		EVUTIL_CLOSESOCKET(m_fd);
		m_sink->on_connect(error, this);
		return error;
	}

	event_set(&m_client.ev_write, m_fd, EV_WRITE, CRawTcpConnection::_on_connect, this);

	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_write);
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_client.ev_read);


	m_timeout.tv_sec = 15;		//五秒
	m_timeout.tv_usec = 0;

	event_add(&m_client.ev_write, &m_timeout);

	return ERROR_NEWORK_SUCCESS;
}

