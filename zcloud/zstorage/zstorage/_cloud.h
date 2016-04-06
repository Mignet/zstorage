
#ifndef _cloud_h
#define _cloud_h

#pragma once

#include "c_base.h"
#include "c_cloud.h"
class _cloud
	: public IAnalyzer
{
public:
	_cloud() 
	{
		/*
		PUSH
		LIMIT
		ONLINE
		OFFLINE
		QUERY
		SIZE
		READ
		*/
	}
	~_cloud() {}

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

		wisdom_cloud l = c_cloud_admin::get_instance()->find_is_create(key.data());
		/*
		PUSH
		LIMIT
		ONLINE
		OFFLINE
		QUERY
		SIZE
		INDEX
		READ
		*/
		if (strcmp(ptr, "PUSH") == 0)
		{
			push(param, index, io, l);
		}
		else if (strcmp(ptr, "LIMIT") == 0)
		{
			limit(param, index, io, l);
		}
		else if (strcmp(ptr, "ONLINE") == 0)
		{
			online(param, index, io, l);
		}
		else if (strcmp(ptr, "QUERY") == 0)
		{
			query(param, index, io, l);
		}
		else if (strcmp(ptr, "SIZE") == 0)
		{
			size(param, index, io, l);
		}
		else if (strcmp(ptr, "READ") == 0)
		{
			read(param, index, io, l);
		}
		else if (strcmp(ptr, "INDEX") == 0)
		{
			index_(param, index, io, l);
		}
		else
		{
			ZERROR_RESULT(__tos("keyword Can't identify. keyword:" << ptr));
		}
	}

	void push(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->push(param, index, io);
	}

	void limit(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->limit(param, index, io);
	}

	void online(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->online(param, index, io);
	}

	void query(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->query(param, index, io);
	}

	void size(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->size(param, index, io);
	}

	void read(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->read(param, index, io);
	}

	void index_(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_cloud& l)
	{
		l->index_(param, index, io);
	}
};



#endif
