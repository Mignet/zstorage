#include "stdafx.h"
#include "c_mlist.h"
#include "c_timeout.h"

#define MAX_LIST_KEY_LEN	256

#define MLIST_INIT()	\
if (!init()){	\
	io->push(ZRESULT_ERROR);	\
	io->push("mlist create error.");	\
	return;}

c_mlist::c_mlist(const char* key)
: m_max_list(1024)
, m_key(key)
, m_init(false)
{
	m_effective = time(0) + c_server::get_instance()->object_effective();
	m_max_list = c_server::get_instance()->mlist_max_count();
	m_head.type = T_MLIST;
}

c_mlist::~c_mlist()
{

}

bool c_mlist::load_value(const string& value, _zmsg_head* head)
{
	int len = value.length();
	char* ptr = (char*)value.c_str();

	if (head->type != T_MLIST)
		return false;

	memcpy(&m_head, head, sizeof(_zmsg_head));
	ptr += sizeof(_zmsg_head);
	len -= sizeof(_zmsg_head);

	char szkey[255];
	for (int i = 0; i < len;)
	{
		memset(szkey, 0, sizeof(szkey));
		uint8 n = *ptr;
		++i;
		i += n;
		if (i >= len)
			break;

		memcpy(szkey, ptr, n);
		m_list.push_back(szkey);
	}

	return true;
}

bool c_mlist::init()
{
	m_effective = time(0) + c_server::get_instance()->object_effective();
	c_wlock lock(&m_lock);
	if (m_init)
		return true;

	string value;
	_zmsg_head* head = NULL;
	if (c_base::get_value(m_key, value, &head) == 0)
	{
		if (!load_value(value, head))
		{
			return false;
		}
	}

	m_init = true;

	return true;
}

void c_mlist::list_del_key(leveldb::WriteBatch& bh, const string& keyId)
{
	bh.Delete(__tos(m_key << "@" << keyId));
}

void c_mlist::list_update(leveldb::WriteBatch& bh)
{
	int len = m_list.size() * MAX_LIST_KEY_LEN + sizeof(_zmsg_head);
	char* buffer = (char*)TLS_MALLOC(len);
	char* ptr = buffer;
	memcpy(ptr, &m_head, sizeof(_zmsg_head));
	ptr += sizeof(_zmsg_head);
	for (list<string>::iterator pos = m_list.begin(); pos != m_list.end(); ++pos)
	{
		string keyId = *pos;
		*ptr = keyId.length();
		++ptr;
		memcpy(ptr, keyId.c_str(), keyId.length());
		ptr += keyId.length();
	}
	((_zmsg_head*)buffer)->s_crc(c_crc::crc16(0, (uint8*)buffer + sizeof(_zmsg_head), (ptr - buffer) - sizeof(_zmsg_head)));
	leveldb::Slice value(buffer, ptr - buffer);
	bh.Put(m_key, value);

	TLS_FREE(buffer);
}

void c_mlist::list_push(leveldb::WriteBatch& bh, const string& keyId, leveldb::Slice& value)
{
	m_list.push_back(keyId);
	bh.Put(__tos(m_key << "@" << keyId), value);
}

bool c_mlist::list_get(const string& keyId, string& value)
{
	return c_base::get(__tos(m_key << "@" << keyId), value);
}

void c_mlist::push(const string& keyId, char* data, int n, wisdom_IOStream& io)
{
	MLIST_INIT();
	c_wlock lock(&m_lock);

	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	if (m_list.size() >= m_max_list)
	{
		string delKey = m_list.front();
		m_list.pop_front();
		list_del_key(bh, delKey);
	}

	leveldb::Slice value(data, n);
	list_push(bh, keyId, value);

	list_update(bh);

	if (c_base::write(op, bh))
	{
		io->push(ZRESULT_OK);
	}
	else
	{
		io->push(ZRESULT_ERROR);
		io->push("write error.");
	}
}

void c_mlist::seek(const string& keyId, uint32 count, wisdom_IOStream& io)
{
	MLIST_INIT();

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	uint32 i = 0;
	bool is_find = false;
	for (list<string>::iterator pos = m_list.begin(); i < count && pos != m_list.end(); ++pos)
	{
		if (!is_find)
		{
			if (keyId != *pos)
			{
				continue;
			}
			else
			{
				is_find = true;
				continue;
			}
		}

		string value;
		if (list_get(*pos, value))
		{
			io->push((char*)value.c_str(), value.length());
			i++;
		}
	}

	//ÏòºóÈ¡count
	if (!is_find)
	{
		int size = m_list.size();

		int ibegin = size - count >= 0 ? (size - count) : 0;

		for (list<string>::iterator pos = m_list.begin(); i < count && pos != m_list.end(); ++pos)
		{
			if (ibegin-- > 0)
				continue;

			string value;
			if (list_get(*pos, value))
			{
				io->push((char*)value.c_str(), value.length());
			}
		}
	}
}

void c_mlist::size(wisdom_IOStream& io)
{
	MLIST_INIT();

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	io->push(itostr(m_list.size()));
}

void c_mlist::remove(wisdom_IOStream& io)
{
	MLIST_INIT();

	c_wlock lock(&m_lock);
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	for (list<string>::iterator pos = m_list.begin(); pos != m_list.end();)
	{
		list_del_key(bh, *pos);
		m_list.erase(pos++);
	}
	list_delete(bh);

	if (!c_base::write(op, bh))
	{
		io->push(ZRESULT_ERROR);
		io->push("list write error.");
		return;
	}

	io->push(ZRESULT_OK);
}


void c_mlist::expire(uint32 ms, wisdom_IOStream& io)
{
	MLIST_INIT();
	c_wlock lock(&m_lock);
	m_head.s_effective(time(0) + ms);
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	list_update(bh);

	if (!c_base::write(op, bh))
	{
		io->push(ZRESULT_ERROR);
		io->push("list write error.");
		return;
	}

	c_timeout::get_instance()->push(m_head.type, m_key, ms);

	io->push(ZRESULT_OK);
}


void c_mlist::time_(wisdom_IOStream& io)
{
	MLIST_INIT();

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	uint32 time_s = m_head.g_effective() - time(0) > 0 ? m_head.g_effective() - time(0) : 0;
	io->push(itostr(time_s));
}


void c_mlist::list_delete(leveldb::WriteBatch& bh)
{
	bh.Delete(m_key);
}

bool c_mlist::is_release()
{
	return time(0) > m_effective;
}
