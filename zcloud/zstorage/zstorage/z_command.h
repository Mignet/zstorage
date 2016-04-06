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


COMMAND_OBJECT(COMMAND_CHECK)
{
	CONSTRUCTION_OBJECT("CHECK", COMMAND_CHECK, 0x1,
		"CHECK *Command routing");

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
		REGISTER_OBJECT(COMMAND_CHECK);
	}
	~z_command() {}
};

static z_command	_z_command;


#endif

