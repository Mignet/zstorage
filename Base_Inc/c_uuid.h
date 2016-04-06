#ifndef _c_uuid_h_
#define _c_uuid_h_

#ifndef _MSC_VER
#include <uuid/uuid.h>
#else
#pragma comment(lib, "Rpcrt4.lib")
#endif

#include "base64.h"


#pragma once

typedef	unsigned char _uuid16[16];					//不可见
typedef char _uuid32[32 + 1];						//可见 
typedef char _b64[24 + 1];							//可见64进制


static char basis_62[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static char index_62[128] = {
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

static string _SHORT_URL_TO_STRING(uint64_t id)
{
	vector<char> short_url;
	while (id > 0)
	{
		int imod = id % 62;
		short_url.push_back(basis_62[imod]);
		id /= 62;
	}

	short_url.push_back(0);

	return &short_url[0];
}

static uint64_t _POWER62(uint64_t x, int n)
{
	if (n == 0) return x;
	return _POWER62(x*62, n -1);
}

static uint64_t _SHORT_URL_TO_LONG(const string& short_url)
{
	if (short_url.length() > 11)
		return 0;

	uint8* url = (uint8*)short_url.c_str();

	uint64_t id = 0;

	for (size_t i = 0; i < short_url.length(); i++)
	{
		if (index_62[url[i]] < 0)
			return 0;

		uint64_t n = index_62[url[i]];
		id += _POWER62(n, i);
	}

	return id;
}

static bool uuid62_check(const string& u62)
{
	for (size_t i = 0; i < u62.length(); i++)
	{
		if (strchr(index_62, u62.c_str()[i]) == NULL)
			return false;
	}

	return true;
}

class c_unique
{
public:
	static uint64_t	create_unipue()
	{
		static uint64_t _unipue;
		return ++_unipue;
	}
};

class c_uuid
{
public:
	static void create_uuid(_uuid16 uuid)
	{
#ifdef _MSC_VER
		if (UuidCreateSequential((UUID*)uuid) != RPC_S_OK)
			UuidCreate((UUID*)uuid);
#else
		uuid_t uu;
		uuid_generate(uu);
		memcpy(uuid, uu, sizeof(uuid_t));
#endif
	}

	static string to_uuid62()
	{
		string result;
		_uuid16 uuid16;
		create_uuid(uuid16);
		uint64_t high;
		uint64_t low;
		memcpy(&high, uuid16, 8);
		memcpy(&low, &uuid16[8], 8);
		return _SHORT_URL_TO_STRING(high) + _SHORT_URL_TO_STRING(low);
	}

	static string to_uuid32()
	{
		_uuid16 uuid16;
		_uuid32 uuid32;
		create_uuid(uuid16);
		uuid16to32(uuid16, uuid32);
		return uuid32;
	}

	static void uuid16to32(const _uuid16 uuid16, _uuid32 uuid32)
	{
		for (int i = 0; i < 16; i++)
		{
			sprintf((char*)&uuid32[i * 2], "%02x", uuid16[i]);
		}
		uuid32[32] = '\0';
	}

	static void uuid32to16(const _uuid32 uuid32, _uuid16 uuid16)
	{
		for (int i = 0; i < 16; i++)
		{
			int c = 0;
			sscanf((const char*)&uuid32[i*2], "%2x", &c);
			uuid16[i] = c;
		}
	}
};

#endif //_c_uuid_h_
