#include "Server.h"
#include "SupportFunctions.h"
#include "HTTPManager.h"
#include "FileManager.h"
#include <cstdio>
#include <WS2tcpip.h>

namespace RestAPIServer
{
	char* RootDirectory;

	void SendRequest(SOCKET SendTo,const char* HTTPType)
	{
		if (send(SendTo, HTTPType, strlen(HTTPType), 0) == SOCKET_ERROR)
			printf("Error send %s: %d\n", HTTPType, WSAGetLastError());
	}

	void ProcessGetRequest(HTTPHeader* Request, SOCKET Socket)
	{
		char* PathTo = Realpath(RootDirectory, Request->URI);
		HANDLE hFile;
		WIN32_FIND_DATAA* FileInfo = (WIN32_FIND_DATAA*)calloc(1, sizeof(WIN32_FIND_DATAA));
		if (PathTo[strlen(PathTo) - 1] == '/')
			PathTo[strlen(PathTo) - 1] = '\0';
		hFile = FindFirstFileA(PathTo, FileInfo);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (FileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char* List = GetDirectoryList(PathTo);
				char* Buf = (char*)calloc(1, DEFAULT_BUF_SIZE);
				sprintf_s(Buf, DEFAULT_BUF_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", strlen(List));
				send(Socket, Buf, strlen(Buf), 0);
				send(Socket, List, strlen(List), 0);
				printf("%s structure sended\n", Request->URI);
			}
			else
			{
				FindClose(hFile);
				FILE* SrcFile;
				fopen_s(&SrcFile, PathTo, "rb");
				char* Buf = (char*)calloc(1, DEFAULT_BUF_SIZE);
				LARGE_INTEGER Size;

				hFile = CreateFileA(PathTo, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					if (GetFileSizeEx(hFile, &Size))
					{
						if (Size.QuadPart > 0)
						{
							sprintf_s(Buf, DEFAULT_BUF_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", Size.QuadPart);
							send(Socket, Buf, strlen(Buf), 0);
							while (!feof(SrcFile))
							{
								ZeroMemory(Buf, DEFAULT_BUF_SIZE);
								int Count = fread(Buf, 1, DEFAULT_BUF_SIZE, SrcFile);
								send(Socket, Buf, Count, 0);
							}
							printf("%s sended\n", Request->URI);
						}
						else
						{
							printf("File %s is empty\n", Request->URI);
							SendRequest(Socket, NO_CONTENT);
						}
					}
					else
					{
						printf("%d\n", GetLastError());
					}
					CloseHandle(hFile);
				}
			}
		}
		else
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				printf("%s not found\n", Request->URI);
				SendRequest(Socket, FILE_NOT_FOUND);
			}
			else
			{
				printf("Bad Request\n");
				SendRequest(Socket, BAD_REQUEST);
			}
		}
		free(PathTo);
		free(FileInfo);
	}

	void ProcessDeleteRequest(HTTPHeader* Request, SOCKET Socket)
	{
		char* PathTo = Realpath(RootDirectory, Request->URI);
		HANDLE hFile;
		WIN32_FIND_DATAA* FileInfo = (WIN32_FIND_DATAA*)calloc(1, sizeof(WIN32_FIND_DATAA));
		hFile = FindFirstFileA(PathTo, FileInfo);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (FileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FindClose(hFile);
				ClearDir(PathTo);
				RemoveDirectoryA(PathTo);
				printf("%s deleted\n", Request->URI);
				SendRequest(Socket, OK);
			}
			else
			{
				FindClose(hFile);
				if (DeleteFileA(PathTo))
				{
					printf("%s deleted\n", Request->URI);
					SendRequest(Socket, OK);
				}
				else
				{
					DWORD ErrCode = GetLastError();
					if (ErrCode == ERROR_FILE_NOT_FOUND)
					{
						printf("%s not found\n", Request->URI);
						SendRequest(Socket, FILE_NOT_FOUND);
					}
					else if (ErrCode == ERROR_ACCESS_DENIED)
					{
						printf("%s access denied\n", Request->URI);
						SendRequest(Socket, ACCESS_DENIED);
					}
					else
					{
						printf("Bad Request\n");
						SendRequest(Socket, BAD_REQUEST);
					}
				}
			}
		}
		else
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				printf("%s not found\n", Request->URI);
				SendRequest(Socket, FILE_NOT_FOUND);
			}
			else
			{
				printf("Bad Request\n");
				SendRequest(Socket, BAD_REQUEST);
			}
		}
		free(PathTo);
		free(FileInfo);
	}

	void ProcessHeadRequest(HTTPHeader* Request, SOCKET Socket)
	{
		char* PathTo = Realpath(RootDirectory, Request->URI);
		HANDLE hFind;
		WIN32_FIND_DATAA* FileInfo = (WIN32_FIND_DATAA*)calloc(1, sizeof(WIN32_FIND_DATAA));
		hFind = FindFirstFileA(PathTo, FileInfo);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (!(FileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char* Buf = (char*)calloc(1, DEFAULT_BUF_SIZE);

				HANDLE hFile = CreateFileA(PathTo, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				LARGE_INTEGER Size;
				GetFileSizeEx(hFile, &Size);

				sprintf_s(Buf, DEFAULT_BUF_SIZE, "HTTP/1.1 200 OK\r\nX-File-Size: %d\r\nX-Type: File\r\nX-Name: %s\r\n\r\n", Size.QuadPart, FileInfo->cFileName);
				send(Socket, Buf, strlen(Buf), 0);
			}
			else
			{
				printf("%s method not allowed\n", Request->URI);
				SendRequest(Socket, METHOD_NOT_ALLOWED);
			}
		}
		else
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				printf("%s not found\n", Request->URI);
				SendRequest(Socket, FILE_NOT_FOUND);
			}
			else
			{
				printf("Bad Request\n");
				SendRequest(Socket, BAD_REQUEST);
			}
		}
		free(PathTo);
		free(FileInfo);
	}

	void ProcessPutRequest(HTTPHeader* Request, SOCKET Socket)
	{
		char* PathTo = Realpath(RootDirectory, Request->URI);
		if (FindInHeadersList(Request, "X-Copy-From") == NULL)
		{
			bool IsSuccess = true;
			int DataSize = Request->ContentLength;
			FILE* NewFile;
			if (DataSize == 0)
			{
				SendRequest(Socket, BAD_REQUEST);
				return;
			}
			CreateDirectoryPath(PathTo, RootDirectory);
			fopen_s(&NewFile, PathTo, "wb");
			printf("%s", Request->Content);

			fwrite(Request->Content, 1, strlen(Request->Content), NewFile);
			DataSize -= strlen(Request->Content);

			char* DataBuf = (char*)calloc(1, DEFAULT_BUF_SIZE);
			while ((DataSize > 0) && IsSuccess)
			{
				int Count = recv(Socket, DataBuf, DEFAULT_BUF_SIZE, 0);
				if (Count == 0 || Count == SOCKET_ERROR)
				{
					IsSuccess = false;
				}
				else
				{
					fwrite(DataBuf, Count, 1, NewFile);
					DataSize -= Count;
					ZeroMemory(DataBuf, DEFAULT_BUF_SIZE);
				}
			}

			if (IsSuccess)
				SendRequest(Socket, CREATED);
			free(DataBuf);
			fclose(NewFile);
		}
		else
		{
			char* PathTo = (char*)calloc(1, strlen(RootDirectory) + strlen(Request->URI));
			strcpy_s(PathTo, strlen(RootDirectory) + strlen(Request->URI), RootDirectory);
			strcat_s(PathTo, strlen(RootDirectory) + strlen(Request->URI), Request->URI);
			char* PathFrom = (char*)calloc(1, strlen(RootDirectory) + strlen(FindInHeadersList(Request, "X-Copy-From")));
			strcpy_s(PathFrom, strlen(RootDirectory) + strlen(FindInHeadersList(Request, "X-Copy-From")), RootDirectory);
			strcat_s(PathFrom, strlen(RootDirectory) + strlen(FindInHeadersList(Request, "X-Copy-From")), FindInHeadersList(Request, "X-Copy-From"));
			printf("%s %s\n", PathTo, PathFrom);
			if (CopyFileA(PathFrom, PathTo, false) == 0)
			{
				DWORD ErrCode = GetLastError();
				if (ErrCode == ERROR_ACCESS_DENIED)
					SendRequest(Socket, ACCESS_DENIED);
				else if (ErrCode == ERROR_PATH_NOT_FOUND || ErrCode == ERROR_FILE_NOT_FOUND)
					SendRequest(Socket, FILE_NOT_FOUND);
				else
					SendRequest(Socket, BAD_REQUEST);
			}
			else
				SendRequest(Socket, CREATED);
		}
	}

	DWORD WINAPI ProcessingThread(LPVOID lpParam)
	{
		SOCKET AcceptedSocket = *(SOCKET*)lpParam;
		char* RecvBuf = (char*)calloc(1, DEFAULT_BUF_SIZE);
		HTTPHeader* Header;
		recv(AcceptedSocket, RecvBuf, DEFAULT_BUF_SIZE, 0);
		Header = ParseHTTPHeader(RecvBuf);
		printf("%s\n", Header->StartingLine);
		if (!strcmp(Header->Method, "GET"))
			ProcessGetRequest(Header, AcceptedSocket);
		else if (!strcmp(Header->Method, "DELETE"))
			ProcessDeleteRequest(Header, AcceptedSocket);
		else if (!strcmp(Header->Method, "HEAD"))
			ProcessHeadRequest(Header, AcceptedSocket);
		else if (!strcmp(Header->Method, "PUT"))
			ProcessPutRequest(Header, AcceptedSocket);
		else
			SendRequest(AcceptedSocket, NOT_IMPLEMENTED);
		CleanUpHeaderMemory(Header);

		shutdown(AcceptedSocket, SD_BOTH);
		closesocket(AcceptedSocket);
		free(RecvBuf);
		return 0;
	}

	void Start(const char* IpAddress, const int Port, const char* ServerRoot)
	{
		RootDirectory = const_cast<char*>(ServerRoot);

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			printf("WSAStartup return error: %d\n", WSAGetLastError());
			return;
		}

		SOCKET ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ListeningSocket == INVALID_SOCKET)
		{
			printf("ListeningSocket: socket return error: %d\n", WSAGetLastError());
			return;
		}

		sockaddr_in* ListeningAddress = (sockaddr_in*)calloc(1, sizeof(sockaddr_in));
		InetPtonA(AF_INET, IpAddress, &ListeningAddress->sin_addr);
		ListeningAddress->sin_family = AF_INET;
		ListeningAddress->sin_port = htons(Port);

		if (bind(ListeningSocket, (sockaddr*)ListeningAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			printf("ListeningSocket: bind return error: %d\n", WSAGetLastError());
			return;
		}

		if (listen(ListeningSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			printf("ListeningSocket: listen return error: %d\n", WSAGetLastError());
			return;
		}

		sockaddr_in* AcceptedAddress = (sockaddr_in*)calloc(1, sizeof(sockaddr_in));
		int AddressSize = sizeof(sockaddr_in);
		SOCKET AcceptedSocket;
		printf("Waiting...\n");
		while (AcceptedSocket = accept(ListeningSocket, (sockaddr*)AcceptedAddress, &AddressSize))
		{
			if (AcceptedSocket != INVALID_SOCKET)
			{
				CreateThread(NULL, 0, &ProcessingThread, &AcceptedSocket, 0, NULL);
			}
			else
			{
				printf("accept return error: %d\n", WSAGetLastError());
			}
		}
	}
}