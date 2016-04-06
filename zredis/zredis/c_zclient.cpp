#include "stdafx.h"
#include "c_zclient.h"
#include "c_config.h"


redis_cil::c_zclient::c_zclient(ICB* cb)
	: m_zcon(NULL)
	, m_cb(cb)
{
	finf("this:" << _this << "|zclient create:" << m_cb->constr());
	c_json json(m_cb->constr());
	m_db.server = json.to_string("server");
	m_db.port = json.to_uint16("port");
	m_db.account = json.to_string("auth");
	m_db.pwd = json.to_string("pwd");

	assert(!m_db.server.empty());
	assert(m_db.port > 0);

	//connect();
}

redis_cil::c_zclient::~c_zclient(void)
{
}

void redis_cil::c_zclient::onConnect( c_zconnect* zcon )
{
	auth();
	
	rq4096 ring;
	m_cb->doLogin(this, ring);
	if (ring.Count() > 0)
	{
		rawSend(ring);
	}

	doWrite();
}

void redis_cil::c_zclient::onRelease( c_zconnect* zcon )
{
	finf("this:" << _this << "|zclient release:" << m_cb->constr());
	m_zcon = NULL;
	m_cb->onRelease(this);
	m_ring.clear();
}

int redis_cil::c_zclient::onData( c_zconnect* zcon, wisdom_IOData& io )
{
	return m_cb->onData(this, io);

	return 0;
}

int redis_cil::c_zclient::send( wisdom_IOStream& os )
{
	if (m_ring.Count() > g_max_send_buffer)
	{
		m_ring.clear();
	}


	os->serialize(&m_ring);
	if (m_zcon == NULL)
	{
		//Á¬½Ó
		connect();
	}
	else
	{
		doWrite();
	}

	return 0;
}

int redis_cil::c_zclient::doWrite()
{
	if (m_zcon == NULL) return 0;

	while (m_ring.Count() > 0)
	{
		unsigned char* ptr;
		int len = m_ring.Attach(&ptr);
		if (m_zcon->_send(ptr, len) != ERROR_NEWORK_SUCCESS)
			break;
		m_ring.Advance(len);
	}

	return 0;
}

int redis_cil::c_zclient::sendCmd( const string& cmd )
{
	wisdom_IOStream os = new c_ostream_array;
	os->push(cmd);
	return send(os);
}

int redis_cil::c_zclient::sendCmd( const string& cmd, const string& arg_1 )
{
	wisdom_IOStream os = new c_ostream_array;
	os->push(cmd);
	os->push(arg_1);
	return send(os);
}

int redis_cil::c_zclient::sendCmd( const string& cmd, const string& arg_1, const string& arg_2 )
{
	wisdom_IOStream os = new c_ostream_array;
	os->push(cmd);
	os->push(arg_1);
	os->push(arg_2);
	return send(os);
}

int redis_cil::c_zclient::sendCmd( wisdom_IOStream& os )
{
	return send(os);
}



int redis_cil::c_zclient::onWrite( c_zconnect* zcon )
{
	doWrite();
	return 0;
}

int redis_cil::c_zclient::auth()
{
	if (m_db.account.empty())
		return 0;

	CRingQueue<4096> ring;
	wisdom_IOStream os = new c_ostream_array;
	os->push("AUTH");
	os->push(m_db.account);
	os->push(m_db.pwd);
	os->serialize(&ring);

	rawSend(ring);
	return 0;
}

int redis_cil::c_zclient::connect()
{
	m_zcon = new c_zconnect(this);
	int err = m_zcon->_connect(m_db.server, m_db.port);

	if (err != ERROR_NEWORK_SUCCESS)
	{
		ferr("_connect err:" << err);
		delete m_zcon;
		return -1;
	}

	return 0;
}

int redis_cil::c_zclient::rawSend( rq4096& ring )
{
	if (m_zcon == NULL) return 0;

	while (ring.Count() > 0)
	{
		unsigned char* ptr;
		int len = ring.Attach(&ptr);
		if (m_zcon->_send(ptr, len) != ERROR_NEWORK_SUCCESS)
			break;
		ring.Advance(len);
	}

	return 0;
}

