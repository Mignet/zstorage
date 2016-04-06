#ifndef system_h_
#define system_h_

#ifdef _MSC_VER
#include <Psapi.h>
#include <assert.h>
#include "cpuid.h"
#include "MyTypes.h"
#pragma comment (lib, "Psapi.lib")
#else
#endif


namespace zn
{
#ifdef _MSC_VER
	//获取内存使用
	static int get_memory_usage(uint64_t* mem, uint64_t* vmem)  
	{  
		PROCESS_MEMORY_COUNTERS pmc;  
		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))  
		{  
			if(mem) *mem = pmc.WorkingSetSize;  
			if(vmem) *vmem = pmc.PagefileUsage;  
			return 0;  
		}  
		return -1;  
	}  

	// 获得CPU的核数  
	static int get_processor_number()  
	{  
		SYSTEM_INFO info;  
		GetSystemInfo(&info);  
		return (int)info.dwNumberOfProcessors;  
	}  

	/// 时间转换  
	static uint64_t file_time_2_utc(const FILETIME* ftime)  
	{  
		LARGE_INTEGER li;  
		li.LowPart = ftime->dwLowDateTime;  
		li.HighPart = ftime->dwHighDateTime;  
		return li.QuadPart;  
	}  

	//获取CPU占用
	static int get_cpu_usage()  
	{  
		//cpu数量  
		static int processor_count_ = -1;  
		//上一次的时间  
		static int64_t last_time_ = 0;  
		static int64_t last_system_time_ = 0;

		FILETIME now;  
		FILETIME creation_time;  
		FILETIME exit_time;  
		FILETIME kernel_time;  
		FILETIME user_time;  
		int64_t system_time;  
		int64_t time;  
		int64_t system_time_delta;  
		int64_t time_delta;  

		int cpu = -1;  


		if(processor_count_ == -1)  
		{  
			processor_count_ = get_processor_number();  
		}  

		GetSystemTimeAsFileTime(&now);  

		if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,  
			&kernel_time, &user_time))  
		{  
			// We don't assert here because in some cases (such as in the Task Manager)  
			// we may call this function on a process that has just exited but we have  
			// not yet received the notification.  
			return 0;  
		}  
		system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time))   

			/  
			processor_count_;  
		time = file_time_2_utc(&now);  

		if ((last_system_time_ == 0) || (last_time_ == 0))  
		{  
			// First call, just set the last values.  
			last_system_time_ = system_time;  
			last_time_ = time;  
			return 0;  
		}  

		system_time_delta = system_time - last_system_time_;  
		time_delta = time - last_time_; 

		if (time_delta == 0)  
			return 0;  

		// We add time_delta / 2 so the result is rounded.  
		cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);  
		last_system_time_ = system_time;  
		last_time_ = time;  
		return cpu;  
	} 

	//获取IO读写
	static int get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes)  
	{  
		IO_COUNTERS io_counter;  
		if(GetProcessIoCounters(GetCurrentProcess(), &io_counter))  
		{  
			if(read_bytes) *read_bytes = io_counter.ReadTransferCount;  
			if(write_bytes) *write_bytes = io_counter.WriteTransferCount;  
			return 0;  
		}  
		return -1;  
	}


#define GHZ_UNIT (1000*1000*1000)
#define MHZ_UNIT (1000*1000)

	//获取CPU信息
	static string getCPUInfo()
	{
		char tmp[1024];
		string result;
		CPUID cpu;
		result += cpu.GetBrand();
		result += "\t";
		LONGLONG freq = cpu.GetFrequency();
		char szfreq[1024];
		if (freq > GHZ_UNIT)
		{
			double freqGHz = (double)freq/GHZ_UNIT;
			sprintf(szfreq, "%.2fGHz",freqGHz); //保留小数点后两位输出
		}
		else
		{
			double freqGHz = (double)freq/MHZ_UNIT;
			sprintf(szfreq, "%.2fMHz",freqGHz); //保留小数点后两位输出
		}

		result += szfreq;


		sprintf(tmp, "%u number of CPU", get_processor_number());
		result += "\t";
		result += tmp;

		sprintf(tmp, "\r\n\tCPU usage %u", get_cpu_usage());
		result += tmp;

		MEMORYSTATUS memory;
		GlobalMemoryStatus(&memory);
		sprintf(tmp, "\r\n\tmemory %d M use memory %d M",
			memory.dwTotalPageFile / (1024*1024),
			(memory.dwTotalPageFile - memory.dwAvailPageFile) / (1024*1024));
		result += tmp;

		uint64_t mem, vmem;
		get_memory_usage(&mem, &vmem);
		sprintf(tmp, "\r\n\tUsing Memory %d K Virtual Memory %d K", (long)(mem / 1024), (long)(vmem / 1024));
		result += tmp;

		uint64_t read_bytes, write_bytes;
		get_io_bytes(&read_bytes, &write_bytes);
		sprintf(tmp, "\r\n\tread bytes %d K write bytes %d K", (long)(read_bytes / 1024), (long)(write_bytes / 1024));
		result += tmp;

		return result;
	}
#else				//linux


