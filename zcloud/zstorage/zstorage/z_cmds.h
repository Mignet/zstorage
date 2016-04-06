
#ifndef z_cmds_h
#define z_cmds_h

#pragma once
#include "stdafx.h"
#include "zbase.h"
#include "_cmd.h"
#include "_list.h"
#include "_mlist.h"
#include "_seq.h"
#include "_cloud.h"

class z_cmds
	: public c_singleton<z_cmds>
{
public:
	z_cmds(){}
	~z_cmds(){}
	void register_cmd()
	{
		m_command.insert(make_pair(T_GET_SET, "DEL"));
		m_command.insert(make_pair(T_LIST, "LIST"));
		m_command.insert(make_pair(T_MLIST, "MLIST"));
		m_command.insert(make_pair(T_SEQ, "SEQ"));

		m_cmds.insert(make_pair("GET", new _get));
		m_cmds.insert(make_pair("SET", new _set));
		m_cmds.insert(make_pair("DEL", new _del)); 
		m_cmds.insert(make_pair("EXISTS", new _exists));
		m_cmds.insert(make_pair("RECYCLE", new _recycle));
		m_cmds.insert(make_pair("LIST", new _list));
		m_cmds.insert(make_pair("MLIST", new _mlist));
		m_cmds.insert(make_pair("SEQ", new _seq));
		m_cmds.insert(make_pair("CLOUD", new _cloud));
	}

	IAnalyzer* get_type(uint8 type)
	{
		map<uint8, string>::iterator pos = m_command.find(type);
		if (pos != m_command.end())
		{
			return get_cmdId(pos->second.c_str());
		}
		
		return NULL;
	}

	IAnalyzer* get_cmdId(const char* cmd)
	{
		map<string, IAnalyzer*>::iterator pos = m_cmds.find(cmd);
		if (pos != m_cmds.end())
			return pos->second;
		return NULL;
	}

private:
	map<string, IAnalyzer*>			m_cmds;
	map<uint8, string>				m_command;
};


#endif
