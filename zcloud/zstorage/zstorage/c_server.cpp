#include "stdafx.h"
#include "c_server.h"


c_server::c_server()
: m_threads(8)
, m_cache_size(1024)
, m_list_max_count(1024)
, m_mlist_max_count(99)
, m_write_buffer_size(100)
, m_seq_valid_time(720)
, m_object_effective(15)
, m_seq_clear_time(30)
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
	//¸ù½Úµã
	config.FindElem();

	if (config.FindChildElem("server", true))
	{
		config.IntoElem();

		if (config.FindChildElem("ip", true))
			m_ip = config.GetChildData();

		if (config.FindChildElem("port", true))
			c_config::get_instance()->m_port = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("threads", true))
			m_threads = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("leavedb", true))
			m_leavedb = config.GetChildData();

		if (config.FindChildElem("cache", true))
			m_cache_size = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("list_max_count", true))
			m_list_max_count = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("mlist_max_count", true))
			m_mlist_max_count = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("seq_valid_time", true))
			m_seq_valid_time = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("write_buffer_size", true))
			m_write_buffer_size = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("object_effective", true))
			m_object_effective = atoi(config.GetChildData().c_str());

		if (config.FindChildElem("seq_clear_time", true))
			m_seq_clear_time = atoi(config.GetChildData().c_str());

		config.OutOfElem();
	}

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
