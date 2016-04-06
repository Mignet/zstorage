#ifndef wisdom_ptr_h
#define wisdom_ptr_h
#include <assert.h>
#include "df_macro.h"

#define WISDOM_FREE(name, class_name)	\
class class_name##free {	\
public:	\
	inline static void free( name* p) {  if (p) free(p);} }; \
	typedef wisdom_ptr< name, class_name##free>	class_name;

#define WISDOM_PTR(name, class_name)	\
class class_name##free {	\
		public:	\
		inline static void free( name* p) {  if (p) delete p;} }; \
		typedef wisdom_ptr< name, class_name##free>	class_name;

#define WISDOM_RELEASE(name, class_name)	\
class class_name##free {	\
public:	\
	inline static void free( name* p) {  if (p) p->release();} }; \
	typedef wisdom_ptr< name, class_name##free>	class_name;

#define WISDOM_TPTR(name, class_name)	\
class class_name##free {	\
public:	\
	inline static void free( name* p) {  if (p) p->clear();} }; \
	typedef wisdom_ptr< name, class_name##free>	class_name;


template<class T>
class wf_delete
{
public:
	inline static void free(T* p) { if (p) delete p;}
};

class wf_free
{
public:
	inline static void free(void* p) { if (p) free(p);}
};

template<class T, class F>
class wisdom_ptr
	: public pool::CMPool
{
public:
	wisdom_ptr():m_ptr(0), m_ref(0) {}
	wisdom_ptr(T * p) :m_ptr(p) { ref_init();}

	virtual ~wisdom_ptr() { dispose(); }

	wisdom_ptr(const wisdom_ptr<T, F>& rhs)	
		:m_ptr(rhs.m_ptr), m_ref(rhs.m_ref)	
	{
		if (m_ref == 0) return;	//bug
		_atom_add_one(m_ref);	
	}

	inline wisdom_ptr<T, F> & operator = (const wisdom_ptr<T, F> & rhs)
	{
		if( this != &rhs )		
		{
			dispose();
			m_ptr = rhs.m_ptr;
			m_ref = rhs.m_ref;
			if (m_ref == 0) return *this; //bug
			_atom_add_one(m_ref);
		}

		return *this;	
	}

	inline wisdom_ptr<T, F> & operator = (T* ptr)
	{
		if( m_ptr != ptr )		
		{
			dispose();
			m_ptr = ptr;
			ref_init();
		}

		return *this;	
	}

	bool empty()
	{
		return m_ptr == NULL;
	}	

	inline T * operator -> () const	
	{		
		return m_ptr;	
	}

	inline operator T* () const 
 	{
		return m_ptr;
	}
protected:
	inline void ref_init() 
	{
		m_ref = new _atom;
		_atom_init(*m_ref);
		_atom_add_one(m_ref);
	}
private:
	inline void dispose()
	{	
		if (m_ref == 0)
			return;

		if(_atom_sub_one(m_ref) == 0 )
		{
			delete m_ref;
			F::free(m_ptr);
		}
	}
protected:
	T*		m_ptr;
	_atom	*m_ref;
};


#endif
