#pragma once

#ifndef _REF_H_
#define _REF_H_

#include "df_macro.h"

//引用计数器
class CRef
{
public:
	CRef(void);
	~CRef(void);
public:
	int32_t add();
	void dec();
private:
	virtual void on_release() = 0;
private:
	_atom		m_ref;
};

#endif

