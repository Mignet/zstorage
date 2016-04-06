#include "stdafx.h"
#include "c_command.h"

c_command::c_command(void)
{
}


c_command::~c_command(void)
{
}

void c_command::on_command( wisdom_param& param, IConnect* con )
{
	char *ptr1;
	int len1;
	//1.获取第一个参数
	if (!param->get(1, &ptr1, len1))
	{
		wisdom_IOStream result = new c_ostream_error;
		result->push("The first parameter error.");
		RESULT_PARAM(result, con);
		return;
	}

	//2.小写转成大写
	ptr1 = strupr(ptr1);
	ptr1[len1] = 0;
	//3.查找指定权限对象
	MOBJECT::iterator pos = m_object.find(ptr1);
	if (pos == m_object.end())
	{
		wisdom_IOStream result = new c_ostream_error;
		result->push("Unknown command.");
		RESULT_PARAM(result, con);
		return;
	}

	//4.处理协议
	pos->second->deal(param, con);
}

void c_command::help( wisdom_IOStream& result )
{
	for (MOBJECT::iterator pos = m_object.begin(); pos != m_object.end(); ++pos)
	{
		pos->second->explain(result);
	}
}

bool c_command::help( const string& command, wisdom_IOStream& result )
{
	MOBJECT::iterator pos = m_object.find(command);
	if (pos != m_object.end())
	{
		pos->second->explain_child(result);
		return true;
	}

	return false;
}

void c_command::push( const string& cmd, c_object* object )
{
	m_object.insert(make_pair(cmd, object));
}

void c_iplist::push( IConnect* con )
{
	c_wlock lock(&m_lock);
	m_ipmap.insert(make_pair(con, con));
}

void c_iplist::erase( IConnect* con )
{
	c_wlock lock(&m_lock);
	m_ipmap.erase(con);
}

void c_iplist::list( wisdom_IOStream& os )
{
	c_rlock lock(&m_lock);
	for (IPMAP::iterator pos = m_ipmap.begin(); pos != m_ipmap.end(); ++pos)
	{
		string ip; uint16 port;
		pos->second->ipaddr(ip, port);
		ZOS zos;
		os->push((zos << ip << ":" << port << "*" << pos->second->get_power()).str());
	}
}
