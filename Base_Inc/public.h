
#ifndef _public_h_
#define _public_h_

#ifdef _MSC_VER
#include <wtypes.h>
#include <tchar.h>
#include <sys/stat.h>
#include <atlbase.h>
#include <direct.h>
#include <time.h>
#else
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <netdb.h>
#include <dirent.h>
#include <iconv.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <vector>
#include <string>
#include "df_macro.h"
#include "MyTypes.h"
#include "tls_pools.h"
#define MAX_MSG_LENGTH	1024 * 32

using namespace std;


namespace zn
{
	//线程变量
	class tls_buffer
	{
	public:
		char	buffer[8196];
	};

	static char* tls_buf()
	{
		return TlsSingelton<tls_buffer>::tlsInstance()->buffer;
	}

	static string inet_ntoa(uint32 address)
	{
		sockaddr_in addrSrv;
		addrSrv.sin_addr.s_addr = address;
		string ipaddr = inet_ntoa(addrSrv.sin_addr);
		return ipaddr;
	}

	static uint32 x16format_int32(const string& str)
	{
		uint32 _i32 = 0;
		if (str.length() > 1 && str.c_str()[1] == 'x')
			sscanf(str.c_str(), "0x%x", &_i32);
		else if (str.length() > 1 && str.c_str()[1] == 'X')
			sscanf(str.c_str(), "0X%x", &_i32);
		else
			sscanf(str.c_str(), "%x", &_i32);
		
		return _i32;
	}

	const char ltoa_str[] =	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	static int _zltoa(int64_t num, char *str, int radix, int pos)
	{
		int result = 0;
		if (num > 0)
		{
			result = _zltoa(num / radix, str, radix, ++pos);
			str[result] = ltoa_str[num % radix];
			return result + 1;
		}
		else
		{
			str[pos] = '\0';
			return 0;
		}
	}

	static char * zltoa(int64_t num, char *str, int radix)   
	{
		int pos = 0;
		if (num == 0) //0 需要特殊处理
		{
			str[0] = '0';
			str[1] = '\0';
		}
		else if (num < 0 && radix == 10) //十进制特殊处理
		{
			*str = '-';
			_zltoa(-num, &str[1], radix, pos);
		}	
		else //大于0情况
		{
			_zltoa(num, str, radix, pos);
		}
		return str;
	}

	

#ifdef _MSC_VER
	static string domain(const string& host)
	{
		struct hostent *answer = gethostbyname(host.c_str());
		if (answer == NULL) {
			return "";
		}
	
		if ((answer->h_addr_list)[0] != NULL)
			return inet_ntoa(*(in_addr*)answer->h_addr_list[0]);

		return "";
	}
#else
	static struct hostent *gethostnameinfo (const char *host)
	{
		struct hostent hostbuf, *hp;
		size_t hstbuflen;
		char tmphstbuf[1024];
		int res;
		int herr;

		hstbuflen = 1024;

		res = gethostbyname_r (host, &hostbuf, tmphstbuf, hstbuflen,
			&hp, &herr);
		/*  Check for errors.  */
		if (res || hp == NULL)
			return NULL;
		return hp;
	}

	static string domain(const string& host)
	{
		unsigned int ip=0;
		struct hostent* pent = gethostnameinfo(host.c_str());
		if( NULL == pent || NULL == pent->h_addr )
			return "0.0.0.0";
		memcpy( &ip, pent->h_addr, 4 );
		return zn::inet_ntoa(ip);
	}
#endif

#ifndef _MSC_VER
	static char *strupr(char *str) 
	{ 
		char *ptr = str; 

		while (*ptr) { 
			if (islower(*ptr)) 
			*ptr = toupper(*ptr); 
			ptr++;
		}
		return str;
	} 

#endif


#ifdef _MSC_VER
	static int gettimeofday(struct timeval* tv, void*) 
	{
		union {
			long long ns100;
			FILETIME ft;
		} now;

		GetSystemTimeAsFileTime (&now.ft);
		tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
		tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
		return (0);
	}
#endif

