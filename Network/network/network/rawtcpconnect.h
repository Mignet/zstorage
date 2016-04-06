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
	//���ûص���
	virtual void _set_sink(mw_connect_sink* psink);	

	//�ͻ�����ױ
	virtual int _connect(const char* addr, uint16_t port, int asyn = 1);

	//��������
	virtual int _send(unsigned char* date, int32_t size);
	
	//�����׽��ֲ���
	virtual int _set_opt(int32_t type, void* arg, int32_t size);

	//��ȡ�׽��ֲ���
	virtual int _get_opt(int32_t type, void* arg, int32_t size);

	//�Ͽ�����
	virtual int _disconnect(void);

	//�Ƿ����� 0�Ͽ� 1����
	virtual int _isconnect(void);

	//�������Ӷ���
	virtual int _release(void);

	//���߳��첽������Ϣ
	virtual int _signal(void);

	//���������Ȱ󶨵�ָ�����߳�
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
	//���߳��������Ӷ����յ��ź�
	virtual void on_signal();
	//���Ȱ󶨳ɹ�
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
	//rw_lock								m_send_lock;					//���ͻ���
	CRingQueue<RINGQUEUE_BLOCK_GRANULE>	m_send_ring;					// Ring buffer ���С 8K
	//rw_lock								m_recv_lock;					//���ջ���
	CRingQueue<RINGQUEUE_BLOCK_GRANULE>	m_recv_ring;					// Ring buffer ���С 8K
	
	int	m_max_send;						//����ͻ���
	int	m_max_recv;						//�����ջ���
	int m_network_packet_size;			//�����������С
	int m_network_timeout;				//�շ����ݳ�ʱ
	int m_active;						//�������һ�λ�Ծʱ��
public:
	client				m_client;
};

#endif

