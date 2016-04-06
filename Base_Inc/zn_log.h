
#ifndef ZN_LOG_H_
#define ZN_LOG_H_

#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#ifdef _MSC_VER
#include <direct.h>
#include <Windows.h>
#endif
#include <sys/stat.h>
#include "rw_lock.h"

#ifdef _MSC_VER
#define ctid() ::GetCurrentThreadId() 
#else
#define ctid() pthread_self()
#endif

#define _ZN_EVENT_LOG_DEBUG 0
#define _ZN_EVENT_LOG_MSG   1
#define _ZN_EVENT_LOG_WARN  2
#define _ZN_EVENT_LOG_ERR   3

class c_znLog
{
public:
	static int _err(const char *fmt, ...)
	{
		va_list ap;

		va_start(ap, fmt);
		_warn_helper(_ZN_EVENT_LOG_ERR, errno, fmt, ap);
		va_end(ap);
		return 0;
	}

	static int _wan(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		_warn_helper(_ZN_EVENT_LOG_WARN, errno, fmt, ap);
		va_end(ap);
		return 0;
	}

	static int _msg(const char *fmt, ...)
	{
		va_list ap;

		va_start(ap, fmt);
		_warn_helper(_ZN_EVENT_LOG_MSG, -1, fmt, ap);
		va_end(ap);
		return 0;
	}

	static int _dbg(const char *fmt, ...)
	{
		va_list ap;

		va_start(ap, fmt);
		_warn_helper(_ZN_EVENT_LOG_DEBUG, -1, fmt, ap);
		va_end(ap);
	}


	static void _warn_helper(int severity, int log_errno, const char *fmt, va_list ap)
	{
		//return;
		char buf[1024];
		size_t len;

		if (fmt != NULL)
			evutil_vsnprintf(buf, sizeof(buf), fmt, ap);
		else
			buf[0] = '\0';

		if (log_errno >= 0) {
			len = strlen(buf);
			if (len < sizeof(buf) - 3) {
				evutil_snprintf(buf + len, sizeof(buf) - len, "\t\t\t当前系统错误码: %s\r\n",
					strerror(log_errno));
				
			}
		}

		event_log(severity, buf);
	}


	static void event_log(int severity, const char *msg)
	{
		static zn::rw_lock  s_lock;
		zn::c_wlock lock(&s_lock);

		const char *severity_str;
		switch (severity) 
		{
		case _ZN_EVENT_LOG_DEBUG:
			severity_str = "dbg";
			break;
		case _ZN_EVENT_LOG_MSG:
			severity_str = "msg";
			break;
		case _ZN_EVENT_LOG_WARN:
			severity_str = "wan";
			break;
		case _ZN_EVENT_LOG_ERR:
			severity_str = "err";
			break;
		default:
			severity_str = "???";
			break;
		}
		
		if (1)
		{
			static FILE* pf = NULL;
			static time_t current_time = 0;
			if (pf == NULL || current_time / (3600 * 24) != time(0) / (3600 * 24))
			{
				current_time = time(0);
				struct tm* currenttime = localtime(&current_time);
				static char log_name[1024];
#ifdef _MSC_VER
				sprintf(log_name, "log\\log_%d_%d_%d.log",
					currenttime->tm_year + 1900, currenttime->tm_mon + 1, currenttime->tm_mday);
				_mkdir("log");
#else
				sprintf(log_name, "log/log_%d_%d_%d.log", 
					currenttime->tm_year + 1900, currenttime->tm_mon + 1, currenttime->tm_mday);
				mkdir("log", 0777);
#endif

				pf = fopen(log_name, "ab");
			}
			
			if (pf)
			{
				time_t ct = time(0);
				struct tm* t = localtime(&ct);
				
				(void)fprintf(pf, "%.2d:%.2d:%.2d %08X [%s]->%s\r\n",
					t->tm_hour, 
					t->tm_min, 
					t->tm_sec, 
					(uint32)ctid(),
					severity_str, 
					msg);

				fflush(pf);

#ifndef _NOT_PRINT_SCREEN
				(void)fprintf(stderr, "%.2d:%.2d:%.2d %08X [%s]->%s\r\n", 
					t->tm_hour, 
					t->tm_min, 
					t->tm_sec, 
					(uint32)ctid(),
					severity_str, 
					msg);
#endif

				errno = 0;
			}
		}
	}

private:
	static int evutil_snprintf(char *buf, size_t buflen, const char *format, ...)
	{
		int r;
		va_list ap;
		va_start(ap, format);
		r = evutil_vsnprintf(buf, buflen, format, ap);
		va_end(ap);
		return r;
	}

	static int evutil_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap)
	{
#ifdef _MSC_VER
		int r = _vsnprintf(buf, buflen, format, ap);
		buf[buflen-1] = '\0';
		if (r >= 0)
			return r;
		else
			return _vscprintf(format, ap);
#else
		int r = vsnprintf(buf, buflen, format, ap);
		buf[buflen-1] = '\0';
		return r;
#endif
	}

};

#define _this	radix16((uint64_t)this)
#define ferr(s)	c_znLog::_err(__tos(s))
#define finf(s)	c_znLog::_msg(__tos(s))

#endif //ZN_LOG_H_
