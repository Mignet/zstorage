
#ifndef c_object_public_h
#define c_object_public_h

#pragma once

#include "c_resolver.h"
#include "c_object.h"
#include "c_command.h"

//PING
class IConnect;

COMMAND_OBJECT(COMMAND_EXIT)
{
	CONSTRUCTION_OBJECT("EXIT", COMMAND_EXIT, 0x1, 
		"EXIT *result disconnect");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};



COMMAND_OBJECT(COMMAND_PING)
{
	CONSTRUCTION_OBJECT("PING", COMMAND_PING, 0x1, 
		"PING [] *result 'PONG'");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

//AUTH
COMMAND_OBJECT(COMMAND_AUTH)
{
	CONSTRUCTION_OBJECT("AUTH", COMMAND_AUTH, 0x1, 
		"AUTH [account] [password] *result 'OK'");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_VER)
{
	CONSTRUCTION_OBJECT("!", COMMAND_VER, 0x1, 
		"! [] *result service ver");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};


//MONITOR
COMMAND_OBJECT(COMMAND_MONITOR)
{
	CONSTRUCTION_OBJECT("@", COMMAND_MONITOR, 0x4,
		"@ [ip|userId] *monitro ip or userId message");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

//IPCONFIG
COMMAND_OBJECT(COMMAND_IPCONFIG)
{
	CONSTRUCTION_OBJECT("IPCONFIG", COMMAND_IPCONFIG, 0x1,
		"IPCONFIG [] *result local ip");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_TIME)
{
	CONSTRUCTION_OBJECT("TIME", COMMAND_TIME, 0x1,
		"TIME [] *result server time");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};


COMMAND_OBJECT(COMMAND_LIST)
{
	CONSTRUCTION_OBJECT("LS", COMMAND_LIST, 0x1,
		"LS [] *result command explain");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_IPLIST)
{
	CONSTRUCTION_OBJECT("IPLIST", COMMAND_IPLIST, 0x1,
		"IPLIST [] *result zredis client list");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

//帐号管理
COMMAND_OBJECT(COMMAND_SYSTEM)
{
	CONSTRUCTION_OBJECT("SYSTEM", COMMAND_SYSTEM, 0x80,
		"LS SYSTEM");

	//是否存在子集
	CONSTRUCTION_DEFINE_CHILD();

	CONSTRUCTION_EXPLAIN();

	CONSTRUCTION_EXPLAIN_CHILD_METHOD();
	//执行方法
	CONSTRUCTION_EXECUTE_METHOD();
};

//配置表获取
COMMAND_CHILD(SYSTEM_CONFIG)
{
	CONSTRUCTION_CHILD("CONFIG", SYSTEM_CONFIG, COMMAND_SYSTEM,
		"SYSTEM CONFIG *result config");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(SYSTEM_COMMAND)
{
	CONSTRUCTION_OBJECT("#", SYSTEM_COMMAND, 0x8,
		"# [cmd] *result cmd result");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

class c_system_public
{
public:
	c_system_public()
	{
		REGISTER_OBJECT(COMMAND_SYSTEM);
		REGISTER_OBJECT(SYSTEM_CONFIG);
		REGISTER_OBJECT(SYSTEM_COMMAND);
	}
};

static c_system_public _system_public;

//帐号管理
COMMAND_OBJECT(COMMAND_ACCOUNT)
{
	CONSTRUCTION_OBJECT("ACCOUNT", COMMAND_ACCOUNT, 0x80,
		"LS ACCOUNT");

	//是否存在子集
	CONSTRUCTION_DEFINE_CHILD();

	CONSTRUCTION_EXPLAIN();

	CONSTRUCTION_EXPLAIN_CHILD_METHOD();
	//执行方法
	CONSTRUCTION_EXECUTE_METHOD();
};

//帐号列表
COMMAND_CHILD(ACCOUNT_LIST)
{
	CONSTRUCTION_CHILD("LS", ACCOUNT_LIST, COMMAND_ACCOUNT,
		"ACCOUNT LS *result account list");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};

//创建帐号
COMMAND_CHILD(ACCOUNT_ADD)
{
	CONSTRUCTION_CHILD("ADD", ACCOUNT_ADD, COMMAND_ACCOUNT,
		"ACCOUNT [ADD] {account} {password} {power} *result ok add account set account and password and power");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};

//修改密码
COMMAND_CHILD(ACCOUNT_MPWD)
{
	CONSTRUCTION_CHILD("MPWD", ACCOUNT_MPWD, COMMAND_ACCOUNT,
		"ACCOUNT [MPWD] {account} {password} *result ok modify account password");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};

//修改权限
COMMAND_CHILD(ACCOUNT_MPOWER)
{
	CONSTRUCTION_CHILD("MPOWER", ACCOUNT_MPOWER, COMMAND_ACCOUNT,
		"ACCOUNT [MPOWER] {account} {power} *result ok modify account power");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};

//删除帐号
COMMAND_CHILD(ACCOUNT_DEL)
{
	CONSTRUCTION_CHILD("DEL", ACCOUNT_DEL, COMMAND_ACCOUNT,
		"ACCOUNT [DEL] {account} *result ok del account");
	CONSTRUCTION_EXPLAIN()
public:
	virtual void child_execute(wisdom_param& param, IConnect* con);
};


class c_object_public
{
public:
	c_object_public()
	{
		REGISTER_OBJECT(COMMAND_EXIT);
		REGISTER_OBJECT(COMMAND_PING);
		REGISTER_OBJECT(COMMAND_AUTH);
		REGISTER_OBJECT(COMMAND_MONITOR);
		REGISTER_OBJECT(COMMAND_IPCONFIG);
		REGISTER_OBJECT(COMMAND_TIME);
		REGISTER_OBJECT(COMMAND_LIST);
		REGISTER_OBJECT(COMMAND_IPLIST);
		REGISTER_OBJECT(COMMAND_ACCOUNT);
		REGISTER_OBJECT(ACCOUNT_LIST);
		REGISTER_OBJECT(ACCOUNT_ADD);
		REGISTER_OBJECT(ACCOUNT_MPWD);
		REGISTER_OBJECT(COMMAND_VER);
	}
};

static c_object_public _object_public;
#endif //c_object_public_h