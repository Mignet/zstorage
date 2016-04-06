#include "stdafx.h"
#include "c_broadcast.h"

void c_broadcast::Join(IConnect* con, uint8 type)
{
	c_wlock lock(&m_lock);
	MC::iterator pos = m_mc.find(con);
	if (pos == m_mc.end())
		m_mc.insert(make_pair(con, type));
	else
	{
		pos->second = type;
	}
}

void c_broadcast::Leave( IConnect* con )
{
	c_wlock lock(&m_lock);
	m_mc.erase(con);
}

void c_broadcast::broadcast(char* data, int len, uint8 type)
{
	c_rlock lock(&m_lock);
	for (MC::iterator iter = m_mc.begin(); iter != m_mc.end(); ++iter)
	{
		if (iter->second == 0 || type == iter->second)
			iter->first->asyn_send((unsigned char *)data, len);
	}
}

void c_broadcast::random( char* data, int len )
{
	c_rlock lock(&m_lock);
	if (m_mc.empty())
		return;

	int irand = rand() % m_mc.size();
	for (MC::iterator iter = m_mc.begin(); iter != m_mc.end(); ++iter)
	{
		if (irand-- == 0)
		{
			iter->first->asyn_send((unsigned char *)data, len);
			break;
		}
		
	}
}

bool c_broadcast::is_broadcast()
{
	c_rlock lock(&m_lock);
	return !m_mc.empty();
}

void c_broadcast::get_iplist( wisdom_IOStream& os )
{
	c_rlock lock(&m_lock);
	for (MC::iterator iter = m_mc.begin(); iter != m_mc.end(); ++iter)
	{
		string ip;
		uint16 port;
		iter->first->ipaddr(ip, port);
		ip += ":";
		ip += itostr(port);
		os->push(ip);
	}
}

wisdom_broadcast c_broadcast_admin::Join( uint32 tag, IConnect* con, uint8 type)
{
	return Join(itostr(tag), con, type);
}

wisdom_broadcast c_broadcast_admin::Join(const string& tag, IConnect* con, uint8 type)
{
	wisdom_broadcast broadcast = get(tag);
	broadcast->Join(con, type);
	return broadcast;
}

wisdom_broadcast c_broadcast_admin::get( uint32 tag )
{
	return get(itostr(tag));
}

wisdom_broadcast c_broadcast_admin::get( const string& tag )
{
	wisdom_broadcast broadcast;
	c_wlock lock(&m_lock);
	MB::iterator pos = m_broadcast.find(tag);
	if (pos == m_broadcast.end())
	{
		broadcast = new c_broadcast;
		m_broadcast.insert(make_pair(tag, broadcast));
	}
	else
	{
		broadcast = pos->second;
	}

	return broadcast;
}

void c_broadcast_admin::Leave( const string& tag, IConnect* con )
{
	c_wlock lock(&m_lock);
	MB::iterator pos = m_broadcast.find(tag);
	if (pos != m_broadcast.end())
	{
		pos->second->Leave(con);
	}
}

void c_broadcast_admin::Leave(const string& tag)
{
	c_wlock lock(&m_lock);
	MB::iterator pos = m_broadcast.find(tag);
	if (pos != m_broadcast.end())
	{
		if (!pos->second->is_broadcast())
		{
			m_broadcast.erase(pos);
		}
	}
}
