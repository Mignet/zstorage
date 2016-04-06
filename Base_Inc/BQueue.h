#if !defined(_BQUEUE_H)
#define _BQUEUE_H

#pragma once

#include <stdlib.h>
#include <list>
#include <assert.h>
#include "wisdom_ptr.h"
using namespace std;

#ifndef min
	#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

class CBuffer
{
public:
	CBuffer(char* buffer, int length, void (*_free)(void*) = free) 
		: m_buffer(buffer)
		, m_length(length)
		, m_free(_free)
		, m_tick(time(0)){}

	~CBuffer() {m_free(m_buffer);}
	char*	Buffer() {return m_buffer;}
	int	Length() {return m_length;}
	void	tick() {m_tick = time(0);}
	time_t	get_tick() {return m_tick;}	
private:
	void	(*m_free)(void*);
	char*	m_buffer;
	int	m_length;
	time_t	m_tick;
};



class BQueue
{
public:
	class CBuffer_free
	{
	public:
		static void free( CBuffer* p) {  if (p) delete p;}
	};

	typedef wisdom_ptr< CBuffer, CBuffer_free>	wisdom_CBuffer;


	BQueue(wisdom_CBuffer& buffer)
		: m_buffer(buffer)
		, _p(m_buffer->Length())
		, _b(0)
		, _l(m_buffer->Length())
		, _timeout(time(0))
	{
		m_buffer->tick();
	}

	char* Buffer() 
	{
		if (_p <= _b)	//Ô½½ç
			return NULL;
		return m_buffer->Buffer() + _b;
	}
	
	int Len() { return _p - _b; }

	void Advance(int n) { _b += n; }

	int Position() {return _b;};

public:
	time_t				_timeout;

private:
	wisdom_CBuffer		m_buffer;
	int					_p;
	int					_b;
	int					_l;
	
};


#endif //_BQUEUE_H