//linux 	
#define		cpu_name		"model name"
#define		cpu_MHz			"cpu MHz"
#define		cpu_count		"processor"
#define		mem_total		"MemTotal"
#define		mem_free		"MemFree"

static string get_cpuname()
{
	string result;
	FILE* pf = fopen(Z_CPUINFO, "rb");
	if (pf == NULL)
	{
		assert(0);
		return "";
	}

	char line[1024];

	const char* pos = NULL;
	while (fgets(line, sizeof(line), pf))
	{
		if (strncmp(line, cpu_name, strlen(cpu_name)) == 0)
		{
			pos = strchr(line, ':');
			pos++;
			result = zn::str_trim(pos);
			break;
		}
	}
	fclose(pf);

	return result;
}

static string get_cpuMHz()
{
	string result;
	FILE* pf = fopen(Z_CPUINFO, "rb");
	if (pf == NULL)
	{
		assert(0);
		return "";
	}

	char line[1024];

	const char* pos = NULL;
	while (fgets(line, sizeof(line), pf))
	{
		if (strncmp(line, cpu_MHz, strlen(cpu_MHz)) == 0)
		{
			pos = strchr(line, ':');
			pos++;
			result = zn::str_trim(pos);
			break;
		}
	}
	fclose(pf);

	return result;
}

static string get_cpu_count()
{
	string result;
	FILE* pf = fopen(Z_CPUINFO, "rb");
	if (pf == NULL)
	{
		assert(0);
		return "1";
	}

	char line[1024];

	const char* pos = NULL;
	while (fgets(line, sizeof(line), pf))
	{
		if (strncmp(line, cpu_count, strlen(cpu_count)) == 0)
		{
			pos = strchr(line, ':');
			pos++;
			char tmp[20];
			sprintf(tmp, "%d", atoi(zn::str_trim(pos).c_str()) + 1);
			result = tmp;
		}
	}
	fclose(pf);

	return result;
}

static string get_total_mem()
{
	string result;
	FILE* pf = fopen(Z_MEMINFO, "rb");
	if (pf == NULL)
	{
		assert(0);
		return "1";
	}

	char line[1024];

	const char* pos = NULL;
	while (fgets(line, sizeof(line), pf))
	{
		if (strncmp(line, mem_total, strlen(mem_total)) == 0)
		{
			pos = strchr(line, ':');
			pos++;
			result = zn::str_trim(pos);
			break;
		}
	}
	fclose(pf);

	return result;
}

static string get_free_mem()
{
	string result;
	FILE* pf = fopen(Z_MEMINFO, "rb");
	if (pf == NULL)
	{
		assert(0);
		return "1";
	}

	char line[1024];

	const char* pos = NULL;
	while (fgets(line, sizeof(line), pf))
	{
		if (strncmp(line, mem_free, strlen(mem_free)) == 0)
		{
			pos = strchr(line, ':');
			pos++;
			result = zn::str_trim(pos);
			break;
		}
	}
	fclose(pf);

	return result;
}

typedef struct PACKED			//定义一个cpu occupy的结构体
{
	char name[20];				//定义一个char类型的数组名name有20个元素
	unsigned int user;			//定义一个无符号的int类型的user
	unsigned int nice;			//定义一个无符号的int类型的nice
	unsigned int system;		//定义一个无符号的int类型的system
	unsigned int idle;			//定义一个无符号的int类型的idle
}CPU_OCCUPY;

static void get_cpuoccupy (CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{   
	FILE *fd;                    
	char buff[256]; 
	CPU_OCCUPY *cpu_occupy;
	cpu_occupy=cpust;

	fd = fopen (Z_STAT, "rb"); 
	fgets (buff, sizeof(buff), fd);

	sscanf (buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);

	fclose(fd);     
}

static string get_pids()
{
	string result;
	char pid_path[100];
	sprintf(pid_path, "/proc/%u/task", getpid());

	struct dirent *ptr;    
	DIR *pdir = opendir(pid_path);
	if (pdir == NULL)
	{
		return "";
	}

	while ((ptr = readdir(pdir)) != NULL )
	{
		//跳过'.'和'..'两个目录
		if(ptr->d_name[0] == '.')
			continue;

		if (!result.empty())
			result += ",";

		result += ptr->d_name;
	}

	closedir(pdir);

	return result;

}

static bool z_top(string& strtop)
{
	char system_top[1024];
	char line[1024];
	string result;
	char top_file[100];
	sprintf(top_file, "cpu.top");
	sprintf(system_top, "top -b -n 1 -d 0 -p %s", get_pids().c_str());
	//调用系统函数
	FILE* pp = NULL;
	if( (pp = popen(system_top, "r")) == NULL )
    {
        printf("popen(%s) error!\n", system_top);
		return false;
    }
	while (fgets(line, sizeof(line), pp))
	{
		printf("%s\n", line);
		strtop += line;
	}

	pclose(pp);


	printf("popen(%s)\n", system_top);

	return true;
}

//获取CPU信息
static string getCPUInfo()
{
	string result;
	result += get_cpuname();
	result += "\t";
	result += get_cpuMHz();
	result += "\t";
	result += get_cpu_count();
	result +=  " number of CPU\r\n\r\n\r\n";
	return result;
}

#endif
}


#endif

