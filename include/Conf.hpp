#ifndef CONF_HPP
# define CONF_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <vector>
# include <sstream>
# include <cstdlib>
# include <unordered_set>

# include "ServerInfo.hpp"

# define DEFAULT_KEVENT_SIZE 1024
# define DEFAULT_LISTEN_SIZE 1024

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
	
	std::string GetRootedLocation(const std::string& targetDir, int port) const;
	std::string GetDefaultPage(const std::string& targetDir, int port) const;
	std::string GetDefaultErrorPage(const std::string& targetDir, int port) const;
	bool IsValidHttpMethod(const std::string& targetDir, int port, const std::string& method) const; // path에서 해당 http method가 허용되어 있는지 검사
	void PrintConfData() const;

private:
	void Parse(const std::string& contFile);
	bool ParseServerInfo(std::vector<std::string> confInfo, std::unordered_set<std::string> hasPort);
	bool ParseLocationInfo(ServerInfo* serverInfo, const std::vector<std::string>& confInfo, size_t& confInfoIndex, std::unordered_set<std::string>& hasLocation);

	size_t GetLocationInfoIndexByTargetDirectory(const std::string &targetDir, size_t serverInfoIndex) const;
	size_t GetServerInfoIndexByPort(int port) const;

private:
	std::vector<ServerInfo> mServerInfos;
	bool mIsValid;
	int mkeventSize;
	int mListenSize;
};

#endif
