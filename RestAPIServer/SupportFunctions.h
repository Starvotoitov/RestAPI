#ifndef SUPPORT_FUNCTIONS
#define SUPPORT_FUNCTIONS

namespace RestAPIServer
{
	char* Realpath(const char* Root, const char* Path);
	int IndexOf(const char* Str, const int Position, const char Symbol);
	int FindInString(const char* FindIn, const char* FindValue);
}

#endif