	static uint64_t get_ms()
	{
		struct timeval tl;
		gettimeofday(&tl, NULL);
		return tl.tv_sec * 1000 + tl.tv_usec / 1000 ;
	}

	static string output_16x(unsigned char * buf, int len)
	{
		string result;
		for(int i = 0 ; i < len ; i ++)
		{
			char sztmp[10];
			sprintf(sztmp, " %02x ", buf[i]);

			result += sztmp;
		}
		result += " ";
		return result;
	}

	static string itostr(long i)
	{
		char str[20];
		//sprintf(str, "%llu", i);
		zltoa(i, str, 10);
		return str;
	}
	
	static bool check_md5(char* md5, int len)
	{
		const char* cmd5 = "1234567890abcdefABCDEF";

		for (int i = 0; i < min(32, len); i++)
		{
			if (!strchr(cmd5, md5[i]))
				return false;
		}

		return true;
	}
	// 替换字符串中特征字符串为指定字符串

	static string replace_str(const char *src, const char *sMatchStr, const char *sReplaceStr)
	{

		int  StringLen;
		
		char cresult[MAX_MSG_LENGTH];
		char caNewString[MAX_MSG_LENGTH];

		strcpy(cresult, src);

		const char *FindPos = strstr(cresult, sMatchStr);

		if( (!FindPos) || (!sMatchStr) )
			return cresult;


		while( FindPos )

		{

			memset(caNewString, 0, sizeof(caNewString));

			StringLen = FindPos - cresult;

			strncpy(caNewString, cresult, StringLen);

			strcat(caNewString, sReplaceStr);

			strcat(caNewString, FindPos + strlen(sMatchStr));

			strcpy(cresult, caNewString);

			FindPos = strstr(cresult, sMatchStr);

		}


		return cresult;
	}

	static string SQLCommareplace(string sql, char c)
	{
		vector<char> v;

		for (string::iterator iter = sql.begin();
			iter != sql.end(); ++iter)
		{
			v.push_back(*iter);
			if (*iter == c)
			{
				v.push_back(*iter);
			}
		}
		v.push_back(0);

		return &v[0];
	}

	static string mysql_escape(const char* sql)
	{
		vector<char> v;
		char* iter = (char*)sql;
		for (; *iter; ++iter)
		{
			switch (*iter)
			{
			case '\n':
			case '\r':
			case '\\':
			case '%':
				v.push_back('\\');
				break;
			default:
				break;
			}

			v.push_back(*iter);
		}
		v.push_back(0);

		return &v[0];
	}

	static string json_escape(const char* sql)
	{
		vector<char> v;
		char* iter = (char*)sql;
		for (; *iter; ++iter)
		{
			switch (*iter)
			{
			case '\n':
			case '\t':
			case '\r':
			case ' ':
				break;
			default:
				v.push_back(*iter);
				break;
			}

			
		}
		v.push_back(0);

		return &v[0];
	}

	static bool isdir(const char* path)
	{
#ifdef _MSC_VER
		struct _stat64 buf;
		if (_stat64(path, &buf) == -1) {
			return false;
		} else {
			if (S_ISDIR(buf.st_mode)) {		
				return true;
			}   
		}
#else
		struct stat64 buf;
		if (stat64(path, &buf) == -1) {
			return false;
		} else {
			if (S_ISDIR(buf.st_mode)) {
				return true;
			}
		}
#endif

		return false;
	}

	static bool isfile(const char* fpath)
	{
#ifdef _MSC_VER
		struct _stat64 buf;
		if (_stat64(fpath, &buf) == -1) {
			return false;
		} else {
			if (!S_ISDIR(buf.st_mode)) {		
				return true;
			}   
		}
#else
		struct stat64 buf;
		if (stat64(fpath, &buf) == -1) {
			return false;
		} else {
			if (!S_ISDIR(buf.st_mode)) {
				return true;
			}
		}
#endif
		return false;
	}

