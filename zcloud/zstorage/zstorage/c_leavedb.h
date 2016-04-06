
#ifndef c_leavedb_h
#define c_leavedb_h

#pragma once

#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/write_batch.h"
#include "leveldb/options.h"
#include <iostream>
using namespace std;

class Iter
{
public:
	virtual bool read(const leveldb::Slice& key, const leveldb::Slice& value) = 0;
};

class _LEAVEDB
	: public c_singleton<_LEAVEDB>
{
public:
	bool open(const string& db);
	leveldb::DB* get() const { return m_db; }
private:
	leveldb::DB*		m_db;
};

class c_leavedb
{
public:
	c_leavedb();
	~c_leavedb();
public:
	bool get(const leveldb::ReadOptions& readOptions, leveldb::Slice& key, string& value);
	bool put(const leveldb::WriteOptions& writeOptions, leveldb::Slice& key, leveldb::Slice& value);
	bool get(const leveldb::Slice& key, string& value);
	bool put(const leveldb::Slice& key, leveldb::Slice& value);
	bool write(leveldb::WriteOptions& writeOptions, leveldb::WriteBatch& batch);
	void traverse(Iter* it, IConnect* con = NULL);
private:
	leveldb::DB*		m_db;
};



class _tbuffer
{
public:
	_tbuffer(){
		len = MAX_VALUE_LEN;
		buf = (char*)malloc(MAX_VALUE_LEN);
		pos = 0;
	}
	~_tbuffer() {
		free(buf);
	}
	char*	buf;
	int		len;
	int		pos;
};


#endif 


