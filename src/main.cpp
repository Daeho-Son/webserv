#include "HttpServer.hpp"

int main(void)
{
	Conf conf(8080, "webserv", 1024);
	HttpServer server(conf);

	server.Run();
	server.Stop();
	return 0;
}
