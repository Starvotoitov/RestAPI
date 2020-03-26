#ifndef SERVER
#define SERVER

#include <WinSock2.h>
#include "HTTPManager.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUF_SIZE 1024 * 1024

namespace RestAPIServer
{
	void SendRequest(SOCKET SendTo, const char* HTTPType);
	void ProcessGetRequest(HTTPHeader* Request, SOCKET Socket);
	void ProcessDeleteRequest(HTTPHeader* Request, SOCKET Socket);
	void ProcessHeadRequest(HTTPHeader* Request, SOCKET Socket);
	void ProcessPutRequest(HTTPHeader* Request, SOCKET Socket);
	DWORD WINAPI ProcessingThread(LPVOID lpParam);

	void Start(const char* IpAddress, const int Port, const char* ServerRoot);
}

#endif
