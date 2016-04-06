#pragma once
#include "c_base.h"
#include "c_mlist.h"
class _mlist
	: public IAnalyzer
{
public:
	_mlist() 
	{
		/*
		PUSH keyId value
		SIZE
		SEEK keyId count
		EXPIRE
		DEL
		TIME
		*/
	}
	~_mlist() {}

private:
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
	{
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
		io->push(ZRESULT_ERROR);
		io->push("value not exists.");
		return;
		}

		ptr = strupr(ptr);
		ptr[len] = 0;

		wisdom_mlist l = c_mlist_admin::get_instance()->find_is_create(key.data());

		if (strcmp(ptr, "PUSH") == 0)
		{
			push(param, index, io, l);
		}
		else if (strcmp(ptr, "SIZE") == 0)
		{
			size(param, index, io, l);
		}
		else if (strcmp(ptr, "SEEK") == 0)
		{
			seek(param, index, io, l);
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
		else
		{
			ZERROR_RESULT(__tos("keyword Can't identify. keyword:" << ptr));
		}
	}

	virtual void remove(const string& key, wisdom_IOStream& io)
	{
		wisdom_mlist l = c_mlist_admin::get_instance()->find_is_create(key.data());
		l->remove(io);
	}
private:
	void push(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		string keyId = param->to_string(++index);
		if (keyId.empty())
		{
			io->push(ZRESULT_ERROR);
			io->push("keyid is null.");
			return;
		}
		char* ptr = NULL;
		int len = 0;
		if (!param->get(++index, &ptr, len))
		{
			io->push(ZRESULT_ERROR);
			io->push("value not exists.");
			return;
		}

		l->push(keyId, ptr, len, io);

	}

	void size(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		l->size(io);
	}
	void seek(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		string keyId = param->to_string(++index);
		if (keyId.empty())
		{
			io->push(ZRESULT_ERROR);
			io->push("keyid is null.");
			return;
		}

		int count = param->to_integer(++index);
		l->seek(keyId, count, io);
	}

	void expire(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		uint32 ms = param->to_integer(++index);
		l->expire(ms, io);
	}

	void del(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		l->remove(io);
	}

	void time_(wisdom_param& param, int index, wisdom_IOStream& io, wisdom_mlist& l)
	{
		l->time_(io);
	}
};

