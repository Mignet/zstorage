#ifndef _MY_TYPES_H
#define _MY_TYPES_H

#ifndef	int64_t
#ifdef _WIN32
#define int8_t				char
#define int16_t				short
#define int32_t				int
#define int64_t				__int64
#define	uint8_t				unsigned char
#define uint16_t			unsigned short
#define uint32_t			unsigned int
#define uint64_t			unsigned __int64
#else
#define int8_t				char
#define int16_t				short
#define int32_t				int
#define int64_t				long long
#define	uint8_t				unsigned char
#define uint16_t			unsigned short
#define uint32_t			unsigned int
#define uint64_t			unsigned long long
#endif
#endif

#define int8				int8_t
#define uint8				uint8_t
#define int16				int16_t
#define uint16				uint16_t
#define int32				int32_t
#define uint32				uint32_t

#define	_userid 			uint32_t
#define	_groupid 			uint32_t
#define _appid				uint32_t

#endif	//_MY_TYPES_H
