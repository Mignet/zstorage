#include "stdafx.h"
#include "c_object_public.h"
#include "c_auth.h"
#include "c_broadcast.h"
#include "c_config.h"

void COMMAND_EXIT::execute( wisdom_param& param, IConnect* con )
{
	con->ansy_disconnect();
	return;
}

void COMMAND_PING::execute( wisdom_param& param, IConnect* con )
{
	RESULT_OK(con, "PONG");
	return;
}

void COMMAND_AUTH::execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(2);
	string pwd = param->to_string(3);

	if (!c_auth::get_instance()->auth(account, pwd))
	{
		RESULT_ERROR(con, "Account or password error.");
		return;
	}

	con->set_power(c_auth::get_instance()->power(account));

	RESULT_OK(con, "ok");
	return;
}

void COMMAND_VER::execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(2);
	string pwd = param->to_string(3);

	wisdom_IOStream os = new c_ostream_success;

	ZOS zos;
	os->push((zos << ZREDIS_VRE << "\t" << c_config::get_instance()->m_ver).str());
	RESULT_PARAM(os, con);
	return;
}

void COMMAND_MONITOR::execute( wisdom_param& param, IConnect* con )
{
	string monitor = param->to_string(2);
	if (monitor.empty())
	{
		RESULT_ERROR(con, "The 2 parameter error.");
		return;
	}

	uint32 type = param->to_integer(3);

	//如果是IP
	con->subscribe(monitor, type);

	return;
}

void COMMAND_IPCONFIG::execute( wisdom_param& param, IConnect* con )
{
	string ip; uint16 port;
	con->ipaddr(ip, port);

	char szIP[50];
	sprintf(szIP, "%s:%u", ip.c_str(), port);
	RESULT_OK(con, szIP);

	return;
}

void COMMAND_TIME::execute( wisdom_param& param, IConnect* con )
{
	struct timeval tl;
	gettimeofday(&tl, NULL);
	wisdom_IOStream result = new c_ostream_array;
	result->push(itostr(tl.tv_sec));
	result->push(itostr(tl.tv_usec));
	RESULT_PARAM(result, con);

	return;
}

void COMMAND_LIST::execute( wisdom_param& param, IConnect* con )
{
	char *ptr;
	int len;
	wisdom_IOStream result = new c_ostream_array;
	//1.获取参数
	if (!param->get(2, &ptr, len))
	{
		c_command::get_instance()->help(result);
	}
	else
	{
		ptr = strupr(ptr);
		ptr[len] = 0;
		if (!c_command::get_instance()->help(ptr, result))
		{
			RESULT_ERROR(con, "The 2 parameter is not command.");
			return;
		}
	}

		
	RESULT_PARAM(result, con);
	return;
}

void COMMAND_IPLIST::execute( wisdom_param& param, IConnect* con )
{
	wisdom_IOStream result = new c_ostream_array;
	c_iplist::get_instance()->list(result);
	RESULT_PARAM(result, con);

	return;
}

void ACCOUNT_LIST::child_execute( wisdom_param& param, IConnect* con )
{
	wisdom_IOStream result = new c_ostream_array;
	c_auth::get_instance()->list(result);
	RESULT_PARAM(result, con);
	return;
}

void ACCOUNT_ADD::child_execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(3).c_str();
	if (account.empty())
	{
		RESULT_ERROR(con, "The 3 parameter error.");
		return;
	}

	string password = param->to_string(4).c_str();
	if (password.empty())
	{
		RESULT_ERROR(con, "The 4 parameter error.");
		return;
	}

	string power = param->to_string(5).c_str();
	uint8 ipower = atoi(power.c_str());
	if (power.empty() || ipower <= 0)
	{
		RESULT_ERROR(con, "The 5 parameter error.");
		return;
	}

	if (true)
	{
		CMarkupSTL config;
		if(!config.Load(TS_CONFIG))
		{
			RESULT_ERROR(con, "server open db error.");
			return;
		}
		//根节点
		config.FindElem();

		if(config.FindChildElem("auth", true))
		{
			config.IntoElem();

			if (config.AddChildElem("user"))
			{
				config.IntoElem();

				config.AddChildElem("account", account.c_str());
				config.AddChildElem("pwd", password.c_str());
				config.AddChildElem("power", itostr(ipower).c_str());
				
				config.OutOfElem();

				config.Save(TS_CONFIG);
			}
			
			config.OutOfElem();
		}
	}

	RESULT_OK(con, "OK");
	return;
}


