#pragma once

#ifndef _LENTCPACCEPTOR_H_
#define _LENTCPACCEPTOR_H_

#include "socketbase.h"

class CRawTcpAcceptor 
	: public mw_acceptor
	, public CSocketBase
	, public CMPool
{
public:
	CRawTcpAcceptor(mw_acceptor_sink* pSink, e_class_connect e_type);
	virtual ~CRawTcpAcceptor(void);
private:
	//开始监听一个端口
	virtual int _start_listen(uint16_t port, uint32_t addr = 0);

	//停止监听一个端口
	virtual int _stop_listen(void);

	//析构连接接收器
	virtual int _release();
private:
	static void on_accept(int32_t fd, short ev, void *arg);
private:
	void OnConnectIndication(int32_t fd, short ev);
public:
	virtual void on_close(int32_t error);
	virtual void on_release();
private:
	mw_acceptor_sink*	m_sink;
	struct event		m_ev_accept;
	unsigned short		m_listen_port;
	e_class_connect		m_e_type;
};

#endif

