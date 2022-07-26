#ifndef CONF_HPP
# define CONF_HPP

# include <exception>
# include <iostream>
# include <string>
# include <fstream>
# include <vector>
# include <sstream>
# include <cstdlib>
# include <set>

# include "ServerInfo.hpp"

# define DEFAULT_KEVENT_SIZE 65536
# define DEFAULT_LISTEN_SIZE 65536

class Conf
{
public:
	Conf();
	Conf(const std::string& confFile);
	Conf(const Conf& other);
	Conf& operator=(const Conf& other);
	virtual ~Conf();

	const std::vector<ServerInfo>& GetServerInfos() const;
	bool IsValid() const;
	int GetKeventsSize() const;
	int GetListenSize() const;

	bool IsValidCgiExtension(const std::string& targetDir, int port, const std::string& cgiExtension) const;
	std::string GetRootedLocation(const std::string& targetDir, int port) const;
	std::string GetDefaultPage(const std::string& targetDir, int port) const;
	std::string GetDefaultErrorPage(const std::string& targetDir, int port) const;
	const std::string& GetRedirectTarget(const std::string& target, int port) const;
	size_t GetClientBodySize(const std::string& tartgetDir, int port) const;
	bool IsValidHttpMethod(const std::string& targetDir, int port, const std::string &method) const;
	void PrintConfData() const;
	bool IsRootFolder(const std::string& targetDir, int port) const;
	bool IsAutoIndex(const std::string& targetDir, int port) const;
	bool IsRedirectedTarget(const std::string& target, int port) const;

private:
	void Parse(const std::string& contFile);
	bool ParseServerInfo(std::vector<std::string> confInfo, std::set<std::string> hasPort);
	bool ParseLocationInfo(ServerInfo* serverInfo, const std::vector<std::string>& confInfo, size_t& confInfoIndex, std::set<std::string>& hasLocation);

	size_t GetLocationInfoIndexByTargetDirectory(const std::string& targetDir, size_t serverInfoIndex) const;
	size_t GetServerInfoIndexByPort(int port) const;
	const LocationInfo& GetLocationInfo(const std::string& targetDir, int port) const;

private:
	std::vector<ServerInfo> mServerInfos;
	bool mIsValid;
	int mkeventSize;
	int mListenSize;
};

#endif
