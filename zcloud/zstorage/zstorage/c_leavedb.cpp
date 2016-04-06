#include "stdafx.h"
#include "c_leavedb.h"
#include "c_server.h"

c_leavedb::c_leavedb()
{
	m_db = _LEAVEDB::get_instance()->get();
}


c_leavedb::~c_leavedb()
{
}


bool c_leavedb::write(leveldb::WriteOptions& writeOptions, leveldb::WriteBatch& batch)
{
	return m_db->Write(writeOptions, &batch).ok();
}

bool c_leavedb::get(const leveldb::ReadOptions& readOptions, leveldb::Slice& key, string& value)
{
	return m_db->Get(readOptions, key, &value).ok();
}

bool c_leavedb::get(const leveldb::Slice& key, string& value)
{
	leveldb::ReadOptions readOptions;
	readOptions.fill_cache = true;
	return m_db->Get(readOptions, key, &value).ok();
}

bool c_leavedb::put(const leveldb::WriteOptions& op, leveldb::Slice& key, leveldb::Slice& value)
{
	return m_db->Put(op, key, value).ok();
}

bool c_leavedb::put(const leveldb::Slice& key, leveldb::Slice& value)
{
	leveldb::WriteOptions op;
	return m_db->Put(op, key, value).ok();
}

void c_leavedb::traverse(Iter* it, IConnect* con)
{
	leveldb::Iterator* iter = m_db->NewIterator(leveldb::ReadOptions());
	int64_t counts = 0;
	int64_t recover_count = 0;
	for (iter->SeekToFirst(); iter->Valid(); iter->Next())
	{
		if (++counts % 10000 == 0)
		{
			string str = __tos("ergodic counts:" << counts << " time stamp:" << get_ms() << "\r\n");
			if (con)
				con->send((uint8*)str.c_str(), str.length());
		}
			
		if (!it->read(iter->key(), iter->value()))
		{
			//Êä³ö
			string str = __tos("recover key:" << iter->key().data() << " recover count" << ++recover_count <<"\r\n");
			if (con)
				con->send((uint8*)str.c_str(), str.length());
		}
	}
	delete iter;
}

bool _LEAVEDB::open(const string& db)
{
	leveldb::Options op;
	op.create_if_missing = true;
	op.write_buffer_size = c_server::get_instance()->write_buffer_size() * 1024 * 1024;
	op.block_cache = leveldb::NewLRUCache(c_server::get_instance()->cache_size() * 1024 * 1024);  // 100MB cache
	leveldb::Status s = leveldb::DB::Open(op, db, &m_db);
	return s.ok();
}
