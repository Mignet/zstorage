#include "stdafx.h"
#include "c_resolver.h"
#include "c_command.h"

int c_stream::analyze( wisdom_param& param, IConnect* con )
{
	if (m_param == NULL)
	{
		m_param = new c_istream;
	}

	if (m_commandline == e_commandline)
	{
		while (m_ring.Count())
		{
			char* ptr = NULL;
			int len = m_ring.Attach((unsigned char **)&ptr);
			for (int i = 0; i < len; i++)
			{
				//去掉前面的\r\n
				if (m_vparam.empty())
				{
					if (ptr[i] == '\r' || ptr[i] == '\n' ||
						ptr[i] == SPACE || ptr[i] == TAB)
					{
						m_ring.Advance(1);
						break;
					}
				}
				//一个参数解析完成
				if (ptr[i] == SPACE || ptr[i] == TAB)
				{
					m_vparam.push_back('\r');
					m_vparam.push_back('\n');
					m_vparam.push_back(0);
					
					m_param->push(&m_vparam[0], m_vparam.size() - 1);
					if (con)
						con->broadcast((unsigned char *)&m_vparam[0], m_vparam.size() - 1);
					m_vparam.clear();

					m_ring.Advance(1);
					break;
				}

				if (ptr[i] == '\r' || ptr[i] == '\n')
				{
					m_vparam.push_back('\r');
					m_vparam.push_back('\n');
					m_vparam.push_back(0);
					
					m_param->push(&m_vparam[0], m_vparam.size() - 1);
					if (con)
						con->broadcast((unsigned char *)&m_vparam[0], m_vparam.size() - 1);
					m_vparam.clear();
					m_ring.Advance(1);

					param = m_param;
					m_param = NULL;
					return 0;
				}

				m_vparam.push_back(ptr[i]);
				m_ring.Advance(1);
			}
		}
	}
	else if (m_commandline == e_redis)
	{
		while (m_ring.Count() > 0)
		{
			int len = min(m_ring.Count(), 1024);
			unsigned char* ptr = NULL;
			m_ring.Attach(&ptr, len);
			int adv = m_param->read((const char *)ptr, len);
			if (adv < 0)
				return adv;

			if (m_param->finish())
			{
				param = m_param;
				m_param = NULL;
				if (con)
				{
					con->broadcast((unsigned char *)ptr, adv);
					SEND_IPADDR(con, "SEND");
				}
				m_ring.Advance(adv);
				return 0;
			}
			if (con)
				con->broadcast((unsigned char *)ptr, adv);

			m_ring.Advance(adv);
		}
	}
	

	return -1;
}
