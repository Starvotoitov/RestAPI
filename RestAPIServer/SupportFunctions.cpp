#include "SupportFunctions.h"
#include <stdlib.h>
#include <cstring>

namespace RestAPIServer
{
	char* Realpath(const char* Root, const char* Path)
	{
		char* Result = (char*)calloc(1, strlen(Root) + strlen(Path) + 1);
		strcpy_s(Result, strlen(Root) + strlen(Path) + 1, Root);
		strcat_s(Result, strlen(Root) + strlen(Path) + 1, Path);
		return Result;
	}

	int IndexOf(const char* Str, const int Position, const char Symbol)
	{
		for (int i = Position; i < strlen(Str); i++)
		{
			if (Str[i] == Symbol)
			{
				return i;
			}
		}
		return -1;
	}

	int FindInString(const char* FindIn, const char* FindValue)
	{
		int Index = 0, Len = strlen(FindIn);
		if (strlen(FindIn) >= strlen(FindValue))
		{
			char* CmpStr = (char*)calloc(1, strlen(FindValue) + 1);
			do
			{
				memcpy(CmpStr, FindIn++, strlen(FindValue));
				Index++;
			} while (strcmp(CmpStr, FindValue) && strlen(FindIn) >= strlen(FindValue));
			free(CmpStr);
			return Index <= Len ? Index - 1 : -1;
		}
		else
		{
			return -1;
		}
	}
}