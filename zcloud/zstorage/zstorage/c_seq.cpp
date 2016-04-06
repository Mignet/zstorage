#include "stdafx.h"
#include "c_seq.h"
#include "c_timeout.h"

//#define SEQ_RINGQUEUE()		T_RINGQUEUE(8192) 

class T_SEQ_BUFFER
{
public:
	T_SEQ_BUFFER()
	{
		buf = (char*)malloc(MAX_VALUE_LEN);
	}
	~T_SEQ_BUFFER()
	{
		free(buf);
	}
	char*	buf;
};

#define TSEQ_BUF() TLS(T_SEQ_BUFFER)->buf

#define SEQ_INIT() \
	if (!init()){ \
	io->push(ZRESULT_ERROR); \
	io->push("seq init error."); \
	return;	}

c_seq::c_seq(const char* key)
: m_init(false)
, m_key(key)
{
	m_effective = time(0) + c_server::get_instance()->object_effective();
	memset(&m_seq_head, 0, sizeof(_SEQ_HEAD));
	m_seq_head.m_head.type = T_SEQ;
	m_seq_head.m_head.s_effective(PERMANENT_BASE);
	m_seq_head.s_delay(0);
	m_seq_head.s_cleantime(time(0));
}


c_seq::~c_seq()
{
}

bool c_seq::load_value(const string& value, _zmsg_head* head)
{
	if ( value.length() != sizeof(_SEQ_HEAD) )
		return false;

	int len = value.length();
	char* ptr = (char*)value.c_str();
	if (head->type != T_SEQ)
		return false;

	memcpy(&m_seq_head, ptr, len);

	return true;
}

bool c_seq::init()
{
	m_effective = time(0) + c_server::get_instance()->object_effective();
	c_wlock lock(&m_lock);
	if (m_init)
		return true;

	string value;
	_zmsg_head* head = NULL;
	if (c_base::get_value(m_key, value, &head) == 0)
	{
		if (!load_value(value, head))
		{
			return false;
		}
	}

	m_init = true;

	return true;
}

void c_seq::seq_update(leveldb::WriteBatch& bh)
{
	m_seq_head.s_uptime(time(0));
	m_seq_head.m_head.s_crc(c_crc::crc16(0, 
		(uint8*)&m_seq_head + sizeof(_zmsg_head), 
		sizeof(m_seq_head) - sizeof(_zmsg_head)));
	leveldb::Slice value( (const char*)&m_seq_head, sizeof(m_seq_head) );
	bh.Put(m_key, value);
}

void c_seq::seq_set(leveldb::WriteBatch& bh, leveldb::Slice& value)
{
	uint32 keyId = m_seq_head.g_index() + 1;

	m_seq_head.s_index(keyId);
	m_seq_head.s_count(m_seq_head.g_count() + 1);

	_zmsg_head head;
	head.type = T_SEQ_VALUE;
	head.s_effective(time(0));
	head.s_crc(c_crc::crc16(0, (uint8*)value.data(), value.size()));

	memcpy(TSEQ_BUF(), &head, sizeof(_zmsg_head));
	memcpy(TSEQ_BUF() + sizeof(_zmsg_head), (void*)value.data(), value.size());
	int len = sizeof(_zmsg_head) + value.size();
	if (len > 0)
	{
		leveldb::Slice data(TSEQ_BUF(), len);
		bh.Put(__tos(m_key << "@" << keyId), data);
	}
}

bool c_seq::seq_get(uint32 keyId, string& value)
{
	_zmsg_head* head = NULL;
	if (c_base::get_value(__tos(m_key << "@" << keyId), value, &head) != 0)
	{
		return false;
	}

	if (head->type != T_SEQ_VALUE)
		return false;

	return true;
}

void c_seq::seq_del(uint32 keyId, leveldb::WriteBatch& bh)
{
	bh.Delete(__tos(m_key << "@" << keyId));
	m_seq_head.s_count(m_seq_head.g_count() - 1);
	//LOG4_ERROR("SEQ DEL KEY:" << __tos(m_key << "@" << keyId));
}

