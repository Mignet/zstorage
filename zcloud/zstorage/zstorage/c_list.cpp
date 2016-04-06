#include "stdafx.h"
#include "c_list.h"
#include "c_timeout.h"
#define LIST_INIT() \
	if (!init()){ \
		io->push(ZRESULT_ERROR); \
		io->push("list init error."); \
		return;	}
c_list::c_list(const char* key)
: m_max_list(1024)
, m_key(key)
, m_init(false)
{
	m_effective = time(0) + c_server::get_instance()->object_effective();
	m_max_list = c_server::get_instance()->list_max_count();
	m_head.type = T_LIST;
}

c_list::~c_list()
{

}


bool c_list::load_value(const string& value, _zmsg_head* head)
{
	if ((value.length()) % 2 != 0)
		return false;

	int len = value.length();
	char* ptr = (char*)value.c_str();
	if (head->type != T_LIST)
		return false;

	memcpy(&m_head, head, sizeof(_zmsg_head));
	ptr += sizeof(_zmsg_head);
	len -= sizeof(_zmsg_head);
	for (int i = 0; i < len; i += 2)
	{
		uint16 list_key;
		memcpy(&list_key, &ptr[i], 2);
		list_key = htons(list_key);
		m_list.push_back(list_key);
	}

	return true;
}

bool c_list::init()
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

void c_list::push(char* data, int n, wisdom_IOStream& io)
{
	LIST_INIT();
	c_wlock lock(&m_lock);

	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	uint16 keyId;
	if (m_list.size() >= m_max_list)
	{
		keyId = m_list.front();
		m_list.pop_front();
		list_del_key(bh, keyId);
	}

	leveldb::Slice value(data, n);
	list_push(bh, value);

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


void c_list::pop(wisdom_IOStream& io)
{
	LIST_INIT();

	c_wlock lock(&m_lock);
	if (m_list.empty())
	{
		io->push(ZRESULT_ERROR);
		io->push("empty.");
		return;
	}

	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	
	uint16 keyId;
	string value;
	keyId = m_list.front();
	m_list.pop_front();
	if (!list_get(keyId, value))
	{
		io->push(ZRESULT_ERROR);
		io->push("list get error.");
		return;
	}

	list_del_key(bh, keyId);
	list_update(bh);

	if (!c_base::write(op, bh))
	{
		io->push(ZRESULT_ERROR);
		io->push("list write error.");
		return;
	}

	io->push(ZRESULT_OK);
	io->push((char*)value.c_str(), value.length());
}


void c_list::limit(uint32 count, wisdom_IOStream& io)
{
	LIST_INIT();

	c_rlock lock(&m_lock);
	uint32 i = 0;
	io->push(ZRESULT_OK);
	for (list<uint16>::iterator pos = m_list.begin(); i++ < count && pos != m_list.end(); ++pos)
	{
		uint32 keyId = *pos;
		string value;
		if (list_get(keyId, value))
		{
			io->push((char*)value.c_str(), value.length());
		}
	}
}

void c_list::size(wisdom_IOStream& io)
{
	LIST_INIT();

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	io->push(itostr(m_list.size()));
}

void c_list::time_(wisdom_IOStream& io)
{
	LIST_INIT();

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	uint32 time_s = m_head.g_effective() - time(0) > 0 ? m_head.g_effective() - time(0) : 0;
	io->push(itostr(time_s));
}

void c_list::update(wisdom_param& param, int index, wisdom_IOStream& io)
{
	char* ptr = NULL;
	int len = 0;
	LIST_INIT();
	c_wlock lock(&m_lock);
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	//===========delete============
	for (list<uint16>::iterator pos = m_list.begin(); pos != m_list.end();)
	{
		list_del_key(bh, *pos);
		m_list.erase(pos++);
	}
	//=========================================
	while (true)
	{
		if (!param->get(++index, &ptr, len) || len <= 0)
		{
			break;
		}

		leveldb::Slice value(ptr, len);
		list_push(bh, value);
	}

	list_update(bh);

	if (!c_base::write(op, bh))
	{
		io->push(ZRESULT_ERROR);
		io->push("list write error.");
		return;
	}

	io->push(ZRESULT_OK);
}

void c_list::seek(int iseek, uint32 ilimit, wisdom_IOStream& io)
{
	LIST_INIT();

	io->push(ZRESULT_OK);

	c_rlock lock(&m_lock);
	if (m_list.size() <= iseek)
		return;


	for (list<uint16>::iterator pos = m_list.begin(); 
		pos != m_list.end() && ilimit != 0; ++pos)
	{
		if (iseek-- > 0)
		{
			continue;
		}

		string value;
		if (c_base::get(__tos(m_key << "@" << *pos), value))
		{
			io->push((char *)value.c_str(), value.length());
			ilimit--;
		}
	}
}



void c_list::expire(uint32 ms, wisdom_IOStream& io)
{
	LIST_INIT();
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


void c_list::remove(wisdom_IOStream& io)
{
	LIST_INIT();
	c_wlock lock(&m_lock);
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	for (list<uint16>::iterator pos = m_list.begin(); pos != m_list.end();)
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

void c_list::list_del_key(leveldb::WriteBatch& bh, uint16 keyId)
{
	bh.Delete(__tos(m_key << "@" << keyId));
}

void c_list::list_update(leveldb::WriteBatch& bh)
{
	int len = m_list.size() * 2 + sizeof(_zmsg_head);
	char* buffer = (char*)TLS_MALLOC(len);
	char* ptr = buffer;
	memcpy(ptr, &m_head, sizeof(_zmsg_head));
	ptr += sizeof(_zmsg_head);
	for (list<uint16>::iterator pos = m_list.begin(); pos != m_list.end(); ++pos)
	{
		uint16 keyId = *pos;
		keyId = ntohs(keyId);
		memcpy(ptr, &keyId, 2);
		ptr += 2;
	}
	
	((_zmsg_head*)buffer)->s_crc( c_crc::crc16(0, (uint8*)buffer + sizeof(_zmsg_head), len - sizeof(_zmsg_head) ));
	leveldb::Slice value(buffer, len);

	bh.Put(m_key, value);

	TLS_FREE(buffer);
}

void c_list::list_push(leveldb::WriteBatch& bh, leveldb::Slice& value)
{
	uint16 keyId = list_get_keyId();
	m_list.push_back(keyId);
	bh.Put(__tos(m_key << "@" << keyId), value);
}

uint16 c_list::list_get_keyId()
{
	if (m_list.empty())
		return 1;
	else
		return m_list.back() + 1;
}

bool c_list::list_get(uint16 keyId, string& value)
{
	return c_base::get(__tos(m_key << "@" << keyId), value);
}

void c_list::list_delete(leveldb::WriteBatch& bh)
{
	bh.Delete(m_key);
}

bool c_list::is_release()
{
	return time(0) > m_effective;
}


