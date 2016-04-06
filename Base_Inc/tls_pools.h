#if !defined(__TLS_POOLS__)
#define __TLS_POOLS__

#ifdef _MSC_VER
#pragma warning(disable: 4200)
#pragma warning(disable: 4786)
#pragma warning(disable: 4355 4244 4267 4311 4312)
#endif

#include <time.h>

#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <assert.h>
using namespace std;

#include "df_macro.h"
#include "tls.h"
#include "rw_lock.h"
#include "c_singleton.h"
#ifndef _TLS_NO_WRITE_LOG
#include "zn_log.h"
#endif
using namespace zn;

#define TLS_MALLOC(n)	pool::CMemory::tls_malloc(n)
#define TLS_FREE(p)	pool::CMemory::tls_free(p)
namespace pool {

	const uint16_t	buffer_mem_size		= 512;			//最小内存单位
	const int16_t	buffer_reserved		= sizeof(uint32_t) + sizeof(void*); //预留字节数,前4位识别码后面保存对像指针
	const uint32_t	max_granule			= 0X21000;		//128K
	const uint32_t	max_allocate_size	= 0X20000;		//允许分配内存

	inline int32_t calculate_size(int32_t nSize)
	{
		return nSize + 
			(buffer_mem_size - (nSize % buffer_mem_size)) + buffer_reserved;
	}

	//内存统计类
	class c_memory_statis
		: public c_singleton<c_memory_statis>
	{
		typedef map<thread_t, double>		MStatis;
	public:
		void push(thread_t tid, double size)
		{
			c_wlock lock(&m_lock);
			MStatis::iterator pos = m_statis.find(tid);
			if (pos == m_statis.end())
			{
				m_statis.insert(make_pair(tid, size));
			}
			else
			{
				pos->second = size;
			}
		}

		double size()
		{
			double result = 0;
			c_rlock lock(&m_lock);
			for (MStatis::iterator pos = m_statis.begin(); pos != m_statis.end(); ++pos)
			{
				result += pos->second;
			}

			return result;
		}

	private:
		rw_lock		m_lock;
		MStatis		m_statis;
	};

	class CMemory
	{
		typedef struct tick_
		{
			tick_() :utick(0) {}
			time_t utick;
		}_tick;

		class Container
		{
		public:
			typedef queue<CMemory*> pool_list;
			Container()
			{
				m_allpool = new pool_list;
				m_idle = new pool_list;
			}

			pool_list* m_allpool;
			pool_list* m_idle;
		};

		typedef Container*	PContainer;
		class _tls_space
		{
		public:
			_tls_space(){memset(m_container, 0, sizeof(m_container));}
		public:
			PContainer operator[](int n) {
				if (m_container[n] == NULL)
					m_container[n] = new Container;

				return m_container[n];
			}
		public:
			PContainer		m_container[max_granule];
		};
		//typedef map<uint32_t, Container*> MAP;

	private:
		CMemory(int32_t nSize)
			: m_allocate_size(nSize)
		{
			//printf("new size=%d\t", nSize);

			//1.分配内存大小
			m_pData = (unsigned char*)calloc(1, nSize);

			//2.获取实际内存大小
			int32_t _size = mem_size(m_pData);

			//3.写入内存标示
			int32_t* position = (int32_t*)&m_pData[_size - buffer_reserved];
			//保留字前4位为内存实际大小
			/*
			(*position) = _size;
			position++;
			//保留字后4位为所在对象指针  TODO:64BIT 需要修改
			(*position) = (const long)this;
			*/
			(*position) = _size;
			position++;
			 *((long*)position) = (const long)this;			

			m_nCrtThreadID = ctid();
			m_nRefThreadID = 0;
		}

		~CMemory()
		{
			free(m_pData);
		}

		inline unsigned char* buffer()
		{
			return m_pData;
		}

