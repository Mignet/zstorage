
#ifndef c_file_h
#define c_file_h
#include <stdio.h>
#include "
namespace zn
{

	class c_file
	{
	public:
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

			if (!isdir(path)) mkdir(path);
#else
			if (strlen(szpath) <= 2)
				strcat(szpath, "\\");

			makedir(szpath);

			if (!isdir(path)) mkdir(path, S_IRWXU);
#endif

		}

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
			
	};



}



#endif //c_file_h
