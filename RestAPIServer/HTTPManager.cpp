#include "HTTPManager.h"
#include "SupportFunctions.h"
#include <stdlib.h>
#include <cstring>

namespace RestAPIServer
{
	char* FindInHeadersList(HTTPHeader* Pack, const char* FindValue)
	{
		int i, Index;
		char* CurrentStr = NULL, * Result;
		if (Pack->HeadersCount > 0)
		{
			for (i = 0; i < Pack->HeadersCount; i++)
			{
				Index = FindInString(*(Pack->Headers + i), ":");
				CurrentStr = (char*)calloc(1, Index + 1);
				memcpy(CurrentStr, *(Pack->Headers + i), Index);
				if (!strcmp(CurrentStr, FindValue))
				{
					Result = (char*)calloc(1, strlen(*(Pack->Headers + i)) - Index - 1);
					memcpy(Result, *(Pack->Headers + i) + Index + 2, strlen(*(Pack->Headers + i)) - Index - 2);
					free(CurrentStr);
					return Result;
				}
			}
			free(CurrentStr);
		}
		return NULL;
	}

	HTTPHeader* ParseHTTPHeader(const char* Header)
	{
		HTTPHeader* ResValue = (HTTPHeader*)calloc(1, sizeof(HTTPHeader));
		int Index;

		Index = FindInString(Header, "\r\n");
		ResValue->StartingLine = (char*)calloc(1, Index + 1);
		memcpy(ResValue->StartingLine, Header, Index);

		ResValue->HeadersCount = 0;
		Header += Index + 2;

		while (*Header != '\r' && *(Header + 1) != '\n')
		{
			ResValue->Headers = (char**)realloc(ResValue->Headers, sizeof(char*) * (ResValue->HeadersCount + 1));
			Index = FindInString(Header, "\r\n");
			*(ResValue->Headers + ResValue->HeadersCount) = (char*)calloc(1, Index + 1);
			memcpy(*(ResValue->Headers + ResValue->HeadersCount), Header, Index);
			Header += Index + 2;
			(ResValue->HeadersCount)++;
		}

		Header += 2;

		ResValue->Content = NULL;
		ResValue->ContentLength = 0;
		if (strlen(Header))
		{
			ResValue->Content = (char*)calloc(1, strlen(Header) + 1);
			memcpy(ResValue->Content, Header, strlen(Header));
			ResValue->ContentLength = atoi(FindInHeadersList(ResValue, "content-length"));
		}

		int SecondIndex;

		Index = IndexOf(ResValue->StartingLine, 0, ' ');
		ResValue->Method = (char*)calloc(1, Index + 1);
		memcpy(ResValue->Method, ResValue->StartingLine, Index);

		SecondIndex = IndexOf(ResValue->StartingLine, Index + 1, ' ');
		ResValue->URI = (char*)calloc(1, SecondIndex - Index);
		memcpy(ResValue->URI, ResValue->StartingLine + Index + 1, SecondIndex - Index - 1);

		ResValue->Version = (char*)calloc(1, 9);
		memcpy(ResValue->Version, ResValue->StartingLine + SecondIndex + 1, 8);

		return ResValue;
	}

	void CleanUpHeaderMemory(HTTPHeader* Header)
	{
		int i;
		free(Header->Method);
		free(Header->StartingLine);
		free(Header->URI);
		free(Header->Version);

		for (i = 0; i < Header->HeadersCount; i++)
			free(*(Header->Headers + i));

		free(Header->Headers);
		free(Header);
	}
}