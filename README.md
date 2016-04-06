# zstorage
云存储服务器

## 测试环境

  云服务器 ： telnet 172.25.2.14 7777 
       
## 预览好友消息

### 用法：
      TTL 0 KEY CLOUD{我的用户ID} CLOUD LIMIT {好友用户ID} {当前位置} {条数} 
    我的ID(1241304)   好友的ID(1241305)

### 示例：
    1)从最后开始拉取我的好友聊天记录10条。 
      TTL 0 KEY CLOUD1241304 CLOUD LIMIT 1241305 0 -10 

    2)从5位置向下拉10条消息。 
      TTL 0 KEY CLOUD1241304 CLOUD LIMIT 1241305 5 10 

    3)从最后开始拉取我的系统消息10条。 
      TTL 0 KEY CLOUD1241304 CLOUD LIMIT SYSTEM 0 -10 

    4)从最后开始拉取我的推送消息10条。  
	  TTL 0 KEY CLOUD1241304 CLOUD LIMIT PUSH 0 -10 

## 预览群消息 

### 用法：
    TTL 0 KEY GROUP_CLOUD{群ID} CLOUD LIMIT {群ID} {当前位置} {条数} 
    群的ID(100491)
### 示例：
    TTL 0 KEY GROUP_CLOUD100491 CLOUD LIMIT 100491 0 -10 
