// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//
#pragma once

#ifndef _STDAFX_H_
#define _STDAFX_H_

#ifdef _MSC_VER
#define _WIN32_WINNT	0x0500

#pragma warning(disable: 4005)
#pragma warning(disable: 4267 4311 4312 4244 4273)
#pragma warning(disable: 4355)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include "config.h"
#else
#include "config.h"
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/atmioc.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <sys/types.h>

#endif

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <queue>
#include <string>

#ifdef _MSC_VER
#undef socklen_t
#define socklen_t	int
#define IS_TRY_AGAIN	(WSAEWOULDBLOCK == WSAGetLastError())
#define IS_EINPROGRESS	(1)
#define ctid() ::GetCurrentThreadId()
#else
#define SOCKET_ERROR            (-1)
#define SO_UPDATE_ACCEPT_CONTEXT 0x700B

#define IS_TRY_AGAIN	(EAGAIN == errno)
#define IS_EINPROGRESS	(EVUTIL_SOCKET_ERROR() == EINPROGRESS)
#define ctid() pthread_self()
#endif

//环形队列的块大小为8k
#define RINGQUEUE_BLOCK_GRANULE					(8192)
#define DEFAULT_MAX_SEND_SIZE					(1024 * 512)		//256k
#define DEFAULT_MAX_RECV_SIZE					(1024 * 512)		//256k
#define DEFAULT_NETWORK_NETWORK_PACKET			(1400)				//默认网络包大小
#define DEFAULT_NETWORK_ACTIVE_TIMEOUT			(20)

#include "MyTypes.h"
#include "df_macro.h"
#include "rw_lock.h"
#include "event.h"
#include "log.h"
#include "evutil.h"
#include "network.h"
#include "c_rwlock.h"
#include "c_event.h"
#include "c_thread.h"
#include "tls_pools.h"
#include "RingQueue.h"
#include "c_singleton.h"
using namespace pool;
using namespace zn;

#include "ref.h"

extern _output* s_log();

#ifdef _MSC_VER
#undef socklen_t
#define socklen_t	int
#endif


// TODO: 在此处引用程序要求的附加头文件

#endif //_STDAFX_H_

