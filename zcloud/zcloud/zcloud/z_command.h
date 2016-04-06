#ifndef z_command_h
#define z_command_h

#pragma once


COMMAND_OBJECT(COMMAND_TTL)
{
	CONSTRUCTION_OBJECT("TTL", COMMAND_TTL, 0x1,
		"TTL *Command routing");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_GET)
{
	CONSTRUCTION_OBJECT("GET", COMMAND_GET, 0x1,
		"GET *Command routing");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};


COMMAND_OBJECT(COMMAND_LOAD)
{
	CONSTRUCTION_OBJECT("LOAD", COMMAND_LOAD, 0x1,
		"LOAD *Command load xml");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_TSIZE)
{
	CONSTRUCTION_OBJECT("TSIZE", COMMAND_TSIZE, 0x1,
		"TSIZE *Command");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};

COMMAND_OBJECT(COMMAND_DELAY)
{
	CONSTRUCTION_OBJECT("DELAY", COMMAND_DELAY, 0x1,
		"DELAY *Command");

	CONSTRUCTION_EXPLAIN()
public:
	virtual void execute(wisdom_param& param, IConnect* con);
};


class z_command
{
public:
	z_command()
	{
		REGISTER_OBJECT(COMMAND_TTL);
		REGISTER_OBJECT(COMMAND_GET);
		REGISTER_OBJECT(COMMAND_LOAD);
		REGISTER_OBJECT(COMMAND_TSIZE);
		REGISTER_OBJECT(COMMAND_DELAY);
	}
	~z_command() {}
};

static z_command	_z_command;


#endif

