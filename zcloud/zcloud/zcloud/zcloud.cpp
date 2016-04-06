// zstorage.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "c_server.h"
#include "c_thread_refresh.h"
#include "_container.h"

typedef map<c_interval, int> M;
int main()
{
	//M m;
	//m.insert(make_pair(c_interval(0, 100), 1));
	//m.insert(make_pair(c_interval(101, 200), 2));
	//m.insert(make_pair(c_interval(201, 300), 3));
	//m.insert(make_pair(c_interval(301, 400), 4));
	//m.insert(make_pair(c_interval(401, 500), 5));
	//m.insert(make_pair(c_interval(501, 600), 6));

	//while (true)
	//{
	//	int input = 0;
	//	printf("input:");
	//	scanf("%u", &input);

	//	M::iterator pos = m.find(input);
	//	if (pos != m.end())
	//	{
	//		printf("find:%u\r\n", pos->second);
	//	}
	//	else
	//	{
	//		printf("not find\r\n");
	//	}
	//}

	//return 0;


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

	network_thread_cb(new _container);
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

