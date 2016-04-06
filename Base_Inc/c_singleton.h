
//C++单体模式
#ifndef	_c_singleton_h_ 
#define _c_singleton_h_

#pragma once

template<class T>
class c_singleton
{
public:
	static T* get_instance() {static T type; return &type;}
};


#endif //_c_singleton_h_

