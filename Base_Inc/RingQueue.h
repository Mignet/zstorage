#if !defined(_RINGQUEUE_H)
#define _RINGQUEUE_H

#pragma once

#include <stdlib.h>
#include <list>
#include <assert.h>

#include "tls_pools.h"
using namespace pool;

using namespace std;

#ifndef min
	#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

//class T  as unsigned char[]
template <int T> 
class CDataBlock
	//: public CMPool
{
public:	//内存池管理
#ifndef __android
	void * operator new(size_t size) { return CMemory::tls_malloc(size, false); }
	void operator delete (void * p) { CMemory::tls_free(p); }
#endif
public:
	CDataBlock() : _l(T) , _p(0) , _b(0) {}
public:
	//追加数据
	bool Append(void* v, int n)	{ 

		if (_p + n > _l) return false;

		memcpy(&_T[_p], v, n); 	_p += n;

		return true;
	}

	//开始指针移位
	void Advance(int n) { _b += n; }
	//重置
	void Reset(void) {	_p = 0; _b = 0; }
	//获取可用内存
	unsigned char* Buffer() { return &_T[_b]; }
	//可能内存长度
	int Len() { return _p - _b; }

private:
	unsigned char	_T[T];
	int				_p;
	int				_b;
	int				_l;
};

template <int T>  
class CRingQueue
	: public CMPool
{
	typedef list<CDataBlock<T>*> Q;
public:
	CRingQueue() : m_size(0) {}
	~CRingQueue() 
	{
		while (!m_q.empty())
		{
			delete m_q.front();
			m_q.pop_front();
		}
	}
public:
	void clear()
	{
		while (!m_q.empty())
		{
			delete m_q.front();
			m_q.pop_front();
		}
		m_size = 0;
	}

	inline void Append(const char* str){
		Append((void*)str, strlen(str));
	}

	inline void Append(void* pData, int nLen)
	{
		if (nLen <= 0)
			return;

		if (nLen > T)
		{

			int nLength = min(T, nLen);
	
			Append(pData , nLength);

			Append( (void*) & ( ((unsigned char*)pData)[nLength] ), nLen - nLength);

			return;
		}

		bool bState = true;
		if (!m_q.empty())
		{
			bState = m_q.front()->Append(pData, nLen);
		}

		if (m_q.empty() || !bState)  //如果环形队列为空,或第一块内容压入失败,需要申请新的内存
		{
			m_q.push_front(new CDataBlock<T>());
			if (!m_q.front()->Append(pData, nLen))
				assert(0);
		}

		m_size += nLen;
	}

	inline int Attach(unsigned char** pBuffer)
	{
		if (m_q.back()->Len() == 0) 
			m_q.pop_back();

		*pBuffer = m_q.back()->Buffer();
		return m_q.back()->Len();
	}

	void Attach(unsigned char** pBuffer, int nLen)
	{
		//1.检查长度是否合法
		if (nLen > T || nLen > m_size)
		{
			*pBuffer = NULL;
			return;
		}


		//2.长度是否在一块用满足
		if (nLen > m_q.back()->Len())
		{
			list<CDataBlock<T>*> t;
			m_temp.Reset();
			do
			{
				//初始化

				int n = min(m_q.back()->Len(), nLen);
				m_temp.Append(m_q.back()->Buffer(), n);
				nLen -= n;

				if (nLen == 0)
					break;

				t.push_back(m_q.back());
				m_q.pop_back();

			} while (true);

			while (!t.empty())
			{
				m_q.push_back(t.front());
				t.pop_front();
			}

			*pBuffer = m_temp.Buffer();
		}
		else
		{
			//满足直接返回
			*pBuffer = m_q.back()->Buffer();
		}
	}

	inline void Attach(unsigned char* pBuffer, int& nLen)
	{
		if (nLen > Count())
		{
			return;
		}

		Q temp;
		for (typename Q::iterator iter = m_q.begin(); 
			iter != m_q.end(); ++iter)
		{
			temp.push_front(*iter);
		}

		int pos = 0;
		for (typename Q::iterator iter = temp.begin(); 
			iter != temp.end(); ++iter)
		{
			memcpy(&pBuffer[pos], (*iter)->Buffer(), (*iter)->Len());
			pos += (*iter)->Len();
		}

		nLen = pos;
	}

	int GetBuffer(unsigned char*pBuffer, int nLen)
	{
		if (nLen > m_size)
			return -1;

		int nPos = 0;
		unsigned char* buffer = NULL;
		while (nPos < nLen)
		{
			int n = Attach(&buffer);

			int m = min(nLen - nPos, n);

			memcpy(&pBuffer[nPos], buffer, m);

			Advance(m);
			nPos += m;
		}

		return nPos;
	}

	inline void Advance(int nLen)
	{
		if (nLen > m_size)
		{
			assert(0);
			return;
		}

		m_size -= nLen;

		while (nLen)
		{

			int n = min(m_q.back()->Len(), nLen);

			m_q.back()->Advance(n);

			nLen -= n;

			//如果没有可取内存,移除环形队列
			if (m_q.back()->Len() == 0)
			{
				delete m_q.back();
				m_q.pop_back();
			}
		}
	}

	int Count()
	{
		return m_size;
	}

private:
	Q				m_q;
	int				m_size;
	CDataBlock<T>	m_temp;
};


#define T_RINGQUEUE(n)	TlsSingelton< CRingQueue< n > >::tlsInstance()

#endif //_RINGQUEUE_H
