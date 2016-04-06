#include "stdafx.h"
#include "z_command.h"
#include "_container.h"
class _count
{
public:
	_count()
	{
		_atom_init(m_count);
	}
	void add(){
		
		if (_atom_add_one(&m_count) % 10000 == 0)
		{
			c_time	m_time(time(0));
			LOG4_INFO("count:" << _atom_read(&m_count) << " time:" << m_time.time_stamp());
		}
	}
private:
	
	_atom	m_count;
};

static _count g_count;

void COMMAND_TTL::execute(wisdom_param& param, IConnect* con)
{
	g_count.add();
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

	//KEY VALUE
	if (!param->get(++index, &ptr, len))
	{
		RESULT_ERROR(con, __tos("key empty. serial:" << serial));
		return;
	}
	ptr[len] = 0;

	uint16 crc = 0;
	char* pos = strchr(ptr, '@');
	if (pos != NULL)
	{
		crc = c_crc::crc16(0, (uint8*)ptr, pos - ptr);
	}
	else
	{
		crc = c_crc::crc16(0, (uint8*)ptr, len);
	}

	con->bind(TLS(_container));
	
	ICB* cb = c_server::get_instance()->find_storage(crc);
	if (cb == NULL)
	{
		RESULT_ERROR(con, __tos("not find storage. serial:" << serial));
		return;
	}

	wisdom_IOStream os = new c_ostream_array;

	wisdom_request req = new _request;
	req->m_serial = serial;
	req->m_con = con;
	req->m_key = ptr;
	uint32 seq = TLS(_container)->push(req);
	os->push("TTL");
	os->push(itostr(seq));
	for (int i = 2; i < param->count(); )
	{
		char* buf = NULL;
		int buf_len = 0;
		if (!param->get(++i, &buf, buf_len))
		{
			RESULT_ERROR(con, "param get error.");
			return;
		}

		os->push(buf, buf_len);
	}

	
	THREAD_ZCLIENT(cb)->sendCmd(os);

	//RESULT_PARAM(io, con);
}

void COMMAND_GET::execute(wisdom_param& param, IConnect* con)
{
	g_count.add();
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

	//KEY VALUE
	if (!param->get(++index, &ptr, len))
	{
		RESULT_ERROR(con, __tos("key empty. serial:" << serial));
		return;
	}
	ptr[len] = 0;

	uint16 crc = 0;
	char* pos = strchr(ptr, '@');
	if (pos != NULL)
	{
		crc = c_crc::crc16(0, (uint8*)ptr, pos - ptr);
	}
	else
	{
		crc = c_crc::crc16(0, (uint8*)ptr, len);
	}

	con->bind(TLS(_container));

	ICB* cb = c_server::get_instance()->find_storage(crc);
	if (cb == NULL)
	{
		RESULT_ERROR(con, __tos("not find storage. serial:" << serial));
		return;
	}

	wisdom_IOStream os = new c_ostream_array;

	wisdom_request req = new _request;
	req->m_serial = serial;
	req->m_con = con;
	req->m_key = ptr;
	uint32 seq = TLS(_container)->push(req);
	os->push("TTL");
	os->push(itostr(seq));
	for (int i = 2; i < param->count();)
	{
		char* buf = NULL;
		int buf_len = 0;
		if (!param->get(++i, &buf, buf_len))
		{
			RESULT_ERROR(con, "param get error.");
			return;
		}

		os->push(buf, buf_len);
	}


	THREAD_ZCLIENT(cb)->sendCmd(os);

}


void COMMAND_LOAD::execute(wisdom_param& param, IConnect* con)
{
	wisdom_IOStream os = new c_ostream_array;
	c_server::get_instance()->loadStorage(os);

	RESULT_PARAM(os, con);
}

void COMMAND_TSIZE::execute(wisdom_param& param, IConnect* con)
{
	wisdom_IOStream os = new c_ostream_array;
	os->push(itostr(TLS(_container)->size()));

	RESULT_PARAM(os, con);
}

void COMMAND_DELAY::execute(wisdom_param& param, IConnect* con)
{
	wisdom_broadcast bd = c_broadcast_admin::get_instance()->Join("DELAY", con);
}

