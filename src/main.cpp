#include "HttpServer.hpp"
#include <string>
#include <iostream>

int main(void)
{
	Conf conf(8080, "webserv", 1024);
	HttpServer server(conf);

	HttpResponse response(200, "Hello, world");
	std::cout << response.GetHttpMessage() << std::endl;
	server.Run();
	server.Stop();
	return 0;
}
