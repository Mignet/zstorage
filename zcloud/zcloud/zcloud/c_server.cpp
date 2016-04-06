#include "stdafx.h"
#include "c_server.h"
#include "_storage_cb.h"

c_server::c_server()
: m_threads(8)
, m_delay(100)
{
}


c_server::~c_server()
{
}

bool c_server::LoadConfig()
{
	CMarkupSTL config;
	if (!config.Load(TS_CONFIG))
		return false;
	//根节点
	config.FindElem();

	if (config.FindChildElem("server", true))
	{
		config.IntoElem();

		if (config.FindChildElem("port", true))
			c_config::get_instance()->m_port = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("threads", true))
			m_threads = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("delay", true))
			m_delay = atoi(config.GetChildData().c_str());

		config.OutOfElem();
	}

	wisdom_IOStream os = new c_ostream_array;
	loadStorage(os);

	return true;
}

int c_server::listen()
{
	c_config::get_instance()->m_acceptor = create_acceptor(this);

	if (c_config::get_instance()->m_port <= 0 ||
		c_config::get_instance()->m_acceptor->_start_listen(c_config::get_instance()->m_port) != ERROR_NEWORK_SUCCESS)
	{
		LOG4_ERROR("listen:" << c_config::get_instance()->m_port << " error!");
	}
	else
	{
		LOG4_INFO("listen:" << c_config::get_instance()->m_port << " ok!");
	}

	return 0;
}

int c_server::on_accept(mw_connect* con, unsigned short port)
{
	char ipaddr[16];
	unsigned short _port = 0;
	int timeout = 5;
	con->_set_opt(OPT_NETWORK_SET_KEEPLIVE_TIMEOUT, &timeout, sizeof(timeout));
	con->_get_opt(OPT_NETWORK_REMOTE_ADDR, ipaddr, sizeof(ipaddr));
	con->_get_opt(OPT_NETWORK_REMOTE_PORT, &_port, sizeof(_port));

	if (port == c_config::get_instance()->m_port)
	{
		new c_zredis_connect(con);
		LOG4_INFO(" ip:" << ipaddr << ":" << _port
			<< "{" << ip_query::ipquery::get_instance()->area(ipaddr)
			<< ":" << ip_query::ipquery::get_instance()->agent(ipaddr) << "}" << "	new connect to port:" << port);
	}

	return 0;
}

int c_server::on_timer(mw_net_timer *timer)
{
	return 0;
}

void c_server::loadStorage(wisdom_IOStream& os)
{
	CMarkupSTL config;
	if (!config.Load(TS_CONFIG))
		return;

	c_wlock lock(&m_lock);
	m_storage.clear();

	//根节点
	config.FindElem();

	while (config.FindChildElem("storage"))
	{
		config.IntoElem();

		wisdom_storage og = new _storage;
		og->cb = new _storage_cb;
		string interval;
		if (config.FindChildElem("interval", true))
			interval = config.GetChildData().c_str();
		sscanf(interval.c_str(), "%x#%x", &og->ibegin, &og->iend);

		if (config.FindChildElem("constr", true))
			og->cb->m_constr = config.GetChildData().c_str();

		LOG4_INFO("storage begin:" << og->ibegin << "|end:" <<
			og->iend << "|constr:" << og->cb->constr());
	
		os->push(__tos("storage begin:" << og->ibegin << "|end:" <<
			og->iend << "|constr:" << og->cb->constr()));
		config.OutOfElem();

		m_storage.insert(make_pair(c_interval(og->ibegin, og->iend), og));
		
	}

	return;
}
