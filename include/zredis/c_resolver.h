
#ifndef c_resolver_h
#define c_resolver_h
#pragma once
#include "zstream.h"

#define		MPARAM		'*'		//多参
#define		MLEN		'$'		//"$" 返回一个块数据长度
#define		CRLF		"\r\n"
#define		CR			'\r'
#define		SPACE		32
#define		TAB			9

#define		RESP_STATUS	'+'		//"+" 代表一个状态信息 如 +ok
#define		RESP_ERROR	'-'		//"-" 代表发送了错误  （如：操作运算操作了错误的类型）
#define		RESP_INT	':'		//:" 返回的是一个整数  格式如：":11\r\n。
#define		M_LEN		1024

#define		STREAM_PARTIAL			(-1)
#define		STREAM_ERROR			(-2)
#define		STREAM_OK				(0)

typedef CRingQueue< 4096 >	rq4096;
WISDOM_TPTR(rq4096, wisdom_rq4096);

class c_punit
	: public CMPool
{
public:
	c_punit(int n)
		: m_len(n+2), m_pos(0)
	{
		if (m_len > pool::max_allocate_size)
			m_str = (char*)malloc(m_len+1);
		else
			m_str = (char*)TLS_MALLOC(m_len+1);
	}
	~c_punit()
	{
		if (m_len > pool::max_allocate_size)
			free(m_str);
		else
			TLS_FREE(m_str);
	}

	inline int read(const char* ptr, int n)
	{
		if (m_pos >= m_len) return 0;
		int result = min(m_len - m_pos, n);
		memcpy(&m_str[m_pos], ptr, result);
		m_pos += result;
		//\r\n显示问题
		if (m_len == m_pos)
			m_str[m_len - 2] = '\0';

		return result;
	}

	inline bool finish()
	{
		return m_len == m_pos;
	}

	char*	m_str;
	int		m_len;
	int		m_pos;
};
WISDOM_PTR(c_punit, wisdom_punit);
typedef vector<wisdom_punit>	VP;

class c_istream
	: public CMPool
{	
public:
	c_istream():m_count(0), m_index(0), m_line(true) {}
	int read(const char* ptr, int n)
	{
		if (m_line)
		{
			const char* pos = zstrstr(ptr, n, "\r\n", 2);
			if (pos == NULL)
				return -1;
			/*if (pos - ptr > n || pos == NULL)
				return -1;
			if (pos - ptr + 2 > n)
				return -1;*/

			return read_param(ptr, pos - ptr + 2);
		}

		return read_data(ptr, n);
	}

	int read_param(const char* ptr, int n)
	{
		if (m_count == 0)
		{
			if (ptr[0] != MPARAM)
			{
				printf("STREAM_ERROR 1\r\n");
				return -2;
			}
			
			m_count = atoi(&ptr[1]);
			//check len
			if (m_count <= 0)
			{
				printf("STREAM_ERROR 2\r\n");
				return -2;
			}

			return n;
		}

		if (ptr[0] != MLEN)
		{
			printf("STREAM_ERROR 3\r\n");
			return -2;
		}

		int len = atoi(&ptr[1]);
		//check len
		//len == 0 
		if (len < 0)
		{
			printf("STREAM_ERROR 4\r\n");
			return -2;
		}

		wisdom_punit unit = new c_punit(len);
		m_vp.push_back(unit);
		m_index++;
		m_line = false;

		return n;
	}

	int read_data(const char* ptr, int n)
	{
		if (m_vp.empty())
		{
			printf("STREAM_ERROR 5\r\n");
			return -2;
		}

		if (m_vp.size() != m_index)
		{
			printf("STREAM_ERROR 6\r\n");
			return -2;
		}

		int result = m_vp[m_index - 1]->read(ptr, n);
		if (m_vp[m_index - 1]->finish())
		{
			m_line = true;
		}

		return result;
	}

	bool finish()
	{
		return (m_vp.size() == m_count && m_vp[m_vp.size() - 1]->finish());
	}

	void push(char* ptr, int len)
	{
		//去掉 \r\n
		wisdom_punit unit = new c_punit(len - 2);
		unit->read(ptr, len);
		m_vp.push_back(unit);
	}

	//参数从0开始
	bool get(int index, char** ptr, int& len)
	{
		if (index <= 0)
			return false;

		if (m_vp.size() <= index - 1)
			return false;

		*ptr = m_vp[index - 1]->m_str;
		//去掉\r\n
		len = m_vp[index - 1]->m_pos - 2;
		return true;
	}

	uint64_t to_integer(int index)
	{
		return atol(to_string(index).c_str());
	}

	string to_string(int index)
	{
		char* ptr;
		int len = 0;
		if (get(index, &ptr, len))
		{
			return ptr;
		}

		return "";
	}

	uint32 count()
	{
		return m_vp.size();
	}

private:
	VP			m_vp;
	int			m_count;				//参数个数
	int			m_index;				//参数索引
	bool		m_line;					//是否要读取一行
};

WISDOM_PTR(c_istream, wisdom_param);

