
#ifndef df_macro_h
#define df_macro_h


#ifndef __android
#ifndef _MSC_VER 
#ifdef __x86_64__
#include "atomic64.h"
#else
#include "atomic32.h"
#endif
#endif
#endif




//获取线程ID
#ifdef _MSC_VER
#define _tid() ::GetCurrentThreadId() 
#else
#define _tid() pthread_self()
#endif

//获取内存大小
#ifdef _MSC_VER
#define _mem_size(x)  _msize(x)
#else
#define _mem_size(x)  malloc_usable_size(x)
#endif

#ifdef _MSC_VER
#define _atom long
#elif __android
#define _atom int
#else
#define _atom atomic_t
#endif

//初始化
#ifdef _MSC_VER
#define _atom_init(x) 	(x=0)
#elif __android
#define _atom_init(x) 	(x=0)
#else
#define _atom_init(x) 	atomic_set(&x, 0)
#endif


//原子读取
#ifdef _MSC_VER
#define _atom_read(x) (*x)
#elif __android
#define _atom_read(x) (*x)
#else
#define _atom_read(x) atomic_read(x)
#endif

//原子加一
#ifdef _MSC_VER
#define _atom_add_one(x) InterlockedIncrement(x)
#elif __android
#define _atom_add_one(x) (++(*x))
#else
#define _atom_add_one(x) atomic_add_return(1, x)
#endif

//原子加N
#ifdef _MSC_VER
	#define _atom_add(x, n)	InterlockedExchangeAdd(x, n)
#elif __android
	#define _atom_add(x, n)	((*x)+=n)

#else
	#define _atom_add(x, n)	atomic_add_return(n, x)
#endif

#ifdef _MSC_VER
#define _atom_sub(x, n)	InterlockedExchangeAdd(x, -n)
#elif __android
#define _atom_sub(x, n)	((*x) -=n)
#else
#define _atom_sub(x, n)	atomic_add_return(-n, x)
#endif

//原子自减一
#ifdef _MSC_VER
#define _atom_sub_one(x) InterlockedDecrement(x)
#elif __android
#define _atom_sub_one(x) (--(*x))
#else
#define _atom_sub_one(x) atomic_sub_return(1, x)
#endif


//原子比较
#ifdef _MSC_VER
#define _atom_compare(x,y) (InterlockedCompareExchange(x, y, y) == y)
#elif __android
#define _atom_compare(x,y) (x == y)
#else
#define _atom_compare(x,y) (atomic_read(x) == y)
#endif


//切换线程函数
#ifdef _MSC_VER
#define switch_thread()		SwitchToThread()
#else
#define switch_thread()		sched_yield()
#endif

//休息毫秒
#ifdef _MSC_VER
#ifndef _sleep
#define _sleep(x)			Sleep(x)
#endif
#else
#define _sleep(x)			usleep(x * 1000)
#endif

#ifdef _MSC_VER
#define S_ISREG(m) (((m) & 0170000) == (0100000))
#define S_ISDIR(m) (((m) & 0170000) == (0040000))
#endif

#ifdef WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#endif

#define SATOI(x)	atoi(x.c_str())

static int urlcasecmp(const char* url, const char* sub, int len)
{
	if (strncasecmp(url, sub, len) == 0)
	{
		if (url[len] == 0 || url[len] == '\\' || url[len] == '/' || url[len] == '?')
			return 0;
	}

	return -1;
}


#ifndef _LIBEVENT2_X
#if defined(_MSC_VER) && _MSC_VER < 1300
#define __func__ "??"
#else
#define __func__ __FUNCTION__
#endif

#ifdef _MSC_VER
#undef socklen_t
#define socklen_t	int
#endif

#endif


//休息毫秒
#ifdef _MSC_VER
#define thread_t			unsigned long		
#else
#define thread_t			pthread_t
#endif

#ifdef _MSC_VER
#define 	z_atoi				_atoi64
#else
#define         z_atoi          atoll
#endif

#define		$64					"%llu"

#ifndef _MSC_VER
#define		Z_MEMINFO			"/proc/meminfo"
#define		Z_CPUINFO			"/proc/cpuinfo"
#define		Z_STAT				"/proc/stat"
#else
#define		Z_MEMINFO			"e:/meminfo"
#define		Z_CPUINFO			"e:/cpuinfo"
#define		Z_STAT				"e:/stat"
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS	64
#endif

#define MAX_UCHAT			255

#endif   //df_mac