	public:
		inline static void* tls_malloc(int32_t nSize, bool bCalculate = true)  //分配内存
		{
			//1.获取TLS内存对像,不存在则创建
			_tls_space* _space = TLS(_tls_space);
#ifdef _POOL_LOG
			_tick* tick = TLS(_tick);

			//===========================输出本线程内存块实用情况================================

			if (time(0) - tick->utick > 180)
			{
				double total_size = 0.0;
				for (int i = 0; i < max_granule; i++)
				{
					if (_space->m_container[i] == NULL)
						continue;
					c_znLog::_msg("thread=%8x, block=%8d, count=%8d, idle=%8d\r\n", 
						ctid(),
						i,
						_space->m_container[i]->m_allpool->size(),
						_space->m_container[i]->m_idle->size());
					//计算总的分配的内存大小
					total_size += (_space->m_container[i]->m_allpool->size() * i) / 1024.0;
				}

				c_memory_statis::get_instance()->push(ctid(), total_size);

				//打印总内存
				c_znLog::_msg("thread=%8x, thread total=%.2f Kb, total=%.2f Kb\r\n",
					ctid(), total_size, c_memory_statis::get_instance()->size());

				
				tick->utick = time(0);
			}
#endif
			//===========================30秒输出一次内存块的实用情况================================

			//2.通过内存颗粒进行计算  如果传入参数bCalculate为false为不加入颗粒统计,只加预留字节buffer_reserved
			int32_t allocate_size = bCalculate ? calculate_size(nSize) : (nSize + buffer_reserved);

			//3.分配内存大于内存池定义的最大颗粒而不参与内存池分配
			if (allocate_size > max_granule)
			{
				assert(0);
				return calloc(1, allocate_size);
			}

			
			CMemory* pResult = NULL;
			Container* pContainer = (*_space)[allocate_size];

			//5.检查其他线程是否有删除情况, 有就回收到空闲内存区中
			if (!pContainer->m_allpool->empty()) 
			{
				pResult = pContainer->m_allpool->front();

				//调整位置
				pContainer->m_allpool->pop();
				pContainer->m_allpool->push(pResult);

				if (!pResult->IsDelete())  
				{
					//如果正在使用
					pResult = NULL;
				}
				else
				{	
					//如果其他线程删除的内存把释放线程ID设置为0
					pResult->m_nRefThreadID = 0;
				}
			}
			
			//6.如果没有回收到内存
			if (pResult == NULL) 
			{
				if (!pContainer->m_idle->empty())
				{
					//从空闲内存区中取一块内存
					pResult = pContainer->m_idle->front();
					pContainer->m_idle->pop();
				}
				else  
				{
					//如果还是没有可用内存则分配
					pResult = new CMemory(allocate_size);
					pContainer->m_allpool->push(pResult);
				/*	LOG4_INFO("NEW:" << allocate_size);
					if (pContainer->m_allpool->size() > 10000)
					{
						assert(0);
					}*/
				}
			}

			return pResult->buffer();
		}

		inline static void tls_free(void* T) //回收
		{
			//1.获取要释放内存大小
			int32_t _size = mem_size(T);

			//2.效验内存是否通过内存池分配
			int32_t* position = (int32_t*)&((const char*)T)[_size - buffer_reserved];

			//3.效验内存大小不是否有效
			if (*position != _size)
			{
				assert(0);
				return;
			}

			//4.指向内存对像
			++position;
			CMemory* memory = (CMemory*)((*(long*)position));

			memory->Release();
		}

	private:
		//释放/回收
		inline void Release()
		{
			//如果创建的线程与加收线程同一个直接回收
			if (ctid() == m_nCrtThreadID)  
			{
				//1.获取TLS内存对像,不存在则创建
				_tls_space* _space = TLS(_tls_space);
				(*_space)[m_allocate_size]->m_idle->push(this);
			}
			else
			{
				m_nRefThreadID = ctid();
			}
		}

	public:
		inline bool IsDelete(void)
		{
			return m_nRefThreadID != 0 && m_nRefThreadID != m_nCrtThreadID;
		}
	public:
		unsigned char*		m_pData;
		int32_t				m_allocate_size;
		thread_t			m_nCrtThreadID;				//创建时线程ID
		thread_t			m_nRefThreadID;				//删除时线程ID
	};	

	//对象内存池
	class CMPool
	{
	public:
#ifndef __android
		inline void * operator new(size_t size) { return CMemory::tls_malloc(size, false); }
		inline void operator delete (void * p) { CMemory::tls_free(p); }
#endif
	};

	//线程实例化
	template<class T> class TLS
	{
		typedef queue<T*>	Q;
	public:
		static T* get() 
		{
			T* v = NULL;
			if (!TlsSingelton<Q>::tlsInstance()->empty())
			{
				v =  TlsSingelton<Q>::tlsInstance()->front();
				TlsSingelton<Q>::tlsInstance()->pop();
			}

			return v == NULL ? new T : v;
		}

		static void recover(T* p)
		{
			TlsSingelton<Q>::tlsInstance()->push(p);
		}
	};

}

#endif //__TLS_POOLS__