	static void nstrcpy(char* dest, const char* sour, int size)
	{
		if (sour != NULL && size > 0)
		{
			strncpy(dest, sour, size);
			dest[size - 1] = 0;
		}
	}

	static int zstrcmp(const char* src, const char* dst, int n)
	{
		for (int i = 0; i < n; i++)
		{
			if (src[i] > dst[i])
				return 1;
			else if(src[i] < dst[i])
				return -1;
		}

		return 0;
	}
	// str:1234567890 len:10 sub:67 n:2
	static const char* zstrstr(const char* str, int len, const char* sub, int n)
	{
		for ( int i = 0; i <= len - n; i++ )
		{
			if (memcmp(&str[i], sub, n) == 0)
				return &str[i];
		}

		return NULL;
	}

	static void makedir(const char* path)
	{
#ifdef _MSC_VER
		if (strlen(path) <= 3)
			return;
#else
		if (strlen(path) <= 1)
			return;
#endif
		if (isdir(path)) return;

		char szpath[260];
		nstrcpy(szpath, path, sizeof(szpath));
		
		char* pos = strrchr(szpath, '\\');
		if (pos == 0)
		{
			pos = strrchr(szpath, '/');
		}

		if (pos)
			*pos = 0;
		
#ifdef _MSC_VER
		if (strlen(szpath) <= 2)
			strcat(szpath, "\\");

		makedir(szpath);

		if (!isdir(path)) _mkdir(path);
#else
		if (strlen(szpath) <= 2)
			strcat(szpath, "\\");

		makedir(szpath);

		if (!isdir(path)) mkdir(path, S_IRWXU);
#endif

	}

	static string str_itoa(int n)
	{
		char str[64];
		sprintf(str, "%d", n);
		return str;
	}


	static string str_ltrim(const string& str) 
	{ 
		return str.substr(str.find_first_not_of(" \n\r\t")); 
	} 

	static string str_rtrim(const string& str) 
	{ 
		return str.substr(0,str.find_last_not_of(" \n\r\t")+1); 
	} 

	static string str_trim(const string& str) 
	{ 
		return str_ltrim(str_rtrim(str)); 
	}

	//转大写
	static string str_toupper(const char* c)
	{
		char* r = strdup(c);
		for (int i = 0; r[i] != 0; i++)
		{
			r[i] = toupper(r[i]);
		}
		string result = r;
		free(r);
		return result;
	}

	//转小写
	static string str_tolower(const char* c)
	{
		char* r = strdup(c);
		for (int i = 0; r[i] != 0; i++)
		{
			r[i] = tolower(r[i]);
		}
		string result = r;
		free(r);
		return result;
	}

	static string byte_tran_char(uint64_t ubyte)
	{
		char tran_char[64];
		if (ubyte / (1024 * 1024 * 1024) > 0)
		{
			sprintf(tran_char, "%.1f GB", ubyte / (1024 * 1024 * 1024.0));
		}
		else if (ubyte / (1024 * 1024) > 0)
		{
			sprintf(tran_char, "%.1f MB", ubyte / (1024 * 1024.0));
		}
		else if (ubyte / 1024 > 0)
		{
			sprintf(tran_char, "%.1f KB", ubyte / (1024.0));
		}
		else
		{
			sprintf(tran_char, "%llu B", ubyte);
		}

		return tran_char;
	}

	static void string_split(const char* str, char split, vector<string>& v)
	{
		char sub[260];
		const char* pbegin = str;
		const char* pos = NULL;
		do 
		{
			pos = strchr(pbegin, split);
			if (pos != NULL)
			{
				memset(sub, 0, sizeof(sub));
				strncpy(sub, pbegin, pos - pbegin);
				pbegin = pos + 1;
				v.push_back(sub);
			}

		} while (pos);
	}
#ifndef _MSC_VER
	static void guard_process(const char* exeName, int seconds)
	{
		pid_t pid;

FORK_LOOP:    
		pid = fork();

		if(pid < -1)
		{
			fprintf(stdout, "\r\n%s create child process error.\r\n", exeName);
			exit(-1);
		}
		else if(pid > 0)
		{
			int nStatus;
			pid_t childPid;

			if((childPid = waitpid(pid, &nStatus, 0)) < 0)
				exit(-1);
			if(0 == nStatus)
				exit(-1);

			if(WIFSIGNALED(nStatus))
			{
				char szCoreFile[1024] = {0};
				fprintf(stdout, "\r\n\r\n%s child process quitted for signal %d, %s will create anther child process %d seconds later.\r\n\r\n", exeName, WTERMSIG(nStatus), exeName, seconds);
				sprintf(szCoreFile, "core.%d", pid);
				chmod(szCoreFile, 0666);
			}
			sleep(seconds);
			goto FORK_LOOP;
		}
	}
#endif

