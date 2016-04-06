
#ifndef _list_h
#define _list_h

#pragma once

#include "c_base.h"
#include "c_list.h"

class _list
	: public IAnalyzer
{
public:
	_list() 
	{
		/*
		PUSH
		POP
		SIZE
		LIMIT
		EXPIRE
		DEL
		TIME
		UPDATE
		SEEK  LIMIT  
		*/
	}
	~_list() {}
private:
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

		wisdom_list l = c_list_admin::get_instance()->find_is_create(key.data());

		if (strcmp(ptr, "PUSH") == 0)
		{
			push(param, index, io, l);
		}
		else if (strcmp(ptr, "POP") == 0)
		{
			pop(param, index, io, l);
		}
		else if (strcmp(ptr, "SIZE") == 0)
		{
			size(param, index, io, l);
		}
		else if (strcmp(ptr, "LIMIT") == 0)
		{
			limit(param, index, io, l);
		}
		else if (strcmp(ptr, "EXPIRE") == 0)
		{
			expire(param, index, io, l);
		}
		else if (strcmp(ptr, "DEL") == 0)
		{
			del(param, index, io, l);
		}
		else if (strcmp(ptr, "TIME") == 0)
		{
			time_(param, index, io, l);
		}
		else if (strcmp(ptr, "UPDATE") == 0)
		{
			update(param, index, io, l);
		}
		else if (strcmp(ptr, "SEEK") == 0)
		{
			seek(param, index, io, l);
		}
		else
		{
			ZERROR_RESULT(__tos("keyword Can't identify. keyword:" << ptr));
		}

	}

	virtual void remove(const string& key, wisdom_IOStream& io)
	{
		wisdom_list l = c_list_admin::get_instance()->find_is_create(key.data());
		l->remove(io);
	}
private:
	void push(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			ZERROR_RESULT("list create error.");
		}

		l->push(ptr, len, io);

	}
	void pop(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		l->pop(io);
	}
	void size(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		l->size(io);
	}
	void limit(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		int count = param->to_integer(++index);
		l->limit(count, io);
	}

	void expire(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		uint32 ms = param->to_integer(++index);
		l->expire(ms, io);
	}

	void del(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		l->remove(io);
	}

	void time_(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		l->time_(io);
	}

	void update(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		l->update(param, index, io);
	}

	void seek(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_list& l)
	{
		int iseek = param->to_integer(++index);

		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			ZERROR_RESULT("command error.");
		}

		ptr = strupr(ptr);
		ptr[len] = 0;
		if (strcmp(ptr, "LIMIT") != 0)
		{
			ZERROR_RESULT("command error.");
		}

		uint32 ilimit = param->to_integer(++index);

		l->seek(iseek, ilimit, io);
	}
};


#endif