bool c_seq::seq_exists(uint32 keyId)
{
	string value;
	return seq_get(keyId, value);
}

uint32 c_seq::seq_begin_index()
{
	return m_seq_head.g_index() - m_seq_head.g_count() + 1;
}



void c_seq::push(char* data, int n, wisdom_IOStream& io, uint32 idelay)
{
	SEQ_INIT();
	c_wlock lock(&m_lock);

	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;

	uint32 count = m_seq_head.g_count();
	uint32 index = m_seq_head.g_index();
	uint32 uptime = m_seq_head.g_uptime();
	uint32 delay = m_seq_head.g_delay();

	leveldb::Slice value(data, n);
	seq_set(bh, value);

	if (idelay > 0)
		m_seq_head.s_delay(idelay);

	seq_update(bh);

	if (c_base::write(op, bh))
	{
		io->push(ZRESULT_OK);
		io->push(itostr(m_seq_head.g_index()));
	}
	else
	{
		m_seq_head.s_count(count);
		m_seq_head.s_index(index);
		m_seq_head.s_uptime(uptime);
		m_seq_head.s_delay(delay);
		io->push(ZRESULT_ERROR);
		io->push("write error.");
	}
}

void c_seq::index_(wisdom_IOStream& io)
{
	SEQ_INIT();
	if (is_clean())
	{
		wisdom_IOStream os = new c_ostream_array;
		remove(os);
	}

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	io->push(itostr(m_seq_head.g_index()));
}

void c_seq::size(wisdom_IOStream& io)
{
	SEQ_INIT();

	if (is_clean())
	{
		wisdom_IOStream os = new c_ostream_array;
		remove(os);
	}

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	io->push(itostr(m_seq_head.g_count()));
}

void c_seq::remove(wisdom_IOStream& io)
{
	SEQ_INIT();
	if (!is_clean())
		return;
	c_wlock lock(&m_lock);
	uint32 del_count = 0;
	uint64_t ms = get_ms();
	uint32 last = m_seq_head.g_cleantime();
	uint32 count = m_seq_head.g_count();
	uint32 ibegin = seq_begin_index();

	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	
	while (true)
	{
		string value;
		_zmsg_head* head = NULL;
		if (c_base::get_value(__tos(m_key << "@" << ibegin), value, &head) != 0)
		{
			break;
		}

		if (head->type != T_SEQ_VALUE)
		{
			LOG4_ERROR("SEQ KEY:" << __tos(m_key << "@" << ibegin) << " error.");
			break;
		}

		uint32 idelay = m_seq_head.g_delay() == 0 ? c_server::get_instance()->seq_valid_time() : m_seq_head.g_delay();
		if (head->g_effective() + idelay > time(0))
		{
			break;
		}

		seq_del(ibegin, bh);
		del_count++;
		
		ibegin++;
	}
	
	m_seq_head.s_cleantime(time(0));
	seq_update(bh);
	if (!c_base::write(op, bh))
	{
		m_seq_head.s_count(count);
		ZERROR_RESULT("seq write error.");
	}

	io->push(ZRESULT_OK);

	LOG4_INFO("SEQ " << m_key << " clean record:" << del_count 
		<< " last min " << ((time(0) - last) / 60)
		<< " cost " << (uint32)(get_ms() - ms) << " ms");
}

void c_seq::seek(uint32 keyId, int index, wisdom_IOStream& io)
 {
	SEQ_INIT();

	if (is_clean())
	{
		wisdom_IOStream os = new c_ostream_array;
		remove(os);
	}

	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);

	if (keyId == 0)
	{
		keyId = m_seq_head.g_index();
	}

	bool down_up = index > 0 ? true : false;
	index = abs(index);

	for (int i = 0; i < index; i++)
	{
		string value;
		if (!seq_get(keyId, value))
		{
			break;
		}
		char* ptr = (char*)value.c_str() + sizeof(_zmsg_head);
		int len = value.length() - sizeof(_zmsg_head);
		_zmsg_head* h = (_zmsg_head*)value.c_str();
		c_time t(h->g_effective());
		io->push(__tos(keyId << "," << t.time_stamp() << ","));
		io->push(ptr, len);

		if (down_up)
			keyId++;
		else
			keyId--;
	}
}

