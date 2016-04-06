#include "stdafx.h"
#include "c_cloud.h"
#include "c_seq.h"
#include "zcloud_sdk.h"

//im 消息存储定制协议
bool is_result_ok(wisdom_IOStream& os)
{
	if (os->empty())
		return false;

	wisdom_punit unit = os->get(0);
	if (unit == NULL)
		return false;

	char* ptr = unit->m_str;
	int len = unit->m_len - 2;
	ptr[len] = 0;
	if (memcmp(ptr, ZRESULT_OK, 2) != 0)
	{
		return false;
	}

	return true;
}

string result_get_string(int index, wisdom_IOStream& os)
{
	wisdom_punit unit = os->get(index);
	if (unit == NULL)
		return "";

	char* ptr = unit->m_str;
	int len = unit->m_len - 2;
	ptr[len] = 0;

	return ptr;
}

uint32 result_get_integer(int index, wisdom_IOStream& os)
{
	wisdom_punit unit = os->get(index);
	if (unit == NULL)
		return 0;

	char* ptr = unit->m_str;
	int len = unit->m_len - 2;
	ptr[len] = 0;

	return atoi(ptr);
}

char* result_get(int index, wisdom_IOStream& os, int& len)
{
	wisdom_punit unit = os->get(index);
	if (unit == NULL)
		return NULL;

	char* ptr = unit->m_str;
	len = unit->m_len - 2;
	ptr[len] = 0;

	return ptr;
}

c_cloud::c_cloud(const char* key)
{
	m_key = "CLOUD_";
	m_key += key;
	m_effective = time(0) + c_server::get_instance()->object_effective();
	m_head.type = T_GET_SET;
}

c_cloud::~c_cloud()
{
}

void c_cloud::push(wisdom_param& param, int index, wisdom_IOStream& io)
{
	string wildcard = param->to_string(++index);

	char* ptr = NULL;
	int len = 0;

	if (!param->get(++index, &ptr, len))
	{
		ZERROR_RESULT("data is null.");
	}
	leveldb::Slice value(ptr, len);
	
	wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << wildcard));
	wisdom_IOStream os;
	os = new c_ostream_array;
	seq->push(ptr, len, os, 0);

	if (!is_result_ok(os))
	{
		ZERROR_RESULT("seq push error");
	}


	os = new c_ostream_array;
	seq->index_(os);
	if (!is_result_ok(os))
	{
		ZERROR_RESULT("seq index error");
	}

	string indexId = result_get_string(1, os);

	io->push(ZRESULT_OK);
	io->push(indexId);

}

void c_cloud::size(wisdom_param& param, int index, wisdom_IOStream& io)
{
	io->push(ZRESULT_OK);
	string wildcard = param->to_string(++index);

	wisdom_IOStream os = new c_ostream_array;
	wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << wildcard));
	seq->size(os);
	if (!is_result_ok(os))
	{
		return;
	}

	io->push(result_get_string(1, os));
}

void c_cloud::read(wisdom_param& param, int index, wisdom_IOStream& io)
{
	string keyword = param->to_string(++index);
	string id = param->to_string(++index);

	uint32 old_id = get_read(keyword);

	LOG4_INFO("key:" << m_key << " keyword : " << keyword << " read id : " << id << " old id : " << old_id);

	if (atoi(id.c_str()) <= old_id)
		return;

	leveldb::Slice value(id);
	if (!c_base::put(__tos(m_key << "@READ@" << keyword), value, time(0) + KEY_TIMEOUT))
	{
		ZERROR_RESULT("write db error");
	}

	io->push(ZRESULT_OK);
}

void c_cloud::limit(wisdom_param& param, int index, wisdom_IOStream& io)
{
	string wildcard = param->to_string(++index);
	uint32 location = param->to_integer(++index);
	int count = param->to_integer(++index);

	if (wildcard.empty())
	{
		ZERROR_RESULT("wildcard is null.");
	}

	if (count == 0 || abs(count) > 5000)
	{
		ZERROR_RESULT("count error.");
	}

	io->push(ZRESULT_OK);

	wisdom_IOStream os;
	wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << wildcard));
	os = new c_ostream_array;
	seq->seek(location, count, os);
	if (is_result_ok(os))
	{
		for (int i = 1; true; i+=2)
		{
			string s = result_get_string(i, os);
			if (s.empty())
				break;
			vector<string>	vs;
			zn::string_split(s.c_str(), ',', vs);
			if (vs.size() != 2)
				break;

			uint32 indexId = atoi(vs[0].c_str());
			string stime = vs[1];
			tlv_12 tlv;
			tlv.push(zcloud_sdk::msg_seek::indexId, itostr(indexId));
			tlv.push(zcloud_sdk::msg_seek::ptime, stime);
			int n = 0;
			char* data = result_get(i+1, os, n);
			if (n > 0)
			{
				tlv.push(zcloud_sdk::msg_seek::packet, data, n);
			}

			char* ptr = NULL;
			int len = 0;
			tlv.encode(&ptr, len);
			if (len > 0)
			{
				io->push(ptr, len);
			}
		}
	}
}


