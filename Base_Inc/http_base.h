
#ifndef c_http_base_h
#define c_http_base_h


#pragma once

namespace http
{
#define BSIZE		4096
	enum {
		error_succeed				= 0,		//成功
		error_code_socket_error		= 1,		//socket 错误
		error_code_http_error		= 2,
	};
	class c_head
	{
		typedef map<string, string>		M;
	public:
		c_head() {
		}

		string head_to_str()
		{
			string result;
			for (M::iterator pos = m_head.begin(); pos != m_head.end(); ++pos)
			{
				result += pos->first;
				result += ":";
				result += pos->second;
				result += "\r\n";
			}

			result += "\r\n\r\n";

			return result;
		}

		string& operator [] (const string& value){
			return m_head[value];
		}
	private:
		M	m_head;
	};

	class http_con
	{
	public:
		virtual const char* get_session() = 0;
		virtual const char* head(const string& header) = 0;
		virtual uint32 length() = 0;
		virtual int buffer_len() = 0;
		virtual void buffer_get(char** ptr, int& len) = 0;
		virtual void buffer_adv(int len) = 0;
	};

	class post_cb
	{
	public:
		virtual void cb_error(http_con* http, int reason, const string& error) = 0;
		virtual void cb_progress(http_con* http, uint32 buf_size) = 0;
		virtual void cb_release(http_con* http) = 0;
	};

	class c_http_base
		: public mw_connect_sink
	{
	public:
		c_http_base() : m_con(NULL) {}
	private:
		//连接回收
		virtual void http_on_release(mw_connect* con) = 0;
		//HTTP连接成功
		virtual void http_on_connect(mw_connect* con) = 0;
		//HTTP数据推送
		virtual void http_post(mw_connect* con, int32_t count) = 0;
		//HTTP数据接收
		virtual void http_recv(mw_connect* con, int32_t count) = 0;
	public:
		int connect(const char* host, uint16 port, int asyn = 0)
		{
			m_con = create_connect(this);
			return m_con->_connect(zn::domain(host).c_str(), port, asyn);
		}
	private:
		//连接情况
		virtual int on_connect(int32_t reason, mw_connect* con)
		{
			if (reason != 0)
			{
				__zprintf("connect reason:" << reason);
				con->_release();
				return 0;
			}

			http_on_connect(con);

			return 0;
		}

		//可读  int count 为可读字节数 
		virtual	int on_read(mw_connect* con, int32_t count)
		{
			http_recv(con, count);
			return 0;
		}

		//可写 int count 为还需要发送的字节数
		virtual int on_write(mw_connect* con, int32_t count)
		{
			http_post(con, count);
			return 0;
		}

		//关闭连接
		virtual int on_disconnect(int32_t reason, mw_connect* con)
		{
			con->_release();
			return 0;
		}

		//连接释放
		virtual int on_release(mw_connect* con)
		{
			http_on_release(con);
			return 0;
		}

		//可以收到本线程所有连接信号
		virtual void on_signal(mw_connect* con) {}

		//连接线程被重先绑定
		virtual void on_rebind(mw_connect* con) {}

	private:
		mw_connect*			m_con;
	};

