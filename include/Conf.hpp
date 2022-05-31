#ifndef CONF_HPP
# define CONF_HPP

# include <string>

class Conf
{
public:
	Conf(int port, std::string &serverName, int clientBodySize);
	Conf(const Conf& other);
	Conf& operator=(const Conf& other);
	int GetPort();
	std::string GetServerName();
	int GetClientBodySize();
protected:
	virtual ~Conf();
private:
	Conf();
private:
	int mPort;
	std::string mServerName;
	int mClientBodySize;
};

#endif
