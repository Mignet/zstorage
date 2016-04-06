#include "stdafx.h"
#include "c_zredis_connect.h"
#include "c_config.h"

c_zredis_connect::c_zredis_connect(mw_connect* con)
	: m_con(con)
	, m_power(0x1)
	, m_port(0)
	, m_Tag(NULL)
	, m_Bind(NULL)
	, m_tid(_tid())
	, m_asyn_disconnect(false)
{
	if (!m_con)
		return;

	m_con->_set_sink(this);

	c_zredis_admin::get_instance()->push(this);

	int timeout = 500;
	con->_set_opt(OPT_NETWORK_SET_KEEPLIVE_TIMEOUT, &timeout, sizeof(timeout));

	int sendbuffer = 64 * 1024;
	m_con->_set_opt(OPT_NETWORK_IP_SET_SENDBUFFER, &sendbuffer, sizeof(sendbuffer));

	sendbuffer = 64 * 1024;
	m_con->_set_opt(OPT_NETWORK_LOCAL_SET_SENDBUFFER, &sendbuffer, sizeof(sendbuffer));

	int tcp_packet_size = 1400;
	m_con->_set_opt(OPT_NETWORK_SET_PACKET_SIZE, &tcp_packet_size, sizeof(tcp_packet_size));

	//int timeout = 5;
	//con->_set_opt(OPT_NETWORK_SET_KEEPLIVE_TIMEOUT, &timeout, sizeof(timeout));


	char addr[16];
	unsigned short uport = 0;
	int handler_count = 0;
	m_con->_get_opt(OPT_NETWORK_REMOTE_ADDR, addr, sizeof(addr));
	m_con->_get_opt(OPT_NETWORK_REMOTE_PORT, &uport, sizeof(uport));
	m_ip = addr;
	m_port = uport;


	c_iplist::get_instance()->push(this);

	char szIP[50];
	sprintf(szIP, "%s:%u", addr, uport);
	//获取广播对象
	m_broadcast = c_broadcast_admin::get_instance()->get(szIP);

	m_con->_get_opt(OPT_NETWORK_GET_HANDLE_COUNT, &handler_count, sizeof(handler_count));
	static int shandler_count = 0;
	if (handler_count > shandler_count)
		shandler_count = handler_count;

	finf("this:" << _this << "|zredis_connect ip:" << szIP << "|handler:" << handler_count);
}


c_zredis_connect::~c_zredis_connect(void)
{
}

#define READ_LEN	4096
int c_zredis_connect::on_read( mw_connect* con, int count )
{
	char packet[READ_LEN];
	int packet_len = 0;
	if (count <= 0)
		return 0;

	//接收所有数据
	while (count > 0)
	{
		size_t ptr = 0;
		int len = min(READ_LEN, count);
		con->_get_opt(OPT_NETWORK_GET_RECV, &ptr, len);

		m_stream.recv((char*)ptr, len);

		memcpy(packet, (char*)ptr, len);
		packet_len = len;

		con->_set_opt(OPT_NETWORK_SET_RECV, &len, sizeof(int));
		count -= len;
	}
	


	for(int i = 0; true; i++)
	{
		wisdom_param param;
		int result = m_stream.analyze(param, this);
		if (result == STREAM_ERROR)
		{
			//assert(0);
			ferr("ERROR:" << zn::output_16x((uint8*)packet, packet_len));
			ferr("STREAM_ERROR OO");
			con->_disconnect();
			return 0;
		}
		else if (result == STREAM_PARTIAL)
		{
			break;
		}
		else if (result == STREAM_OK)
		{
			c_command::get_instance()->on_command(param, this);
		}
	}

	return 0;
}

int c_zredis_connect::on_write( mw_connect*, int )
{
	on_transmit();

	if (m_Tag)
		m_Tag->on_send(this);
	return 0;
}

int c_zredis_connect::on_disconnect( int, mw_connect* con )
{
	int handler_count = 0;
	con->_get_opt(OPT_NETWORK_GET_HANDLE_COUNT, &handler_count, sizeof(handler_count));

	con->_release();
	return 0;
}

int c_zredis_connect::on_release( mw_connect*  con)
{
	finf("zredis_connect:" << radix16((size_t)this) << "|release");

	c_zredis_admin::get_instance()->erase(this);
	clear_wait();

	//如果有订阅 取消操作
	for (LSUB::iterator pos = m_subscribe.begin(); pos != m_subscribe.end(); ++pos)
	{
		(*pos)->Leave(this);
	}
	m_subscribe.clear();

	int handler_count = 0;
	m_con->_get_opt(OPT_NETWORK_GET_HANDLE_COUNT, &handler_count, sizeof(handler_count));

	c_iplist::get_instance()->erase(this);

	if (m_Tag != NULL)
	{
		m_Tag->release();
		m_Tag = NULL;
	}

	if (m_Bind != NULL)
	{
		m_Bind->unbind(this);
	}
	delete this;
	return 0;
}

