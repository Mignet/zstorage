
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
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//tlv协议
#ifndef	_TLV_H_
#define _TLV_H_

#pragma warning(disable: 4018 4244)

#include "tls_pools.h"
#include "wisdom_ptr.h"
#include "RingQueue.h"
using namespace pool;
#ifndef int64_t
#define int64_t			long long
#endif

#ifndef uint64_t
#define uint64_t		unsigned long long
#endif

//#define TLV_TEST	1
#define TLV_LOG		LOG4_INFO

// host long 64 to network
static uint64_t hl64ton(uint64_t host)
{
	uint64_t   ret = 0;   
	unsigned long high,low;
	low = host & 0xFFFFFFFF;
	high = (host >> 32) & 0xFFFFFFFF;
	low = htonl(low);
	high = htonl(high);
	ret = low;
	ret <<= 32;
	ret |= high;
	return ret;
}

//network to host long 64
static uint64_t ntohl64(uint64_t host)
{
	uint64_t ret = 0; 
	unsigned long high,low;
	low	= host & 0xFFFFFFFF;
	high =  (host >> 32) & 0xFFFFFFFF;
	low	= ntohl(low);
	high = ntohl(high);
	ret = low;
	ret <<= 32;
	ret |= high;
	return ret;
}

namespace TLV
{
	//分配内存部件
	template <class _L>
	class alloc_block
	{
	public:
		alloc_block(void* v, _L n) :_n(n) { _v = malloc(_n + 1); memcpy(_v, v, _n); ((char*)_v)[_n] = 0;}
		~alloc_block() { free(_v); }
		static bool alloc() {return true;}
		void* _v; _L _n;
	};

	//不分配内存部件
	template <class _L>
	class block
	{
	public:
		block(void* v, _L n) :_n(n), _v(v){}
		~block() {}
		void* _v; _L _n;
	};

