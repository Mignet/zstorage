#pragma once

#ifndef _REF_H_
#define _REF_H_

#include "df_macro.h"

//���ü�����
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

