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

	const uint16_t	buffer_mem_size		= 512;			//��С�ڴ浥λ
	const int16_t	buffer_reserved		= sizeof(uint32_t) + sizeof(void*); //Ԥ���ֽ���,ǰ4λʶ������汣�����ָ��
	const uint32_t	max_granule			= 0X21000;		//128K
	const uint32_t	max_allocate_size	= 0X20000;		//��������ڴ�

	inline int32_t calculate_size(int32_t nSize)
	{
		return nSize + 
			(buffer_mem_size - (nSize % buffer_mem_size)) + buffer_reserved;
	}

	//�ڴ�ͳ����
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

			//1.�����ڴ��С
			m_pData = (unsigned char*)calloc(1, nSize);

			//2.��ȡʵ���ڴ��С
			int32_t _size = mem_size(m_pData);

			//3.д���ڴ��ʾ
			int32_t* position = (int32_t*)&m_pData[_size - buffer_reserved];
			//������ǰ4λΪ�ڴ�ʵ�ʴ�С
			/*
			(*position) = _size;
			position++;
			//�����ֺ�4λΪ���ڶ���ָ��  TODO:64BIT ��Ҫ�޸�
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
		inline static void* tls_malloc(int32_t nSize, bool bCalculate = true)  //�����ڴ�
		{
			//1.��ȡTLS�ڴ����,�������򴴽�
			_tls_space* _space = TLS(_tls_space);
#ifdef _POOL_LOG
			_tick* tick = TLS(_tick);

			//===========================������߳��ڴ��ʵ�����================================

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
					//�����ܵķ�����ڴ��С
					total_size += (_space->m_container[i]->m_allpool->size() * i) / 1024.0;
				}

				c_memory_statis::get_instance()->push(ctid(), total_size);

				//��ӡ���ڴ�
				c_znLog::_msg("thread=%8x, thread total=%.2f Kb, total=%.2f Kb\r\n",
					ctid(), total_size, c_memory_statis::get_instance()->size());

				
				tick->utick = time(0);
			}
#endif
			//===========================30�����һ���ڴ���ʵ�����================================

			//2.ͨ���ڴ�������м���  ����������bCalculateΪfalseΪ���������ͳ��,ֻ��Ԥ���ֽ�buffer_reserved
			int32_t allocate_size = bCalculate ? calculate_size(nSize) : (nSize + buffer_reserved);

			//3.�����ڴ�����ڴ�ض�������������������ڴ�ط���
			if (allocate_size > max_granule)
			{
				assert(0);
				return calloc(1, allocate_size);
			}

			
			CMemory* pResult = NULL;
			Container* pContainer = (*_space)[allocate_size];

			//5.��������߳��Ƿ���ɾ�����, �оͻ��յ������ڴ�����
			if (!pContainer->m_allpool->empty()) 
			{
				pResult = pContainer->m_allpool->front();

				//����λ��
				pContainer->m_allpool->pop();
				pContainer->m_allpool->push(pResult);

				if (!pResult->IsDelete())  
				{
					//�������ʹ��
					pResult = NULL;
				}
				else
				{	
					//��������߳�ɾ�����ڴ���ͷ��߳�ID����Ϊ0
					pResult->m_nRefThreadID = 0;
				}
			}
			
			//6.���û�л��յ��ڴ�
			if (pResult == NULL) 
			{
				if (!pContainer->m_idle->empty())
				{
					//�ӿ����ڴ�����ȡһ���ڴ�
					pResult = pContainer->m_idle->front();
					pContainer->m_idle->pop();
				}
				else  
				{
					//�������û�п����ڴ������
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

		inline static void tls_free(void* T) //����
		{
			//1.��ȡҪ�ͷ��ڴ��С
			int32_t _size = mem_size(T);

			//2.Ч���ڴ��Ƿ�ͨ���ڴ�ط���
			int32_t* position = (int32_t*)&((const char*)T)[_size - buffer_reserved];

			//3.Ч���ڴ��С���Ƿ���Ч
			if (*position != _size)
			{
				assert(0);
				return;
			}

			//4.ָ���ڴ����
			++position;
			CMemory* memory = (CMemory*)((*(long*)position));

			memory->Release();
		}

	private:
		//�ͷ�/����
		inline void Release()
		{
			//����������߳�������߳�ͬһ��ֱ�ӻ���
			if (ctid() == m_nCrtThreadID)  
			{
				//1.��ȡTLS�ڴ����,�������򴴽�
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
		thread_t			m_nCrtThreadID;				//����ʱ�߳�ID
		thread_t			m_nRefThreadID;				//ɾ��ʱ�߳�ID
	};	

	//�����ڴ��
	class CMPool
	{
	public:
#ifndef __android
		inline void * operator new(size_t size) { return CMemory::tls_malloc(size, false); }
		inline void operator delete (void * p) { CMemory::tls_free(p); }
#endif
	};

	//�߳�ʵ����
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