class IOStream
{
public:
	virtual void push(const char* result) { push((char*)result, strlen(result));}
	virtual void push(const string& result) { push((char*)result.c_str(), result.length());}
	virtual void push(int result){}
	virtual void push(char* data, int len) = 0;
	virtual bool serialize(rq4096* ring) = 0;
	virtual bool empty() = 0;
	virtual void release() = 0;
	virtual wisdom_punit get(int index) { return NULL; }
};

WISDOM_RELEASE(IOStream, wisdom_IOStream);

class c_ostream_error
	: public IOStream
	, public CMPool
{
public:
	virtual void release() {delete this;}
	virtual void push(const string& result)
	{
		m_result = result;
	}

	virtual void push(char* data, int len)
	{
		m_result = string(data, len);
	}

	virtual bool serialize(rq4096* ring)
	{
		ring->Append(__tos("-" << m_result << "\r\n"));
		return true;
	}

	virtual bool empty()
	{
		return m_result.empty();
	}
private:
	string m_result;
};

class c_ostream_success
	: public IOStream
	, public CMPool
{
public:
	virtual void release() {delete this;}
	virtual void push(const string& result)
	{
		m_result = result;
	}

	virtual void push(char* data, int len)
	{
		m_result = string(data, len);
	}

	virtual bool serialize(rq4096* ring)
	{
		ring->Append(__tos("+" << m_result << "\r\n"));
		return true;
	}

	virtual bool empty()
	{
		return m_result.empty();
	}
private:
	string m_result;
};


class c_ostream_data
	: public IOStream
	, public CMPool
{
public:
	c_ostream_data() :m_result(NULL), m_len(0) {}
	~c_ostream_data() { if (m_result) free(m_result); }
	virtual void release() {delete this;}
	virtual void push(int result)
	{
		result = htonl(result);
		m_len = sizeof(int);
		m_result = (char*)malloc(m_len);
		memcpy(m_result, &result, m_len);
	}

	virtual void push(const string& result)
	{
		m_len = result.length();
		m_result = (char*)malloc(m_len);
		memcpy(m_result, result.c_str(), m_len);
	}

	virtual void push(char* data, int len)
	{
		m_len = len;
		m_result = (char*)malloc(m_len);
		memcpy(m_result, data, m_len);
	}

	virtual bool serialize(rq4096* ring)
	{
		ring->Append(__tos("$" << m_len << "\r\n"));
		ring->Append(m_result, m_len);
		ring->Append((void*)"\r\n", 2);
		return true;
	}

	virtual bool empty()
	{
		return m_len == 0;
	}
private:
	char*	m_result;
	int		m_len;
};

class c_ostream_array
	: public IOStream
	, public CMPool
{
public:
	~c_ostream_array() {}
	virtual void release() {delete this;}
	virtual void push(int result)
	{
		result = htonl(result);
		wisdom_punit unit = new c_punit(sizeof(int));
		unit->read((char *)&result, sizeof(int));
		m_result.push_back(unit);
	}

	virtual void push(const string& result)
	{
		wisdom_punit unit = new c_punit(result.length());
		unit->read((char *)result.c_str(), result.length());
		m_result.push_back(unit);
	}

	virtual void push(char* ptr, int n)
	{
		if (n <= 0) return;
		wisdom_punit unit = new c_punit(n);
		unit->read(ptr, n);
		m_result.push_back(unit);
	}

	virtual bool serialize(rq4096* ring)
	{
		if (m_result.empty())
			return false;

		ring->Append(__tos("*" << (uint32)m_result.size() << "\r\n"));
		for (VP::iterator pos = m_result.begin(); pos != m_result.end(); ++pos)
		{
			ring->Append(__tos("$" << (*pos)->m_pos << "\r\n"));
			ring->Append((*pos)->m_str, (*pos)->m_pos);
			ring->Append((void*)"\r\n", 2);
		}

		return true;
	}

	virtual bool empty()
	{
		return m_result.empty();
	}

	virtual wisdom_punit get(int index)
	{
		if (m_result.size() <= index)
			return NULL;
		return m_result[index];
	}
	
private:
	VP						m_result;
};


class IConnect;
class c_stream
	: public CMPool
{
	enum e_command
	{
		e_defalt		= 0,		
		e_commandline	= 1,		//命令行协议
		e_redis			= 2,		//redis协议
	};
public:
	c_stream(void):m_commandline(e_defalt){}
	~c_stream(void){}
 	inline int recv(char* data, int n)
	{
		if (m_commandline == e_defalt)
		{
			if (data[0] == '*')
				m_commandline = e_redis;
			else
				m_commandline = e_commandline;
		}
		m_ring.Append(data, n);
		return m_ring.Count();
	}

	int analyze(wisdom_param& param, IConnect* con);

	bool get_param(wisdom_param& param)
	{
		param = m_param;
		m_param = NULL;
	}
private:
	CRingQueue<4096>	m_ring;
	wisdom_param		m_param;
	vector<char>		m_vparam;
	e_command			m_commandline;
};



#endif //c_resolver_h
