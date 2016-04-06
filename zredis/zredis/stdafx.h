    // stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#pragma warning(disable: 4996)

#define SERVER_VER	"zredis 1.0"
//#define DEBUG_TEST	1


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
#include <direct.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/atmioc.h>
#include <netinet/tcp.h>
#include <unistd.h>
#endif
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <queue>

using namespace std;

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "MyTypes.h"
#include "df_macro.h"

#include "public.h"
#include "tlv.h"
#include "c_markup_stl.h"
#include "network.h"
#include "system.h"
#include "tls_pools.h"
#include "c_singleton.h"
#include "zn_log.h"
#include "RingQueue.h"
#include "c_thread.h"
#include "rw_lock.h"
using namespace zn;
#include "c_md5.h"
#include "c_crc.h"
#include "wisdom_ptr.h"
#include "zpacket.h"
#include "c_json.h"
//redis 头文件

using namespace zn;
using namespace json;

extern _output* s_log();


#ifdef _MSC_VER
#define TS_CONFIG	"conf\\config.xml"
#else
#define TS_CONFIG	"conf/config.xml"
#endif


// TODO: 在此处引用程序需要的其他头文件
