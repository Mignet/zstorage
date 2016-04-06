
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

//ͨ�Ų�汾1.0
#define NETWORK_VERSION					(10)

//���ζ��еĿ��СΪ8k
#define RINGQUEUE_BLOCK_GRANULE					(8192)

enum
{
	ERROR_NEWORK_SUCCESS	= 0,		//�ɹ�
	ERROR_NEWORK_TIMEOUT,				//��ʱ
	ERROR_NEWORK_LOCAL_GET_SENDBUFFER,	//���ͻ�������
	ERROR_NEWORK_CREATE_SOCK,			//�������ʧ��
	ERROR_NEWORK_UNCONNECT,				//δ����
};

enum OPT_NETWORK
{
	OPT_NETWORK_DEFAULT						= 0x1000,

	OPT_NETWORK_REMOTE_ADDR,				//char[16]				Զ�̵�ַ

	OPT_NETWORK_REMOTE_PORT,				//unsigned short		Զ�̶˿�

	OPT_NETWORK_LOCAL_ADDR,					//char[16]				���ص�ַ

	OPT_NETWORK_LOCAL_PORT,					//unsigned short		���ض˿�

	OPT_NETWORK_RINGQUEUE_BLOCK_GRANULE,	//int					�շ����ݵĻ��ζ��еĿ�����С,�����շ�����

	OPT_NETWORK_IP_SET_SENDBUFFER,			//int					����TCPЭ���ķ��ͻ���

	OPT_NETWORK_IP_GET_SENDBUFFER,			//int					��ȡTCPЭ���ķ��ͻ���

	OPT_NETWORK_IP_SET_RECVDBUFFER,			//int					����TCPЭ���Ľ��ջ���

	OPT_NETWORK_IP_GET_RECVBUFFER,			//int					��ȡTCPЭ���Ľ��ջ���

	OPT_NETWORK_LOCAL_SET_SENDBUFFER,		//int					���ñ��صķ�����󻺴�

	OPT_NETWORK_LOCAL_GET_SENDBUFFER,		//int					��ȡ���صķ�����󻺴�

	OPT_NETWORK_LOCAL_SET_RECVDBUFFER,		//int					���ñ��صĽ�����󻺴�

	OPT_NETWORK_LOCAL_GET_RECVBUFFER,		//int					��ȡ���صĽ�����󻺴�

	OPT_NETWORK_GET_RECV_SIZE,				//int					��ȡ�ѽ��յ����ݴ�С

	OPT_NETWORK_GET_RECV,					//size_t				��ȡ�ѽ��յ���������,�����ڻ��ζ��еĿ�����С
	
	OPT_NETWORK_SET_RECV,					//int					ȷ�Ͻ��ճ���

	OPT_NETWORK_LOCAL_GET_SEND_SIZE,		//int					��ȡ���ڻ����еķ������ݴ�С

	OPT_NETWORK_SET_PACKET_SIZE,			//int					����Internet����ÿ��һ�����Ĵ�С

	OPT_NETWORK_GET_PACKET_SIZE,			//int					����Internet����ÿ��һ�����Ĵ�С

	OPT_NETWORK_TCP_NODELAY,				//int					����Nagle�㷨������������ʱ

	OPT_NETWORK_GET_SO_ERROR,				//int					��ȡ�׽��ִ���

	OPT_NETWORK_GET_HANDLE_COUNT,			//int					��ȡ�������
	
	OPT_NETWORK_SET_KEEPLIVE_TIMEOUT,		//int					����һ������N����û���շ�����Ϊ��ʱ

	OPT_NETWORK_GET_KEEPLIVE_TIMEOUT,		//int					��ȡ�շ����ݳ�ʱʱ��,���û����Ϊ0,û�г�ʱ
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
	//�������
	virtual int on_connect(int32_t, mw_connect*) {return 0;}

	//�ɶ�  int count Ϊ�ɶ��ֽ��� 
	virtual	int on_read(mw_connect* con, int32_t count) = 0;

	//��д int count Ϊ����Ҫ���͵��ֽ���
	virtual int on_write(mw_connect* con, int32_t count) = 0;

	//�ر�����
	virtual int on_disconnect(int32_t reason, mw_connect* con)	= 0;

	//�����ͷ�
	virtual int on_release(mw_connect* con) = 0;
	
	//�����յ����߳����������ź�
	virtual void on_signal(mw_connect* con) = 0;

	//�����̱߳����Ȱ�
	virtual void on_rebind(mw_connect* con) {}
};

//���Ӷ���
class mw_connect
{
public:
	//���ûص���
	virtual void _set_sink(mw_connect_sink* psink) = 0;	
	
	//�ͻ������ӣ�Ĭ��Ϊ�첽����
	virtual int _connect(const char* addr, uint16_t port, int asyn = 1) = 0;

	//��������
	virtual int _send(unsigned char* date, int32_t size) = 0;
	
	//�����׽��ֲ���
	virtual int _set_opt(int32_t type, void* arg, int32_t size) = 0;
	
	//��ȡ�׽��ֲ���
	virtual int _get_opt(int32_t type, void* arg, int32_t size) = 0;

	//�Ͽ�����
	virtual int _disconnect(void) = 0;

	//�Ƿ����� 0�Ͽ� 1����
	virtual int _isconnect(void) = 0;

	//�������Ӷ���
	virtual int _release(void) = 0;

	//���߳��첽������Ϣ
	virtual int _signal(void) = 0;

	//���Ȱ��߳�
	virtual int _rebind(thread_t bind_tid) = 0;
};

class mw_acceptor;
class mw_acceptor_sink
{
public:
	//ĳһ�˿ڽ��յ��µ�����
	virtual int on_accept(mw_connect* con, uint16_t port) = 0;

	//ĳһ�˿�ֹͣ����
	virtual int on_stop(uint16_t port) = 0;

	//��������
	virtual int on_release(mw_acceptor* acceptor) = 0;
};

//���ӽ�����
class mw_acceptor
{
public:
	//��ʼ����һ���˿�
	virtual int _start_listen(uint16_t port, uint32_t addr = 0) = 0;

	//ֹͣ����һ���˿�
	virtual int _stop_listen(void) = 0;

	//�������ӽ�����
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

//ͨ���̳߳�ʼ���ص����� network_init ֮ǰ
EXTERN_C NETWORK_API void network_thread_cb(mw_thread_cb* cb);

//��ʼ��ͨ�Ų�
EXTERN_C NETWORK_API int network_init(int32_t threads = 8, _output* out = 0);

//����ͨ�Ų�
EXTERN_C NETWORK_API void network_fini(void);

//ͨ������ѭ��
EXTERN_C NETWORK_API void network_dispatch(void);

//����һ������
EXTERN_C NETWORK_API mw_connect* create_connect(mw_connect_sink* sink, e_class_connect e = class_raw_tcp);

//�������ӽӹ���
EXTERN_C NETWORK_API mw_acceptor* create_acceptor(mw_acceptor_sink* sink, e_class_connect e = class_raw_tcp);

//����һ����ʱ��
EXTERN_C NETWORK_API mw_net_timer* create_net_timer(mw_net_timer_sink* sink);

//����һ���ܵ�
EXTERN_C NETWORK_API mw_net_pipe* create_net_pipe(mw_net_pipe_sink* sink);



#endif //_NETWORK_H_

