
#ifndef c_server_h
#define c_server_h

#pragma once
class c_server
	: public mw_acceptor_sink
	, public mw_net_timer_sink
	, public c_singleton<c_server>
{
public:
	c_server();
	~c_server();
	bool LoadConfig();
	int listen();
	const string& leavedb() const { return m_leavedb; }
	uint16 threads() const { return m_threads; }
	uint32 cache_size() const { return m_cache_size; }
	uint16 list_max_count() const { return m_list_max_count; }
	uint16 mlist_max_count() const { return m_mlist_max_count; }
	uint32 write_buffer_size() const { return m_write_buffer_size; }
	uint32 seq_valid_time() const { return m_seq_valid_time * 60; }
	uint32 object_effective() const { return m_object_effective * 60; }
	uint32 seq_clear_time() const { return m_seq_clear_time * 60; }
public:
	/*mw_acceptor_sink*/
	//某一端口接收到新的连接
	virtual int on_accept(mw_connect* con, unsigned short port);

	//某一端口停止监听
	virtual int on_stop(unsigned short){ return 0; }

	//对象被析构
	virtual int on_release(mw_acceptor*) { return 0; }

	virtual int on_timer(mw_net_timer *timer);
private:
	string		m_ip;
	uint16		m_threads;
	string		m_leavedb;
	uint32		m_cache_size;
	uint32		m_list_max_count;
	uint32		m_mlist_max_count;
	uint32		m_seq_valid_time;
	uint32		m_write_buffer_size;
	uint32		m_object_effective;
	uint32		m_seq_clear_time;
};



#endif