	static uint64_t getfilesize(const char* filename)
	{
		int file_exists = 1;

#ifdef _MSC_VER
		struct _stat64 buf;
		if (_stat64(filename, &buf) == -1) {
			file_exists = 0;
		} else {
			if (S_ISDIR(buf.st_mode)) {		
				file_exists = 0;
			}   
		}
#else
		struct stat64 buf;
		if (stat64(filename, &buf) == -1) {
			file_exists = 0;
		} else {
			if (S_ISDIR(buf.st_mode)) {
				file_exists = 0;
			}
		}

#endif

		if (!file_exists)
			return 0;

		return buf.st_size;
	}

	static const char* getFilename(const char* fullname)
	{
		const char *pos, *pos1, *pos2;
		pos1 = strrchr(fullname, '\\');
		pos2 = strrchr(fullname, '/');
		pos = pos1 > pos2 ? pos1 : pos2;
		
		return pos == NULL ? fullname : &pos[1];
	}

	static const char* getExtname(const char* fullname)
	{
		const char* pos = strrchr(fullname, '.');

		return pos == NULL ? "" : &pos[1];
	}

	static string getExeFullPath()
	{
		string strExeFullPath;
		char szFullPath[1024]	= {0};
#ifdef _MSC_VER	
		GetModuleFileNameA(NULL, szFullPath, 1024);
#else
		int rslt = readlink("/proc/self/exe", szFullPath, 1024); 
#endif
		strExeFullPath = szFullPath;
		return strExeFullPath;
	}

	static string exe_path()
	{
		string strExePath = getExeFullPath();

#ifdef _MSC_VER
		CHAR szDrive[_MAX_DRIVE]		= {0};
		CHAR szPath[_MAX_DIR]			= {0};
		CHAR szTitle[_MAX_FNAME]		= {0};
		CHAR szExt[_MAX_EXT]			= {0};	

		CHAR szFullPath[MAX_PATH]		= {0};
		strcpy(szFullPath, strExePath.c_str());
		::_splitpath(szFullPath, szDrive, szPath, szTitle, szExt);
		::_makepath(szFullPath, szDrive, szPath, NULL, NULL);
		strExePath = szFullPath;
#else
		char szFullPath[1024] = {0};
		strcpy(szFullPath, strExePath.c_str());

		size_t len = strlen(szFullPath);
		while(1)
		{
			len --;
			if (szFullPath[len] == '/')
			{
				szFullPath[len+1] = '\0';
				break;
			}
		}
		strExePath = szFullPath;
#endif
		return strExePath;
	}



	static bool file_exists(const char* filename)
	{
		struct stat fst = {0};
#ifdef WIN32
		if(_stat(filename, (struct _stat*)&fst) == 0)
#else
		if(stat(filename, &fst) == 0)
#endif
			return true;

		return false;
	}

	static bool query_file(const char* file, int& filesize, time_t& filetime)
	{
		struct stat fst = {0};
#if defined(WIN32)
		int nRet = _stat(file, (struct _stat*)&fst);
#else
		int nRet = stat(file, &fst);
#endif
		if (nRet == 0)
		{
			filesize = fst.st_size;
			filetime = fst.st_mtime;
			return true;
		}

		return false;
	}