	class _asyn_http
		: public c_http_base
		, public http_con
	{
		enum {
			_ansy_post	= 0,
			_ansy_get	= 1,
		};
	public:
		_asyn_http(const char* url, post_cb* cb) 
			: m_port(80) 
			, m_cb(cb)
			, m_have(false) 
			, m_ok(false)
			, m_length(0)
			, m_ansy(_ansy_post)
		{
			m_session = c_uuid::to_uuid62();
			char server[255];
			m_url[0] = 0;
			//http://
			sscanf(url + 7, "%[^/]%s", server, m_url);
			sscanf(server, "%[^:]%u", m_host, &m_port);
			__zprintf("host:" << m_host << " port:" << m_port << " url:" << m_url);
			m_head_in["Host"] = m_host;
			m_head_in["Connection"] = "keep-alive";
			m_head_in["Accept"]		= "*/*";
			m_head_in["User-Agent"] = "XPOST/1.0";

		}
		void add_header(const char* header, const char* val)
		{
			m_head_in[header] = val;
		}

		void append(char* ptr, int len)
		{
			m_ring.Append(ptr, len);
		}

		const char* get()
		{
			int result = connect(m_host, m_port);
			if (result != ERROR_NEWORK_SUCCESS)
			{
				m_cb->cb_error(this, error_code_socket_error, itostr(result));
				return m_session.c_str();
			}

			m_ansy = _ansy_get;

			return m_session.c_str();
		}

		const char* post()
		{
			int result = connect(m_host, m_port);
			if (result != ERROR_NEWORK_SUCCESS)
			{
				m_cb->cb_error(this, error_code_socket_error, itostr(result));
				return m_session.c_str();
			}

			m_ansy = _ansy_post;
			return m_session.c_str();
		}

	private:
		virtual const char* get_session() { return m_session.c_str(); }
		virtual const char* head(const string& header){ return m_head_out[header].c_str(); }
		virtual uint32 length() { return m_length; }
		virtual int buffer_len(){ return m_ring_recv.Count(); }
		virtual void buffer_get(char** ptr, int& len) {
			len = m_ring_recv.Attach((unsigned char**)ptr);
		}
		virtual void buffer_adv(int len) {
			if (m_ring_recv.Count() >= len)
				m_ring_recv.Advance(len);
		}

	private:
		//连接回收
		virtual void http_on_release(mw_connect* con)
		{
			m_cb->cb_release(this);
			delete this;
		}
		//HTTP连接成功
		virtual void http_on_connect(mw_connect* con)
		{
			if (m_ansy == _ansy_get)
			{
				string strCmd = __tos("GET " << m_url << " HTTP/1.1\r\n");
				con->_send((uint8*)strCmd.c_str(), strCmd.length());
				string str_header = m_head_in.head_to_str();
				con->_send((uint8*)str_header.c_str(), str_header.length());

			}
			else if (m_ansy == _ansy_post)
			{
				string strCmd = __tos("POST " << m_url << " HTTP/1.1\r\n");
				con->_send((uint8*)strCmd.c_str(), strCmd.length());
				m_head_in["Content-Length"] = __tos(m_ring.Count() + 2);
				string str_header = m_head_in.head_to_str();
				con->_send((uint8*)str_header.c_str(), str_header.length());

				http_post(con, 0);
			}
		}
		//HTTP数据推送
		virtual void http_post(mw_connect* con, int32_t count)
		{
			if (m_ansy == _ansy_get)
				return;

			while (m_ring.Count() > 0)
			{
				uint8* ptr = NULL;
				int len = 0;
				len = m_ring.Attach(&ptr);

				if (con->_send(ptr, len) != ERROR_NEWORK_SUCCESS)
				{
					break;
				}

				m_ring.Advance(len);
			}
		}
		//HTTP数据接收
		virtual void http_recv(mw_connect* con, int32_t count)
		{
			string error;
			int rt = have_head(con, count, error);
			switch (rt)
			{
			case have_good:
			{
				produce(con);
			}
			case have_ok:
			{
				m_length = atoi(m_head_out["Content-Length"].c_str());
				produce(con);
			}
				break;
			case have_continue:
				break;
			case hava_error:
			{
				m_cb->cb_error(this, error_code_http_error, error);
			}
				break;
			}
		}

		void produce(mw_connect* con)
		{
			while (true)
			{
				int icount = 0;
				con->_get_opt(OPT_NETWORK_GET_RECV_SIZE, &icount, sizeof(int));
				if (icount <= 0)
					break;
				size_t ptr = 0;
				int size = min(BSIZE, icount);
				con->_get_opt(OPT_NETWORK_GET_RECV, &ptr, size);
				m_ring_recv.Append((void*)ptr, size);
				con->_set_opt(OPT_NETWORK_SET_RECV, &size, sizeof(size));
			}

			m_cb->cb_progress(this, m_ring_recv.Count());
		}

	private:
		enum {
			have_good		= 0,
			have_ok			= 1,
			have_continue	= 2,
			hava_error		= 3,
		};
		int have_head(mw_connect* con, int32_t count, string& error)
		{
			while (count > 0)
			{
				if (m_have)
					return have_good;

				size_t ptr = 0;
				int size = min(BSIZE, count);
				con->_get_opt(OPT_NETWORK_GET_RECV, &ptr, size);

				char* header = (char*)ptr;

				if (strncmp(header, "\r\n", 2) == 0)
				{
					//结束了
					int recv = 2;
					con->_set_opt(OPT_NETWORK_SET_RECV, &recv, sizeof(recv));
					count -= recv;
					m_have = true;
					return have_ok;
				}

				char* pos = strstr(header, "\r\n");
				if (pos == NULL)
				{
					return have_continue;
				}

				*pos = 0;

				if (!m_ok)
				{
					if (strstr(header, "OK") == NULL){
						error = header;
						return hava_error;
					}

					m_ok = true;
				}
				else
				{
					char key[250];
					char val[250];
					
					char* p = strchr(header, ':');
					if (p == NULL) {
						error = "http error.";
						return hava_error;
					}
					*p = '\0';
					strcpy(key, header);
					strcpy(val, ++p);
					m_head_out[zn::str_trim(key)] = zn::str_trim(val);
				}

				int recv = pos - header + 2;
				con->_set_opt(OPT_NETWORK_SET_RECV, &recv, sizeof(recv));
				count -= recv;
			}

			return have_continue;
		}
	private:
		char				m_url[1024];
		char				m_host[1024];
		uint32				m_port;
		c_head				m_head_in;
		c_head				m_head_out;
		uint32				m_length;
		post_cb*			m_cb;
		CRingQueue<BSIZE>	m_ring;
		CRingQueue<BSIZE>	m_ring_recv;
		bool				m_have;
		bool				m_ok;
		int					m_ansy;
		string				m_session;
	};

	class c_http_get
		: public c_http_base
	{
	public:
		c_http_get() {}
		bool http_get(const string& url);
	private:
		string		m_url;
		c_head		m_head;
	};



	static unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	static unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}

	static std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	static std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

}



#endif
