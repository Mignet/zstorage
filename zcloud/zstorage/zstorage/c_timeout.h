
#ifndef c_timeout_h
#define c_timeout_h

#pragma once
#include "z_cmds.h"
class c_timeout
	: public c_singleton<c_timeout>
{
	typedef struct timeout_object
	{
		timeout_object() :m_type(0), m_effective(0) {}
		uint8	m_type;
		string	m_key;
		uint32	m_effective;
	}timeout_object;

	WISDOM_PTR(timeout_object, wisdom_timeout_object);
	typedef map<string, wisdom_timeout_object>		M;
	typedef queue<wisdom_timeout_object>			Q;
public:
	c_timeout() : m_check_long(time(0)) {}
	~c_timeout() {}
	void push(uint8 type, const string& key, uint32 effective)
	{
		if (effective > 3600 * 72)
			return;

		effective = time(0) + effective;

		c_wlock	lock(&m_lock);

		wisdom_timeout_object tobject = new timeout_object;
		tobject->m_type = type;
		tobject->m_effective = effective;
		tobject->m_key = key;
		m_temp.push(tobject);
		return;
	}

	void exec_temp()
	{
		while (true)
		{
			wisdom_timeout_object object;
			if (true)
			{
				c_wlock lock(&m_lock);
				if (m_temp.empty())
				{
					return;
				}

				object = m_temp.front();
				m_temp.pop();
			}

			if (!immediately_set(object))
			{
				long_set(object);
			}
		}
	}

	void check_long()
	{
		for (M::iterator pos = m_long.begin(); pos != m_long.end();)
		{
			if (immediately_set(pos->second))
			{
				m_long.erase(pos++);
			}
			else
			{
				++pos;
			}
		}
	}

	void timeout()
	{
		//处理临时对象
		exec_temp();

		if (time(0) > m_check_long)
		{
			check_long();
			m_check_long = time(0) + 120;
		}

		for (M::iterator pos = m_immediately.begin(); pos != m_immediately.end();)
		{
			if (pos->second->m_effective < time(0))
			{
				IAnalyzer* del = z_cmds::get_instance()->get_cmdId("DEL");
				wisdom_IOStream io = new c_ostream_array;
				wisdom_param param;
				del->exec(param, 0, pos->second->m_key, io);
				LOG4_INFO("timeout key:" << pos->second->m_key);

				m_immediately.erase(pos++);

			}
			else
			{
				++pos;
			}
		}
	}

private:
	bool immediately_set(wisdom_timeout_object& object)
	{
		if (object->m_effective < time(0) + 300)
		{
			M::iterator iter = m_immediately.find(object->m_key);
			if (iter != m_immediately.end())
			{
				m_immediately.erase(iter);
			}
			m_immediately.insert(make_pair(object->m_key, object));

			return true;
		}

		return false;
	}

	void long_set(wisdom_timeout_object& object)
	{
		M::iterator iter = m_long.find(object->m_key);
		if (iter != m_long.end())
		{
			m_long.erase(iter);
		}
		m_long.insert(make_pair(object->m_key, object));
	}

private:
	rw_lock		m_lock;
	Q			m_temp;
	M			m_immediately;
	M			m_long;
	uint32		m_check_long;
};



#endif
