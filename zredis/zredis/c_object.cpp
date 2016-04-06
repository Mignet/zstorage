#include "stdafx.h"
#include "c_object.h"
#include "c_command.h"

void c_object::deal( wisdom_param& param, IConnect* con )
{
	if (!power(con->get_power())) 
	{	
		wisdom_IOStream result = new c_ostream_error;
		result->push("permission denied.");
		RESULT_PARAM(result, con);
		return;
	}

	execute(param, con);
}
