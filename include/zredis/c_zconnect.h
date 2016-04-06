
#ifndef c_zconnect_h
#define c_zconnect_h


#pragma once

#include "c_zclient_public.h"
using namespace redis_cil;

namespace redis_cil
{
	class c_zconnect;

	class zclientSink
	{
	public:
		//连接成功
		virtual void onConnect(c_zconnect* zcon) = 0;
		//连接释放
		virtual void onRelease(c_zconnect* zcon) = 0;
		//数据
		virtual int onData(c_zconnect* zcon, wisdom_IOData& io) = 0;
		//可写
		virtual int onWrite(c_zconnect* zcon) = 0;
	};

	class c_zconnect
		: public mw_connect_sink
	{
	public:
		c_zconnect(zclientSink* sink);
		~c_zconnect(void);
		int _connect(const string& ip, uint16 port);
		int _send(unsigned char* buf, int len);
	private:
		//连接情况
		virtual int on_connect(int32_t, mw_connect*);

		//可读  int count 为可读字节数 
		virtual	int on_read(mw_connect* con, int32_t count);

		//可写 int count 为还需要发送的字节数
		virtual int on_write(mw_connect* con, int32_t count);

		//关闭连接
		virtual int on_disconnect(int32_t reason, mw_connect* con);

		//连接释放
		virtual int on_release(mw_connect* con);

		//可以收到本线程所有连接信号
		virtual void on_signal(mw_connect* con);

		//连接线程被重先绑定
		virtual void on_rebind(mw_connect* con) {}

	private:
		mw_connect*			m_con;
		zclientSink*		m_sink;
		ZIOStream			m_os;
	};

}

#endif

