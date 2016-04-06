#ifndef help_h
#define help_h

/*
i.TTL
TTL {0}			
˵����·��Э�飬0��ʾ��ţ���ſ������ʾ�����ص�һ���ֶ�Ϊ���
ii.KEY
KEY {list_key}	
˵����list_keyΪKEY ����


1.GET
TTL 0 KEY kye_123 GET
˵��:��ȡkye_123 ����Ӧֵ

2.SET
TTL 0 KEY kye_123 SET 000
˵��:����kye_123ֵΪ000
TTL 0 KEY kye_123 SET 000 EXPIRE 20
˵��:����kye_123ֵΪ000 20����Զ����

3.EXISTS
TTL 0 KEY kye_123 EXISTS
˵��:���ڷ���1 ������0

4.DEL
TTL 0 KEY kye_123 DEL
˵��:OK��ʾɾ���ɹ�

5.LIST ����
1).PUSH 
TTL 0 KEY list_123 LIST PUSH 0
˵��:������list_123 ѹ��һ�� 0

2).POP
TTL 0 KEY list_123 LIST POP
˵��:�����list_123 ������ǰ��һ��ֵ�����ڶ������Ƴ�

3).SIZE
TTL 0 KEY list_123 LIST SIZE
˵��:��ȡ���г���

4).LIMIT
TTL 0 KEY list_123 LIST LIMIT -1
˵��:��ȡ��������ֵ

5).EXPIRE
TTL 0 KEY list_123 LIST EXPIRE 5
˵��:���ö���5���Ӻ��Զ�ɾ��

6).DEL
TTL 0 KEY list_123 LIST DEL
˵��:ɾ������

7).TIME
TTL 0 KEY list_123 LIST TIME
˵��:��ȡ��ǰ�����������ڵ�λ��

7).UPDATE
TTL 0 KEY list_123 LIST UPDATE [val_1] [val_2] [...] 
˵��:�Ѷ��и���Ϊ�������ݣ����������

8).SEEK {x} LIMIT {y}
TTL 0 KEY list_123 LIST SEEK 10 LIMIT 3
˵��:�Ӷ��е�ʮ����ʼ����ȡ3�����¼�¼

5.MLIST ��ֵ����
1).PUSH
TTL 0 KEY list_123 MLIST PUSH mkey_123 value_123
˵��:ѹ����� ��KeyΪ mkey_123 ����Ϊ value_123

2).SIZE
TTL 0 KEY list_123 MLIST SIZE
˵��:��ȡ��ǰ���д�С

3).SEEK
TTL 0 KEY list_123 MLIST SEEK mkey_123 99
˵��:����Key mkey_123֮���ȡ���99����¼

4).EXPIRE
TTL 0 KEY list_123 MLIST EXPIRE 5
˵��:���ö���5���Ӻ��Զ�ɾ��

5).DEL
TTL 0 KEY list_123 MLIST DEL
˵��:ɾ������

6).TIME
TTL 0 KEY list_123 MLIST TIME
˵��:��ȡ��ǰ�����������ڵ�λ��


6.SEQ ����

1).PUSH
TTL 0 KEY seq_123 SEQ PUSH value_123 {ʱ�䵥λ:��}
˵��:ѹ������ seq_123 ����Ϊ value_123 ���ص�ǰ��ű��

2).SIZE
TTL 0 KEY seq_123 SEQ SIZE
˵��:��ȡ��ǰ���д�С

3).INDEX
TTL 0 KEY seq_123 SEQ INDEX
˵��:��ȡ��ǰ���б��

4).SEEK {keyId} {y}
TTL 0 KEY seq_123 SEQ SEEK [keyId] [count]
˵��:������ keyId λ�û�ȡ��¼�� count ����Ϊ ���»�ȡ�� count ����Ϊ���ϻ�ȡ

5).DELAY
TTL 0 KEY seq_123 SEQ DELAY [3600]
˵��:����SEQ���� 3600��Ϊ��ʱ

6).LOOKDELAY
TTL 0 KEY seq_123 SEQ LOOKDELAY
˵��:�鿴��ǰ���ڵ���ʱ

7).QUERY
TTL 0 KEY seq_123 SEQ QUERY 2015-1-19 17:30
˵��:��ѯʱ���������һ����ϢID

6.CLOUD ����Ϣ
1).PUSH
TTL 0 KEY cloud_123 CLOUD PUSH keyword value_123
˵��:����������������keyword��ѹ��һ��Ԫ��

2).LIMIT
TTL 0 KEY cloud_123 CLOUD LIMIT keyword index 10
˵��:����������������keyword index λ�����²�ѯ10����¼

2).QUERY
TTL 0 KEY cloud_123 CLOUD QUERY keyword 2015-1-20 10
˵��:����������������keyword 2015-1-20ʱ���λ�����²�ѯ10����¼


4).ONLINE
TTL 0 KEY cloud_123 CLOUD ONLINE	id,id,id,
˵��:������ר��Э������IM����

5).SIZE
TTL 0 KEY cloud_123 CLOUD SIZE keyword,
˵��:��ȡ��������¼��

6��READ
TTL 0 KEY cloud_123 CLOUD READ keyword id
˵��:��Ϣ����Ѿ��Ķ�������IDд��DB
*/
#endif