void c_cloud::query(wisdom_param& param, int index, wisdom_IOStream& io)
{
	io->push(ZRESULT_OK);
	string wildcard = param->to_string(++index);
	string strtime = param->to_string(++index);
	int count = param->to_integer(++index);
	c_time t(strtime);
	wisdom_IOStream os = new c_ostream_array;
	wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << wildcard));
	seq->query(t.unix_time(), os);
	uint32 indexId = 0;
	if (!is_result_ok(os))
	{
		return;
	}

	indexId = result_get_integer(1, os);
	os = new c_ostream_array;
	seq->seek(indexId, count, os);

	if (is_result_ok(os))
	{
		for (int i = 1; true; i += 2)
		{
			string s = result_get_string(i, os);
			vector<string>	vs;
			zn::string_split(s.c_str(), ',', vs);
			if (vs.size() != 2)
				break;

			uint32 indexId = atoi(vs[0].c_str());
			string stime = vs[1];
			tlv_12 tlv;
			tlv.push(zcloud_sdk::msg_seek::indexId, itostr(indexId));
			tlv.push(zcloud_sdk::msg_seek::ptime, stime);
			int n = 0;
			char* data = result_get(i + 1, os, n);
			if (n > 0)
			{
				tlv.push(zcloud_sdk::msg_seek::packet, data, n);
			}

			char* ptr = NULL;
			int len = 0;
			tlv.encode(&ptr, len);
			if (len > 0)
			{
				io->push(ptr, len);
			}
		}
	}
}

void c_cloud::online(wisdom_param& param, int index, wisdom_IOStream& io)
{
	io->push(ZRESULT_OK);
	string parsing = param->to_string(++index);
	vector<string>	v;
	zn::string_split(parsing.c_str(), ',', v);

	for (int i = 0; i < v.size(); i++)
	{
		wisdom_IOStream os = new c_ostream_array;
		wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << v[i]));
		seq->index_(os);
		if (!is_result_ok(os))
			continue;

		uint32 beginId = get_read(v[i]);
		uint32 endId = result_get_integer(1, os);

		if (beginId > endId)
		{
			beginId = endId;
			leveldb::Slice value(itostr(beginId));
			c_base::put(__tos(m_key << "@READ@" << v[i]), value, time(0) + KEY_TIMEOUT);
		}
		uint32 unread = 0;
		
		if (beginId == 0)
		{
			wisdom_IOStream osize = new c_ostream_array;
			seq->size(osize);
			uint32 usize = result_get_integer(1, osize);

			beginId = endId - usize + 1;

			unread = usize;
		}
		else
		{
			unread = endId - beginId;
		}

		if (beginId > endId)
			beginId = endId;

		//获取最后一条数据
		os = new c_ostream_array;
		seq->seek(endId, 1, os);

		tlv_12 tlv;
		tlv.push(zcloud_sdk::msg_online::source, v[i]);
		tlv.push(zcloud_sdk::msg_online::beginId, itostr(beginId));
		tlv.push(zcloud_sdk::msg_online::endId, itostr(endId));
		tlv.push(zcloud_sdk::msg_online::unread, unread);

		if (is_result_ok(os))
		{
			string s = result_get_string(1, os);
			vector<string>	vs;
			zn::string_split(s.c_str(), ',', vs);
			if (vs.size() >= 2)
			{
				tlv.push(zcloud_sdk::msg_online::ptime, vs[1]);
			}

			int n = 0;
			char* data = result_get(2, os, n);
			if (n > 0)
			{
				tlv.push(zcloud_sdk::msg_online::packet, data, n);
			}
		}

		char* ptr = NULL;
		int len = 0;
		tlv.encode(&ptr, len);
		if (len > 0)
		{
			io->push(ptr, len);
		}
	}

}

bool c_cloud::is_release()
{
	return time(0) > m_effective;
}

uint32 c_cloud::get_read(const string& keyword)
{

	string value;
	_zmsg_head* head = NULL;
	if (c_base::get_value(__tos(m_key << "@READ@" << keyword), value, &head) != 0)
	{
		return 0;
	}

	char* ptr = (char*)value.c_str() + sizeof(_zmsg_head);

	vector<char> v;
	for (int i = sizeof(_zmsg_head); i < value.size(); i++)
		v.push_back(value.c_str()[i]);
	v.push_back(0);
	
	return atoi(&v[0]);
}

void c_cloud::index_(wisdom_param& param, int index, wisdom_IOStream& io)
{
	io->push(ZRESULT_OK);
	string wildcard = param->to_string(++index);

	wisdom_IOStream os = new c_ostream_array;
	wisdom_seq seq = c_seq_admin::get_instance()->find_is_create(__tos(m_key << "@" << wildcard));
	seq->index_(os);
	if (!is_result_ok(os))
	{
		return;
	}

	io->push(result_get_string(1, os));
}


