#include "stdafx.h"
#include "c_zclient_public.h"


int redis_cil::ZIOStream::input( char* data, int size, int& pos, wisdom_IOData& os )
{
	if (!m_os)
	{
		switch (data[0])
		{
		case io_status:
		case io_error:
		case io_integer:
		case io_data:
		case io_array:
			m_os = new IOData((_io_type)data[0]);
			break;
		default:
			ferr("io_unknown:" << (int)data[0]);
			return io_unknown;
		}
	}

	int adv = 0;
	int status = m_os->input(data, size, adv);
	if (status == io_success)
	{
		os = m_os;
		m_os = NULL;
	}

	pos += adv;

	return status;
}

int redis_cil::IOData::input( char* data, int len, int& adv )
{
	switch (m_type)
	{
	case io_status:
	case io_error:
	case io_integer:
		{
			const char* ptr = zstrstr(data, len, "\r\n", 2);
			if (ptr == NULL)
				return io_receive;

			if (ptr - data >= len)
				return io_receive;
			
			if (m_type == io_integer)
			{
				m_integer = atol(&data[1]);
			}
			else
			{
				m_str = (char*)malloc(ptr - data);
				memset(m_str, 0, (ptr - data));
				memcpy(m_str, &data[1], ptr - data - 1);
			}

			adv = (ptr - data) + 2;
			
		}
		break;
	case io_data:
		{
			if (!m_data)
			{
				const char* ptr = zstrstr(data, len, "\r\n", 2);
				if (ptr == NULL)
					return io_receive;

				if (ptr - data >= len)
					return io_receive;

				int n = atoi(&data[1]);
				if (n <= 0)
				{
					ferr("1io_azerror:" << &data[1]);
					return io_azerror;
				}

				adv += ((ptr - data) + 2);

				//创建对象
				IOZPACKET(m_data, n + 2);
				IOZPACKET_WRITE(m_data, &data[adv], len - adv);
				adv += IOZPACKET_POS(m_data);
			}
			else
			{
				int n = IOZPACKET_ULEN(m_data);
				IOZPACKET_WRITE(m_data, data, len);
				adv += (n - IOZPACKET_ULEN(m_data));
			}

			if (IOZPACKET_OK(m_data))
			{
				if (!IOZPACKET_CHECK(m_data))
				{
					ferr("2io_azerror IOZPACKET_CHECK");
					return io_azerror;
				}

				m_data->m_len -= 2;
				m_data->m_ptr[m_data->m_len] = '\0';
				return io_success;
			}
			else
			{
				return io_receive;
			}
			
		}
		break;
	case io_array:
		{
			if (m_array_size == 0)
			{
				const char* ptr = zstrstr(data, len, "\r\n", 2);
				if (ptr == NULL)
					return io_receive;

				int n = atoi(&data[1]);
				if (n <= 0)
				{
					ferr("3io_azerror:" << &data[1]);
					return io_azerror;
				}

				adv += ((ptr - data) + 2);
				m_array_size = n;

				//继续
				return io_continue;
			}

			wisdom_iozpk pk;

			if (m_array.size() == 0 || IOZPACKET_OK(m_array[m_array.size() - 1]))
			{
				if (data[0] != io_data)
				{
					ferr("4io_azerror:" << (int)data[0]);
					return io_azerror;
				}

				const char* ptr = zstrstr(data, len, "\r\n", 2);
				if (ptr == NULL)
					return io_receive;

				int n = atoi(&data[1]);
				if (n <= 0)
				{
					ferr("5io_azerror:" << (int)data[0]);
					return io_azerror;
				}

				adv += ((ptr - data) + 2);

				//创建对象
				IOZPACKET(pk, n + 2);
				IOZPACKET_WRITE(pk, &data[adv], len - adv);
				adv += IOZPACKET_POS(pk);
				//压入
				m_array.push_back(pk);
			}
			else
			{
				pk = m_array[m_array.size() - 1];
				int n = IOZPACKET_ULEN(pk);
				IOZPACKET_WRITE(pk, data, len);
				adv += (n - IOZPACKET_ULEN(pk));
			}

			if (IOZPACKET_OK(pk))
			{
				if (!IOZPACKET_CHECK(pk))
				{
					//assert(0);
					ferr("6io_azerror:");
					return io_azerror;
				}

				if (m_array.size() == m_array_size)
				{
					//去掉"\r\n"
					for (int i = 0; i < m_array.size(); i++)
					{
						m_array[i]->m_len -= 2;
						m_array[i]->m_ptr[m_array[i]->m_len] = '\0';
					}
					return io_success;
				}
				else
					return io_continue;
			}
			else
			{
				return io_receive;
			}

		}
		break;
	default:
		//assert(0);
		break;
	}

	return 0;
}

std::string redis_cil::IOData::print()
{
	string result;
	switch (m_type)
	{
	case io_status:
	case io_error:
		return m_str;
	case io_integer:
		return "";
	case io_data:
		{
			for (int i = 0; i < m_data->m_len; i++)
			{
				char sz[10];
				sprintf(sz, "%0x", (uint8)m_data->m_ptr[i]);
				result += sz;
			}
		}
		break;
	case io_array:
		{
			for (int x = 0; x < m_array.size(); x++)
			{
				for (int y = 0; y < m_array[x]->m_len; y++)
				{
					char sz[10];
					sprintf(sz, "%0x", (uint8)m_array[x]->m_ptr[y]);
					result += sz;
				}

				result += "|";
			}
			
		}
		break;
	}

	return result;
}

void redis_cil::ZIO::push( wisdom_iozpk& pk )
{
	m_io.push_back(pk);
}

string redis_cil::ZIO::to_string( int i )
{
	char* ptr = NULL;
	int len = 0;
	if (get(i, &ptr, len))
	{
		return ptr;
	}

	return "";
}

uint32 redis_cil::ZIO::to_int32( int i )
{
	char* ptr = NULL;
	int len = 0;
	if (get(i, &ptr, len))
	{
		return atoi(ptr);
	}
	
	return 0;
}

bool redis_cil::ZIO::get(int i, char** ptr, int& len )
{
	if (i <= 0) return 0;
	if (m_io.size() < i)
		return 0;

	*ptr = m_io[i-1]->m_ptr;
	len = m_io[i-1]->m_len;

	return true;
}

int redis_cil::ZIO::size()
{
	return m_io.size();
}
