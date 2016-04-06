#include "stdafx.h"
#include "c_auth.h"
#include "c_object.h"


#define AUTH_SYSTEM()	\
	wisdom_auth auth = new STAuth;	\
	auth->account="zsoft";auth->pwd="zxcvbnmlkjhgfdsa";auth->power=255; \
	m_auth.insert(make_pair(auth->account, auth));
#define AUTH_SYSTEM_LIMIT(x) x == "zsoft"

c_auth::c_auth(void)
{
}


c_auth::~c_auth(void)
{
}

bool c_auth::loadAuth()
{
	c_wlock lock(&m_lock);

	m_auth.clear();
	AUTH_SYSTEM();
	CMarkupSTL config;
	if(!config.Load(TS_CONFIG))
		return false;
	//¸ù½Úµã
	config.FindElem();

	if(config.FindChildElem("auth", true))
	{
		config.IntoElem();

		while(config.FindChildElem("user"))
		{
			wisdom_auth auth = new STAuth;
			config.IntoElem();
			if (config.FindChildElem("account", true))
			{
				auth->account = config.GetChildData();
			}

			if (config.FindChildElem("pwd", true))
			{
				auth->pwd = config.GetChildData();
			}

			if (config.FindChildElem("power", true))
			{
				auth->power = atoi(config.GetChildData().c_str());
			}

			config.OutOfElem();

			m_auth.insert(make_pair(auth->account, auth));
		}

		config.OutOfElem();
	}

	return true;
}

bool c_auth::auth( const string& account, const string& pwd )
{
	c_rlock lock(&m_lock);
	MAUTH::iterator pos = m_auth.find(account);
	if (pos == m_auth.end())
		return false;

	if (pos->second->pwd != pwd)
		return false;

	return true;
}

uint8 c_auth::power( const string& account )
{
	c_rlock lock(&m_lock);

	MAUTH::iterator pos = m_auth.find(account);
	if (pos == m_auth.end())
		return 0;

	return pos->second->power;
}

void c_auth::list( wisdom_IOStream& os )
{
	c_rlock lock(&m_lock);
	for (MAUTH::iterator pos = m_auth.begin(); pos != m_auth.end(); ++pos)
	{
		if (AUTH_SYSTEM_LIMIT(pos->first))
			continue;
		ZOS zos;
		os->push((zos << pos->second->account 
			<< "@" << pos->second->pwd 
			<< "*" << pos->second->power).str());
	}
}

bool c_auth::check_account( const string& account )
{
	c_rlock lock(&m_lock);
	return m_auth.find(account) != m_auth.end();
}

int c_auth::on_timer( mw_net_timer *timer )
{
	loadAuth();
	return 0;
}

bool c_auth::init()
{
	loadAuth();
	mw_net_timer* timer = create_net_timer(this);
	timer->_schedule(1000 * 1000 * 10);
	return 0;
}