void c_seq::delay(uint32 idelay, wisdom_IOStream& io)
{
	SEQ_INIT();
	c_wlock lock(&m_lock);

	uint32 delay_ = m_seq_head.g_delay();

	m_seq_head.s_delay(idelay);
	leveldb::WriteOptions op;
	leveldb::WriteBatch bh;
	seq_update(bh);

	if (!c_base::write(op, bh))
	{
		m_seq_head.s_delay(delay_);
		ZERROR_RESULT("seq write error.");
	}

	io->push(ZRESULT_OK);
}


void c_seq::lookdelay(wisdom_IOStream& io)
{
	SEQ_INIT();
	c_rlock lock(&m_lock);
	io->push(ZRESULT_OK);
	uint32 idelay = m_seq_head.g_delay() == 0 ? c_server::get_instance()->seq_valid_time() : m_seq_head.g_delay();

	io->push(itostr(idelay));
}

void c_seq::query(uint32 datetime, wisdom_IOStream& io)
{
	SEQ_INIT();
	if (is_clean())
	{
		wisdom_IOStream os = new c_ostream_array;
		remove(os);
	}

	c_rlock lock(&m_lock);

	uint32 begin_index = seq_begin_index();
	uint32 end_index = m_seq_head.g_index();

	/*int last_diff = 0;
	uint32 last_key = 0;*/
	uint32 ibegin = 0;
	uint32 iend = 0;
	map<int, uint32> mfind;
	int diff = 0;
	while (true)
	{
		
		if (diff > 0)
		{
			ibegin = ibegin;
			iend = ibegin + (iend - ibegin) / 2;
		}
		else if (diff < 0)
		{
			ibegin = ibegin + (iend - ibegin) / 2;
			iend = iend;
		}
		else
		{
			ibegin = begin_index;
			iend = end_index;
		}
		
		uint32 keyId = dichotomy(datetime, ibegin, iend, diff, io);
		if (keyId == 0)
			break;

		//exists
		if (diff == 0)
		{
			io->push(ZRESULT_OK);
			io->push(itostr(keyId));
			return;
		}

		
		mfind.insert(make_pair(diff, keyId));
		//no data find
		if (keyId == ibegin || keyId == iend)
		{
			break;
		}
	}

	for (map<int, uint32>::iterator pos = mfind.begin(); pos != mfind.end(); ++pos)
	{
		if (pos->first > 0 || pos == --mfind.end())
		{
			io->push(ZRESULT_OK);
			io->push(itostr(pos->second));
			return;
		}
	}

	io->push(ZRESULT_ERROR);
}


bool c_seq::is_release()
{
	return (uint32)time(0) > m_effective;
}

int c_seq::dichotomy(uint32 datetime, uint32 begin, uint32 end, int& diff, wisdom_IOStream& io)
{
	string value;
	_zmsg_head* head = NULL;
	uint32 keyId = diff < 0 ? (begin + (end - begin) / 2 + 1) : (begin + (end - begin) / 2);
	if (c_base::get_value(__tos(m_key << "@" << keyId), value, &head) != 0)
	{
		return 0;
	}

	if (head->type != T_SEQ_VALUE)
	{
		io->push(ZRESULT_ERROR);
		io->push(_zerror_str(_Zerror_seq_bad));
		return 0;
	}

	diff = head->g_effective() - datetime;

	return keyId;
}

bool c_seq::is_clean()
{
	return time(0) - m_seq_head.g_cleantime() > c_server::get_instance()->seq_clear_time();
}

