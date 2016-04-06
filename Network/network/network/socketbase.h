#pragma once

#ifndef _SOCKETBASE_H_
#define _SOCKETBASE_H_

class CSocketBase
	: public CRef
{
public:
	CSocketBase(void);
	virtual ~CSocketBase(void);
	int open_sock();
	void set_close();
	bool is_close() { return m_onclose; }
public:
	virtual void on_close(int error) = 0;
	virtual void on_release() = 0;
protected:
	//设置套接字参数
	virtual int32_t _set_opt(int32_t type, void* arg, int32_t size);
	//获取套接字参数
	virtual int32_t _get_opt(int32_t type, void* arg, int32_t size);
protected:
	int		m_fd;
	bool	m_onclose;
};

#endif
