#include "HttpRequest.h"

// Definitions //

HttpRequest::HttpRequest(std::string url, httpTypes::REQUEST_METHOD method)
{
	// launch the winsock API on windows machines
#ifdef _PLATFORM_WINDOWS
	this->startWinsock();
#endif

	// Store the parameters for further operations
	std::cout << "http request object created!" << std::endl;
	m_url = url;
	m_requestMethod = method;
	m_protocol = "http";
	m_connected = false;

	// Setup a tcp socket
	this->m_tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _PLATFORM_WINDOWS
	if (m_tcpSocket == INVALID_SOCKET)
	{
		std::cerr << "Could not create tcp socket!" << std::endl;
		return;
	}
#else
	if (m_tcpSocket < 0)
	{
		std::cerr << "Could not create tcp socket!" << std::endl;
		return;
	}
#endif

	// Now extract the hostaddress, protocol(http/https) and request path from the URL 
	std::vector<std::string> tempStr = this->splitString(this->m_url, '/');
	this->m_host = tempStr.at(2);
	this->m_protocol = tempStr.at(0);
	for (int i = 3; i < tempStr.size(); i++)
	{
		this->m_requestPath += tempStr.at(i) + "/";
	}
	std::cout << "Webserver address: " << this->m_host << std::endl;
	std::cout << "Request protocol: " << this->m_protocol << std::endl;

	// Choose the corresponding port for the detected protocol
	int port = 80;
	if (this->m_protocol == "https:") {
		port = 443;
	}

	std::cout << "Connection port: " << port << std::endl;

	// Get the IP Address of the target server and create the connection endpoint
	struct hostent* host = gethostbyname(this->m_host.c_str());
	std::cout << "Final IP Address: " << inet_ntoa(*((struct in_addr*)host->h_addr_list[0])) << std::endl;

	// Create the target webserver endpoint using the 
	// struct sockaddr_in address = {};
	this->m_serverAddr = {};
	m_serverAddr.sin_port = htons(port);
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr*)host->h_addr_list[0])));
}

HttpRequest::~HttpRequest()
{
	// close the connection and the tcp socket
#ifdef _PLATFORM_WINDOWS
	closesocket(m_tcpSocket);
#else
	close(m_tcpSocket);
#endif

	std::cout << "Connection closed" << std::endl;
	std::cout << "request object released!" << std::endl;

	// close the winsock API on windows machines
#ifdef _PLATFORM_WINDOWS
	this->closeWinsock();
#endif
}

void HttpRequest::sendRequest()
{

	// Connect to the webserver if it isnt't done already
	if (m_connected == false)
	{
		// Attempt to connect to the webserver
		std::cout << "Connection ..." << std::endl;
		int error = connect(this->m_tcpSocket, (struct sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
		if (error != 0)
		{
			std::cerr << "Failed to connect to the webserver!" << std::endl;
			return;
		}

		std::cout << "Connection established" << std::endl;
		this->m_connected = true;
	}
	

	// Now construct the HTTP Request Header text
	std::string requestText = this->constructRequestText();
	

	// Send the http request to the webserver
	std::cout << "Sending " << this->m_protocol << " Request ... ";
	int bytesSent = send(m_tcpSocket, requestText.c_str(), (int)strlen(requestText.c_str()), 0);
	std::cout << " Done" << std::endl;

	// Wait for a response message 
	const int maxResponseLength = 2048;

	std::cout << "Waiting for a response ... ";
	char recvBuffer[maxResponseLength];
	int bytesReceived = recv(m_tcpSocket, recvBuffer, sizeof(recvBuffer), 0);
	if (bytesReceived < 0)
	{
		std::cerr << "\nNo response from the webserver!" << std::endl;
	}
	else
	{
		std::cout << " Done" << std::endl;
		// Nullterminate and store the response text
		if (bytesReceived < maxResponseLength)
		{
			recvBuffer[bytesReceived] = '\0';
		}
		this->m_responseText = recvBuffer;
	}

	return;
}

void HttpRequest::addParameter(std::string name, std::string value)
{
	// Add the parameter to the list
	httpTypes::REQUEST_PARAM parameter;
	parameter.name = name;
	parameter.value = value;

	this->m_parameters.push_back(parameter);

	return;
}

void HttpRequest::removeParameter(std::string paramName)
{
	// Remove the parameter with the given name from the list
	for (int i = 0; i < this->m_parameters.size(); i++)
	{
		if (this->m_parameters.at(i).name == paramName) {
			// Remove this one and exit
			this->m_parameters.erase(this->m_parameters.begin() + i);
			break;
		}
	}

	return;
}

void HttpRequest::removeAllParameters()
{
	// Empty the list of parameters
	this->m_parameters.clear();
	return;
}

httpTypes::HTTP_RESPONSE HttpRequest::getResponse()
{
	// create a response 
	httpTypes::HTTP_RESPONSE response = {};

	// ...
	// response.statusCode = 200;
	// response.text = "<p> Just a dummy response </p>";
	response.text = this->m_responseText;

	return response;
}

// Private helper functions //

std::vector<std::string> HttpRequest::splitString(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

std::string HttpRequest::constructRequestText()
{
	std::string requestStr = "";

	// estimate all parts of the request string
	std::string methodStr = "GET";
	if (this->m_requestMethod == httpTypes::REQUEST_METHOD::HTTP_POST)
	{
		methodStr = "POST";
	}
	std::string httpVersion = "HTTP/1.1";

	// put all together
	requestStr += methodStr + " /" + this->m_requestPath;
	// In case of a GET request, append the parameters here
	if (methodStr == "GET") {
		for (int i = 0; i < this->m_parameters.size(); i++)
		{
			// The first GET parameter must have a ? präfix
			if (i == 0)
			{
				requestStr += "?" + this->m_parameters.at(i).name + "=" + this->m_parameters.at(i).value;
			}
			// All other GET parameters must have a & präfix
			else
			{
				requestStr += "&" + this->m_parameters.at(i).name + "=" + this->m_parameters.at(i).value;
			}
		}
	}
	requestStr += " " + httpVersion + "\r\n";
	requestStr += "Host: " + this->m_host + "\r\n\r\n";

	// quickly output the constructed text
	std::cout << std::endl;
	std::cout << requestStr;
	std::cout << std::endl;

	return requestStr;
}

// Winsock functions - WINDOWS ONLY
#ifdef _PLATFORM_WINDOWS

void HttpRequest::startWinsock()
{
	// launch the Winsock API
	WSADATA wsa;
	int error = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (error != 0)
	{
		//	Error!
	}

	return;
}

void HttpRequest::closeWinsock()
{
	// Close the winsock API
	WSACleanup();
	return;
}

#endif
