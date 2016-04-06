#ifndef c_zclient_h
#define c_zclient_h

#pragma once

#include "c_zconnect.h"
#include "c_resolver.h"

namespace redis_cil
{
	class c_zclient
		: public zclientSink
		, public IZClient
	{
		struct DB
		{
		public:
			DB() :port(0) {}
			string server;
			uint16 port;
			string account;
			string pwd;
		};

		typedef queue<wisdom_IOStream>	Q;
	public:
		c_zclient(ICB* cb);
		~c_zclient(void);
	private:
		virtual int sendCmd(const string& cmd);
		virtual int sendCmd(const string& cmd, const string& arg_1);
		virtual int sendCmd(const string& cmd, const string& arg_1, const string& arg_2);
		virtual int sendCmd(wisdom_IOStream& os);
	private:
		//发送数据
		int send(wisdom_IOStream& os);

		int connect();

		int auth();
		//写数据
		int doWrite();

		int rawSend(rq4096& ring);

		//连接成功
		virtual void onConnect(c_zconnect* zcon);
		//连接释放
		virtual void onRelease(c_zconnect* zcon);
		//数据
		virtual int onData(c_zconnect* zcon, wisdom_IOData& io);

		virtual int onWrite(c_zconnect* zcon);

	private:
		c_zconnect*		m_zcon;
		rq4096			m_ring;
		DB				m_db;
		ICB*			m_cb;
	};

	class TThreadClient
	{
		WISDOM_PTR(IZClient, wisdom_zclient);
		typedef map<ICB*, wisdom_zclient>	M;
	public:
		/*
		连接字符串
		{"server":"chat.yunva.com","port":9379,"auth":"root","pwd":"123456"}
		*/
		IZClient* get(ICB* cb)
		{
			M::iterator pos  = m_zclients.find(cb);
			if (pos == m_zclients.end())
			{
				wisdom_zclient zclient = new c_zclient(cb);
				m_zclients.insert(make_pair(cb, zclient));
				return zclient;
			}

			return pos->second;
		}
	private:
		M		m_zclients;
	};

#define THREAD_ZCLIENT(c)	TLS(redis_cil::TThreadClient)->get(c)

}

#endif


