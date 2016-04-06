#pragma once
#include "stdafx.h"
#include "c_basis.h"

class evthread
	: public c_thread
{
public:
	evthread(void) :m_reading(false) {}
	~evthread(void) {}
public:
	virtual void Execute();
public:
	bool	m_reading;
};
