#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// THE PLATFORM MUST BE DEFINED HERE !!! //
#define _PLATFORM_WINDOWS
// #define _PLATFORM_UNIX

// General platform independent includes //
#include <sstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

// Platform dependent includes //
#ifdef _PLATFORM_WINDOWS
	#include <string>
	#include <winsock.h>		// winsock 
	#include <mmsystem.h>		// windows system
#endif

#ifdef _PLATFORM_UNIX
	#include <string.h>
	#include <sys/types.h>
	#include <sys/socket.h>		//socket
	#include <arpa/inet.h>		//inet_addr
	#include <netinet/in.h>
	#include <netdb.h>			//hostent
	#include <unistd.h>
#endif

// Library linking //
#ifdef _PLATFORM_WINDOWS
	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "winmm.lib")
#endif

namespace httpTypes
{
	enum class REQUEST_METHOD { HTTP_GET, HTTP_POST };

	struct HTTP_RESPONSE
	{
		short statusCode;
		std::string text;
	};

	struct REQUEST_PARAM
	{
		std::string name;
		std::string value;
	};
}

class HttpRequest
{

public:
	HttpRequest(std::string, httpTypes::REQUEST_METHOD);
	~HttpRequest();

	void sendRequest();
	void addParameter(std::string, std::string);
	void removeParameter(std::string);
	void removeAllParameters();

	httpTypes::HTTP_RESPONSE getResponse();

private:
	// Helper functions
	std::vector<std::string> splitString(const std::string& s, char delimiter);
	std::string constructRequestText();

	// winsock handling - WINDOWS ONLY
	#ifdef _PLATFORM_WINDOWS 
		void startWinsock();
		void closeWinsock();
	#endif

private:
	bool m_connected;
#ifdef _PLATFORM_WINDOWS
	SOCKET m_tcpSocket;
#else
	int m_tcpSocket;
#endif
	sockaddr_in m_serverAddr;

	std::string m_url, m_host, m_requestPath;
	httpTypes::REQUEST_METHOD m_requestMethod;
	std::vector<httpTypes::REQUEST_PARAM> m_parameters;
	std::string m_protocol;

	std::string m_responseText;
};

