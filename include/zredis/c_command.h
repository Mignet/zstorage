
#ifndef c_command_h
#define c_command_h

#pragma once

#include "c_resolver.h"
#include "c_object.h"

#define SYN_SEND		"SYN_SEND"
#define	SYN_RECV		"SYN_RECV"
#define	SYN_DISCONNECT	"SYN_DISCONNECT"


namespace proxy_type
{
	enum
	{
		server_chatroom = 1,			//聊天室主控连接
		server_chatroom1 = 11,			//聊天室主控连接
		server_chatroom2 = 12,			//聊天室主控连接
		server_chatroom3 = 13,			//聊天室主控连接
		server_chatroom4 = 14,			//聊天室主控连接
		server_chatroom5 = 15,			//聊天室主控连接
		server_chatroom6 = 16,			//聊天室主控连接
		server_chatroom7 = 17,			//聊天室主控连接
		server_chatroom8 = 18,			//聊天室主控连接
		server_chatroom9 = 19,			//聊天室主控连接

		server_avtran	= 2,			//聊天室音视频转发
		server_avtran1	= 21,			//聊天室音视频转发
		server_avtran2	= 22,			//聊天室音视频转发
		server_avtran3	= 23,			//聊天室音视频转发
		server_avtran4	= 24,			//聊天室音视频转发

		server_mserver	= 3,			//多媒体服务器，主要是分发某个房间音视频数据
		server_im		= 4,			//IM服务器
		server_business = 5,			//其他业务服务器
		server_esb		= 6,			//ESB相关协议
		server_zline	= 7,			//连线服务器相关
		server_zline1	= 71,			//连线服务器相关
		server_zline2	= 72,			//连线服务器相关
		server_scc		= 8,			//nginx协议
		server_team		= 9,			//队伍连接
		server_team1	= 91,			//队伍连接
		server_team2	= 92,			//队伍连接
		server_proxy	= 10,
	};
}


static string type_str(uint8 type)
{
	switch (type)
	{
	default:
		case proxy_type::server_chatroom:	return "server_chatroom";
		case proxy_type::server_chatroom1:	return "server_chatroom1";
		case proxy_type::server_chatroom2:	return "server_chatroom2";
		case proxy_type::server_chatroom3:	return "server_chatroom3";
		case proxy_type::server_chatroom4:	return "server_chatroom4";
		case proxy_type::server_chatroom5:	return "server_chatroom5";
		case proxy_type::server_chatroom6:	return "server_chatroom6";
		case proxy_type::server_chatroom7:	return "server_chatroom7";
		case proxy_type::server_chatroom8:	return "server_chatroom8";
		case proxy_type::server_chatroom9:	return "server_chatroom9";
		case proxy_type::server_avtran:		return "server_avtran";
		case proxy_type::server_avtran1:	return "server_avtran1";
		case proxy_type::server_avtran2:	return "server_avtran2";
		case proxy_type::server_avtran3:	return "server_avtran3";
		case proxy_type::server_avtran4:	return "server_avtran4";
		case proxy_type::server_mserver:	return "server_mserver";
		case proxy_type::server_im:			return "server_im";
		case proxy_type::server_business:	return "server_business";
		case proxy_type::server_esb:		return "server_esb";
		case proxy_type::server_zline:		return "server_zline";
		case proxy_type::server_zline1:		return "server_zline1";
		case proxy_type::server_zline2:		return "server_zline2";
		case proxy_type::server_scc:		return "server_scc";
		case proxy_type::server_team:		return "server_team";
		case proxy_type::server_team1:		return "server_team1";
		case proxy_type::server_team2:		return "server_team2";
		case proxy_type::server_proxy:		return "server_proxy";
		break;
	}

	return itostr(type);
}

