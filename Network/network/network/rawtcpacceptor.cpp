#include "stdafx.h"
#include "rawtcpacceptor.h"
#include "rawtcpconnect.h"
#include "libevent.h"

CRawTcpAcceptor::CRawTcpAcceptor( mw_acceptor_sink* pSink, e_class_connect e_type)
: m_sink(pSink)
, m_listen_port(0)
, m_e_type(e_type)
{
}

CRawTcpAcceptor::~CRawTcpAcceptor(void)
{
}

int CRawTcpAcceptor::_start_listen( uint16_t port, uint32_t addr /*= 0*/ )
{
	/* Create our listening socket. This is largely boiler plate*/
	
	int rt = open_sock();

	if (rt != 0)
	{
		return rt;
	}

#ifndef _MSC_VER
	socklen_t on = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) != 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		EVUTIL_CLOSESOCKET(m_fd);
		return error;
	}
#endif
	

	m_listen_port = port;
	
	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = (addr == 0 ? INADDR_ANY : addr);
	listen_addr.sin_port = htons(port);

	if (bind(m_fd, (struct sockaddr *)&listen_addr,
		sizeof(listen_addr)) < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("bind failed fun:%s\r\n", __func__);
		EVUTIL_CLOSESOCKET(m_fd);
		return error;
	}

	if (listen(m_fd, 128) < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("listen failed fun:%s\r\n", __func__);
		EVUTIL_CLOSESOCKET(m_fd);
		return error;
	}

	s_log()->info( "listening on port[%d] fun: %s\r\n", port, __func__ );

	/* Set the socket to non-blocking, this is essential in event
	* based programming with libevent. */
	if (evutil_make_socket_nonblocking(m_fd) < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("failed to set server socket to non-blocking fun:%s\r\n", 
			__func__);
		//event_err(1, "failed to set server socket to non-blocking");
		return error;
	}

	event_set(&m_ev_accept, m_fd, EV_READ | EV_PERSIST, CRawTcpAcceptor::on_accept, this);
	
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_ev_accept);

	event_add(&m_ev_accept, NULL);
	//CBProcess::Interface()->throw_event(network_event_add, &m_ev_accept, NULL);

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpAcceptor::_stop_listen( void )
{
	libevent::get_instance()->close_socket(m_fd, &m_ev_accept, NULL, this);

	return ERROR_NEWORK_SUCCESS;
}

int CRawTcpAcceptor::_release()
{
	dec();
	return ERROR_NEWORK_SUCCESS;
}

void CRawTcpAcceptor::on_accept( int32_t fd, short ev, void *arg )
{
	CRawTcpAcceptor* pAcceptor = (CRawTcpAcceptor*)arg;

	if (true)
	{
		int client_fd;
		struct sockaddr_in client_addr = {0};
		socklen_t client_len = sizeof(client_addr);
		

		client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd == -1) {
			s_log()->err("accept failed, error.:%u\r\n", EVUTIL_SOCKET_ERROR());
			return;
		}

		setsockopt( client_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&fd, sizeof(fd) );

#ifndef _MSC_VER
		int keepAlive = 1; // 开启keepalive属性
		int keepIdle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测 
		int keepInterval = 5; // 探测时发包的时间间隔为5 秒
		int keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
		setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
		setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
		setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
#endif


		/* Set the client socket to non-blocking mode. */
		if (evutil_make_socket_nonblocking(client_fd) < 0)
			s_log()->err("failed to set client socket non-blocking fun:%s\r\n", __func__);

		
		pAcceptor->OnConnectIndication(client_fd, ev);
	}
	
}

void CRawTcpAcceptor::OnConnectIndication( int32_t fd, short ev )
{
	libevent::get_instance()->p_fd(fd, ev, m_sink, m_listen_port);	
}

void CRawTcpAcceptor::on_release()
{
	//析构对像
	m_sink->on_release(this);
	delete this;
}

void CRawTcpAcceptor::on_close(int error )
{
	EVUTIL_CLOSESOCKET(m_fd);
	m_sink->on_stop(m_listen_port);
	//关闭通信层引用记数
	dec();
}
