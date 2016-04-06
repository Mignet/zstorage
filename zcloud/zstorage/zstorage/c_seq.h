#ifndef c_seq_h
#define c_seq_h
#pragma once
#include "c_base.h"

/*
key			count@begin@end =====>容器大小 开始索引，结束索引
key+index	value			=====>内容存储
*/

class c_seq
	: public c_base
{
public:
	c_seq(const char* key);
	~c_seq();
	void push(char* data, int n, wisdom_IOStream& io, uint32 idelay);
	void index_(wisdom_IOStream& io);
	void size(wisdom_IOStream& io);
	void remove(wisdom_IOStream& io);
	void seek(uint32 keyId, int index, wisdom_IOStream& io);
	void delay(uint32 idelay, wisdom_IOStream& io);
	void lookdelay(wisdom_IOStream& io);
	void query(uint32 datetime, wisdom_IOStream& io);
	bool is_release();
private:
	bool load_value(const string& value, _zmsg_head* head);
	bool init();
	bool is_clean();
	void seq_set(leveldb::WriteBatch& bh, leveldb::Slice& value);
	void seq_update(leveldb::WriteBatch& bh);
	bool seq_get(uint32 keyId, string& value);
	void seq_del(uint32 keyId, leveldb::WriteBatch& bh);
	bool seq_exists(uint32 keyId);
	uint32 seq_begin_index();
	int dichotomy(uint32 datetime, uint32 begin, uint32 end, int& diff, wisdom_IOStream& io);
private:
	rw_lock			m_lock;
	_SEQ_HEAD		m_seq_head;
	bool			m_init;
	string			m_key;
	uint32			m_effective;
};



WISDOM_PTR(c_seq, wisdom_seq);

class c_seq_admin
	: public c_singleton<c_seq_admin>
{
	typedef map<string, wisdom_seq>	M;
public:
	wisdom_seq find(const char* key)
	{
		c_rlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
			return pos->second;

		return NULL;
	}

	wisdom_seq find_is_create(const char* key)
	{
		wisdom_seq seq = find(key);
		if (seq != NULL)
			return seq;

		return create(key);
	}

	wisdom_seq create(const char* key)
	{
		c_wlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
		{
			return pos->second;
		}

		wisdom_seq l = new c_seq(key);
		m_list.insert(make_pair(key, l));
		return l;
	}

	void check()
	{
		c_wlock lock(&m_lock);
		for (M::iterator pos = m_list.begin(); pos != m_list.end();)
		{
			if (pos->second->is_release())
			{
				m_list.erase(pos++);
			}
			else
			{
				++pos;
			}
		}
	}
private:
	rw_lock	m_lock;
	M		m_list;
};

#endif


