#pragma once

#ifndef c_pipe_h
#define c_pipe_h

#include "stdafx.h"

class c_pipe
	: public mw_net_pipe
	, public CMPool
{
public:
	c_pipe(mw_net_pipe_sink* sink);
	~c_pipe(void);
private:
	virtual int _create();
	virtual int _notify(const char* data, int size);
	virtual void _cancel();
	virtual void _release();
	
	void process(int fd, short which);
	static void libevent_process(int fd, short which, void *arg);
private:
	int					m_pipe[2];
	event				m_ev;
	mw_net_pipe_sink*	m_sink;
	char				m_buf[1024];
};



#endif //

