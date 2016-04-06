#ifndef _CPUID_H
#define _CPUID_H

#include <string>
#include <windows.h>

using namespace std;
typedef unsigned long DWORD;
typedef __int64 LONGLONG;

static const int S = sizeof(DWORD);

class CPUID
{
public:
	CPUID() :m_eax(0),m_ebx(0),m_ecx(0),m_edx(0) {}
	virtual ~CPUID() {}

	string GetVID() //获取cpu制造商
	{
		char cVID[S*3+1];   // 字符串，用来存储制造商信息
		memset(cVID, 0, sizeof(cVID)); // 把数组清0
		Executecpuid(0); // 执行cpuid指令，使用输入参数 eax = 0
		memcpy(cVID+S*0, &m_ebx, S); // 复制前四个字符到数组
		memcpy(cVID+S*1, &m_edx, S); // 复制中间四个字符到数组
		memcpy(cVID+S*2, &m_ecx, S); // 复制最后四个字符到数组

		return string(cVID); // 以string的形式返回
	}
	string GetBrand() //获取cpu型号
	{
		const DWORD BRANDID = 0x80000002; // 从0x80000002开始，到0x80000004结束
		char cBrand[S*4*3+1];    // 用来存储商标字符串，48个字符
		memset(cBrand, 0, sizeof(cBrand));    // 初始化为0

		for (DWORD i = 0; i < 3; i++)   // 依次执行3个指令
		{
			Executecpuid(BRANDID + i); 
			// 每次执行结束后，保存四个寄存器里的asc码到数组
			memcpy(cBrand + i*S*4 + S*0, &m_eax, S);
			memcpy(cBrand + i*S*4 + S*1, &m_ebx, S);
			memcpy(cBrand + i*S*4 + S*2, &m_ecx, S);
			memcpy(cBrand + i*S*4 + S*3, &m_edx, S);
		}
		return string(cBrand); // 以string的形式返回
	}
	LONGLONG GetFrequency(DWORD sleepTime = 1000)//获取cpu主频
	{
		HANDLE hp = ::GetCurrentProcess(); //获取当前进程
		HANDLE ht = ::GetCurrentThread(); //获取当前线程

		DWORD pc = ::GetPriorityClass( hp ); //获取当前进程优先度
		DWORD tp = ::GetThreadPriority( ht ); //获取当前线程优先度

		BOOL   flag1 = FALSE , flag2 = FALSE;
		DWORD low1 = 0 , high1 = 0 , low2 = 0 , high2 = 0;

		flag1 = ::SetPriorityClass( hp , REALTIME_PRIORITY_CLASS ); //将当前进程优先度设置最高
		flag2 = ::SetThreadPriority( ht , THREAD_PRIORITY_HIGHEST ); //将当前线程优先度设置最高

		::Sleep( 10 );
		LARGE_INTEGER fq,st,ed;
		::QueryPerformanceFrequency(&fq); //精确计时
		::QueryPerformanceCounter(&st); //获得起始时间
		__asm        //获得当前cpu的时间周期数
		{
			rdtsc
				mov low1 , eax
				mov high1 , edx
		}
		::Sleep( sleepTime );     //将线程挂起片刻
		::QueryPerformanceCounter(&ed); //获得结束时间
		__asm        //获得当前cpu的时间周期数
		{
			rdtsc
				mov low2 ,eax
				mov high2 ,edx
		}


		if( flag1 )
			::SetPriorityClass( hp , pc ); //将当前进程优先度恢复原样
		if( flag2 )
			::SetThreadPriority( ht , tp ); //将当前线程优先度恢复原样

		::CloseHandle( hp );
		::CloseHandle( ht );

		//将cpu的时间周期数转化成64位整数
		LONGLONG begin = (LONGLONG)high1<<32 | low1;
		LONGLONG end = (LONGLONG)high2<<32 | low2;

		//将两次获得的cpu时间周期数除以间隔时间，即得到cpu的频率。
		//由于windows的Sleep函数有大约15毫秒的误差，故以windows的精确计时为准
		return (end - begin)*fq.QuadPart/(ed.QuadPart-st.QuadPart);
	}

private:
	void Executecpuid(DWORD veax) //用以执行cpuid指令
	{
		// 因为嵌入式的汇编代码不能识别 类成员变量
		// 所以定义四个临时变量作为过渡
		DWORD deax;
		DWORD debx;
		DWORD decx;
		DWORD dedx;

		__asm
		{
			mov eax, veax
				cpuid 
				mov deax, eax
				mov debx, ebx
				mov decx, ecx
				mov dedx, edx
		}

		m_eax = deax; // 把临时变量中的内容放入类成员变量
		m_ebx = debx;
		m_ecx = decx;
		m_edx = dedx;
	}

	DWORD m_eax;   // 存储返回的eax
	DWORD m_ebx;   // 存储返回的ebx
	DWORD m_ecx;   // 存储返回的ecx
	DWORD m_edx;   // 存储返回的edx
};

#endif 