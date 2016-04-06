
#ifndef zpacket_h
#define zpacket_h
#include "wisdom_ptr.h"

template <class T>
class zpacket
	: public CMPool
{
public:
	zpacket(): m_len(0), m_ptr(NULL) {}

	zpacket(int len):m_len(len), m_ptr(NULL){
		if (m_len > 0)
		{
			m_ptr = (char*)malloc(m_len);
			memset(m_ptr, 0, m_len);
		}
	}

	zpacket(char* ptr, int len):m_len(len), m_ptr(NULL){
		if (m_len > 0)
		{
			m_ptr = (char*)malloc(m_len);
			memcpy(m_ptr, ptr, m_len);
		}
	}

	~zpacket() {
		if (m_ptr)
			free(m_ptr);
	}
public:
	T		m_object;
	int		m_len;
	char*	m_ptr;
};

//WISDOM_PTR(zpacket< int >, wisdom_zpacket_int);
//
//WISDOM_PTR(zpacket< int >, wisdom_zpacket);
//static wisdom_zpacket create_zpacket(char* buf, int len)
//{
//	wisdom_zpacket zpk = new zpacket< int >(buf, len);
//	return zpk;
//}


#endif
