#include "stdafx.h"
#include "c_zconnect.h"


redis_cil::c_zconnect::c_zconnect(zclientSink* sink)
	: m_con(NULL)
	, m_sink(sink)
{
}


redis_cil::c_zconnect::~c_zconnect()
{
}

int redis_cil::c_zconnect::on_connect( int32_t reson, mw_connect* con)
{
	if (reson != 0)
	{
		con->_release();
		return 0;
	}

	m_sink->onConnect(this);

	return 0;
}

#define READ_LEN	2048
int redis_cil::c_zconnect::on_read( mw_connect* con, int32_t count )
{
	if (count <= 0)
		return 0;
	
	while (count > 0)
	{
		size_t ptr = 0;
		int len = min(READ_LEN, count);
		con->_get_opt(OPT_NETWORK_GET_RECV, &ptr, len);

		int adv = 0;
		wisdom_IOData data;
		int status = m_os.input((char*)ptr, len, adv, data);

		con->_set_opt(OPT_NETWORK_SET_RECV, &adv, sizeof(int));
		

		if (status == io_azerror || status == io_unknown)
		{
			ferr("recv error:" << status);
			//assert(0);
			//解析错误段开连接
			m_con->_disconnect();
			return 0;
		}

		if (status == io_success)
		{
			if (m_sink->onData(this, data) < 0)
			{
				m_con->_disconnect();
				return 0;
			}
		}

		if (status == io_receive)
		{
			if (count == len)
				return 0;
		}

		int recv_size = 0;
		con->_get_opt(OPT_NETWORK_GET_RECV_SIZE, &recv_size, sizeof(int));
		count = recv_size;
	}
	
	return 0;
	//return on_read(con, recv_size);
}

int redis_cil::c_zconnect::on_write( mw_connect* con, int32_t count )
{
	m_sink->onWrite(this);
	return 0;
}

int redis_cil::c_zconnect::on_disconnect( int32_t reason, mw_connect* con )
{
	con->_release();
	return 0;
}

int redis_cil::c_zconnect::on_release( mw_connect* con )
{
	m_sink->onRelease(this);
	delete this;
	return 0;
}

void redis_cil::c_zconnect::on_signal( mw_connect* con )
{

}

int redis_cil::c_zconnect::_connect( const string& ip, uint16 port )
{
	m_con = create_connect(this);
	return m_con->_connect(zn::domain(ip).c_str(), port, 0);
}

int redis_cil::c_zconnect::_send(unsigned  char* buf, int len )
{
	return m_con->_send(buf, len);
}
