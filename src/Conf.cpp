#include "Conf.hpp"

Conf::Conf()
{
	this->mPort = 8080;
	this->mServerName = "webserv";
	this->mClientBodySize = 4096;
}

Conf::Conf(int port, const std::string &serverName, int clientBodySize)
{
	this->mPort = port;
	this->mServerName = serverName;
	this->mClientBodySize = clientBodySize;
}

Conf::Conf(const Conf& other)
{
	this->mPort = other.GetPort();
	this->mServerName = other.GetServerName();
	this->mClientBodySize = other.GetClientBodySize();
}

Conf& Conf::operator=(const Conf &other)
{
	this->mPort = other.GetPort();
	this->mServerName = other.GetServerName();
	this->mClientBodySize = other.GetClientBodySize();

	return *this;
}

Conf::~Conf()
{

}

int Conf::GetPort() const
{
	return mPort;
}

std::string Conf::GetServerName() const
{
	return mServerName;
}

int Conf::GetClientBodySize() const
{
	return mClientBodySize;
}


