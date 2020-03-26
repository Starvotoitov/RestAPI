#ifndef FILE_MANAGER
#define FILE_MANAGER

#define LIST_BEGIN "{\r\n\t\"List\": [\r\n"
#define LIST_END "\r\n\t]\r\n}"

namespace RestAPIServer
{
	char* GetDirectoryList(const char* PathTo);
	void CreateDirectoryPath(const char* FullPath, const char* RootDirectory);
	void ClearDir(const char* PathToDir);
}

#endif