// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

//#include "targetver.h"

#pragma warning(disable: 4996)

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#include <stdint.h>
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


#include <stdlib.h>

#define LOG4_ERROR(x) 
#define LOG4_INFO(x) 

#define LOG4_A_INFO(x) 
#define LOG4_B_INFO(x)  
#define LOG4_C_INFO(x) 
#define LOG4_D_INFO(x) 


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
#include "RingQueue.h"
#include "c_thread.h"
#include "c_event.h"
#include "c_uuid.h"
#include "ipquery.h"
#include "rw_lock.h"
#include "zpacket.h"
using namespace zn;
#include "c_md5.h"
#include "c_crc.h"
#include "wisdom_ptr.h"
#include "c_json.h"
using namespace json;
#include "c_time.h"
#include "yunva_lib.h"

#include "c_auth.h"
#include "c_config.h"
#include "c_zredis_connect.h"
#include "c_resolver.h"
#include "c_object.h"
#include "c_command.h"
#include "c_zclient_public.h"
#include "c_zclient.h"
#include "c_object_public.h"
#include "zbase.h"
#include "c_server.h"
#include "help.h"


using namespace redis_cil;


#ifdef _MSC_VER
#define TS_CONFIG	"conf\\config.xml"
#define XML_ROOM_REDIS	"conf\\room_redis.xml" 
#define XML_ISSUE_REDIS	"conf\\issue_redis.xml"
#define MIME_CONFIG	"conf\\mime.types"
#else
#define TS_CONFIG	"conf/config.xml"
#define XML_ROOM_REDIS	"conf/room_redis.xml"
#define XML_ISSUE_REDIS	"conf/issue_redis.xml"
#define MIME_CONFIG	"conf/mime.types"
#endif

// TODO:  在此处引用程序需要的其他头文件
