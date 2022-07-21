#include "HttpServer.hpp"
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";	
		return 1;
	}
	Conf conf(argv[1]);
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
	signal(SIGPIPE, SIG_IGN);
	server.Run();
	server.Stop();
	return 0;
}
