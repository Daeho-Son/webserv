#ifndef CONF_HPP
# define CONF_HPP

# include <string>

class Conf
{
public:
	Conf();
	Conf(int port, const std::string &serverName, int clientBodySize);
	Conf(const Conf& other);
	Conf& operator=(const Conf& other);
	virtual ~Conf();

	int GetPort() const;
	std::string GetServerName() const;
	int GetClientBodySize() const;

private:
	int mPort;
	std::string mServerName;
	int mClientBodySize;
};

#endif
