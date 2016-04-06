
#ifndef _sqlite_h_
#define _sqlite_h_

#define  RESULT_CHILD				0XF0		//����
#define  RESULT_ROW					0XF1		//��
#define  RESULT_COL					0XF2		//��
#define  RESULT_CURRENT_ROW			0XF3		//��ǰ��
#define  RESULT_CODE				0XFE		//���ش������
#define  RESULT_ERROR				0XFF		//���ش���

#define  ERROR_REMOTE_NOT_CONNECT	(-1)		//Զ������δ�ɹ�

typedef TLV::alloc_block<unsigned short> ALLOC_BLOCK;
typedef TLV::container<unsigned char, unsigned short, ALLOC_BLOCK> sqlite_tlv;

#ifdef _MSC_VER

#ifdef SQLITE_EXPORTS
#define DB_API __declspec(dllexport)
#else
#define DB_API __declspec(dllimport)
#endif
#define EXTERN_C	extern "C"
#else

#define SQLITE_EXPORTS

#define EXTERN_C 	extern "C"
#define DB_API			
#endif

namespace sqlite_db
{
	class _sqlite
	{
	public:
		virtual int db_open(const char* filename) = 0;
		virtual int db_exec(const char* sql, sqlite_tlv* result) = 0;
		virtual int db_query(const char* sql, sqlite_tlv* result) = 0;
		virtual int	db_commit()	= 0;
		virtual int db_close() = 0;
		virtual const char* error() = 0;
	};

	class _sqlite_remote
	{
	public:
		virtual int db_exec(int req, const char* sql) = 0;
		virtual int db_query(int req, const char* sql) = 0;
	};

	class _sqlite_remote_sink
	{
	public:
		typedef enum
		{
			sqlite_connect_success	= 0,		//�������ݿ�ɹ�
			sqlite_connect_fail,				//�������ݿ�ʧ��
			sqlite_disconnect,					//���ݿ����ӶϿ�
		}sqlite_state;
		virtual int	sqlite_state_result(sqlite_state respon) = 0;
		virtual int remote_db_on_result(int req, sqlite_tlv* result) = 0;
	};
	
	EXTERN_C DB_API	_sqlite* get_sqlite();

	EXTERN_C DB_API int start_sqlite_monitor(unsigned short listen);

	EXTERN_C DB_API _sqlite_remote* create_sqlite_connector(const char* ipaddress,
		unsigned short port, _sqlite_remote_sink* sink);
	
	EXTERN_C DB_API uint64_t sqlite_memory_used();
	
}


#endif //_sqlite_h_
