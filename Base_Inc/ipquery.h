#ifndef ipquery_h
#define ipquery_h

#include "c_singleton.h"

namespace ip_query
{
#pragma pack(push, 1)
	typedef struct st_ipinfo
	{
		unsigned int	m_begin;
		unsigned int	m_end;
		char			m_area[100];
		char			m_agent[100];
	}st_ipinfo;

#ifdef _MSC_VER
#define ip_sorcue_file			"ip\\ipsorcue"
#define ip_config_file			"ip\\ipconfig"
#else
#define ip_sorcue_file			"ip/ipsorcue"
#define ip_config_file			"ip/ipconfig"
#endif

#pragma pack (pop)
	class ipobject
	{
	public:
		ipobject(st_ipinfo& info)
		{
			memcpy(&m_info, &info, sizeof(info));
		}
		ipobject(const char* ipline)
		{
			memset(&m_info, 0, sizeof(m_info));
			vector<char> begin;
			vector<char> end;
			vector<char> area;
			vector<char> agent;
			const char* line = ipline;
			int len = strlen(ipline);
			const char* end_line = ipline + len;

			//BEGIN IP
			while (*line && 
				*line != ' ' && *line != '\r' && *line != '\n'
				&& line < end_line) begin.push_back(*line++);
			while (*line && *line == ' ' && line < end_line) line++;
			begin.push_back(0);
			//END IP
			while (*line && 
				*line != ' ' && *line != '\r' && *line != '\n'
				&& line < end_line) end.push_back(*line++);
			while (*line && *line == ' ' && line < end_line) line++;
			end.push_back(0);
			//AREA
			while (*line &&
				*line != ' ' && *line != '\r' && *line != '\n'
				&& line < end_line) area.push_back(*line++);
			while (*line && *line == ' ' && line < end_line) line++;
			area.push_back(0);
			//AGENT
			while (*line 
				&& *line != ' ' && *line != '\r' && *line != '\n'
				&& line < end_line) agent.push_back(*line++);
			while (*line && *line == ' ' && line < end_line) line++;
			agent.push_back(0);

			//printf("%s	%s	%s	%s\r\n", &begin[0], &end[0], &m_area[0], &m_agent[0]);
			m_info.m_begin = ntohl(inet_addr(&begin[0]));
			m_info.m_end = ntohl(inet_addr(&end[0]));
			
			string str_area = &area[0];
			string str_agent = &agent[0];

			if (area.size() > 1 && area.size() < sizeof(m_info.m_area))
				zn::nstrcpy(m_info.m_area, zn::str_trim(&area[0]).c_str(), sizeof(m_info.m_area));

			if (agent.size() > 1 && agent.size() < sizeof(m_info.m_agent))
				zn::nstrcpy(m_info.m_agent, zn::str_trim(&agent[0]).c_str(), sizeof(m_info.m_agent));
		}

		bool write(FILE* pf)
		{
			return fwrite(&m_info, 1, sizeof(m_info), pf) == sizeof(m_info);
		}
	public:
		const char* area() {return m_info.m_area;}
		const char* agent() {return m_info.m_agent;}
		unsigned int begin() {return m_info.m_begin;}
		unsigned int end() {return m_info.m_end;}
	private:
		st_ipinfo		m_info;
	};

	class ipoperator
	{
	public:
		ipoperator(st_ipinfo& info)
			: m_begin(info.m_begin), m_end(info.m_end) {}

		ipoperator(unsigned int begin, unsigned int end)
			: m_begin(begin), m_end(end) {}

		ipoperator(const char* ip)
		{
			unsigned int iaddr = ntohl(inet_addr(ip));
			m_begin = iaddr;
			m_end = iaddr;
		}

		ipoperator(const ipoperator& oper): m_begin(oper.m_begin), m_end(oper.m_end) {}


		bool operator > (const ipoperator& ipaddr) const
		{
			if (m_begin > ipaddr.m_end)
				return true;

			return false;
		}

		bool operator < (const ipoperator& ipaddr) const
		{
			if (m_end < ipaddr.m_begin)
				return true;

			return false;
		}

		bool operator == (const char* ip) const
		{
			unsigned int iaddr = ntohl(inet_addr(ip));

			if (iaddr >= m_begin && iaddr <= m_end)
				return true;

			return false;
		}

		bool operator == (const ipoperator& ipaddr) const
		{
			assert(ipaddr.m_begin == ipaddr.m_end);

			if (ipaddr.m_begin >= m_begin && ipaddr.m_begin <= m_end)
				return true;

			return false;
		}
	private:
		unsigned int	m_begin;
		unsigned int	m_end;
	};

	

	class ipquery
		: public c_singleton<ipquery>
	{
		typedef map<ipoperator, ipobject*>	MapIP;
	public:
		void ipquery_init()
		{
			char line[1024];
			int i = 0;

			//¥Ê‘⁄≈‰÷√Œƒº˛
			if (zn::file_exists(ip_config_file))
			{
				ip_query::st_ipinfo ipinfo;
				FILE* pf = fopen(ip_config_file, "rb");
				if (!pf) assert(0);
				while (fread(&ipinfo, 1, sizeof(ipinfo), pf) == sizeof(ipinfo))
				{
					if (ipinfo.m_end <= 0) continue;
					ip_query::ipobject* object = new ip_query::ipobject(ipinfo);
					ip_query::ipoperator oper(object->begin(), object->end());
					m_mapip.insert(make_pair(oper, object));
				}
				fclose(pf);
			}
			else if (zn::file_exists(ip_sorcue_file))
			{
				remove(ip_config_file);
				FILE* pf = fopen(ip_sorcue_file, "rb");
				FILE* wf = fopen(ip_config_file, "wb");
				if (!pf) assert(0);
				if (!wf) assert(0);

				while (fgets(line, sizeof(line), pf))
				{
					ip_query::ipobject* object = new ip_query::ipobject(line);
					ip_query::ipoperator oper(object->begin(), object->end());
					if (!object->write(wf)) assert(0);
					m_mapip.insert(make_pair(oper, object));
					i++;
					if (i % 1000 == 0)
						printf("\r%u", i);
				}
				fclose(pf);
				fclose(wf);
			}
		}

		const char* area(const char* ipaddr)
		{
			MapIP::iterator iter = m_mapip.find(ipaddr);
			if (iter != m_mapip.end())
			{
				return iter->second->area();
			}

			return "";
		}

		const char* agent(const char* ipaddr)
		{
			MapIP::iterator iter = m_mapip.find(ipaddr);
			if (iter != m_mapip.end())
			{
				return iter->second->agent();
			}

			return "";
		}

	private:
		MapIP		m_mapip;
	};
}



#endif //ipquery_h

