

#ifndef c_ts_upgrade_h_
#define c_ts_upgrade_h_

#pragma once

#include <string>
#include <vector>
#include <map>
#include "c_config.h"
using namespace std;
using namespace zn;

#define TICK_TIME			"TICK_TIME"
#define TICK_TIMEOUT		30

struct pg_conn;
class c_ts_upgrade
	: public CMPool
{
	typedef map<string, c_config*> M;
public:
	c_ts_upgrade(const string& host, const string& port, 
		const string& dbname, const string& dbuser, 
		const string& dbpwd)
		: m_strDbHost(host)
		, m_strDbPort(port)
		, m_strDbName(dbname)
		, m_strDbUser(dbuser)
		, m_strDbPwd(dbpwd)
		, m_pg_conn(NULL)
	{
	}

	~c_ts_upgrade(void) 
	{
		for (M::iterator iter = m_config.begin(); iter != m_config.end(); ++iter)
		{
			delete iter->second;
		}
		m_config.clear();
	}

public:
	bool check_ver(const string& manufacturers, const string& model, 
		const string& screen, int ttype, c_config** config)
	{
		char szType[10];
		sprintf(szType, "%u", ttype);

		string config_key;
		config_key += manufacturers;
		config_key += "|";
		config_key += model;
		config_key += "|";
		config_key += screen;
		config_key += "|";
		config_key += szType;
		
		M::iterator iter = m_config.find(config_key);
		if (iter != m_config.end())
		{
			if (time(0) - iter->second->to_integer(TICK_TIME) < TICK_TIMEOUT) //如果没有超时
			{
				*config = iter->second;
				return true;
			}
			else//超时需要删除
			{
				delete iter->second;
				m_config.erase(iter);
			}
		}

		if (!OpenDB())
			return false;

		char sql[1024] = {0};

		sprintf(sql, "select * from sp_get_upgrade('%s', '%s', '%s', %d) \
					 as query(fmd5 character varying,linked_file character varying,filepath character varying,ver integer,force_ver integer, resid integer)",
					 AnsiToUtf8(manufacturers).c_str(),
					 AnsiToUtf8(model).c_str(),
					 AnsiToUtf8(screen).c_str(),
					 ttype);

		PGresult* res = PQexec(m_pg_conn, sql);
		if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
			c_znLog::_err("%s:error :%s\r\n", sql, Utf8ToAnsi(PQresultErrorMessage(res)).c_str());
			PQclear(res);

			return false;
		}
		
		c_config* p_config = new c_config;

		int nRecords = PQntuples (res);
		for (int i = 0; i < nRecords; i++)
		{
			string fmd5 = PQgetvalue(res, i, 0);
			string linked_file = PQgetvalue(res, i, 1);
			string filepath = PQgetvalue(res, i, 2);
			int32_t ver = atoi(PQgetvalue(res, i, 3));
			int32_t force_ver = atoi(PQgetvalue(res, i, 4));
			uint32_t resid = atoi(PQgetvalue(res, i, 5));
		
			c_config* child = new c_config;
			child->push("fmd5", fmd5.c_str());
			child->push("linked_file", linked_file.c_str());
			child->push("filepath", filepath.c_str());
			child->push("ver", ver);
			child->push("force_ver", force_ver);
			child->push("resid", resid);
			p_config->push("item", child);
		}
		PQclear(res);
		
		p_config->push(TICK_TIME, time(0));			//压入当前时间
		m_config.insert(make_pair(config_key, p_config));
		*config = p_config;

		return true;
	}

	bool check_ver(int appid, const string& manufacturers, const string& model, 
		const string& screen, int ttype, c_config** config)
	{
		char szType[10];
		sprintf(szType, "%u", ttype);

		string config_key;
		config_key += zn::str_itoa(appid);
		config_key += manufacturers;
		config_key += "|";
		config_key += model;
		config_key += "|";
		config_key += screen;
		config_key += "|";
		config_key += szType;

		M::iterator iter = m_config.find(config_key);
		if (iter != m_config.end())
		{
			if (time(0) - iter->second->to_integer(TICK_TIME) < TICK_TIMEOUT) //如果没有超时
			{
				*config = iter->second;
				return true;
			}
			else//超时需要删除
			{
				delete iter->second;
				m_config.erase(iter);
			}
		}

		if (!OpenDB())
			return false;

		char sql[1024] = {0};

		sprintf(sql, "select * from sp_get_upgrade(%u, '%s', '%s', '%s', %d) \
					 as query(fmd5 character varying,linked_file character varying, \
					 filepath character varying,ver integer,force_ver integer, \
					 resid integer, file character varying)",
					 appid,
					 AnsiToUtf8(manufacturers).c_str(),
					 AnsiToUtf8(model).c_str(),
					 AnsiToUtf8(screen).c_str(),
					 ttype);

		PGresult* res = PQexec(m_pg_conn, sql);
		if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
			c_znLog::_err("%s:error :%s\r\n", sql, Utf8ToAnsi(PQresultErrorMessage(res)).c_str());
			PQclear(res);

			return false;
		}

		c_config* p_config = new c_config;

		int nRecords = PQntuples (res);
		for (int i = 0; i < nRecords; i++)
		{
			string fmd5 = PQgetvalue(res, i, 0);
			string linked_file = PQgetvalue(res, i, 1);
			string filepath = PQgetvalue(res, i, 2);
			int32_t ver = atoi(PQgetvalue(res, i, 3));
			int32_t force_ver = atoi(PQgetvalue(res, i, 4));
			uint32_t resid = atoi(PQgetvalue(res, i, 5));
			string file = PQgetvalue(res, i, 6);

			c_config* child = new c_config;
			child->push("fmd5", fmd5.c_str());
			child->push("linked_file", linked_file.c_str());
			child->push("filepath", filepath.c_str());
			child->push("ver", ver);
			child->push("force_ver", force_ver);
			child->push("resid", resid);
			child->push("file", file.c_str());
			p_config->push("item", child);
		}
		PQclear(res);

		p_config->push(TICK_TIME, time(0));			//压入当前时间
		m_config.insert(make_pair(config_key, p_config));
		*config = p_config;

		return true;
	}

	bool check_ver(int appid, const string& manufacturers, const string& model, 
		const string& screen, int ttype, int clientid, c_config** config)
	{
		char szType[10];
		char szclientid[10];
		sprintf(szType, "%u", ttype);
		sprintf(szclientid, "%u", clientid);

		string config_key;
		config_key += zn::str_itoa(appid);
		config_key += manufacturers;
		config_key += "|";
		config_key += model;
		config_key += "|";
		config_key += screen;
		config_key += "|";
		config_key += szType;
		config_key += "|";
		config_key += szclientid;


		M::iterator iter = m_config.find(config_key);
		if (iter != m_config.end())
		{
			if (time(0) - iter->second->to_integer(TICK_TIME) < TICK_TIMEOUT) //如果没有超时
			{
				*config = iter->second;
				return true;
			}
			else//超时需要删除
			{
				delete iter->second;
				m_config.erase(iter);
			}
		}

		if (!OpenDB())
			return false;

		char sql[1024] = {0};

		sprintf(sql, "select * from sp_get_upgrade(%u, '%s', '%s', '%s', %d, %d) \
					 as query(fmd5 character varying,linked_file character varying, \
					 filepath character varying,ver integer,force_ver integer, \
					 resid integer, file character varying)",
					 appid,
					 AnsiToUtf8(manufacturers).c_str(),
					 AnsiToUtf8(model).c_str(),
					 AnsiToUtf8(screen).c_str(),
					 ttype,
					 clientid);

		PGresult* res = PQexec(m_pg_conn, sql);
		if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
			c_znLog::_err("%s:error :%s\r\n", sql, Utf8ToAnsi(PQresultErrorMessage(res)).c_str());
			PQclear(res);

			return false;
		}

		c_config* p_config = new c_config;

		int nRecords = PQntuples (res);
		for (int i = 0; i < nRecords; i++)
		{
			string fmd5 = PQgetvalue(res, i, 0);
			string linked_file = PQgetvalue(res, i, 1);
			string filepath = PQgetvalue(res, i, 2);
			int32_t ver = atoi(PQgetvalue(res, i, 3));
			int32_t force_ver = atoi(PQgetvalue(res, i, 4));
			uint32_t resid = atoi(PQgetvalue(res, i, 5));
			string file = PQgetvalue(res, i, 6);

			c_config* child = new c_config;
			child->push("fmd5", fmd5.c_str());
			child->push("linked_file", linked_file.c_str());
			child->push("filepath", filepath.c_str());
			child->push("ver", ver);
			child->push("force_ver", force_ver);
			child->push("resid", resid);
			child->push("file", file.c_str());
			p_config->push("item", child);
		}
		PQclear(res);

		p_config->push(TICK_TIME, time(0));			//压入当前时间
		m_config.insert(make_pair(config_key, p_config));
		*config = p_config;

		return true;
	}

private:
	bool OpenDB()
	{
		if (m_pg_conn == NULL)
		{
			m_pg_conn = PQsetdbLogin(m_strDbHost.c_str(), m_strDbPort.c_str(), 
				NULL, NULL, m_strDbName.c_str(), m_strDbUser.c_str(), m_strDbPwd.c_str());
		}

		if (PQstatus(m_pg_conn) != CONNECTION_OK)
		{
			printf("\r\n\r\n打开配置数据库失败!\r\n\r\n");
			CloseDB();
			return false;
		}
		PQsetClientEncoding(m_pg_conn, "utf-8");
		return true;
	}

	void CloseDB()
	{
		if (m_pg_conn)
		{
			PQfinish(m_pg_conn);
			m_pg_conn = NULL;
		}	
	}
private:
	string				m_strDbHost;		//数据库主机
	string				m_strDbPort;		//数据库端口
	string				m_strDbName;		//数据库名称
	string				m_strDbUser;		//数据库用户
	string				m_strDbPwd;			//数据库密码
	pg_conn*			m_pg_conn;

private:
	M					m_config;			//所有机型配置缓存
};

#endif //c_ts_upgrade_h_
