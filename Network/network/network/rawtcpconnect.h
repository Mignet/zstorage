#pragma once

#ifndef _LENTCPCONNECTION_H_
#define _LENTCPCONNECTION_H_

#include "socketbase.h"
#include "libevent.h"

class CRawTcpConnection 
	: public mw_connect
	, public IConnectSink
	, public CSocketBase
	, public CMPool
{
	typedef struct client {
		struct event ev_read;
		struct event ev_write;
		char buffer[ 8192 ];
	}client;
public:
	CRawTcpConnection(mw_connect_sink* psink);
	CRawTcpConnection(int fd, const string& remote_ip);
	virtual ~CRawTcpConnection(void);
protected: /*mw_connect*/
	//设置回调器
	virtual void _set_sink(mw_connect_sink* psink);	

	//客户端连妆
	virtual int _connect(const char* addr, uint16_t port, int asyn = 1);

	//发送数据
	virtual int _send(unsigned char* date, int32_t size);
	
	//设置套接字参数
	virtual int _set_opt(int32_t type, void* arg, int32_t size);

	//获取套接字参数
	virtual int _get_opt(int32_t type, void* arg, int32_t size);

	//断开连接
	virtual int _disconnect(void);

	//是否连接 0断开 1连接
	virtual int _isconnect(void);

	//析构连接对像
	virtual int _release(void);

	//多线程异步发送信息
	virtual int _signal(void);

	//把连接重先绑定到指定的线程
	virtual int _rebind(thread_t bind_tid);
private:
	virtual	int syn_connect(uint32 ipaddr, uint16 port);
	virtual int asyn_connect(uint32 ipaddr, uint16 port);
private:
	static void _on_connect(int32_t fd, short ev, void *arg);
	static void _on_read(int32_t fd, short ev, void *arg);
	static void _on_write(int32_t fd, short ev, void *arg);
public:
	virtual void on_connect(int32_t fd, short ev);
	virtual void on_recv(int32_t fd, short ev);
	virtual void on_send(int32_t fd, short ev);
	//本线程所有连接都会收到信号
	virtual void on_signal();
	//重先绑定成功
	virtual void on_rebind();
public:
	bool is_timeout();
	virtual void on_close(int32_t error);
	virtual void on_release();
private:
	int do_send(void);
private:
	mw_connect_sink*	m_sink;
	uint32_t			m_addr;
	uint16_t			m_port;
	uint16_t			m_bConnection;
	timeval				m_timeout;
	thread_t			m_threadid;
	thread_t			m_original_threadid;
	string				m_remote_ip;
private:
	//rw_lock								m_send_lock;					//发送缓存
	CRingQueue<RINGQUEUE_BLOCK_GRANULE>	m_send_ring;					// Ring buffer 块大小 8K
	//rw_lock								m_recv_lock;					//接收缓存
	CRingQueue<RINGQUEUE_BLOCK_GRANULE>	m_recv_ring;					// Ring buffer 块大小 8K
	
	int	m_max_send;						//最大发送缓存
	int	m_max_recv;						//最大接收缓存
	int m_network_packet_size;			//发送网络包大小
	int m_network_timeout;				//收发数据超时
	int m_active;						//连接最近一次活跃时间
public:
	client				m_client;
};

#endif