	static string path_conv(string path)
	{
		vector<char> vpath;
		for (string::iterator iter = path.begin(); iter != path.end(); ++iter)
		{
#ifdef _MSC_VER
			if (*iter == '/')
				vpath.push_back('\\');
#else
			if (*iter == '\\')
				vpath.push_back('/');
#endif
			else
				vpath.push_back(*iter);
		}
		vpath.push_back(0);

		return &vpath[0];
	}

#ifdef _MSC_VER
	static wstring AnsiToUnicode(const char* buf)
	{
		int len = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
		if (len == 0) return L"";

		std::vector<wchar_t> unicode(len);
		::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], len);

		return &unicode[0];
	}

	static string UnicodeToAnsi(const wchar_t* buf)
	{
		int len = ::WideCharToMultiByte(CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL);
		if (len == 0) return "";

		std::vector<char> utf8(len);
		::WideCharToMultiByte(CP_ACP, 0, buf, -1, &utf8[0], len, NULL, NULL);

		return &utf8[0];
	}

	static wstring Utf8ToUnicode(const char* buf)
	{
		int len = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
		if (len == 0) return L"";

		std::vector<wchar_t> unicode(len);
		::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &unicode[0], len);

		return &unicode[0];	
	}

	static string UnicodeToUtf8(const wchar_t* buf)
	{
		int len = ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
		if (len == 0) return "";

		std::vector<char> utf8(len);
		::WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], len, NULL, NULL);

		return &utf8[0];
	}

	static string AnsiToUtf8_Win32(const string& src)
	{
		return UnicodeToUtf8(AnsiToUnicode(src.c_str()).c_str());
	}

	static string Utf8ToAnsi_Win32(const string& src)
	{
		return UnicodeToAnsi(Utf8ToUnicode(src.c_str()).c_str());
	}
#else
	static int iconv_convert(iconv_t cd, const char* inbuf, size_t inlen, char *outbuf, size_t* outlen)
	{
		char **pin = (char**)&inbuf;
		char **pout = &outbuf;

		memset(outbuf, 0, *outlen);
		return iconv(cd, pin, &inlen, pout, outlen);
	}

	static string AnsiToUtf8_Linux(const string& src)
	{
		string sRet;
		iconv_t cd = iconv_open("utf-8", "gb18030");

		size_t src_len = src.length();
		size_t buf_len = src_len * 4;
		char* out_buf = new char[buf_len+1];
		if (out_buf)
		{
			memset(out_buf, 0, buf_len+1);
			const char* ptr_in = src.c_str();
			int nRet = iconv_convert(cd, ptr_in, src_len, out_buf, &buf_len);
			if (nRet == 0)
				sRet = out_buf;
			delete[] out_buf;
		}
		iconv_close(cd);
		return sRet;
	}

	static string Utf8ToAnsi_Linux(const string& src)
	{
		string sRet;
		iconv_t cd = iconv_open("gb18030", "utf-8");

		size_t src_len = src.length();
		size_t buf_len = src_len * 4;
		char* out_buf = new char[buf_len+1];
		if (out_buf)
		{
			memset(out_buf, 0, buf_len+1);
			const char* ptr_in = src.c_str();
			int nRet = iconv_convert(cd, ptr_in, src_len, out_buf, &buf_len);
			if (nRet == 0)
				sRet = out_buf;
			delete[] out_buf;
		}
		iconv_close(cd);
		return sRet;
	}

	static wstring AnsiToUnicode(const char* buf)
	{
		const string src = buf;

		wstring sRet;
		iconv_t cd = iconv_open("unicode", "gb18030");


		size_t src_len = src.length();
		size_t buf_len = src_len;
		wchar_t* out_buf = new wchar_t[buf_len+1];
		if (out_buf)
		{
			memset(out_buf, 0, (buf_len+1)*sizeof(wchar_t));
			const char* ptr_in = src.c_str();
			//int nRet = iconv_convert(cd, ptr_in, src_len, (char*)out_buf, &buf_len);
			//printf("iconv_convert ret:%d \r\n\r\n", nRet);
			size_t size_to = (buf_len+1)*sizeof(wchar_t);
			int nRet = iconv(cd, (char**)&ptr_in, &src_len,(char**)&out_buf,&size_to);
			printf("iconv_convert ret:%d \r\n\r\n", nRet);
			if (nRet == 0)
				sRet = out_buf;
			delete[] out_buf;
		}
		iconv_close(cd);
		return sRet;
	}
	static string UnicodeToAnsi(const wchar_t* buf)
	{
		const wstring src = buf;

		string sRet;
		iconv_t cd = iconv_open("gb18030", "unicode");

		size_t src_len = src.length();
		size_t buf_len = src_len;
		char* out_buf = new char[buf_len+1];
		if (out_buf)
		{
			memset(out_buf, 0, buf_len+1);
			const char* ptr_in = (const char*)src.c_str();
			int nRet = iconv_convert(cd, ptr_in, src_len, out_buf, &buf_len);
			if (nRet == 0)
				sRet = out_buf;
			delete[] out_buf;
		}
		iconv_close(cd);
		return sRet;
	}

	static wstring Utf8ToUnicode(const char* buf)
	{
		const string src = buf;

		wstring sRet;
		iconv_t cd = iconv_open("unicode", "utf-8");

		size_t src_len = src.length();
		size_t buf_len = src_len*2;
		wchar_t* out_buf = new wchar_t[buf_len+1];
		if (out_buf)
		{
			memset(out_buf, 0, (buf_len+1)*sizeof(wchar_t));
			const char* ptr_in = src.c_str();
			int nRet = iconv_convert(cd, ptr_in, src_len, (char*)out_buf, &buf_len);
			if (nRet == 0)
				sRet = out_buf;
			delete[] out_buf;
		}
		iconv_close(cd);
		return sRet;
	}
