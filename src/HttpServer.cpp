#include "HttpServer.hpp"

HttpServer::HttpServer(Conf& conf)
{

}

int HttpServer::Run() // 서버를 실행합니다. Init()이 실행된 후여야 합니다.
{

}

int HttpServer::Stop() // 서버를 종료합니다.
{

}

HttpServer::~HttpServer()
{

}

// 실행되면 안되는 Canonical form methods
HttpServer::HttpServer()
{

}

HttpServer::HttpServer(const HttpServer& other)
{

}

HttpServer& HttpServer::operator=(const HttpServer& other)
{

}
