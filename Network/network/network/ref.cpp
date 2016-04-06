#include "stdafx.h"
#include "ref.h"

CRef::CRef(void)
{
	_atom_init(m_ref);
	_atom_add_one(&m_ref);
}

CRef::~CRef(void)
{

}

int32_t CRef::add()
{
	return _atom_add_one(&m_ref);
}

void CRef::dec()
{
	if ( _atom_sub_one(&m_ref) == 0)
	{
		on_release();
		return;
	}
}