#endif


	static string AnsiToUtf8(const string& src)
	{
#ifdef _MSC_VER
		return AnsiToUtf8_Win32(src);
#else
		return AnsiToUtf8_Linux(src);
#endif // WIN32
	}

	static string Utf8ToAnsi(const string& src)
	{
#ifdef _MSC_VER
		return Utf8ToAnsi_Win32(src);
#else
		return Utf8ToAnsi_Linux(src);
#endif // WIN32
	}

	static wstring MbcsToUnicode(const string& buf)
	{
		wstring strResult;
#ifdef WIN32
		USES_CONVERSION;
		strResult = CA2W(buf.c_str());
#else
		if (1)
		{
			iconv_t cd = iconv_open("wchar_t", "gb18030");

			size_t src_len = buf.length();
			size_t buf_len = src_len * 3;
			wchar_t* out_buf = new wchar_t[buf_len+1];
			if (out_buf)
			{
				memset(out_buf, 0, buf_len+1);
				const char* ptr_in = buf.c_str();
				int nRet = iconv_convert(cd, ptr_in, src_len, (char*)out_buf, &buf_len);
				//printf("MbcsToUnicode() result = %d, src_len=%d, dst_len=%d.\r\n", nRet, src_len, buf_len);
				if (nRet == 0)
					strResult = out_buf;
				delete[] out_buf;
			}
			iconv_close(cd);
		}
#endif
		return strResult;
	}

	static string UnicodeToMbcs(const wstring& buf)
	{
		string strResult;
#ifdef WIN32
		USES_CONVERSION;
		strResult = CW2A(buf.c_str());
#else
		if (1)
		{
			iconv_t cd = iconv_open("utf-8", "wchar_t");

			size_t src_len = buf.length();
			size_t buf_len = src_len * 3;
			char* out_buf = new char[buf_len+1];
			if (out_buf)
			{
				memset(out_buf, 0, buf_len+1);
				const char* ptr_in = (const char*)buf.c_str();
				int nRet = iconv_convert(cd, ptr_in, src_len, out_buf, &buf_len);
				if (nRet == 0)
					strResult = out_buf;
				delete[] out_buf;
			}
			iconv_close(cd);
		}
#endif
		return strResult;
	}
}

#endif //_public_h_

