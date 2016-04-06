#ifndef c_auth_h
#define c_auth_h

#pragma once

#include "c_resolver.h"

enum auth_power
{
	power_admin		=	0xFF,
	power_31		=	0x1F,
	power_15		=	0x0F,
	power_D			=	0x0D,
	power_C			=	0x0C,
	power_B			=	0x0B,
	power_A			=	0x0A,
	power_9			=	0x09,
	power_8			=	0x08,
	power_7			=	0x07,
	power_6			=	0x06,
	power_5			=	0x05,
	power_4			=	0x04,
	power_3			=	0x03,
	power_2			=	0x02,
	power_1			=	0x01,
	power_0			=	0x00,
};

struct STAuth
{
	STAuth():power(0) {}
	string account;
	string pwd;
	uint8 power;
};

WISDOM_PTR(STAuth, wisdom_auth);
typedef map<string, wisdom_auth> MAUTH;


class c_auth
	: public c_singleton<c_auth>
	, public mw_net_timer_sink
{
public:
	c_auth(void);
	~c_auth(void);
	bool init();
	//认证
	bool auth(const string& account, const string& pwd);

	bool check_account(const string& account);
	//获取角色
	uint8 power(const string& account);

	void list(wisdom_IOStream& os);

private:
	bool loadAuth();
	virtual int on_timer(mw_net_timer *timer);
public:
	rw_lock		m_lock;
	MAUTH		m_auth;
};



#endif //c_auth_h