#pragma once

#include "stdafx.h"
#include "evthread.h"
#include "socketbase.h"
#include "evthread.h"

typedef struct accept_fd
{
	int					fd;
	short				ev;
	unsigned short		port;
	mw_acceptor_sink*	sink;
	string				remote_ip;
}accept_fd;

class IConnectSink;
typedef struct connect_fd
{
	uint32				addr;
	uint16				port;
	IConnectSink*		sink;
}connect_fd;

class IConnectSink
{
public:
	virtual int asyn_connect(uint32 ipaddr, uint16 port) = 0;
};

class evthread;
class mw_net_pipe;

class libevent
	: public c_singleton<libevent> 
{
	typedef queue<accept_fd> L_FD;
	typedef queue<connect_fd> C_FD;
	typedef map<thread_t, c_basis*> M_BASIS;
public:
	libevent(void) : m_thread_pipe(NULL), m_threads(0), m_man_threadid(0), m_thread_cb(NULL) {_atom_init(m_handle_count);}
	~libevent(void){}
	uint32 threads() const {return m_threads;}
	void set_thread_cb( mw_thread_cb* cb);
	mw_thread_cb* thread_cb() {return m_thread_cb;}
	void p_fd(int fd, short ev, mw_acceptor_sink* sink, unsigned short	port);
	void p_connect_fd(IConnectSink* sink, uint32 addr, uint16 port);

	bool g_fd(accept_fd& accept);
	bool g_cd(connect_fd& connect);
public:
	int init(int threads);
	void close_socket(int fd,  struct event* ev_read, struct event* ev_write, CSocketBase* sock);
	void push_basis( c_basis* basis );
	void notify_basis();
	void exit_basis();
	void set_thread_pipe(mw_net_pipe* thread_pipe) {m_thread_pipe = thread_pipe; }
	void signal(thread_t tid, int32 fd);
	int rebind(thread_t tid, CRawTcpConnection* con);
public:
	_atom					m_handle_count;
private:
	vector<evthread*>		m_qthread;
	rw_lock					m_rwlock;
	L_FD					m_qfd;
	C_FD					m_qcd;
	rw_lock					m_base_rwlock;
	M_BASIS					m_basis;
	mw_net_pipe*			m_thread_pipe;
	uint32_t				m_threads;
	thread_t				m_man_threadid;
	mw_thread_cb*			m_thread_cb;
};
