#include "evthread.h"

void evthread::Execute()
{
	//1.��ʼ�����̶߳���
	s_log()->info("network thread %s start.\r\n", 
		event_base_get_method(TlsSingelton<c_basis>::tlsInstance()->base()));
	m_reading = true;


	TlsSingelton<c_basis>::tlsInstance()->dispatch();


	s_log()->info("network thread end.\r\n");
}
