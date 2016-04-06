
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
		//���ӳɹ�
		virtual void onConnect(c_zconnect* zcon) = 0;
		//�����ͷ�
		virtual void onRelease(c_zconnect* zcon) = 0;
		//����
		virtual int onData(c_zconnect* zcon, wisdom_IOData& io) = 0;
		//��д
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
		//�������
		virtual int on_connect(int32_t, mw_connect*);

		//�ɶ�  int count Ϊ�ɶ��ֽ��� 
		virtual	int on_read(mw_connect* con, int32_t count);

		//��д int count Ϊ����Ҫ���͵��ֽ���
		virtual int on_write(mw_connect* con, int32_t count);

		//�ر�����
		virtual int on_disconnect(int32_t reason, mw_connect* con);

		//�����ͷ�
		virtual int on_release(mw_connect* con);

		//�����յ����߳����������ź�
		virtual void on_signal(mw_connect* con);

		//�����̱߳����Ȱ�
		virtual void on_rebind(mw_connect* con) {}

	private:
		mw_connect*			m_con;
		zclientSink*		m_sink;
		ZIOStream			m_os;
	};

}

#endif

