
#ifndef CCOUNTEDPTR_H
#define CCOUNTEDPTR_H
#include <assert.h>
#include "df_macro.h"

template<class T>
class autoptr
{
public:
	autoptr():m_Ptr(0), m_RefCount(NULL) {}
	autoptr( T * p, void (*_dispose)(T*) = 0) 
		:m_Ptr( p ), m_dispose(_dispose)
	{
		m_RefCount = new _atom;
		_atom_init(*m_RefCount);
		_atom_add_one(m_RefCount);
	}
	
	autoptr(const autoptr<T>& rhs)		
		:m_Ptr(rhs.m_Ptr), m_dispose(rhs.m_dispose), m_RefCount(rhs.m_RefCount)
	{
		_atom_add_one(m_RefCount);
	}	
	
	~autoptr()	
	{
		dispose();	
	}	
	
	autoptr<T> & operator = (const autoptr<T> & rhs)
	{
		if( this != &rhs )		
		{			
			dispose();			
			m_Ptr = rhs.m_Ptr;	
			m_dispose = rhs.m_dispose;
			m_RefCount = rhs.m_RefCount;			
			_atom_add_one(m_RefCount);		
		}
			
		return *this;	
	}

	bool empty()
	{
		return m_Ptr == NULL;
	}
	
	T & operator * () const	
	{		
		return * m_Ptr;	
	}	
	
	T * operator -> () const	
	{		
		return m_Ptr;	
	}	
	
	T * get() const	
	{
		return m_Ptr;	
	}
	
	T * ptr() const	
	{
		return m_Ptr;	
	}
	
private:		
	void dispose()	
	{		
		if (m_RefCount == NULL)
			return;

		if(_atom_sub_one(m_RefCount) == 0 )		
		{			
			delete m_RefCount;
			if (m_Ptr)		
			{
				if (m_dispose)
					m_dispose(m_Ptr);
				else
					delete m_Ptr;	
			}
		}	
	}

private:
	T *		m_Ptr;	
	_atom	*m_RefCount;
	//析构对象函数
	void	(*m_dispose)(T*);
};
	
#endif

