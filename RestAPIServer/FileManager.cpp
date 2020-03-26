#include "FileManager.h"
#include "SupportFunctions.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <strstream>
#include <Windows.h>

namespace RestAPIServer
{
	using namespace std;

	char* GetDirectoryList(const char* PathTo)
	{
		char* Result = (char*)calloc(1, strlen(LIST_BEGIN) + 1);
		strcpy_s(Result, strlen(LIST_BEGIN) + 1, LIST_BEGIN);

		WIN32_FIND_DATAA FileInfo;
		HANDLE hFind;
		char* NewPath = (char*)calloc(1, strlen(PathTo) + 5);
		strcpy_s(NewPath, strlen(PathTo) + 5, PathTo);
		strcat_s(NewPath, strlen(PathTo) + 5, "/*.*");
		hFind = FindFirstFileA(NewPath, &FileInfo);
		int OldSize, NewSize;
		OldSize = strlen(Result);
		
		do
		{
			
			if (strcmp(FileInfo.cFileName, ".") && strcmp(FileInfo.cFileName, ".."))
			{
				int AddedSize = 21 + strlen("\"Type\": \"\",") + strlen("\"Name\": \"\"") + strlen(FileInfo.cFileName) + (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? strlen("Directory") : strlen("File"));
				NewSize = OldSize + AddedSize;
				Result = (char*)realloc(Result, NewSize);
				sprintf_s(Result + OldSize, AddedSize, "\t\t{\r\n\t\t\t\"Type\": \"%s\",\r\n\t\t\t\"Name\": \"%s\"\r\n\t\t},\r\n", (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? "Directory" : "File"), FileInfo.cFileName);
				OldSize = NewSize;
			}
		} while (FindNextFileA(hFind, &FileInfo));

		Result = (char*)realloc(Result, OldSize + strlen(LIST_END) + 1);
		sprintf_s(Result + OldSize - 3, strlen(LIST_END) - 2, "%s", LIST_END);
		return Result;
	}

	void CreateDirectoryPath(const char* FullPath, const char* RootDirectory)
	{
		char* Buf;
		int OldIndex = IndexOf(FullPath, strlen(RootDirectory), '/'), NewIndex;
		while ((NewIndex = IndexOf(FullPath, OldIndex + 1, '/')) != -1)
		{
			Buf = (char*)calloc(1, NewIndex + 1);
			memcpy(Buf, FullPath, NewIndex);
			CreateDirectoryA(Buf, NULL);
			OldIndex = NewIndex;
			free(Buf);
		}
	}

	void ClearDir(const char* PathToDir)
	{
		char* PathToDirFiles;
		if (PathToDir[strlen(PathToDir) - 1] == '/')
		{
			PathToDirFiles = (char*)calloc(1, strlen(PathToDir) + 4);
			strcpy_s(PathToDirFiles, strlen(PathToDir + 4), PathToDir);
			strcat_s(PathToDirFiles, strlen(PathToDir + 4), "*.*");
		}
		else
		{
			PathToDirFiles = (char*)calloc(1, strlen(PathToDir) + 5);
			strcpy_s(PathToDirFiles, strlen(PathToDir) + 5, PathToDir);
			strcat_s(PathToDirFiles, strlen(PathToDir) + 5, "/*.*");
		}

		WIN32_FIND_DATAA FileInfo;
		HANDLE hFind = FindFirstFileA(PathToDirFiles, &FileInfo);
		do
		{
			if (strcmp(FileInfo.cFileName, ".") && strcmp(FileInfo.cFileName, ".."))
			{
				char* NewPath = (char*)calloc(1, strlen(PathToDir) + strlen(FileInfo.cFileName) + 2);
				strcpy_s(NewPath, strlen(PathToDir) + strlen(FileInfo.cFileName) + 2, PathToDir);
				strcat_s(NewPath, strlen(PathToDir) + strlen(FileInfo.cFileName) + 2, "/");
				strcat_s(NewPath, strlen(PathToDir) + strlen(FileInfo.cFileName) + 2, FileInfo.cFileName);
				if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					ClearDir(NewPath);
					if (!RemoveDirectoryA(NewPath))
					{
						printf("Fail to delete %s %d", NewPath, GetLastError());
						RemoveDirectoryA(NewPath);
					}
				}
				else
				{
					if (!DeleteFileA(NewPath))
					{
						printf("Fail to delete %s %d", NewPath, GetLastError());
					}
				}
				free(NewPath);
			}
		} while (FindNextFileA(hFind, &FileInfo));
		FindClose(hFind);

		free(PathToDirFiles);
	}
}