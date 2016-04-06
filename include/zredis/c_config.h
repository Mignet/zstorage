#ifndef c_config_h
#define c_config_h

#pragma once

#define ZREDIS_VRE			"zredis version 1.0 author zensoft updated 2014.1.24"

static const int g_max_send_buffer			= 1024 * 1024 * 64;  //�Ӵ� 64M ����ļ�����

class c_config
	: public c_singleton<c_config>
{
public:
	c_config() :m_acceptor(NULL), m_port(0) {}

public:
	mw_acceptor*				m_acceptor;
	uint16						m_port;
	string						m_ver;
};


#endif //c_auth_h