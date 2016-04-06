#ifndef zcloud_sdk_h
#define zcloud_sdk_h

namespace zcloud_sdk
{
	//上线
	namespace msg_online
	{
		enum {
			/*string*/		source		= 1,	//来源
			/*string*/		beginId		= 2,	//开始索引
			/*string*/		endId		= 3,	//结束索引
			/*string*/		ptime		= 4,	//结束索引时间
			/*byte*/		packet		= 5,	//结束索引内容
			/*uint32*/		unread		= 6,	//未读数
		};
	}

	//上线
	namespace msg_seek
	{
		enum {
			/*string*/		indexId		= 1,	//序号
			/*string*/		ptime		= 2,	//时间
			/*byte*/		packet		= 3,	//内容
		};
	}
}

#endif
