#ifndef _cmd_h
#define _cmd_h
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
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io);
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
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io);
	virtual void remove(const string& key)
	{
		leveldb::WriteOptions op;
		leveldb::WriteBatch bh;
		bh.Delete(key);
		c_base::write(op, bh);
	}
};


class _del
	: public c_base
	, public IAnalyzer
{
public:
	_del()
	{
		m_head.type = 0;
	}
	~_del() {}

	static void del(leveldb::Slice& key);
public:
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io);
	virtual void remove(const string& key, wisdom_IOStream& io);
};



class _exists
	: public c_base
	, public IAnalyzer
{
public:
	_exists()
	{
	}
	~_exists() {}
public:
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io);
};

class _recycle
	: public c_base
	, public IAnalyzer
{
public:
	_recycle()
	{
	}
	~_recycle() {}
public:
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io);
};


#endif

