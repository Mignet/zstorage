#ifndef c_json_h
#define c_json_h

#ifdef _MSC_VER
#pragma warning(disable: 4001)
#pragma warning(disable: 4200 4706)
#endif

#include "MyTypes.h"
#include "cJSON/cJSON.h"
#include "base64.h"
#include "wisdom_ptr.h"
#include "tls_pools.h"
using namespace pool;

static const char* _is(uint32 k)
{
	sprintf(tls_buf(), "%u", k);
	return tls_buf();
}

#define js_byte		char

#define js_push(h,v) push(_is(h),v)
#define js_to_int8(h) to_int8(_is(h))
#define js_to_uint8(h) to_uint8(_is(h))
#define js_to_int16(h) to_int16(_is(h))
#define js_to_uint16(h) to_uint16(_is(h))
#define js_to_int32(h) to_int32(_is(h))
#define js_to_uint32(h) to_uint32(_is(h))
#define js_to_string(h) to_string(_is(h))
#define js_to_buffer(h,l) to_buffer(_is(h), l)
#define js_to_object(h) to_object(_is(h))
#define js_to_array(h) to_array(_is(h))

#define c_json_array c_json::json_array
namespace json
{
	class js_byte_free
	{
	public:
		static void free( js_byte* p) {
			if (p)
				::free(p);}
	};

	typedef wisdom_ptr< js_byte, js_byte_free>	wisdom_js_byte;

	class c_json
	{
	public:
		class json_array
		{
			friend class c_json;
		private:
			json_array& operator =(cJSON* json);
			json_array& operator = (c_json_array& json);
		public:
			json_array(): m_auto_free(false) { m_json = cJSON_CreateArray(); }
			json_array(cJSON* json): m_auto_free(true) { m_json = json; }
			~json_array() { if (!m_auto_free && m_json) cJSON_Delete(m_json); }
			void json_set(cJSON* json) {m_json = json;}
			void json_free() { if (m_json) cJSON_Delete(m_json); }
			operator cJSON* () const {return m_json;}
			void push(int8 v)	{push((uint32)v);}
			void push(uint8 v)	{push((uint32)v);}
			void push(int16 v)  {push((uint32)v);}
			void push(uint16 v) {push((uint32)v);}
			void push(int32 v)	{push((uint32)v);}
			void push(uint32 v) {cJSON_AddNumberToObject(m_json, "", (double)v);}
			void push(const string& v) { push(v.c_str()); }
			void push(unsigned char* v) { push((const char*)v); }
			void push(char* v)  { push((const char*)v); }
			void push(const char* v) {	cJSON_AddStringToObject(m_json, "", v); }
			void push(c_json_array& v)	{cJSON_AddItemToObject(m_json, "", v.m_json); v.m_json = NULL;}
			void push(c_json& v) { cJSON_AddItemToObject(m_json, "", v.m_json); v.m_json = NULL;}
			void push(void* v, int len){
				int out_max = len * 4;
				char* out = (char*)malloc(out_max);
				unsigned out_len = 0; 
				base64::encode64((const char *)v, len, out, out_max, &out_len);
				if (out_len > 0)
					push(out);
				free(out);
			}
			int array_size() {
				if (m_json == NULL) return 0;
				return cJSON_GetArraySize(m_json);}
			int8	to_int8(int i) { return (int8)to_uint64(i); }
			uint8	to_uint8(int i) { return (uint8)to_uint64(i); }
			int16	to_int16(int i) { return (int16)to_uint64(i); }
			uint16	to_uint16(int i) { return (uint16)to_uint64(i); }
			int32	to_int32(int i) { return (int32)to_uint64(i); }
			uint32	to_uint32(int i) { return (uint32)to_uint64(i);	}
			int64_t	to_int64(int i) {return (int64_t)to_uint64(i);}
			uint64_t	to_uint64(int i){
				if (m_json == NULL) return 0;
				cJSON* js = cJSON_GetArrayItem(m_json, i); 
				if (js && js->type == cJSON_Number) 
					return (uint64_t)js->valuedouble;
				return 0;
			}