void c_zredis_connect::on_signal(mw_connect* con)
{
	if (m_asyn_disconnect)
	{
		m_con->_disconnect();
		m_asyn_disconnect = false;
		return;
	}

	on_transmit();
}


int c_zredis_connect::on_transmit()
{
	if (ring_Count() > g_max_send_buffer)
	{
		finf("c_zredis_connect g_max_send_buffer:" << m_ring.Count() << " this:0x" << this);
		ansy_disconnect();
		return 0;
	}

	while (ring_Count() > 0)
	{
		int len = min(ring_Count(), 1024);
		unsigned char* ptr = NULL;
		ring_Attach(&ptr, len);
		if (m_con->_send(ptr, len) != ERROR_NEWORK_SUCCESS)
		{
			break;
		}

		ring_Advance(len);
	}
	

	return 0;
}

int c_zredis_connect::send( uint8* data, int len )
{
	if (true)
	{
		ring_Append(data, len);
	}
	
	if (m_tid == _tid())
		 on_transmit();
	else
		m_con->_signal();
	return 0;
}

int c_zredis_connect::asyn_send( uint8* data, int len )
{
	if (true)
	{
		ring_Append(data, len);
	}
	
	if (m_tid == _tid())
		on_transmit();
	else
		m_con->_signal();

	return 0;
}

int c_zredis_connect::ansy_packag( uint8* data, int len )
{
	wisdom_IOStream result = new c_ostream_data;
	result->push((char*)data, len);
	RESULT_PARAM(result, this);

	return 0;
}



int c_zredis_connect::get_power()
{
	return m_power;
}

int c_zredis_connect::set_power( uint8 role )
{
	m_power = role;
	return m_power;
}

int c_zredis_connect::subscribe(const string& tag, uint8 type)
{
	finf("this:" << _this << "|SUB tag:" << tag << "|type:" << (uint32)type);
	wisdom_broadcast subscribe = c_broadcast_admin::get_instance()->
		Join(tag, this, type);

	m_subscribe.push_back(subscribe);

	return 0;
}

int c_zredis_connect::unsubscribe( const string& tag )
{
	c_broadcast_admin::get_instance()->
		Leave(tag, this);

	return 0;
}


int c_zredis_connect::broadcast(uint8* data, int len, uint8 type)
{
	if (m_broadcast)
	{
		m_broadcast->broadcast((char *)data, len, type);
	}

	return 0;
}

int c_zredis_connect::random( uint8* data, int len )
{
	if (m_broadcast)
	{
		m_broadcast->random((char *)data, len);
	}

	return 0;
}


bool c_zredis_connect::is_broadcast()
{
	if (!m_broadcast)
		return false;

	return m_broadcast->is_broadcast();
}


int c_zredis_connect::ipaddr(string& ip, uint16& port )
{
	ip = m_ip;
	port = m_port;
	return 0;
}

bool c_zredis_connect::ring_Attach( uint8**buf, int& len )
{
	c_wlock lock(&m_lock);
	
	if (m_ring.Count() < len)
		return false;

	uint8* ptr = NULL;
	m_ring.Attach(&ptr, len);
	*buf = ptr;
	return true;
}

void c_zredis_connect::ring_Advance(int len)
{
	c_wlock lock(&m_lock);
	m_ring.Advance(len);
}

void c_zredis_connect::ring_Clear()
{
	c_wlock lock(&m_lock);
	m_ring.clear();
}



void c_zredis_connect::ring_Append( uint8* buf, int len )
{
	c_wlock lock(&m_lock);
	m_ring.Append(buf, len);
}

int c_zredis_connect::ring_Count()
{
	c_wlock lock(&m_lock);
	return m_ring.Count();
}

int c_zredis_connect::tagW( ITag* tag )
{
	m_Tag = tag;
	return 0;
}

ITag* c_zredis_connect::tagR()
{
	return m_Tag;
}

void c_zredis_connect::bind( IBind* bind )
{
	m_Bind = bind;
}

void c_zredis_connect::setSeqNum( uint32 seqNum )
{
	c_wlock lock(&m_lock);
	uint32 t = time(0);
	m_waits.insert(make_pair(seqNum, t));
}

void c_zredis_connect::clear_wait()
{
	c_wlock lock(&m_lock);
	m_waits.clear();
}

bool c_zredis_connect::checkSeqNum( uint32 seqNum )
{
	c_rlock lock(&m_lock);
	return m_waits.find(seqNum) != m_waits.end();
}

void c_zredis_connect::eraseSeqNum( uint32 seqNum )
{
	c_wlock lock(&m_lock);
	m_waits.erase(seqNum);
}

void c_zredis_connect::ansy_disconnect()
{
	m_asyn_disconnect = true;
	m_con->_signal();
}

