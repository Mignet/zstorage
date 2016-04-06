#ifndef help_h
#define help_h

/*
i.TTL
TTL {0}			
说明：路由协议，0表示序号，序号可任意表示，返回第一个字段为序号
ii.KEY
KEY {list_key}	
说明：list_key为KEY 内容


1.GET
TTL 0 KEY kye_123 GET
说明:获取kye_123 所对应值

2.SET
TTL 0 KEY kye_123 SET 000
说明:设置kye_123值为000
TTL 0 KEY kye_123 SET 000 EXPIRE 20
说明:设置kye_123值为000 20秒后自动清除

3.EXISTS
TTL 0 KEY kye_123 EXISTS
说明:存在返回1 不存在0

4.DEL
TTL 0 KEY kye_123 DEL
说明:OK表示删除成功

5.LIST 队列
1).PUSH 
TTL 0 KEY list_123 LIST PUSH 0
说明:往队列list_123 压入一个 0

2).POP
TTL 0 KEY list_123 LIST POP
说明:向队列list_123 弹出最前面一个值，并在队列中移除

3).SIZE
TTL 0 KEY list_123 LIST SIZE
说明:获取队列长度

4).LIMIT
TTL 0 KEY list_123 LIST LIMIT -1
说明:获取队列所有值

5).EXPIRE
TTL 0 KEY list_123 LIST EXPIRE 5
说明:设置队列5秒钟后自动删除

6).DEL
TTL 0 KEY list_123 LIST DEL
说明:删除队列

7).TIME
TTL 0 KEY list_123 LIST TIME
说明:获取当前队列生命周期单位秒

7).UPDATE
TTL 0 KEY list_123 LIST UPDATE [val_1] [val_2] [...] 
说明:把队列更新为最新数据，老数据清空

8).SEEK {x} LIMIT {y}
TTL 0 KEY list_123 LIST SEEK 10 LIMIT 3
说明:从队列第十条开始，获取3条最新记录

5.MLIST 键值队列
1).PUSH
TTL 0 KEY list_123 MLIST PUSH mkey_123 value_123
说明:压入队列 子Key为 mkey_123 内容为 value_123

2).SIZE
TTL 0 KEY list_123 MLIST SIZE
说明:获取当前队列大小

3).SEEK
TTL 0 KEY list_123 MLIST SEEK mkey_123 99
说明:从子Key mkey_123之后获取最大99条记录

4).EXPIRE
TTL 0 KEY list_123 MLIST EXPIRE 5
说明:设置队列5秒钟后自动删除

5).DEL
TTL 0 KEY list_123 MLIST DEL
说明:删除队列

6).TIME
TTL 0 KEY list_123 MLIST TIME
说明:获取当前队列生命周期单位秒


6.SEQ 序列

1).PUSH
TTL 0 KEY seq_123 SEQ PUSH value_123 {时间单位:分}
说明:压入序列 seq_123 内容为 value_123 返回当前序号编号

2).SIZE
TTL 0 KEY seq_123 SEQ SIZE
说明:获取当前序列大小

3).INDEX
TTL 0 KEY seq_123 SEQ INDEX
说明:获取当前序列编号

4).SEEK {keyId} {y}
TTL 0 KEY seq_123 SEQ SEEK [keyId] [count]
说明:从序列 keyId 位置获取记录数 count 正数为 向下获取， count 负数为向上获取

5).DELAY
TTL 0 KEY seq_123 SEQ DELAY [3600]
说明:设置SEQ窗口 3600秒为超时

6).LOOKDELAY
TTL 0 KEY seq_123 SEQ LOOKDELAY
说明:查看当前窗口的延时

7).QUERY
TTL 0 KEY seq_123 SEQ QUERY 2015-1-19 17:30
说明:查询时间内最近的一条消息ID

6.CLOUD 云消息
1).PUSH
TTL 0 KEY cloud_123 CLOUD PUSH keyword value_123
说明:在云容器中子序列keyword中压入一个元素

2).LIMIT
TTL 0 KEY cloud_123 CLOUD LIMIT keyword index 10
说明:在云容器中子序列keyword index 位置向下查询10条记录

2).QUERY
TTL 0 KEY cloud_123 CLOUD QUERY keyword 2015-1-20 10
说明:在云容器中子序列keyword 2015-1-20时间点位置向下查询10条记录


4).ONLINE
TTL 0 KEY cloud_123 CLOUD ONLINE	id,id,id,
说明:云容器专用协议用于IM下线

5).SIZE
TTL 0 KEY cloud_123 CLOUD SIZE keyword,
说明:获取子容器记录数

6）READ
TTL 0 KEY cloud_123 CLOUD READ keyword id
说明:消息如果已经阅读过，把ID写入DB
*/
#endif
