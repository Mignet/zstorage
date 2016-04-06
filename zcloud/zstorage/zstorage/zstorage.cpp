// zstorage.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "c_leavedb.h"
#include "c_server.h"
#include "c_thread_refresh.h"
#include "z_cmds.h"
#include "c_time.h"
int main()
{
	//char test[1024];
	//memset(test, 0, 1024);
	//test[100] = '1';
	//string temp;
	//temp.assign(test, 1024);
	//printf("%d\r\n", temp.length());
	//char aaa[1024];
	//memcpy(aaa, temp.c_str(), temp.length());

	string path = zn::exe_path();

	chdir(path.c_str());

#ifndef _MSC_VER
	zn::guard_process(zn::getFilename(getExeFullPath().c_str()), 5);

	struct rlimit r;
	if (getrlimit(RLIMIT_NOFILE, &r) < 0)
	{
		fprintf(stderr, "getrlimit error\n");
		exit(1);
	}

	/** set limit **/
	r.rlim_cur = 204800;
	r.rlim_max = 204800;
	if (setrlimit(RLIMIT_NOFILE, &r) < 0)
	{
		fprintf(stderr, "setrlimit error\n");
		exit(1);
	}

	/** get value of set **/
	if (getrlimit(RLIMIT_NOFILE, &r) < 0)
	{
		fprintf(stderr, "getrlimit error\n");
		exit(1);
	}
#endif

	c_server::get_instance()->LoadConfig();

	if (!_LEAVEDB::get_instance()->open(c_server::get_instance()->leavedb()))
	{
		LOG4_ERROR("leave db open error " << c_server::get_instance()->leavedb().c_str());
		exit(1);
	}

	//注册所有的命令
	z_cmds::get_instance()->register_cmd();

	network_init(c_server::get_instance()->threads(), NULL);

	c_auth::get_instance()->init();
	c_config::get_instance()->m_ver = "1.00";

	c_thread_refresh::get_instance()->Start();
	c_thread_refresh::get_instance()->waitEvent();


	if (c_server::get_instance()->listen() < 0)
	{
		LOG4_ERROR("listen error.");
		exit(0);
	}


	LOG4_ERROR("server start.");

	network_dispatch();
	
	return 0;
}