			int8* to_string(int i){
				if (m_json == NULL) return (int8*)"";
				cJSON* js = cJSON_GetArrayItem(m_json, i); 
				if (js && js->type == cJSON_String) 
					return js->valuestring;
				return (int8*)"";
			}

			void* to_buffer(int i, int& len) {
				if (m_json == NULL) return NULL;
				cJSON* js = cJSON_GetArrayItem(m_json, i); 
				const char* data = (js && js->type == cJSON_String) ? js->valuestring : "";
				unsigned int in_len = strlen(data);
				if (in_len <= 0){
					len = 0;
					return NULL;
				}

				unsigned int out_len = in_len;
				char* out = (char*)malloc(out_len);
				base64::decode64(data, strlen(data), out, &out_len);
				len = out_len;
				return out;
			}

			cJSON* to_object(int i) {
				if (m_json == NULL) return NULL;
				cJSON* js = cJSON_GetArrayItem(m_json, i);
				if (js && js->type == cJSON_Object) {
					return js;
				}

				return NULL;
			}

			cJSON* to_array(int i) {
				if (m_json == NULL) return NULL;
				cJSON* js = cJSON_GetArrayItem(m_json, i);
				if (js && js->type == cJSON_Array) {
					return js;
				}

				return NULL;
			}

			void json_free(void* p) {free(p);}

			cJSON* json() const {return m_json; }
		private:
			cJSON*		m_json;
			bool		m_auto_free;
		};

	private:
		c_json& operator =(cJSON* json);
		c_json& operator = (c_json& json);
	public:
		c_json(): m_auto_free(false) { m_json = cJSON_CreateObject(); }
		c_json(cJSON* json): m_auto_free(true) { m_json = json; }
		c_json(const char* body): m_auto_free(false) { m_json = cJSON_Parse(body); }
		c_json(char* body, int len): m_auto_free(false) {
			if (body[len - 1] == 0) {
				m_json = cJSON_Parse(body);
			}else{
				char* buffer = (char*)malloc(len + 1);
				memcpy(buffer, body, len);
				buffer[len] = '\0';
				m_json = cJSON_Parse(buffer);
				free(buffer);
			}
		}
		c_json(wisdom_js_byte boday): m_auto_free(false) { m_json = cJSON_Parse((char*)boday); }
		~c_json() {if (!m_auto_free && m_json) cJSON_Delete(m_json);}
		operator cJSON* () const {return m_json;}
		//~c_json() { if (m_json) cJSON_Delete(m_json); }
		void json_set(cJSON* json) {m_json = json;}
		void json_free() { cJSON_Delete(m_json); }
		wisdom_js_byte body() { 
			if(m_json != NULL) 
				return cJSON_PrintUnformatted(m_json);
			return NULL;}
		void push(const string& k, int8 v) {push(k, (uint32)v);	}
		void push(const string& k, uint8 v){push(k, (uint32)v);}
		void push(const string& k, int16 v){push(k, (uint32)v);}
		void push(const string& k, uint16 v){push(k, (uint32)v);}
		void push(const string& k, int32 v){push(k, (uint32)v);}
		void push(const string& k, uint32 v){cJSON_AddNumberToObject(m_json, k.c_str(), (double)v);}
		void push(const string& k, const string& v){push(k, v.c_str());}
		void push(const string& k, char* v){push(k, (const char*)v);}
		void push(const string& k, unsigned char* v){push(k, (const char*)v);}
		void push(const string& k, const char* v){cJSON_AddStringToObject(m_json, k.c_str(), v);}
		void push(const string& k, void* v, int len){
			int out_max = len * 4;
			char* out = (char*)malloc(out_max);
			unsigned out_len = 0; 
			base64::encode64((const char *)v, len, out, out_max, &out_len);
			if (out_len > 0)
				push(k, out);
			free(out);
		}

