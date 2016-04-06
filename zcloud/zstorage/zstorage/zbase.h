
#ifndef zbase_h
#define zbase_h
#pragma pack(push)
#pragma pack(1)

#define VERSION_BASE			1
#define PERMANENT_BASE			0XFFFFFFFF
#define MAX_VALUE_LEN			(1024 *1024 * 64)	//32MB

#define ZRESULT_OK					"OK"
#define ZRESULT_ERROR				"ERROR"


enum _zerror_
{
	_zerror_not_extsis				= 1,
	_zerror_data_bad				= 2,
	_zerror_crc_bad					= 3,
	_zerror_type_bad				= 4,
	_zerror_illegal_access			= 5,
	_zerror_not_find_data			= 6,
	_Zerror_seq_bad					= 7,
};

/*TTL SERIAL EXEC KEY GET*/
enum ___cmd
{
	_TTL		= 0,
	_PING		= 2,
	_CHECKDB	= 3,
	_CLEANUP	= 4,
	_INFO		= 5,
};

enum _type
{
	T_GET_SET	= 1,
	T_HASH		= 2,
	T_LIST		= 3,
	T_MLIST		= 4,
	T_MAP		= 5,
	T_SEQ		= 6,
	T_SEQ_VALUE	= 7,
};

//��Ϣ�洢ͷ
typedef struct _zmsg_head
{
	_zmsg_head()
		: version(VERSION_BASE)
	{
		s_effective(PERMANENT_BASE);
	}
	void s_effective(uint32 effective) { m_effective = ntohl(effective); }
	void s_crc(uint16 crc) { m_crc = ntohs(crc); }

	uint32 g_effective() { return htonl(m_effective); }
	uint16 g_crc() { return htons(m_crc); }
	bool check_type() {
		switch (type)
		{
		case T_GET_SET:
		case T_HASH:
		case T_LIST:
		case T_MLIST:
		case T_MAP:
		case T_SEQ:
		case T_SEQ_VALUE:
			return true;
		default:
			break;
		}

		return false;
	}

	uint8	version;			//�汾��
	uint8	type;				//����
	uint32	m_effective;		//��Ч��
	uint16	m_crc;				//����crc16ֵ
	uint32	m_reserve;			//Ԥ��
}_zmsg_head;

//��������ͷ����
typedef struct _SEQ_HEAD
{
	_SEQ_HEAD() :m_count(0), m_index(0), m_uptime(0), m_delay(0) {}
	void s_count(uint32 count) { m_count = ntohl(count); }
	void s_index(uint32 index) { m_index = ntohl(index); }
	void s_uptime(uint32 uptime) { m_uptime = ntohl(uptime); }
	void s_delay(uint32 delay) { m_delay = ntohl(delay); }
	void s_cleantime(uint32 cleantime) { m_cleantime = ntohl(cleantime); }
	uint32 g_count() { return htonl(m_count);}
	uint32 g_index() { return htonl(m_index);}
	uint32 g_uptime() { return htonl(m_uptime);}
	uint32 g_cleantime() { return htonl(m_cleantime); }
	uint32 g_delay() { return htonl(m_delay); }
	_zmsg_head	m_head;
	uint32		m_count;
	uint32		m_index;
	uint32		m_uptime;
	uint32		m_cleantime;		//�ϴ�����ʱ��
	uint32		m_delay;
	
}_SEQ_HEAD;


#pragma pack(pop)
#endif
