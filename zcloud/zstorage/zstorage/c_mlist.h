
#ifndef c_mlist_h
#define c_mlist_h

#pragma once

#include "c_base.h"

/*
key [len|skey][len|skey][len|skey][len|skey][len|skey][len|skey]===>len 1BYTE skey is number
key+skey data
key+skey data
*/

class c_mlist
	: public c_base
{
public:
	c_mlist(const char* key);
	~c_mlist();
public:
	void push(const string& keyId, char* data, int n, wisdom_IOStream& io);
	void seek(const string& keyId, uint32 count, wisdom_IOStream& io);
	void size(wisdom_IOStream& io);
	void remove(wisdom_IOStream& io);
	void expire(uint32 ms, wisdom_IOStream& io);
	void time_(wisdom_IOStream& io);
	bool is_release();
private:
	bool load_value(const string& value, _zmsg_head* head);
	bool init();

	void list_del_key(leveldb::WriteBatch& bh, const string& keyId);
	void list_delete(leveldb::WriteBatch& bh);
	void list_update(leveldb::WriteBatch& bh);
	void list_push(leveldb::WriteBatch& bh, const string& keyId, leveldb::Slice& value);
	bool list_get(const string& keyId, string& value);
private:
	rw_lock			m_lock;
	bool			m_init;
	string			m_key;
	list<string>	m_list;
	uint32			m_effective;
	uint16			m_max_list;
};

WISDOM_PTR(c_mlist, wisdom_mlist);



class c_mlist_admin
	: public c_singleton<c_mlist_admin>
{
	typedef map<string, wisdom_mlist>	M;
public:
	wisdom_mlist find(const char* key)
	{
		c_rlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
			return pos->second;

		return NULL;
	}

	wisdom_mlist find_is_create(const char* key)
	{
		c_wlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
		{
			return pos->second;
		}

		wisdom_mlist l = new c_mlist(key);
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