void ACCOUNT_MPWD::child_execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(3).c_str();
	if (account.empty())
	{
		RESULT_ERROR(con, "The 3 parameter error.");
		return;
	}

	string password = param->to_string(4).c_str();
	if (password.empty())
	{
		RESULT_ERROR(con, "The 4 parameter error.");
		return;
	}

	if (!c_auth::get_instance()->check_account(account))
	{
		RESULT_ERROR(con, "account is not exists.");
		return;
	}

	if (true)
	{
		CMarkupSTL config;
		if(!config.Load(TS_CONFIG))
		{
			RESULT_ERROR(con, "server open db error.");
			return;
		}
		//根节点
		config.FindElem();

		if(config.FindChildElem("auth", true))
		{
			config.IntoElem();

			while (true)
			{
				if (config.FindChildElem("user"))
				{
					config.IntoElem();
					
					string strAccount;
					if (config.FindChildElem("account", true))
					{
						strAccount = config.GetChildData();

						if (strAccount == account)
						{
							if (config.FindChildElem("pwd", true))
							{
								config.SetChildData(password.c_str());
								config.Save(TS_CONFIG);
								break;
							}
						}
					}
					config.OutOfElem();
				}
				else
				{
					break;
				}
			}
			
			config.OutOfElem();
		}
	}

	RESULT_OK(con, "OK");
	return;
}


void ACCOUNT_MPOWER::child_execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(3).c_str();
	if (account.empty())
	{
		RESULT_ERROR(con, "The 3 parameter error.");
		return;
	}

	string power = param->to_string(4).c_str();
	uint8 ipower = atoi(power.c_str());
	if (power.empty())
	{
		RESULT_ERROR(con, "The 4 parameter error.");
		return;
	}

	if (!c_auth::get_instance()->check_account(account))
	{
		RESULT_ERROR(con, "account is not exists.");
		return;
	}

	if (true)
	{
		CMarkupSTL config;
		if(!config.Load(TS_CONFIG))
		{
			RESULT_ERROR(con, "server open db error.");
			return;
		}
		//根节点
		config.FindElem();

		if(config.FindChildElem("auth", true))
		{
			config.IntoElem();

			while (true)
			{
				if (config.FindChildElem("user"))
				{
					config.IntoElem();

					string strAccount;
					if (config.FindChildElem("account", true))
					{
						strAccount = config.GetChildData();

						if (strAccount == account)
						{
							if (config.FindChildElem("power", true))
							{
								config.SetChildData(itostr(ipower).c_str());
								config.Save(TS_CONFIG);
								break;
							}
						}
					}
					config.OutOfElem();
				}
				else
				{
					break;
				}
			}

			config.OutOfElem();
		}
	}

	RESULT_OK(con, "OK");
	return;
}


void ACCOUNT_DEL::child_execute( wisdom_param& param, IConnect* con )
{
	string account = param->to_string(3).c_str();
	if (account.empty())
	{
		RESULT_ERROR(con, "The 3 parameter error.");
		return;
	}

	if (!c_auth::get_instance()->check_account(account))
	{
		RESULT_ERROR(con, "account is not exists.");
		return;
	}

	if (true)
	{
		CMarkupSTL config;
		if(!config.Load(TS_CONFIG))
		{
			RESULT_ERROR(con, "server open db error.");
			return;
		}
		//根节点
		config.FindElem();

		if(config.FindChildElem("auth", true))
		{
			config.IntoElem();

			while (true)
			{
				if (config.FindChildElem("user"))
				{
					config.IntoElem();

					string strAccount;
					if (config.FindChildElem("account", true))
					{
						strAccount = config.GetChildData();

						if (strAccount == account)
						{
							config.OutOfElem();
							config.RemoveChildElem();
							config.Save(TS_CONFIG);
							break;
						}
					}
					config.OutOfElem();
				}
				else
				{
					break;
				}
			}

			config.OutOfElem();
		}
	}

	RESULT_OK(con, "OK");
	return;
}

void SYSTEM_CONFIG::child_execute(wisdom_param& param, IConnect* con)
{
	FILE* pf = fopen("conf/config.xml", "rb");
	if (!pf)
	{
		RESULT_ERROR(con, "conf/config.xml is not exists.");
		return;
	}

	char buffer[81960];
	fread(buffer, 1, sizeof(buffer), pf);
	fclose(pf);

	RESULT_OK(con, buffer);

}

void SYSTEM_COMMAND::execute(wisdom_param& param, IConnect* con)
{
	wisdom_IOStream result = new c_ostream_array;
	string command;
	for (int i = 2; true; i++)
	{
		string cmd = param->to_string(i);
		if (cmd.empty())
			break;
		command += cmd;
		command += " ";
	}
#ifndef _MSC_VER
	result->push("##################################\r\n");
	FILE *pp = popen(command.c_str(), "r");
	if (!pp) {
		return;
	}
	char buf[4096];
	uint32 tick = time(0);
	while (true)
	{
		if (time(0) - tick > 5)
			break;
		int n = fread(buf, 1, sizeof(buf), pp);
		if (n <= 0)
			break;
		result->push(buf, n);
	}
	pclose(pp);
#else
	result->push(command);
#endif

	RESULT_PARAM(result, con);
}
