
/*
* Copyright (c) 2005-2010 Tony Zhengjq <tony.zheng@sky-mobi.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _NETWORK_H_
#define _NETWORK_H_

#pragma once

#ifdef _MSC_VER

#ifdef NETWORK_EXPORTS
#define NETWORK_API __declspec(dllexport)
#else
#define NETWORK_API /*__declspec(dllimport)*/
#endif
#define EXTERN_C	extern "C"
#else

#define NETWORK_API	
#define EXTERN_C 	extern "C"

#endif

//通信层版本1.0
#define NETWORK_VERSION					(10)

//环形队列的块大小为8k
#define RINGQUEUE_BLOCK_GRANULE					(8192)

enum
{
	ERROR_NEWORK_SUCCESS	= 0,		//成功
	ERROR_NEWORK_TIMEOUT,				//超时
	ERROR_NEWORK_LOCAL_GET_SENDBUFFER,	//发送缓存已满
	ERROR_NEWORK_CREATE_SOCK,			//创建句柄失败
	ERROR_NEWORK_UNCONNECT,				//未连接
};

enum OPT_NETWORK
{
	OPT_NETWORK_DEFAULT						= 0x1000,

	OPT_NETWORK_REMOTE_ADDR,				//char[16]				远程地址

	OPT_NETWORK_REMOTE_PORT,				//unsigned short		远程端口

	OPT_NETWORK_LOCAL_ADDR,					//char[16]				本地地址

	OPT_NETWORK_LOCAL_PORT,					//unsigned short		本地端口

	OPT_NETWORK_RINGQUEUE_BLOCK_GRANULE,	//int					收发数据的环形队列的颗粒大小,用于收发参数

	OPT_NETWORK_IP_SET_SENDBUFFER,			//int					设置TCP协议层的发送缓存

	OPT_NETWORK_IP_GET_SENDBUFFER,			//int					获取TCP协议层的发送缓存

	OPT_NETWORK_IP_SET_RECVDBUFFER,			//int					设置TCP协议层的接收缓存

	OPT_NETWORK_IP_GET_RECVBUFFER,			//int					获取TCP协议层的接收缓存

	OPT_NETWORK_LOCAL_SET_SENDBUFFER,		//int					设置本地的发送最大缓存

	OPT_NETWORK_LOCAL_GET_SENDBUFFER,		//int					获取本地的发送最大缓存

	OPT_NETWORK_LOCAL_SET_RECVDBUFFER,		//int					设置本地的接收最大缓存

	OPT_NETWORK_LOCAL_GET_RECVBUFFER,		//int					获取本地的接收最大缓存

	OPT_NETWORK_GET_RECV_SIZE,				//int					获取已接收的数据大小

	OPT_NETWORK_GET_RECV,					//size_t				获取已接收的数据内容,不大于环形队列的颗粒大小
	
	OPT_NETWORK_SET_RECV,					//int					确认接收长度

	OPT_NETWORK_LOCAL_GET_SEND_SIZE,		//int					获取还在缓存中的发送数据大小

	OPT_NETWORK_SET_PACKET_SIZE,			//int					设置Internet网上每发一个包的大小

	OPT_NETWORK_GET_PACKET_SIZE,			//int					设置Internet网上每发一个包的大小

	OPT_NETWORK_TCP_NODELAY,				//int					开关Nagle算法来控制网络延时

	OPT_NETWORK_GET_SO_ERROR,				//int					获取套接字错误

	OPT_NETWORK_GET_HANDLE_COUNT,			//int					获取句柄总数
	
	OPT_NETWORK_SET_KEEPLIVE_TIMEOUT,		//int					设置一条连接N分钟没有收发数据为超时

	OPT_NETWORK_GET_KEEPLIVE_TIMEOUT,		//int					获取收发数据超时时间,如果没设置为0,没有超时
};


typedef enum e_class_connect
{
	class_raw_tcp	= 'a',
	class_len_tcp,
	class_raw_udp

}e_class_connect;

class mw_connect;
class mw_connect_sink
{
public:
	//连接情况
	virtual int on_connect(int32_t, mw_connect*) {return 0;}

