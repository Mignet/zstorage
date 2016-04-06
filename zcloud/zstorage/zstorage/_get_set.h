#ifndef _get_set_h
#define _get_set_h
#pragma once

#include "c_base.h"

class _get
	: public c_base
	, public IAnalyzer
{
public:
	_get() 
	{
		m_head.type = T_GET_SET;
	}
	~_get() {}
public:
	virtual void exec(wisdom_param& param, int index, leveldb::Slice& key, wisdom_IOStream& io)
	{
		string value;
		if (!c_base::get(key, value) || value.length() <= sizeof(_zmsg_head))
		{
			io->push(ZRESULT_ERROR);
			io->push("value bad.");
			return;
		}
		
		io->push(ZRESULT_OK);
		
		
		io->push((char*)value.c_str() + sizeof(_zmsg_head), value.length() - sizeof(_zmsg_head));
	}
};



class _set
	: public c_base
	, public IAnalyzer
{
public:
	_set() 
	{
		m_head.type = T_GET_SET;
	}
	~_set() {}
public:
	virtual void exec(wisdom_param& param, int index, leveldb::Slice& key, wisdom_IOStream& io)
	{
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			io->push(ZRESULT_ERROR);
			io->push("value not exists.");
			return;
		}

		uint32 timeout = param->to_integer(++index);

		leveldb::Slice value(ptr, len);
		if (!c_base::put(key, value, timeout))
		{
			io->push(ZRESULT_ERROR);
			io->push("db error.");
			return;
		}

		io->push(ZRESULT_OK);
	}
};


#endif

