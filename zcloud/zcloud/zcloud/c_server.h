
#ifndef c_server_h
#define c_server_h
#pragma once
#include "c_interval.h"

class c_server
	: public mw_acceptor_sink
	, public mw_net_timer_sink
	, public c_singleton<c_server>
{
	
	typedef struct _storage
	{
		uint32	ibegin;
		uint32	iend;
		ICB*	cb;
	}_storage;

	WISDOM_PTR(_storage, wisdom_storage);

	typedef map<c_interval, wisdom_storage>	M;
public:
	c_server();
	~c_server();
	bool LoadConfig();
	void loadStorage(wisdom_IOStream& os);
	int listen();
	uint16 threads() const { return m_threads; }
	uint32 delay() const { return m_delay; }
private:
	/*mw_acceptor_sink*/
	//某一端口接收到新的连接
	virtual int on_accept(mw_connect* con, unsigned short port);

	//某一端口停止监听
	virtual int on_stop(unsigned short){ return 0; }

	//对象被析构
	virtual int on_release(mw_acceptor*) { return 0; }

	virtual int on_timer(mw_net_timer *timer);
public:
	ICB* find_storage(uint16 crc)
	{
		c_rlock lock(&m_lock);
		M::iterator pos = m_storage.find(crc);
		if (pos != m_storage.end())
		{
			return pos->second->cb;
		}

		return NULL;
	}
private:
	uint16		m_threads;
	rw_lock		m_lock;
	M			m_storage;
	uint32		m_delay;
};



#endif
