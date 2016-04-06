
#ifndef c_broadcast_h
#define c_broadcast_h
#pragma once

#include "c_command.h"

#define TLS_BROADCAST


//广播
class c_broadcast
{
	typedef map<IConnect*, uint8>	MC;
public:
	void Join(IConnect* con, uint8 type);
	void Leave(IConnect* con);
	//向随机一个连接发送广播消息
	void random(char* data, int len);
	void broadcast(char* data, int len, uint8 type);
	bool is_broadcast();
	void get_iplist(wisdom_IOStream& os);
private:
	rw_lock m_lock;
	MC		m_mc;
};

//定义广播
WISDOM_PTR(c_broadcast, wisdom_broadcast);

class c_broadcast_admin
	: public c_singleton<c_broadcast_admin>
{
	typedef map<string, wisdom_broadcast>	MB;
public:
	wisdom_broadcast Join(uint32 tag, IConnect* con, uint8 type = 0);
	wisdom_broadcast get(uint32 tag);
	wisdom_broadcast Join(const string& tag, IConnect* con, uint8 type = 0);
	wisdom_broadcast get(const string& tag);
	void Leave(const string& tag, IConnect* con);
	void Leave(const string& tag);
private:
	rw_lock		m_lock;
	MB			m_broadcast;
};


#endif //c_broadcast_h


