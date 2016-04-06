
#ifndef c_zredis_connect_h
#define c_zredis_connect_h
#pragma once
#include "c_command.h"
#include "c_broadcast.h"

class c_zredis_connect
		: public mw_connect_sink
		, public IConnect
		, public CMPool
{
	typedef map<uint32, uint32>		MWAIT;
	typedef list<wisdom_broadcast>	LSUB;
public:
	c_zredis_connect(mw_connect* con);
	~c_zredis_connect(void);

private:

	//可读  int count 为可读字节数 
	virtual	int on_read(mw_connect* con, int count);

	//可写 int count 为还需要发送的字节数
	virtual int on_write(mw_connect* con, int count);

	//关闭连接
	virtual int on_disconnect(int reason, mw_connect* con);

	//连接释放
	virtual int on_release(mw_connect* con);

	//收到信号
	virtual void on_signal(mw_connect* con);

	virtual int send(uint8* data, int len);
	virtual int ansy_packag(uint8* data, int len);
	virtual int asyn_send(uint8* data, int len);
	virtual int set_power(uint8 role);
	virtual int get_power();
	virtual int subscribe(const string& tag, uint8 type);
	virtual int unsubscribe(const string& tag);
	virtual int broadcast(uint8* data, int len, uint8 type = 0);
	virtual int random(uint8* data, int len);
	virtual bool is_broadcast();
	virtual int ipaddr(string& ip, uint16& port);
	virtual int tagW(ITag* tag);
	virtual ITag* tagR();
	virtual void bind(IBind* bind);
	virtual void setSeqNum(uint32 seqNum);
	virtual bool checkSeqNum(uint32 seqNum);
	virtual void eraseSeqNum(uint32 seqNum);
	virtual void ansy_disconnect();
private:
	int		on_transmit();
	void	ring_Append(uint8* buf, int len);
	bool	ring_Attach(uint8**buf, int& len);
	void	ring_Advance(int len);
	void	ring_Clear();
	int		ring_Count();
	void	clear_wait();
	void	check_wait();
private:
	mw_connect*			m_con;
	rw_lock				m_lock;
	CRingQueue<4096>	m_ring;
	c_stream			m_stream;
	uint8				m_power;
	wisdom_broadcast	m_broadcast;
	LSUB				m_subscribe;
	string				m_ip;
	uint16				m_port;
	thread_t			m_tid;
	ITag*				m_Tag;
	IBind*				m_Bind;
	MWAIT				m_waits;
	bool				m_asyn_disconnect;
};

class c_zredis_admin
	: public c_singleton<c_zredis_admin>
{
public:
	typedef map<IConnect*, IConnect*>	MC;
	void push(IConnect* con){
		c_wlock lock(&m_lock);
		m_cons.insert(make_pair(con, con));
	}

	void erase(IConnect* con){
		c_wlock lock(&m_lock);
		m_cons.erase(con);
	}

	bool check(IConnect* con) {
		c_wlock lock(&m_lock);
		return m_cons.find(con) != m_cons.end();
	}

	bool ansy_send(uint32 seqNum, IConnect* con, char* buf, int len)
	{
		c_rlock lock(&m_lock);
		if (m_cons.find(con) != m_cons.end() && con->checkSeqNum(seqNum))
		{
			wisdom_IOStream os = new c_ostream_data;
			os->push(buf, len);
			RESULT_PARAM(os, con);

			con->eraseSeqNum(seqNum);
		}

		return true;
	}

	bool ansy_send_timeout(uint32 seqNum, IConnect* con, char* ptr, int len)
	{
		c_rlock lock(&m_lock);
		if (m_cons.find(con) != m_cons.end() && con->checkSeqNum(seqNum))
		{
			wisdom_IOStream os = new c_ostream_data;
			os->push(ptr, len);
			RESULT_PARAM(os, con);

			con->eraseSeqNum(seqNum);
		}

		return true;
	}

	bool eraseSeqNum(uint32 seqNum, IConnect* con)
	{
		c_rlock lock(&m_lock);
		if (m_cons.find(con) != m_cons.end())
		{
			con->eraseSeqNum(seqNum);
		}

		return true;
	}
private:
	rw_lock	m_lock;
	MC		m_cons;
};

#endif //c_config_connect_h
