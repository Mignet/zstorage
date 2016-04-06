
#if !defined(__TLS_H__)
#define __TLS_H__

#ifdef _MSC_VER
	#include <wtypes.h>
	#include <malloc.h>
	typedef  unsigned long  tls_type;
	#define ctid() ::GetCurrentThreadId() 
	#define mem_size(x)  _msize(x)
#else
	#include <pthread.h>
	#include <malloc.h>
	typedef pthread_key_t tls_type;
	#define ctid() pthread_self()
	#define mem_size(x)  malloc_usable_size(x)
#endif

	//÷–∂œ”¶”√
#define Breakpoint(T) { while (1) { _sleep(1); } }



class TlsIndex
{    
public:
	TlsIndex()
	{
#ifdef _MSC_VER
		m_index = TlsAlloc();
#else
		pthread_key_create(&m_index, NULL);
#endif
	}
	~TlsIndex()
	{
#ifdef _MSC_VER
		::TlsFree(m_index);
#else
		pthread_key_delete(m_index);
#endif
		
	}
public:
	operator tls_type() const
	{
		return m_index;
	}
private:
	tls_type m_index;
};


template<typename TDerived> 
class TlsSingelton
{
	typedef TDerived _Derived;
	typedef TlsSingelton<TDerived> _Base;
 public:
	static _Derived* tlsInstance()
	{
		return tlsCreate();
	}
 protected:
	static _Derived* tlsCreate()
	{
		_Derived* derived = tlsGet();

		if ( derived )
			return derived;

		derived = new _Derived();
		//printf("=============tlsCreate(0x%08x)=================\r\n\r\n", derived);
	#ifdef _MSC_VER
		TlsSetValue(tlsIndex(), derived);
	#else
		pthread_setspecific(tlsIndex(), derived);
	#endif

		return derived;
	}
	static bool tlsDestroy()
	{
		_Derived* derived = tlsGet();

		if ( derived == NULL )
			return false;

		delete derived;
		
		return true;
	}
	static tls_type tlsIndex()
	{
		static TlsIndex s_tlsIndex;
		return s_tlsIndex;
	}
 private:
	static _Derived* tlsGet()
	{
	#ifdef _MSC_VER
		return reinterpret_cast<_Derived*>(::TlsGetValue(tlsIndex()));
	#else
		return reinterpret_cast<_Derived*>(pthread_getspecific(tlsIndex()));
	#endif
	}
	//noncopyable
private:
	TlsSingelton(const _Base&);
	TlsSingelton& operator=(const _Base&);
};

#define TLS(T)	TlsSingelton< T >::tlsInstance()		

#endif
