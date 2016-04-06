
#ifndef c_recycle_h
#define c_recycle_h

#pragma once
#include "c_base.h"
#include "z_cmds.h"

class c_recycle
	: public c_base
	, public Iter
	, public c_singleton<c_recycle>
{
public:
	c_recycle()
	{

	}
	~c_recycle()
	{

	}

	void recycle(IConnect* con = NULL)
	{
		c_base::recycle(this, con);
	}
private:
	virtual bool read(const leveldb::Slice& key, const leveldb::Slice& value)
	{
		if (value.size() <= sizeof(_zmsg_head))
			return true;

		_zmsg_head* head = (_zmsg_head*)value.data();
		if (head->g_crc() != c_crc::crc16(0, (uint8*)value.data() + sizeof(_zmsg_head), value.size() - sizeof(_zmsg_head)))
			return true;

		if (!head->check_type())
			return true;

		if (head->type == T_SEQ_VALUE)
			return true;

		if (head->g_effective() > time(0))
			return true;

		IAnalyzer* del = z_cmds::get_instance()->get_cmdId("DEL");
		wisdom_IOStream io = new c_ostream_array;
		wisdom_param param;
		char szKey[255];
		memset(szKey, 0, sizeof(szKey));
		memcpy(szKey, key.data(), key.size());
		del->exec(param, 0, szKey, io);
		
		return false;
	}
};


#endif
