
#ifndef c_list_h
#define c_list_h

#pragma once

#include "c_base.h"

/*
key [1][2][3][4][5][6][7][8]===>2BYTE
key1 data
key2 data
*/
class c_list
	: public c_base
{
public:
	c_list(const char* key);
	~c_list();
	void push(char* data, int n, wisdom_IOStream& io);
	void pop(wisdom_IOStream& io);
	void limit(uint32 count, wisdom_IOStream& io);
	void size(wisdom_IOStream& io);
	void expire(uint32 ms, wisdom_IOStream& io);
	void remove(wisdom_IOStream& io);
	void time_(wisdom_IOStream& io);
	void update(wisdom_param& param, int index, wisdom_IOStream& io);
	void seek(int iseek, uint32 ilimit, wisdom_IOStream& io);
	bool is_release();
private:
	bool load_value(const string& value, _zmsg_head* head);
	bool init();
	uint16 list_get_keyId();
	void list_del_key(leveldb::WriteBatch& bh, uint16 keyId);
	void list_update(leveldb::WriteBatch& bh);
	void list_delete(leveldb::WriteBatch& bh);
	void list_push(leveldb::WriteBatch& bh, leveldb::Slice& value);
	bool list_get(uint16 keyId, string& value);
private:
	rw_lock			m_lock;
	bool			m_init;
	string			m_key;
	list<uint16>	m_list;
	uint32			m_effective;
	uint16			m_max_list;
};

WISDOM_PTR(c_list, wisdom_list);

class c_list_admin
	: public c_singleton<c_list_admin>
{
	typedef map<string, wisdom_list>	M;
public:
	wisdom_list find(const char* key)
	{
		c_rlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
			return pos->second;

		return NULL;
	}

	wisdom_list find_is_create(const char* key)
	{
		c_wlock lock(&m_lock);
		M::iterator pos = m_list.find(key);
		if (pos != m_list.end())
		{
			return pos->second;
		}

		wisdom_list l = new c_list(key);
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
