#ifndef HTTP_SERVER_HPP
# define HTTP_SERVER_HPP

# include "Conf.hpp"
# include "HttpRequest.hpp"

class HttpServer
{
public:
	HttpServer(Conf& conf);
	int Run(); // 서버를 실행합니다.
	int Stop(); // 서버를 종료합니다.
protected:
	virtual ~HttpServer();
private:
	HttpServer();
	HttpServer(const HttpServer& other);
	HttpServer& operator=(const HttpServer& other);
};

#endif
