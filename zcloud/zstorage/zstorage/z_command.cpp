#include "stdafx.h"
#include "z_command.h"
#include "z_cmds.h"
#include "_cmd.h"
#include "c_recycle.h"


void COMMAND_TTL::execute(wisdom_param& param, IConnect* con)
{
	int index = 1;
	uint32 serial = param->to_integer(++index);
	char* ptr = NULL;
	int len = 0;
	if (!param->get(++index, &ptr, len))
	{
		RESULT_ERROR(con, "param error.");
		return;
	}
	ptr = strupr(ptr);
	ptr[len] = 0;

	if (memcmp(ptr, "KEY", len) != 0)
	{
		RESULT_ERROR(con, __tos("param error. serial:" << serial));
		return;
	}

	string str_key = param->to_string(++index);
	if (str_key.empty())
	{
		RESULT_ERROR(con, __tos("key empty. serial:" << serial));
		return;
	}
	leveldb::Slice key = str_key;

	if (!param->get(++index, &ptr, len))
	{
		RESULT_ERROR(con, __tos("param error. serial:" << serial));
		return;
	}

	ptr = strupr(ptr);
	ptr[len] = 0;

	wisdom_IOStream io = new c_ostream_array;
	io->push(itostr(serial));
	IAnalyzer* analyzer = z_cmds::get_instance()->get_cmdId(ptr);
	if (analyzer == NULL)
	{
		RESULT_ERROR(con, __tos("param error. serial:" << serial));
		return;
	}

	analyzer->exec(param, index, key, io);
	
	RESULT_PARAM(io, con);
}

void COMMAND_CHECK::execute(wisdom_param& param, IConnect* con)
{
	c_recycle::get_instance()->recycle(con);
}
