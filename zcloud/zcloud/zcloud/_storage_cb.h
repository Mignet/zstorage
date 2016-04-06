
#ifndef _storage_cb_h
#define _storage_cb_h

#pragma once
#include "_container.h"

class _storage_cb
	: public ICB
{
public:
	_storage_cb() {}
	~_storage_cb() {}
private:
	virtual int onData(IZClient* client, wisdom_IOData& io)
	{
		switch (io->m_type)
		{
		case io_status:
			break;
		case io_integer:
			break;
		case io_data:
			break;
		case io_error:
			LOG4_ERROR("storage error:" << io->m_str);
			break;
		case io_array:
		{
			if (io->m_array.size() <= 0)
			{
				LOG4_ERROR("storage empty.");
				break;
			}
			uint32 serial = atoi(io->m_array[0]->m_ptr);

			wisdom_request request = TLS(_container)->pop(serial);
			if (request == NULL)
			{
				LOG4_ERROR(__tos("request con null. serial:" << serial));
				break;
			}

			wisdom_IOStream os = new c_ostream_array;
			os->push(itostr(request->m_serial));
			for (int i = 1; i < io->m_array.size(); i++)
			{
				os->push(io->m_array[i]->m_ptr, io->m_array[i]->m_len);
			}

			RESULT_PARAM(os, request->m_con);

		}
			break;
		default:
			break;
		}
		return 0;
	}
};


#endif
