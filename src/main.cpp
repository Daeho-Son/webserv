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
		std::cerr << "Invalid Conf\n";
		return 1;
	}

	HttpServer server(conf);

	HttpResponse response(200, "Hello, world");
	server.Run();
	server.Stop();
	return 0;
}
