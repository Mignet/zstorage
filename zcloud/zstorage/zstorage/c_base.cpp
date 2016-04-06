#include "stdafx.h"
#include "c_base.h"


c_base::c_base()
{
	m_db = TLS(c_leavedb);
	memset(&m_head, 0, sizeof(m_head));
}


c_base::~c_base()
{
}

bool c_base::put(const leveldb::Slice& key, leveldb::Slice& value, uint32 effective)
{
	if (effective == 0) 
		effective = PERMANENT_BASE;
	else
		effective = time(0) + effective;
	if (value.size() + sizeof(_zmsg_head) > MAX_VALUE_LEN)	return false;

	leveldb::Slice val = get_Slice(value, effective);

	return m_db->put(key, val);
}

bool c_base::get(const leveldb::Slice& key, string& value)
{
	return  m_db->get(key, value);

}

leveldb::Slice c_base::get_Slice(leveldb::Slice& value, time_t effective)
{
	m_head.s_effective(effective);
	m_head.s_crc( c_crc::crc16(0, (uint8*)value.data(), value.size()) );
	memcpy(TLS(_tbuffer)->buf, &m_head, sizeof(_zmsg_head));
	TLS(_tbuffer)->pos = sizeof(_zmsg_head);

	memcpy(TLS(_tbuffer)->buf + sizeof(_zmsg_head), value.data(), value.size());
	TLS(_tbuffer)->pos += value.size();

	return leveldb::Slice(TLS(_tbuffer)->buf, TLS(_tbuffer)->pos);
}

bool c_base::write(leveldb::WriteOptions& writeOptions, leveldb::WriteBatch& batch)
{
	return  m_db->write(writeOptions, batch);
}

void c_base::recycle(Iter* it, IConnect* con)
{
	m_db->traverse(it, con);
}


int c_base::get_value(const leveldb::Slice& key, string& value, _zmsg_head** head)
{
	if (!get(key, value))
	{
		return _zerror_not_extsis;
	}

	if (value.length() <= sizeof(_zmsg_head))
	{
		return _zerror_data_bad;
	}

	*head = (_zmsg_head*)value.c_str();

	if (!(*head)->check_type())
	{
		return _zerror_type_bad;
	}

	if ( (*head)->g_crc() !=
		c_crc::crc16(0, (uint8*)value.c_str() + sizeof(_zmsg_head), value.length() - sizeof(_zmsg_head)) )
	{
		return _zerror_crc_bad;
	}

	return 0;
}