	template <class _T, class _L, class _BLOCK>  
	class container 
		: public CMPool
	{
	public:

		class container_free
		{
		public:
			static void free( container* p) {  if (p) delete p;}
		};

		typedef wisdom_ptr< container, container_free>	wisdom_container;


		class object
		{
		public:
			void append(void* v, _L n) 
			{
				_v.push_back(new _BLOCK(v, n));
			}

			~object() 
			{
				while (!_v.empty()) 
				{
					delete (*_v.begin());
					_v.erase(_v.begin());
				}
			}

			void* to_buffer(_L& len, uint32 n) 
			{
				if (_v.size() > n) 
				{
					len = _v[n]->_n;
					return _v[n]->_v;
				}
				return NULL;
			}

		private:
			vector<_BLOCK*> _v;
		};

		typedef map<_T, object*> M;
	public:
		container(){}
		~container() { clear(); }
	private:
		container(container& o);
		container& operator = (container& o);
	public:
		void clear() 
		{
			while (!m_value.empty()) 
			{
				delete m_value.begin()->second;
				m_value.erase(m_value.begin());
			}
		}

		void free_object(container* tlv) 
		{
			delete tlv;
		}

		bool empty(_T cmd)
		{
			return m_value.find(cmd) == m_value.end();
		}

	public: //压入数据

		void push_ex(_T cmd, const char * v)
		{
			push(cmd, v, strlen(v));
		}

		void push_ex(_T cmd, const string& v)
		{
			push(cmd, v.c_str(), v.length());
		}

		void push(_T cmd, char v) 
		{
			push(cmd, &v, sizeof(v)); 
		}

		void push(_T cmd, unsigned char v) 
		{
			push(cmd, &v, sizeof(unsigned char)); 
		}

		void push(_T cmd, int16_t v)
		{
			v = htons(v);
			push(cmd, &v, sizeof(v));
		}

		void push(_T cmd, uint16_t v)
		{
			v = htons(v);
			push(cmd, &v, sizeof(v));
		}

		void push(_T cmd, int32_t v)
		{
			v = htonl(v);
			push(cmd, &v, sizeof(v));
		}

		void push(_T cmd, uint32_t v)
		{
			v = htonl(v);
			push(cmd, &v, sizeof(v));
		}

		void push(_T cmd, int64_t v)
		{
			v = hl64ton(v);
			push(cmd, &v, sizeof(v));
		}

		void push(_T cmd, uint64_t v)
		{
			v = hl64ton(v);
			push(cmd, &v, sizeof(v));
		}


		void push(_T cmd, const string& v)
		{
			push(cmd, (void*)v.c_str(), v.length() + 1);
		}

		void push(_T cmd, const char* v)
		{
			push(cmd, (void*)v, strlen(v) + 1);
		}

		void push(_T cmd, unsigned char* v)
		{
			push(cmd, (void*)v, strlen((const char*)v) + 1);
		}

		void push(_T cmd, container* object)
		{	
			char* buffer = NULL;
			int len = 0;
			object->encode(&buffer, len);
			push(cmd, buffer, (_L)len);
		}

		void push(_T cmd, const char* v, int32_t n)
		{
			push(cmd, (void*)v, n);
		}

		void push(_T cmd, unsigned char* v, int32_t n)
		{
			push(cmd, (void*)v, n);
		}

		void push(_T cmd, void* v, int32_t n)
		{
			//if (typeid(_BLOCK) == typeid(alloc_block<_L>))
			_BLOCK::alloc();
			pack_alloc_block(cmd, v, n);
		}

		void pack_alloc_block(_T cmd, void* v, int32_t n)
		{
			if (n > 0) //空不压入
			{
				push_value(cmd, v, n);
			}
		}

		void push_value(_T cmd, void* v, int32_t n)
		{
			m_ring.Append( (void*)&cmd, sizeof(_T));
			_L len = n;
			len = inet_byte(len);
			m_ring.Append( (void*)&len, sizeof(_L));
			m_ring.Append(v, n);
		}

		void pack(_T cmd, void* v, int32_t n)
		{
			typename M::iterator iter = m_value.find(cmd);
			if (iter != m_value.end())
			{
				iter->second->append(v, n);
			}
			else
			{
				object* value = new object;
				value->append(v, n);
				m_value.insert(make_pair(cmd, value));
			}
		}

	public: //获取数据
		
		void* to_buffer(_T cmd, _L& len, int32_t n = 0)
		{
			typename M::iterator iter = m_value.find(cmd);
			if (iter != m_value.end())
				return iter->second->to_buffer(len, n);
			return NULL;
		}

		const char* to_string(_T cmd, int32_t n = 0)
		{
			_L len = 0;
			const char* buffer = (const char*)to_buffer(cmd, len, n);
			if (buffer == NULL || len == 0)
				return "";

			if (buffer[len - 1] == '\0')	//有 '\0'结束的处理
			{
				return buffer;
			}
			else							//没有'\0'结束处理
			{
				return "";
			}
		}
		

		uint64_t to_number(_T cmd, int32_t n = 0)
		{
			_L len = 0;
			const char* buffer = (const char*)to_buffer(cmd, len, n);
			if (buffer == NULL)
				return 0;

			switch (len)
			{
			case 1:
				return (*(uint8_t*)buffer);
				break;
			case 2:

				return ntohs(*(uint16_t*)buffer);
				break;
			case 4:

				return ntohl(*(uint32_t*)buffer);
				break;

			case 8:

				return ntohl64(*(uint64_t*)buffer) ;
				break;
			default:
				return 0;
				break;
			}
		}
		
		wisdom_container to_object(_T cmd, int32_t n = 0)
		{
			_L len = 0;
			
			char* buffer = (char*)to_buffer(cmd, len, n);

			if (buffer == NULL)
				return NULL;

			wisdom_container object = new container;

			object->decode(buffer, len);

			return object;
		}

		char to_int8(_T cmd, int32_t n = 0)
		{
			return (int8_t)to_number(cmd, n);
		}

		unsigned char to_uint8(_T cmd, int32_t n = 0)
		{
			return (uint8_t)to_number(cmd, n);
		}

		short to_int16(_T cmd, int32_t n = 0)
		{
			return (int16_t)to_number(cmd, n);
		}

		unsigned short to_uint16(_T cmd, int32_t n = 0)
		{
			return (uint16_t)to_number(cmd, n);
		}

		long to_int32(_T cmd, int32_t n = 0)
		{
			return (int32_t)to_number(cmd, n);
		}

		unsigned long to_uint32(_T cmd, int32_t n = 0)
		{
			return (uint32_t)to_number(cmd, n);
		}

		int to_integer(_T cmd, int32_t n = 0)
		{
			return (int32_t)to_number(cmd, n);
		}

		int64_t to_int64(_T cmd, int32_t n = 0)
		{
			return (int64_t)to_number(cmd, n);
		}

		uint64_t to_uint64(_T cmd, int32_t n = 0)
		{
			return (uint64_t)to_number(cmd, n);
		}

	public:

		void decode(char* packet, int len)
		{
			int pos = 0;
			while (pos + sizeof(_T) + sizeof(_L) < len)
			{
				_T type = 0;
				_L length = 0;
				//T
				type = *(_T*)&packet[pos];
				type = inet_byte(type);
				pos += sizeof(_T);
				//L
				length = *(_L*)&packet[pos];
				length = inet_byte(length);
				pos += sizeof(_L);

				if (pos + length > len)
					break;
				//V
				pack(type, &packet[pos], length);
				pos += length;
				
			}
		}

		void encode(char** packet, int& len)
		{
			for (typename M::iterator iter = m_value.begin(); 
				iter != m_value.end(); ++iter)
			{
				int i = 0;
				do 
				{
					_L n = 0;
					const char* v = (const char*)iter->second->to_buffer(n, i++);
					if (v == NULL)
						break;

					//T
					//length += pack_value(ptr, inet_byte(iter->first));
					_T cmd = inet_byte(iter->first);
					m_ring.Append( (void*)&cmd, sizeof(_T));

					//L
					_L len = n;
					len = inet_byte(len);
					m_ring.Append( (void*)&len, sizeof(_L));

					//V
					m_ring.Append((void*)v, n);

				} while (true);
			}

			if (m_ring.Count() <= 0)
			{
				len = 0;
				return;
			}

			uint8* ptr = NULL;
			len = m_ring.Attach(&ptr);
			*packet = (char*)ptr;
		}


		void cmds(list<uint32>& cmds)
		{
			for (typename M::iterator iter = m_value.begin(); 
				iter != m_value.end(); ++iter)
			{
				cmds.push_back(iter->first);
			}
		}

	private:
			template <class T>
			T inet_byte(T _v)
			{
				switch (sizeof(T))
				{
				case 2:
					return (T)htons(_v);
					break;
				case 4:
					return (T)htonl(_v);
					break;
				default:
					return _v;
					break;
				}
			}

			template <class T>
			int pack_value(char*& ptr, T _v)
			{
				*(T*)ptr = _v;
				ptr += sizeof(T);
				return sizeof(T);
			}

			int pack_value(char*& ptr, const void* v, int n)
			{
				memcpy(ptr, v, n);
				ptr += n;
				return n;
			}
	private:
		M						m_value;
		CRingQueue<8196>		m_ring;
	};
}

