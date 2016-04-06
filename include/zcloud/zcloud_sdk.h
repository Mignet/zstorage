#ifndef zcloud_sdk_h
#define zcloud_sdk_h

namespace zcloud_sdk
{
	//����
	namespace msg_online
	{
		enum {
			/*string*/		source		= 1,	//��Դ
			/*string*/		beginId		= 2,	//��ʼ����
			/*string*/		endId		= 3,	//��������
			/*string*/		ptime		= 4,	//��������ʱ��
			/*byte*/		packet		= 5,	//������������
			/*uint32*/		unread		= 6,	//δ����
		};
	}

	//����
	namespace msg_seek
	{
		enum {
			/*string*/		indexId		= 1,	//���
			/*string*/		ptime		= 2,	//ʱ��
			/*byte*/		packet		= 3,	//����
		};
	}
}

#endif