	//可读  int count 为可读字节数 
	virtual	int on_read(mw_connect* con, int32_t count) = 0;

	//可写 int count 为还需要发送的字节数
	virtual int on_write(mw_connect* con, int32_t count) = 0;

	//关闭连接
	virtual int on_disconnect(int32_t reason, mw_connect* con)	= 0;

	//连接释放
	virtual int on_release(mw_connect* con) = 0;
	
	//可以收到本线程所有连接信号
	virtual void on_signal(mw_connect* con) = 0;

	//连接线程被重先绑定
	virtual void on_rebind(mw_connect* con) {}
};

//连接对像
class mw_connect
{
public:
	//设置回调器
	virtual void _set_sink(mw_connect_sink* psink) = 0;	
	
	//客户端连接，默认为异步连接
	virtual int _connect(const char* addr, uint16_t port, int asyn = 1) = 0;

	//发送数据
	virtual int _send(unsigned char* date, int32_t size) = 0;
	
	//设置套接字参数
	virtual int _set_opt(int32_t type, void* arg, int32_t size) = 0;
	
	//获取套接字参数
	virtual int _get_opt(int32_t type, void* arg, int32_t size) = 0;

	//断开连接
	virtual int _disconnect(void) = 0;

	//是否连接 0断开 1连接
	virtual int _isconnect(void) = 0;

	//析构连接对像
	virtual int _release(void) = 0;

	//多线程异步发送信息
	virtual int _signal(void) = 0;

	//重先绑定线程
	virtual int _rebind(thread_t bind_tid) = 0;
};

class mw_acceptor;
class mw_acceptor_sink
{
public:
	//某一端口接收到新的连接
	virtual int on_accept(mw_connect* con, uint16_t port) = 0;

	//某一端口停止监听
	virtual int on_stop(uint16_t port) = 0;

	//对象被析构
	virtual int on_release(mw_acceptor* acceptor) = 0;
};

//连接接收器
class mw_acceptor
{
public:
	//开始监听一个端口
	virtual int _start_listen(uint16_t port, uint32_t addr = 0) = 0;

	//停止监听一个端口
	virtual int _stop_listen(void) = 0;

	//析构连接接收器
	virtual int _release() = 0;
};


class mw_net_timer;
class  mw_net_timer_sink
{
public:
	virtual int on_timer(mw_net_timer *timer) = 0;
};

class  mw_net_timer 
{
public:
	virtual void _schedule(uint32_t sec) = 0;
	virtual void _cancel() = 0;
	virtual void _release() = 0;
};


class mw_net_pipe;
class mw_net_pipe_sink
{
public:
	virtual void on_process(void* data, int32_t size, mw_net_pipe* pipe) = 0;
};

class mw_net_pipe
{
public:
	virtual int _create() = 0;
	virtual int _notify(const char* data, int size) = 0;
	virtual void _cancel() = 0;
	virtual void _release() = 0;
};

class mw_thread_cb
{
public:
	virtual void thread_cb() = 0;
};

typedef struct _output
{
	int (*err)(const char *, ...);
	int (*info)(const char *, ...);
	int (*warn)(const char *, ...);
}_output;

//通信线程初始化回调，在 network_init 之前
EXTERN_C NETWORK_API void network_thread_cb(mw_thread_cb* cb);

//初始化通信层
EXTERN_C NETWORK_API int network_init(int32_t threads = 8, _output* out = 0);

//析构通信层
EXTERN_C NETWORK_API void network_fini(void);

//通信消费循环
EXTERN_C NETWORK_API void network_dispatch(void);

//创建一个连接
EXTERN_C NETWORK_API mw_connect* create_connect(mw_connect_sink* sink, e_class_connect e = class_raw_tcp);

//创建连接接管器
EXTERN_C NETWORK_API mw_acceptor* create_acceptor(mw_acceptor_sink* sink, e_class_connect e = class_raw_tcp);

//创建一个定时器
EXTERN_C NETWORK_API mw_net_timer* create_net_timer(mw_net_timer_sink* sink);

//创建一个管道
EXTERN_C NETWORK_API mw_net_pipe* create_net_pipe(mw_net_pipe_sink* sink);



#endif //_NETWORK_H_

