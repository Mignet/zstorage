
#ifndef _container_h
#define _container_h

#pragma once

typedef struct _request
{
	_request() :m_serial(0), m_con(NULL), m_ms(get_ms()) {}
	uint32		m_serial;
	IConnect*	m_con;
	uint64_t	m_ms;
	string		m_key;
}_request;
WISDOM_PTR(_request, wisdom_request);

class _container
	: public IBind
	, public mw_thread_cb
	, public mw_net_timer_sink
{
	
	typedef map<uint32, wisdom_request>	M;
public:
	_container() :m_index(0) 
	{
		m_delay = c_broadcast_admin::get_instance()->get("DELAY");
	}
	uint32 push(wisdom_request& request)
	{
		m_serial.insert(make_pair(m_index, request));

		return m_index++;
	}

	wisdom_request pop(uint32 serial)
	{
		M::iterator pos = m_serial.find(serial);
		if (pos != m_serial.end())
		{
			if (get_ms() - pos->second->m_ms > c_server::get_instance()->delay())
			{
				string s = __tos("serial:" << pos->second->m_serial << " Key:" << pos->second->m_key
					<< " Time consuming:" << (uint32)(get_ms() - pos->second->m_ms) << " ms\r\n");

				m_delay->broadcast((char*)s.c_str(), s.length() + 1, 0);
			}
			return pos->second;
		}
		return NULL;
	}

	size_t size()
	{
		return m_serial.size();
	}

private:
	virtual void thread_cb()
	{
		create_net_timer(TLS(_container))->_schedule(1000 * 30000);
	}

	virtual int on_timer(mw_net_timer *timer)
	{
		for (M::iterator pos = m_serial.begin(); pos != m_serial.end();)
		{
			if (get_ms() - pos->second->m_ms > 30000)
			{
				m_serial.erase(pos++);
			}
			else
			{
				++pos;
			}
		}
		return 0;
	}
	virtual void unbind(IConnect* con)
	{
		for (M::iterator pos = m_serial.begin(); pos != m_serial.end();)
		{
			if (pos->second->m_con == con)
			{
				m_serial.erase(pos++);
			}
			else
			{
				++pos;
			}
		}
	}
private:
	M					m_serial;
	uint32				m_index;
	wisdom_broadcast	m_delay;
};


#endif

