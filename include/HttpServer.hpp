#ifndef HTTP_SERVER_HPP
# define HTTP_SERVER_HPP

# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <netinet/in.h> // sockaddr_in
# include <string>
# include <sys/event.h> // kqueue()
# include <sys/socket.h>
# include <unistd.h> // close()
# include <unordered_set>
# include <vector>

# include "Conf.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

# define CONF_DEFAULT_TARGET "/index.html"

class HttpServer
{
public:
	HttpServer(Conf& conf);
	virtual ~HttpServer();

	int Run(); // 서버를 실행합니다.
	int Stop(); // 서버를 종료합니다.
private:
	HttpServer();
	HttpServer(const HttpServer& other);
	HttpServer& operator=(const HttpServer& other);

	int addEvent(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
	HttpResponse GetResponseByRequest(HttpRequest& request);
	int GetStatusCode(HttpRequest& httpRequest);
	std::string GetMessageBody(HttpRequest& httpRequest);
private:
	Conf mServerConf;
};

#endif
