
#ifndef c_time_h
#define c_time_h
#include <time.h>
#include <string>
using namespace std;

namespace zn
{
	class c_time
	{
	public:
		c_time()
			: m_time(time(0)) {}

		c_time(time_t t)
			: m_time(t) {}

		c_time(const c_time& t)
			:m_time(t.m_time) {}

		//"2013-04-15 12:12:12"
		c_time(const string& str_time)
		{
			struct tm st;
			memset(&st, 0, sizeof(tm));
			sscanf(str_time.c_str(), "%4u-%d-%d %d:%d:%d",
				&st.tm_year, &st.tm_mon, &st.tm_mday, &st.tm_hour, &st.tm_min, &st.tm_sec);
			st.tm_year -= 1900;
			st.tm_mon -= 1;
			m_time = mktime(&st);
		}

		c_time & operator + (const int& tick)
		{
			this->m_time += tick;

			return *this;	
		}

		c_time & operator += (const int& tick)
		{
			this->m_time += tick;

			return *this;	
		}

		c_time & operator - (const int& tick)
		{
			this->m_time -= tick;

			return *this;	
		}

		c_time & operator -= (const int& tick)
		{
			this->m_time -= tick;

			return *this;	
		}

		c_time & operator = (const c_time & t)
		{
			if( this != &t )		
			{
				this->m_time = t.m_time;
			}

			return *this;	
		}

		c_time & operator = (time_t t)
		{
			if( this->m_time != t )		
			{
				this->m_time = t;
			}

			return *this;	
		}

		time_t operator -> () const	
		{		
			return m_time;	
		}

		operator time_t () const 
		{
			return m_time;
		}

		time_t unix_time()
		{
			return m_time;
		}

		string time_stamp()
		{
			char sztime[50];
			tm* t = localtime(&m_time);
			sprintf(sztime, "%04u-%02u-%02u %02u:%02u:%02u", 
				t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
			return sztime;
		}

		string datatime_stamp()
		{
			char sztime[50];
			tm* t = localtime(&m_time);
			sprintf(sztime, "%02u:%02u:%02u",
				t->tm_hour, t->tm_min, t->tm_sec);
			return sztime;
		}

	private:
		time_t	m_time;
	};

}

#endif //c_time_h
