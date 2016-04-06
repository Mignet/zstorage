
#ifndef	_C_CMD_TARGET_H_ 
#define _C_CMD_TARGET_H_

#ifndef int64_t
#define int64_t			long long
#endif

#ifndef uint64_t
#define uint64_t		unsigned long long
#endif

#include <map>
#include <list>
using namespace std;
#include "c_uuid.h"

#ifdef _MSC_VER
#pragma warning(disable: 4407)
#endif

#define INSTALL_MSG(cmd, target) \
{ uint64_t addr = 0; add_trans(addr, target); _install(cmd, addr);}
//_install(cmd, (_target)target);

#define UNINSTALL_MSG(cmd) \
	_uninstall(cmd);

#define TAR_MSG(T)\
	typedef void (T::*_target)(int cmd, void* packet, int len); \
	virtual void on_message(uint64_t target, int cmd, void* packet, int len){ \
	_target tar; add_trans(tar, target); \
	(this->*tar)(cmd, packet, len); }

template <class T, class F> 
static void add_trans(T& _out, F _in) 
{
	union{
		F	_x;
		T   _y;
	}ut;
	ut._x = _in;
	_out = ut._y;
}

class ICmd
{
public:
	virtual void on_message(int cmd, void* packet, int len) = 0;
};

class c_cmd_target
	: public ICmd
{
public:
	typedef std::map<int, uint64_t>	CMD;
public:
	//返回 void* 需求获取继承对象实行指针地址
	virtual void on_message(uint64_t target, int cmd, void* packet, int len) = 0;

public:
	c_cmd_target(void) {}
	virtual ~c_cmd_target(void) {cmd_clear();}

	void cmd_clear()
	{
		while (!m_lst_uuid.empty())
		{		
			get_unique()->get_object(*m_lst_uuid.front(), true);
			m_lst_uuid.pop_front();
		}

		while (!m_lst_Int.empty())
		{
			get_unique_number()->get_object(m_lst_Int.front(), true);
			m_lst_Int.pop_front();
		}
	}
public:
	static c_unique<c_cmd_target>* get_unique()
	{
		static c_unique<c_cmd_target> s_unique;
		return &s_unique;
	}
	static c_unique_number<c_cmd_target>* get_unique_number()
	{
		static c_unique_number<c_cmd_target> s_unique;
		return &s_unique;
	}

	_uuid16& get_unique_uuid()
	{
		_uuid16* uuid16 = (_uuid16*)new _uuid16;
		c_uuid::create_uuid(*uuid16);
		get_unique()->generate_object(this, *uuid16);
		m_lst_uuid.push_back(uuid16);
		return *uuid16;
	}

	int get_unique_number_req()
	{
		int number = get_unique_number()->generate_object(this);
		m_lst_Int.push_back(number);
		return number;
	}
public:
	void _install(int cmd, uint64_t target)
	{
		CMD::iterator iter = m_cmd.find(cmd);
		if (iter != m_cmd.end())
			m_cmd.erase(iter);
		m_cmd.insert(std::make_pair(cmd, target));
	}

	void _uninstall(int cmd)
	{
		CMD::iterator iter  = m_cmd.find(cmd);
		if (iter != m_cmd.end())
			m_cmd.erase(iter);
	} 

	virtual void on_message(int cmd, void* packet, int len)
	{
		CMD::const_iterator iter = m_cmd.find(cmd);
		if (iter != m_cmd.end())
		{
			on_message(iter->second, cmd, packet, len);
		}
	}

protected:
	list<_uuid16*>		m_lst_uuid;
	list<int>			m_lst_Int;
private:
	CMD		m_cmd;
	
};

#endif //_C_CMD_TARGET_H_
