#include "stdafx.h"
#include "_cmd.h"
#include "z_cmds.h"
#include "c_timeout.h"
#include "c_recycle.h"


void _get::exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
{
	string value;
	_zmsg_head* head = NULL;
	int result = c_base::get_value(key, value, &head);
	
	if (result != 0)
	{
		io->push(ZRESULT_OK);
		return;
	}

	if (head->type != T_GET_SET)
	{
		io->push(ZRESULT_OK);
		return;
	}

	io->push(ZRESULT_OK);
	io->push((char*)value.c_str() + sizeof(_zmsg_head), value.length() - sizeof(_zmsg_head));
}

void _set::exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
{
	char* ptr = NULL;
	int len = 0;
	if (!param->get(++index, &ptr, len))
	{
		ZERROR_RESULT("value not exists.");
	}

	uint32 timeout = 0;
	char* ptr_cmd = NULL;
	int len_cmd = 0;
	if (!param->get(++index, &ptr_cmd, len_cmd))
	{
		timeout = 0;
	}
	else
	{
		ptr_cmd = strupr(ptr_cmd);
		ptr_cmd[len_cmd] = 0;
		if (strcmp(ptr_cmd, "EXPIRE") == 0)
		{
			timeout = param->to_integer(++index);
		}
		else
		{
			ZERROR_RESULT("command format correct.");
		}
	}

	if (timeout == 0)
		timeout = PERMANENT_BASE;

	leveldb::Slice value(ptr, len);
	if (!c_base::put(key, value, timeout))
	{
		ZERROR_RESULT("db error.");
	}

	c_timeout::get_instance()->push(m_head.type, key.data(), timeout);

	io->push(ZRESULT_OK);
}

void _del::exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
{
	string value;
	if (!c_base::get(key, value))
	{
		ZERROR_RESULT("key is not exists.");
	}

	IAnalyzer* az = z_cmds::get_instance()->get_type(ZTYPE(value));
	if (az == NULL)
	{
		ZERROR_RESULT("del date type error.");
	}

	az->remove(key.data(), io);
}

void _del::remove(const string& key, wisdom_IOStream& io)
{
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	bh.Delete(key);
	if (!c_base::write(op, bh))
	{
		ZERROR_RESULT("del error.");
	}

	io->push(ZRESULT_OK);
}

void _exists::exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
{
	string value;
	io->push(ZRESULT_OK);
	if (!c_base::get(key, value))
	{
		io->push("0");
	}
	else
	{
		io->push("1");
	}
}

void _recycle::exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io)
{
	io->push(ZRESULT_OK);
	uint64_t ms = get_ms();
	c_recycle::get_instance()->recycle();

	io->push(itostr(get_ms() - ms));
}