		void push(const string& k, c_json& v){cJSON_AddItemToObject(m_json, k.c_str(), v.m_json); v.m_json = NULL;}
		void push(const string& k, c_json_array& v){cJSON_AddItemToObject(m_json, k.c_str(), v.m_json);  v.m_json = NULL;}

		int8	to_int8(const string& k) { return (int8)to_uint64(k); }
		uint8	to_uint8(const string& k) { return (uint8)to_uint64(k); }
		int16	to_int16(const string& k) { return (int16)to_uint64(k); }
		uint16	to_uint16(const string& k) { return (uint16)to_uint64(k); }
		int32	to_int32(const string& k) { return (int32)to_uint64(k); }
		uint32	to_uint32(const string& k) { return (uint32)to_uint64(k); }
		int64_t	to_int64(const string& k) { return (int64_t)to_uint64(k); }
		bool exists(const string& k) {
			if (m_json == NULL) return false;
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str());
			return js != NULL;
		}
		uint64_t to_uint64(const string& k){
			if (m_json == NULL) return 0;
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str()); 
			if (js && js->type == cJSON_Number) 
				return (uint64_t)js->valuedouble;
			return 0;
		}

		int8* to_string(const string& k){
			if (m_json == NULL) return (int8*)"";
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str()); 
			if (js && js->type == cJSON_String) 
				return js->valuestring;
			return (int8*)"";
		}

		void* to_buffer(const string& k, int& len) {
			if (m_json == NULL) return NULL;
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str()); 
			const char* data = (js && js->type == cJSON_String) ? js->valuestring : "";
			unsigned int in_len = strlen(data);
			if (in_len <= 0){
				len = 0;
				return NULL;
			}

			unsigned int out_len = in_len;
			char* out = (char*)malloc(out_len);
			base64::decode64(data, strlen(data), out, &out_len);
			len = out_len;
			return out;
		}

		cJSON* to_object(const string& k) {
			if (m_json == NULL) return NULL;
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str());
			if (js && js->type == cJSON_Object) {
				return js;
			}

			return NULL;
		}

		cJSON* to_array(const string& k) {
			if (m_json == NULL) return NULL;
			cJSON* js = cJSON_GetObjectItem(m_json, k.c_str());
			if (js && js->type == cJSON_Array) {
				return js;
			}

			return NULL;
		}

		void json_free(void* p) { if (p) free(p);}

		cJSON* json() const {return m_json; }
	private:
		cJSON*		m_json;
		bool		m_auto_free;
	};

	class json_free
	{
	public:
		static void free( c_json* p) {
			if (p && p->json()) {
				delete p;}}
	};

	typedef wisdom_ptr< c_json, json_free>	wisdom_json;

	class json_array_free
	{
	public:
		static void free(c_json_array* p) {
			if (p && p->json()) {
				delete p;
			}
		}
	};

	typedef wisdom_ptr< c_json_array, json_array_free>	wisdom_json_array;



	class c_json_packet
		: public CMPool
	{
	public:

		c_json_packet()
			: m_cmdId(0)
			, m_json("")
		{

		}

		c_json_packet(uint32 cmd, const string& json)
			: m_cmdId(cmd)
			, m_json(json)
		{
		}

		c_json_packet(uint32 cmd, wisdom_json& json)
			: m_cmdId(cmd)
		{
			wisdom_js_byte body = json->body();
			m_json = body;
		}

		~c_json_packet() 
		{
		}

		bool isnull()
		{
			return m_json.empty();
		}

		uint32			m_cmdId;
		string			m_json;
	};

	class json_packet_free
	{
	public:
		static void free( c_json_packet* p) {  if (p) delete p;}
	};

	typedef wisdom_ptr< c_json_packet, json_packet_free>	wisdom_json_packet;

}

#endif //c_json_h

