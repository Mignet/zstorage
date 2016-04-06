#ifndef c_interval_h
#define c_interval_h


class c_interval
{
public:
	c_interval(uint32 begin, uint32 end)
		: m_begin(begin), m_end(end) {}

	c_interval(uint32 no) : m_begin(no), m_end(no) {}

	c_interval(const c_interval& no): m_begin(no.m_begin), m_end(no.m_end) {}

	bool operator > (const c_interval& no) const
	{
		if (m_begin > no.m_end)
			return true;

		return false;
	}

	bool operator < (const c_interval& no) const
	{
		if (m_end < no.m_begin)
			return true;

		return false;
	}

	bool operator == (uint32 no) const
	{
		if (no >= m_begin && no <= m_end)
			return true;

		return false;
	}

	bool operator == (const c_interval& no) const
	{
		assert(no.m_begin == no.m_end);

		if (no.m_begin >= m_begin && no.m_begin <= m_end)
			return true;

		return false;
	}
private:
	uint32	m_begin;
	uint32	m_end;
};

WISDOM_PTR(c_interval, wisdom_interval);

#endif
