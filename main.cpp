#include <iostream>

#include "HttpRequest.h"

using namespace httpTypes;

int main()
{
	std::cout << "App launched!" << std::endl;

	// Create an http request object
	std::unique_ptr<HttpRequest> request2(new HttpRequest("http://www.ele-dev.de/drone/pushStats.php", REQUEST_METHOD::HTTP_GET));

	// Add payload (GET/POST parameters)
	std::string pitchParam, rollParam, yawParam;
	std::cout << "\nType in values for the GET parameters: " << std::endl;
	std::cout << "   pitch = ";
	std::cin >> pitchParam;
	std::cout << "\n   roll = ";
	std::cin >> rollParam;
	std::cout << "\n   yaw = ";
	std::cin >> yawParam;
	std::cout << std::endl;

	request2->addParameter("pitch", pitchParam);
	request2->addParameter("roll", rollParam);
	request2->addParameter("yaw", yawParam);

	// Send the request 
	request2->sendRequest();

	// Output the answer from the webserver
	std::cout << std::endl;
	std::cout << "HTTP Response text: " << std::endl;
	std::cout << request2->getResponse().text << std::endl;

	std::cin.get();

	return EXIT_SUCCESS;
}