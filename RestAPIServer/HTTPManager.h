#ifndef HTTP_MANAGER
#define HTTP_MANAGER

#define FILE_NOT_FOUND "HTTP/1.1 404 Not Found\r\n\r\n"
#define BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n\r\n"
#define NO_CONTENT "HTTP/1.1 204 No Content\r\n\r\n"
#define NOT_IMPLEMENTED "HTTP/1.1 501 Not Implemented\r\n\r\n"
#define OK "HTTP/1.1 200 OK\r\n\r\n"
#define ACCESS_DENIED "HTTP/1.1 403 Forbidden\r\n\r\n"
#define METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed\r\n\r\n"
#define CREATED "HTTP/1.1 201 Created\r\n\r\n"

namespace RestAPIServer
{	
	struct HTTPHeader
	{
		char* StartingLine;
		char** Headers;
		int HeadersCount;
		char* Method;
		char* Version;
		char* URI;
		int ContentLength;
		char* Content;
	};

	char* FindInHeadersList(HTTPHeader* Pack, const char* FindValue);
	HTTPHeader* ParseHTTPHeader(const char* Header);
	void CleanUpHeaderMemory(HTTPHeader* Header);
}

#endif