
#ifndef c_base_h
#define c_base_h
#pragma once

#include "c_leavedb.h"

class ITimer
{
public:
	virtual bool on_timer() = 0;
};

class IAnalyzer
{
public:
	virtual void exec(wisdom_param& param, int index, const leveldb::Slice& key, wisdom_IOStream& io) = 0;
	virtual void remove(const string& key, wisdom_IOStream& io){}
};

class c_base
{
public:
	c_base();
	~c_base();
protected:
	void	set_effective(uint32 effective);
	uint32	get_effective();
	void	set_type(uint8 type);
	uint8	get_type();
protected:
	bool	put(const leveldb::Slice& key, leveldb::Slice& value, uint32 effective);
	bool	get(const leveldb::Slice& key, string& value);
	int		get_value(const leveldb::Slice& key, string& value, _zmsg_head** head);
	bool	write(leveldb::WriteOptions& writeOptions, leveldb::WriteBatch& batch);
	void	recycle(Iter* it, IConnect* con = NULL);
private:
	leveldb::Slice	get_Slice(leveldb::Slice& value, time_t effective);
protected:
	c_leavedb*				m_db;
	_zmsg_head				m_head;
};



#define ZERROR_RESULT(c) \
{LOG4_ERROR(c); \
io->push(ZRESULT_ERROR); \
io->push(c); \
return; }

static const char* _zerror_str(int i_error)
{
	switch (i_error)
	{
	case _zerror_not_extsis:		return "not extsis.";
	case _zerror_data_bad:			return "data bad.";
	case _zerror_crc_bad:			return "crc bad.";
	case _zerror_type_bad:			return "type bad.";
	case _zerror_illegal_access:	return "illegal access.";
	case _zerror_not_find_data:		return "not find data.";
	case _Zerror_seq_bad:			return "seq bad.";
	default:	return "unknown error.";
	}

	return "";
}

#define ZCODE_RESULT(c)	{	\
	LOG4_ERROR(_zerror_str(c)); \
	io->push(ZRESULT_ERROR); \
	io->push(_zerror_str(c)); \
	return;}
	

#define ZTYPE(s)	(((_zmsg_head*)s.c_str())->type)
#define ZTIME(s)	(((_zmsg_head*)s.c_str())->effective)


#endif
