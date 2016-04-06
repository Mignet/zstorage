
#ifndef c_object_h
#define c_object_h

#pragma once
#include "c_resolver.h"

class IConnect;

#define BEGIN_COMMAND		namespace command{
#define END_COMMAND			}

#define COMMAND_OBJECT(c)	\
class c	\
	: public c_object	\
	, public c_singleton<c>

#define COMMAND_CHILD(c)	\
class c	\
	: public c_child	\
	, public c_singleton<c>



#define CONSTRUCTION_OBJECT(name, c, p, ep)	\
	public:	\
	c(): m_power(p),m_explain(ep) { \
	c_command::get_instance()->push(name, this);}	\
	private:	\
	virtual bool power(uint8 power){return (power & m_power) > 0; } \
	uint8	m_power; \


#define CONSTRUCTION_DEFINE_CHILD()	\
	public:	\
	typedef map<string, c_child*>		MCHILD;	\
	MCHILD	m_child;	\
	void push(const string& name, c_child* child){	\
	m_child.insert(make_pair(name, child));	}

#define CONSTRUCTION_EXPLAIN()	\
	private:	\
	virtual void explain(wisdom_IOStream& result) {result->push(m_explain);}	\
	string m_explain;

#define CONSTRUCTION_EXPLAIN_CHILD_METHOD()	\
	public:	\
	virtual void explain_child(wisdom_IOStream& result){	\
	for (MCHILD::iterator pos = m_child.begin(); pos != m_child.end(); ++pos)	\
	pos->second->explain(result);	} \

#define REGISTER_OBJECT(c)	c::get_instance();

#define CONSTRUCTION_CHILD(name, c, parent, ep)	\
	public:	\
	c():m_explain(ep) {parent::get_instance()->push(name, this);}	\


#define	CONSTRUCTION_EXECUTE_METHOD()	\
private:	\
virtual void execute(wisdom_param& param, IConnect* con) \
{	char *ptr;int len;	\
	if (!param->get(2, &ptr, len))	{	\
	RESULT_ERROR(con, "The second parameter error.");	\
	return;	} \
	ptr = strupr(ptr); \
	ptr[len] = 0;	\
	MCHILD::iterator pos = m_child.find(ptr); \
	if (pos == m_child.end()) { \
	RESULT_ERROR(con, "second method unknown command.");	\
	return;	} \
	pos->second->child_execute(param, con); }


class c_object
{
public:
	void deal(wisdom_param& param, IConnect* con);
	virtual void execute(wisdom_param& param, IConnect* con) = 0;
	virtual bool power(uint8 power) = 0;
	virtual void explain(wisdom_IOStream& result) = 0;
	virtual void explain_child(wisdom_IOStream& result){explain(result);}
};

WISDOM_PTR(c_object, wisdom_object);

class c_child
{
public:
	virtual void child_execute(wisdom_param& param, IConnect* con) = 0;
	virtual void explain(wisdom_IOStream& result) = 0;
};

WISDOM_PTR(c_child, wisdom_child);

#endif
