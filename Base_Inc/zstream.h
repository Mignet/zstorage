
#ifndef zstream_h
#define zstream_h

namespace zn
{
	class unixtime
	{
	public:
		unixtime(const uint32 uxtime) :m_unixtime(uxtime){}
		unixtime & operator = (uint32 n){ m_unixtime = n; return *this; }
		time_t	m_unixtime;
	};
	class radix16
	{
	public:
		radix16(const radix16& radix):m_radix(radix.m_radix){}
		radix16(uint64_t n):m_radix(n){}
		radix16(int64_t n):m_radix(n){}
		radix16(uint32 n):m_radix(n){}
		radix16(int32 n):m_radix(n){}
		radix16(uint16 n):m_radix(n){}
		radix16(int16 n):m_radix(n){}
		radix16(uint8 n):m_radix(n){}
		radix16(int8 n):m_radix(n){}
		radix16(unsigned long n):m_radix(n){}
		radix16 & operator = (uint64_t n){ m_radix = n; return *this;	}
		radix16 & operator = (radix16& radix){ m_radix = radix.m_radix; return *this;	}
		uint64_t	m_radix;
	};

	template <int T>
	class _os
		: public CMPool
	{
	public:
		_os(){ m_buf[0] = '\0';}

		template <int X> friend _os< X >& operator << (_os< X >& o, double n);
		template <int X> friend _os< X >& operator << (_os< X >& o, float n);
		template <int X> friend _os< X >& operator << (_os< X >& o, unsigned long n);
		template <int X> friend _os< X >& operator << (_os< X >& o, long n);
		template <int X> friend _os< X >& operator << (_os< X >& o, uint64_t n);
		template <int X> friend _os< X >& operator << (_os< X >& o, int64_t n);
		template <int X> friend _os< X >& operator << (_os< X >& o, uint32 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, int32 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, uint16 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, int16 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, uint8 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, int8 n);
		template <int X> friend _os< X >& operator << (_os< X >& o, char* str);
		template <int X> friend _os< X >& operator << (_os< X >& o, const char* str);
		template <int X> friend _os< X >& operator << (_os< X >& o, const string& str);
		template <int X> friend _os< X >& operator << (_os< X >& o, const radix16& n);
		template <int X> friend _os< X >& operator << (_os< X >& o, const unixtime& n);
		template <int X> friend _os< X >& operator << (_os< X >& o, bool n);
		template <int X> friend _os< X >& operator << (_os< X >& o, void* n);

		inline _os& clear() {
			m_buf[0] = '\0';
			return *this;
		}

		inline const char* str() {
			return (const char*)m_buf;
		}

	public:
		_os& cat(const char* str){
			strcat(m_buf, str);
			return *this;
		}

		char	m_buf[T];
	};

	template <int X>
	_os< X >& operator << (_os< X >& o, double n) {
		char tmp[100];
		sprintf(tmp, "%lf", n);
		return o.cat(tmp);
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, float n) {
		char tmp[100];
		sprintf(tmp, "%f", n);
		return o.cat(tmp);
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, unsigned long n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, long n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, uint64_t n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, int64_t n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, uint32 n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>  
	_os< X >& operator << (_os< X >& o, int32 n) { 
		return o.cat(zltoa(n, tls_buf(), 10)); 
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, uint16 n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, int16 n) {
		return o.cat(zltoa(n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, uint8 n) {
		return o.cat(zltoa((int)n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, int8 n) {
		return o.cat(zltoa((int)n, tls_buf(), 10));
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, char* str) {
		return o.cat(str); 
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, const char* str) {
		return o.cat(str); 
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, const string& str) {
		return o.cat(str.c_str()); 
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, const radix16& n) {
		return o.cat(zltoa(n.m_radix, tls_buf(), 16)); 
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, const unixtime& n) {
		char sztime[50];
		tm* t = localtime(&n.m_unixtime);
		sprintf(sztime, "%04u-%02u-%02u %02u:%02u:%02u",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		return o.cat(sztime);
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, bool n) {
		return n ? o.cat("1") : o.cat("0");
	}

	template <int X>
	_os< X >& operator << (_os< X >& o, void* n) {
		return o.cat(zltoa((unsigned long)n, tls_buf(), 16));
	}
}

typedef zn::_os<8192>	ZOS;

#define __zos(os, msg) (char*)(os << msg).str()
#define __tos(msg) __zos(TLS(ZOS)->clear(),msg)
#define __zprintf(msg) printf(__tos(msg << "\r\n"))
#define __zsprintf(dst,msg) strcpy(dst,__tos(msg))
#define __zstrcat(dst,msg) strcat(dst, __tos(msg))


#endif
