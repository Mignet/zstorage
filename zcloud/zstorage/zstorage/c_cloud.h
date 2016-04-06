#ifndef c_cloud_h
#define c_cloud_h

#pragma once
#include "c_base.h"

#define KEY_TIMEOUT		(3600 * 24 * 30)
class c_cloud
	: public c_base
{
public:
	c_cloud(const char* key);
	~c_cloud();
	void push(wisdom_param& param, int index, wisdom_IOStream& io);
	void limit(wisdom_param& param, int index, wisdom_IOStream& io);
	void query(wisdom_param& param, int index, wisdom_IOStream& io);
	void online(wisdom_param& param, int index, wisdom_IOStream& io);
	void size(wisdom_param& param, int index, wisdom_IOStream& io);
	void read(wisdom_param& param, int index, wisdom_IOStream& io);
	void index_(wisdom_param& param, int index, wisdom_IOStream& io);
	bool is_release();
private:
	uint32 get_read(const string& keyword);
private:
	string			m_key;
	uint32			m_effective;
};

WISDOM_PTR(c_cloud, wisdom_cloud);


class c_cloud_admin
	: public c_singleton<c_cloud_admin>
{
	typedef map<string, wisdom_cloud>	M;
public:
	wisdom_cloud find(const char* key)
	{
		c_rlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
			return pos->second;

		return NULL;
	}

	wisdom_cloud find_is_create(const char* key)
	{
		c_wlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
		{
			return pos->second;
		}

		wisdom_cloud l = new c_cloud(key);
		m_list.insert(make_pair(key, l));
		return l;
	}

	void check()
	{
		c_wlock lock(&m_lock);
		for (M::iterator pos = m_list.begin(); pos != m_list.end();)
		{
			if (pos->second->is_release())
			{
				m_list.erase(pos++);
			}
			else
			{
				++pos;
			}
		}
	}
private:
	rw_lock	m_lock;
	M		m_list;
};


#endif
