
/*
* Copyright (c) 2005-2010 Tony Zhengjq <tony.zheng@sky-mobi.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSsTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _YUNVA_H_
#define _YUNVA_H_

#pragma once

#ifdef _MSC_VER

#ifdef C_YUNVA_EXPORTS
#define YUNVA_API __declspec(dllexport)
#else
#define YUNVA_API /*__declspec(dllimport)*/
#endif
#define EXTERN_C	extern "C"
#else

#define YUNVA_API
#define EXTERN_C 	extern "C"

#endif

//当前支持加密版本
#define ENCRYPT_SY			1			//对称加密码

#define INOUT
#define IN
#define OUT

#ifdef __cplusplus
extern "C" {
#endif

//创建一个GUID
EXTERN_C YUNVA_API void create_guid(INOUT char* uuid);

//crc16生成
EXTERN_C YUNVA_API unsigned short crc16(IN unsigned short tag, IN char* buf, IN int len);

//http tag + crc
EXTERN_C YUNVA_API unsigned short http_crc_tag(INOUT char* guid);

//对称加密码
/*
	参数:
	1.type 加密类型
	2.key  密钥
	3.in_buf 加密内容
	4.in_size 加密长度
*/
EXTERN_C YUNVA_API char* yunva_encrypt(IN int type, IN const char* key, INOUT char* in_buf, IN int in_size);

//对称加解密
/*
	参数:
	1.type 加密类型
	2.key  密钥
	3.in_buf 解密内容
	4.in_size 解密长度
*/
EXTERN_C YUNVA_API char* yunva_decrypt(IN int type, IN const char* key, INOUT char* in_buf, IN int in_size);


EXTERN_C YUNVA_API unsigned int yunva_create_tag(IN unsigned short yunva_tag, IN char* ticket);

#ifdef __cplusplus
}
#endif


#endif //_PUBLIC_H_
