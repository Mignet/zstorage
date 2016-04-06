
#ifndef _seq_h
#define _seq_h
#pragma once
#include "c_base.h"
#include "c_seq.h"


class _seq
	: public IAnalyzer
{
public:
	_seq() 
	{
		/*
		PUSH	
		INDEX	
		SIZE
		SEEK
		DELAY
		LOOKDELAY
		QUERY
		UQUERY
		*/
	}
	~_seq() {}

	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
	{
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			ZERROR_RESULT("value not exists.");
		}

		ptr = strupr(ptr);
		ptr[len] = 0;

		wisdom_seq l = c_seq_admin::get_instance()->find_is_create(key.data());

		if (strcmp(ptr, "PUSH") == 0)
		{
			push(param, index, io, l);
		}
		else if (strcmp(ptr, "INDEX") == 0)
		{
			index_(param, index, io, l);
		}
		else if (strcmp(ptr, "SIZE") == 0)
		{
			size(param, index, io, l);
		}
		else if (strcmp(ptr, "SEEK") == 0)
		{
			seek(param, index, io, l);
		}
		else if (strcmp(ptr, "DELAY") == 0)
		{
			delay(param, index, io, l);
		}
		else if (strcmp(ptr, "LOOKDELAY") == 0)
		{
			lookdelay(param, index, io, l);
		}
		else if (strcmp(ptr, "QUERY") == 0)
		{
			query(param, index, io, l);
		}
		else if (strcmp(ptr, "UQUERY") == 0)
		{
			query(param, index, io, l);
		}
		else
		{
			ZERROR_RESULT(__tos("keyword Can't identify. keyword:" << ptr));
		}

	}


	void push(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			ZERROR_RESULT("list create error.");
		}

		uint32 delay = param->to_integer(++index);
		delay = delay * 60;

		l->push(ptr, len, io, delay);
	}

	void size(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		l->size(io);
	}

	void index_(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		l->index_(io);
	}

	void seek(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		int keyId = param->to_integer(++index);
		int idx = param->to_integer(++index);

		l->seek(keyId, idx, io);
	}

	void delay(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		uint32 idelay = param->to_integer(++index);

		l->delay(idelay, io);
	}

	void lookdelay(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		l->lookdelay(io);
	}

	void query(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		string str_time = param->to_string(++index);
		c_time t(str_time.c_str());

		l->query(t.unix_time(), io);
	}

	void uquery(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		uint32 dt = param->to_integer(++index);

		l->query(dt, io);
	}

	void remove(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_seq& l)
	{
		l->remove(io);
	}
};


#endif


