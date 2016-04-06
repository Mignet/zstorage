#if !defined(__THREAD_H__)
#define __THREAD_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace zn {

	class c_thread
	{
	public:
		c_thread()
#ifdef _MSC_VER
			: m_hThread(NULL)
#else
#endif
		{
			m_bRunning = false;
		}

		virtual ~c_thread()
		{
#ifdef _MSC_VER
			::CloseHandle(m_hThread);
#else

#endif
		}
	public:
		int Start()
			{
			//线程已经被启动
			m_bRunning = true;

#ifdef _MSC_VER
			m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_uThreadID);
			//m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, &m_uThreadID);
			if (m_hThread == NULL)
				return -1;
#else
			pthread_create(&m_uThreadID, 0, ThreadProc, (void*)this);
#endif

			return 0;
		}
		
		bool& Get_Running()
		{
			return m_bRunning; 
		}

		void Stop()
		{
			m_bRunning = false;
		}
		
		unsigned long thread_id()
		{
			return m_uThreadID;
		}

		virtual void Execute()
		{
			//线程被启动
			while (Get_Running())
			{

			}
		}
		
	protected:
		//线程函数体
#ifdef _MSC_VER
		static unsigned long __stdcall ThreadProc(void* lParam)
#else
		static void* ThreadProc(void* lParam)
#endif
		{
			c_thread * pThis = 
				reinterpret_cast<c_thread*>(lParam);
			if(pThis)
			{
				pThis->Execute();
			}

			return 0;
		}

	protected:
#ifdef _MSC_VER
		HANDLE				m_hThread;
#endif
		thread_t			m_uThreadID;
		bool				m_bRunning;
	};
} // end BaseLib

#endif // !defined(__THREAD_H__)