#define BROADCAST_TLV_COMMAND(_cast, _cmd, _buf, _n, _ip, _port, _agreement, _type)	\
	if (_cast && _cast->is_broadcast()) {	\
	const char* _data = _buf; int _len = _n;	\
	wisdom_IOStream result = new c_ostream_array; \
	result->push(__tos("Type:" << type_str(_type) << " msgId:0x" << zn::radix16(_cmd))); \
	string str;			\
	uint8* _ptr = (uint8*)_data;	\
	ZOS zos;	\
	char szNum[100];	\
	for (int i = 0; i < _len; i++){ \
	sprintf(szNum, "%02x", _ptr[i]);	\
	zos << szNum;} \
	result->push(zos.str()); \
	c_time time;	\
	result->push(__tos(_ip << ":(" << _port << ")=>" << _agreement << "#TIME:" << time.datatime_stamp())); \
	rq4096 ring;	\
	if (result->serialize(&ring)){ \
	while (ring.Count() > 0){ \
	uint8* ptr = NULL; \
	int len = ring.Attach(&ptr); \
	_cast->broadcast((char*)ptr, len, _type);	\
	ring.Advance(len); }	\
	} else { ferr("serialize error.");}}



#define SEND_IPADDR(con, agreement)		\
if (con->is_broadcast()){	\
	c_ostream_success stream;	\
	string ip;uint16 port; \
	con->ipaddr(ip, port);	\
	stream.push(__tos(ip << ":" << "(" << port << ")" << "=>" << agreement));	\
	wisdom_rq4096 ring = T_RINGQUEUE(4096);	\
	if (stream.serialize(ring)) { \
			while (ring->Count() > 0){ \
			uint8* ptr = NULL;		\
			int len = ring->Attach(&ptr); \
			con->broadcast(ptr, len);	\
			ring->Advance(len);	\
			}	\
	}	\
}


#define RESULT_PARAM(result, con)	\
	if (true){	\
	rq4096 ring;	\
	if (result->serialize(&ring)){ \
	while (ring.Count() > 0){ \
	uint8* ptr = NULL; \
	int len = ring.Attach(&ptr); \
	con->send(ptr, len); \
	con->broadcast(ptr, len);	\
	ring.Advance(len); } \
	SEND_IPADDR(con, "RECV");	\
	} else { ferr("serialize error.");}}


#define RESULT_ERROR(c, s)	\
	if (true){	\
	wisdom_IOStream result = new c_ostream_error;	\
	ZOS zos;	\
	ferr((zos << s << " explain=>" << m_explain).str());	\
	result->push(zos.str());	\
	RESULT_PARAM(result, c);	}

#define RESULT_OK(c, s)	\
	if (true){	\
	wisdom_IOStream result = new c_ostream_success;	\
	result->push(s);	\
	RESULT_PARAM(result, c);	}


class ITag
{
public:
	virtual void on_send(IConnect* con) {}
	virtual void release() = 0;
};

class IConnect;
class IBind
{
public:
	virtual void unbind(IConnect* con) = 0;
};

class IWait;
class IConnect
{
public:
	virtual int send(uint8* data, int len) = 0;
	virtual int ansy_packag(uint8* data, int len) = 0;
	virtual int asyn_send(uint8* data, int len) = 0;
	virtual int set_power(uint8 role) = 0;
	virtual int get_power() = 0;
	virtual int subscribe(const string& tag, uint8 type) = 0;
	virtual int unsubscribe(const string& tag) = 0;
	virtual int broadcast(uint8* data, int len, uint8 type = 0) = 0;
	virtual bool is_broadcast() = 0;
	virtual int ipaddr(string& ip, uint16& port) = 0;
	virtual int tagW(ITag* tag) = 0;
	virtual ITag* tagR() = 0;
	virtual void bind(IBind* bind) = 0;
	virtual void setSeqNum(uint32 seqNum) = 0;
	virtual bool checkSeqNum(uint32 seqNum) = 0;
	virtual void eraseSeqNum(uint32 seqNum) = 0;
	virtual void ansy_disconnect() = 0;
};

class IWait
{
public:
	virtual void close() = 0;
};


class c_iplist
	: public c_singleton<c_iplist>
{
	typedef map<IConnect*, IConnect*>	IPMAP;
public:
	void push(IConnect* con);
	
	void erase(IConnect* con);

	void list(wisdom_IOStream& os);
private:
	rw_lock		m_lock;
	IPMAP		m_ipmap;
};



class c_command
	: public c_singleton<c_command>
{
	typedef map<string, c_object*>		MOBJECT;
public:
	c_command(void);
	~c_command(void);
	void push(const string& cmd, c_object* object);

	void help(wisdom_IOStream& result);
	bool help(const string& command, wisdom_IOStream& result);
public:
	void on_command(wisdom_param& param, IConnect* con);

private:
	MOBJECT			m_object;
	
};

#endif //