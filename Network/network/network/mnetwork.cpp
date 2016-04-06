// MNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "rawtcpconnect.h"
#include "rawtcpacceptor.h"
#include "nettimer.h"
#include "libevent.h"
#include "c_pipe.h"

_output* s_log()
{
	static _output log;

	return &log;
}

#ifdef NETWORK_EXPORTS

#ifdef _MSC_VER
BOOL APIENTRY DllMain( HANDLE hModule, 
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#else
int main()
{
	return 0;
}
#endif

#endif

int network_init(int threads, _output* out)
{
#ifdef _MSC_VER
	WSADATA wsaData;

	int err = WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
	if ( err != 0 ) {
		s_log()->err("Couldn't find a useable winsock.dll. fun:%s\r\n", __func__);
		return -1;
	}

#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#endif

	if (out)
	{
		s_log()->err = out->err;
		s_log()->info = out->info;
		s_log()->warn = out->warn;
	}
	else
	{
		s_log()->err = s_log()->info = s_log()->warn = printf;
	}

	return libevent::get_instance()->init(threads);
}

void network_thread_cb(mw_thread_cb* cb )
{
	libevent::get_instance()->set_thread_cb(cb);
}


void network_fini(void)
{
	libevent::get_instance()->exit_basis();
#ifdef _MSC_VER
	WSACleanup();
#endif
}

void network_dispatch(void)
{
	s_log()->info("network_dispatch 0\r\n");
	TlsSingelton<c_basis>::tlsInstance()->dispatch(true);
}

//创建一个连接
mw_connect* create_connect(mw_connect_sink* sink, e_class_connect e)
{
	return new CRawTcpConnection(sink);
}

//创建连接接管器
mw_acceptor* create_acceptor(mw_acceptor_sink* sink, e_class_connect e)
{
	return new CRawTcpAcceptor(sink, e);
}

mw_net_timer* create_net_timer(mw_net_timer_sink* sink)
{
	return new CNetTimer(sink);
}

mw_net_pipe* create_net_pipe(mw_net_pipe_sink* sink)
{
	return new c_pipe(sink);
}
