#include "stdafx.h"
#include "socketbase.h"
#include "libevent.h"

CSocketBase::CSocketBase(void)
: m_onclose(false)
{
}

CSocketBase::~CSocketBase(void)
{
}

int CSocketBase::open_sock()
{
	/* Create our listening socket. This is largely boiler plate*/
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_fd < 0)
	{
		int error = EVUTIL_SOCKET_ERROR();
		EVUTIL_SET_SOCKET_ERROR(error);
		s_log()->err("open failed fun:%s \r\n", __func__);
		return error;
	}

	//CBProcess::Interface()->m_ncount++;

	//int reuseaddr_on = 1;
	//if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr_on,
	//	sizeof(reuseaddr_on)) == -1)
	//{
	//	s_log()->err("setsockopt failed __func__:%s\r\n", __func__);
	//	EVUTIL_CLOSESOCKET(m_fd);
	//	return EVUTIL_SOCKET_ERROR();
	//}

	return 0;
}

int CSocketBase::_set_opt( int32_t type, void* arg , int32_t size)
{
		//OPT_NETWORK_IP_SET_SENDBUFFER,			//int					设置TCP协议层的发送缓存
		//OPT_NETWORK_IP_SET_RECVDBUFFER,			//int					设置TCP协议层的接收缓存	
	
		switch (type)
		{
		case OPT_NETWORK_IP_SET_SENDBUFFER:
			{
				return setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (const char*)arg, sizeof(int32_t));
			}
			break;

		case OPT_NETWORK_IP_SET_RECVDBUFFER:
			{
				return setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (const char*)arg, sizeof(int32_t));
			}
			break;

		case OPT_NETWORK_TCP_NODELAY:
			{
				return setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&arg, sizeof(char));   
			}
			break;

		default:
			break;
		}

		return 0;
}

int CSocketBase::_get_opt( int32_t type, void* arg , int32_t size)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	switch (type)
	{
	case OPT_NETWORK_REMOTE_ADDR:
		{
			getpeername(m_fd, (sockaddr*)&addr, &len);
			strcpy((char*)arg, inet_ntoa(addr.sin_addr));
		}
		break;
	case OPT_NETWORK_REMOTE_PORT:
		{
			getpeername(m_fd, (sockaddr*)&addr, &len);
			*(unsigned short*)arg = ntohs(addr.sin_port);
		}
		break;

	case OPT_NETWORK_LOCAL_ADDR:
		{
			getsockname(m_fd, (sockaddr*)&addr, &len);
			strcpy((char*)arg, inet_ntoa(addr.sin_addr));
		}
		break;

	case OPT_NETWORK_LOCAL_PORT:
		{
			getsockname(m_fd, (sockaddr*)&addr, &len);
			*(unsigned short*)arg = ntohs(addr.sin_port);
		}
		break;

	case OPT_NETWORK_IP_GET_SENDBUFFER:
		{
			socklen_t n = sizeof(int);
			return getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, (char*)arg, &n);
		}
		break;

	case OPT_NETWORK_IP_GET_RECVBUFFER:
		{
			socklen_t n = sizeof(int);
			return getsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char*)arg, &n);
		}
		break;

	case OPT_NETWORK_GET_SO_ERROR:
		{
			socklen_t n = sizeof(int);
			return getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)arg, &n);
		}
		break;

	case OPT_NETWORK_GET_HANDLE_COUNT:
		{
			*(int*)arg = _atom_read(&libevent::get_instance()->m_handle_count);
		}
		break;

	default:
		{
			s_log()->err("error _get_opt type: %d fun:%s\r\n", type, __func__);		
		}
		break;
	}	

	return 0;
}

void CSocketBase::set_close()
{
	m_onclose = true;
}
