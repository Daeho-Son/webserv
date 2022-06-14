#include "HttpServer.hpp"
#include <string>
#include <iostream>

int main(void)
{
	Conf conf("./conf/test_1.conf");
	if (conf.IsValid() == true)
	{
		conf.PrintConfData();
	}
	else
	{
		std::cerr << "Invalid Conf" << std::endl;
		return 1;
	}
		
	HttpServer server(conf);

	HttpResponse response(200, "Hello, world");
	std::cout << response.GetHttpMessage() << std::endl;
	server.Run();
	server.Stop();
	return 0;
}
