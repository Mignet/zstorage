
#ifndef c_zclient_public_h
#define c_zclient_public_h
#pragma once

#include "c_resolver.h"

namespace redis_cil
{
	typedef zpacket< int > IOZPacket;
#define IOZPACKET(data,n) \
	data = new IOZPacket((n));	\
	data->m_object = 0;

#define IOZPACKET_LEN(data)		\
	(data->m_len)

#define IOZPACKET_ULEN(data)	\
	(data->m_len - data->m_object)

#define IOZPACKET_POS(data)	\
	(data->m_object)

#define IOZPACKET_OK(data)	\
	(data->m_len == data->m_object)

#define IOZPACKET_CHECK(data)	\
	(memcmp(&data->m_ptr[data->m_len - 2], "\r\n", 2) == 0)

#define IOZPACKET_WRITE(data, buf, len)	\
	if (true) {		\
	int nwrite = min(len, IOZPACKET_ULEN(data));	\
	if (nwrite > 0) {	\
	memcpy(&data->m_ptr[data->m_object], buf, nwrite);	\
	data->m_object += nwrite;	\
	}}


	WISDOM_PTR(IOZPacket, wisdom_iozpk);

	enum _io_type {
		io_default	= -1,
		io_status	= '+',
		io_error	= '-',
		io_integer	= ':',
		io_data		= '$',
		io_array	= '*',
	};

	enum _io_error {
		io_azerror	=	-2,	//解析错误
		io_unknown	=	-1,	//未知错误
		io_success	=	0,	//成功
		io_receive	=	1,	//接收数据中
		io_continue	=	2,	//继续
	};

	class ZIO
	{
	public:
		ZIO(){}
		void push(wisdom_iozpk& pk);
		string to_string(int i);
		uint32 to_int32(int i);
		bool get(int i, char** ptr, int& len);
		int size();
	private:
		vector<wisdom_iozpk>	m_io;
	};

	WISDOM_PTR(ZIO, wisdom_ZIO);

	class IOData
	{
		typedef vector<wisdom_iozpk>	V;
	public:
		IOData(_io_type type) : m_type(type), m_integer(0), m_str(NULL), m_array_size(0){}
		~IOData() { if (m_str) free(m_str); }
		int input(char* data, int len, int& adv);
		string print();
	public:
		_io_type			m_type;
		int64_t				m_integer;
		char*				m_str;
		wisdom_iozpk		m_data;
		V					m_array;
		int					m_array_size;			//数组大小
	};

	WISDOM_PTR(IOData, wisdom_IOData);

	class ZIOStream
	{
	public:
		ZIOStream() {}
		int input(char* data, int size, int& pos, wisdom_IOData& os); 
	private:
		wisdom_IOData		m_os;
	};

	class IZClient
	{
	public:
		virtual int sendCmd(const string& cmd) = 0;
		virtual int sendCmd(const string& cmd, const string& arg_1) = 0;
		virtual int sendCmd(const string& cmd, const string& arg_1, const string& arg_2) = 0;
		virtual int sendCmd(wisdom_IOStream& os) = 0;
	};

	class ICB
	{
	public:
		virtual int storage(IOStream* os) { return 0;}
		const char* constr() const {return m_constr.c_str();}
		virtual int onData(IZClient* client, wisdom_IOData& io){return 0;}
		virtual int doLogin(IZClient* client, rq4096& ring) {return 0;}
		virtual int onRelease(IZClient* client) {return 0;}
	public:
		string		m_constr;

	};
}


#endif