typedef TLV::alloc_block<unsigned long> alloc_block_uint32;
typedef TLV::block<unsigned long> block_uint32;

typedef TLV::alloc_block<unsigned short> alloc_block_uint16;
typedef TLV::block<unsigned short> block_uint16;

typedef TLV::alloc_block<unsigned char> alloc_block_uint8;
typedef TLV::block<unsigned char> block_uint8;

typedef TLV::container<unsigned int, unsigned int, alloc_block_uint32> tlv_44;
typedef TLV::container<unsigned int, unsigned int, block_uint32> tlv_44_analyze;

typedef TLV::container<unsigned short, unsigned int, alloc_block_uint32> tlv_24;
typedef TLV::container<unsigned short, unsigned int, block_uint32> tlv_24_analyze;

typedef TLV::container<unsigned short, unsigned short, alloc_block_uint16> tlv_22;
typedef TLV::container<unsigned short, unsigned short, block_uint16> tlv_22_analyze;

typedef TLV::container<unsigned char, unsigned int, alloc_block_uint32> tlv_14;
typedef TLV::container<unsigned char, unsigned int, block_uint32> tlv_14_analyze;

typedef TLV::container<unsigned char, unsigned short, alloc_block_uint16> tlv_12;
typedef TLV::container<unsigned char, unsigned short, block_uint16> tlv_12_analyze;

WISDOM_PTR(tlv_12_analyze, wisdom_tlv_12_analyze);

typedef tlv_12::wisdom_container			tlv_12_container;
typedef tlv_12_analyze::wisdom_container	tlv_12_analyze_container;


typedef TLV::container<unsigned char, unsigned char, alloc_block_uint8> tlv_11;
typedef TLV::container<unsigned char, unsigned char, block_uint8> tlv_11_analyze;


#endif //_TLV_H_
