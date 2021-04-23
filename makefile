http-client: HttpRequest.o main.o 
	g++ -o http-client HttpRequest.o main.o

main.o: main.cpp
	g++ -c main.cpp -std=c++11

HttpRequest.o: HttpRequest.cpp
	g++ -c HttpRequest.cpp -std=c++11
