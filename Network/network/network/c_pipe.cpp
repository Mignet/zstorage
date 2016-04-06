#include "c_pipe.h"
#include "c_basis.h"

#ifdef _MSC_VER
#define _write(fd,buf,len) send((fd),(buf),(len),0)
#define _read(fd,buf,len) recv((fd),(buf),(len),0)
#else
#define _write(fd,buf,len) write((fd),(buf),(len))
#define _read(fd,buf,len) read((fd),(buf),(len))
#endif

c_pipe::c_pipe(mw_net_pipe_sink* sink)
: m_sink(sink)
{
}

c_pipe::~c_pipe(void)
{
}

int c_pipe::_create()
{

#ifdef _MSC_VER
	//Windows 采用 socketpair 实现
	int rt = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, m_pipe);
#else
	//Linux 采用 pipe 实现
	int rt = pipe(m_pipe);
#endif
	
	evutil_make_socket_nonblocking(m_pipe[0]);
	evutil_make_socket_nonblocking(m_pipe[1]);
	assert (rt == 0);
	
#ifdef _MSC_VER
	event_set(&m_ev, m_pipe[1],
		EV_READ | EV_PERSIST, libevent_process, this);
#else
	event_set(&m_ev, m_pipe[0],
		EV_READ | EV_PERSIST, libevent_process, this);
#endif
	event_base_set(TlsSingelton<c_basis>::tlsInstance()->base(), &m_ev);
	event_add(&m_ev, NULL);

	return 0;
}

int c_pipe::_notify( const char* data, int size )
{
#ifdef _MSC_VER
	return _write(m_pipe[0], data, size);
#else
	return _write(m_pipe[1], data, size);
#endif
}

void c_pipe::_cancel()
{
	event_del(&m_ev);
	EVUTIL_CLOSESOCKET(m_pipe[0]);
	EVUTIL_CLOSESOCKET(m_pipe[1]);
}

void c_pipe::_release()
{
	delete this;
}

void c_pipe::libevent_process( int fd, short which, void *arg )
{
	((c_pipe*)arg)->process(fd, which);
}

void c_pipe::process( int fd, short which)
{
	
	int len = _read(fd, m_buf, sizeof(m_buf));
	
	if (len > 0)
		m_sink->on_process(m_buf, len, this);


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
		process(fd, which);
	}

}
