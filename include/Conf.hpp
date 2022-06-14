#ifndef CONF_HPP
# define CONF_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <vector>
# include <sstream>
# include <cstdlib>
# include <unordered_set>

# include "ConfInfo.hpp"

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

	// TODO: Redirection?
	const std::vector<ConfInfo>& GetConfInfos() const;
	bool IsValid() const;
	const std::string& GetServerName() const;
	const std::vector<int>& GetPorts() const;
	const std::string& GetDefaultErrorFile() const;
	int GetClientBodySize() const;
	int GetKeventsSize() const;
	int GetListenSize() const;
	
	std::string GetRootedLocation(const std::string& targetDir) const;
	std::string GetDefaultPage(const std::string& targetDir) const;
	std::string GetDefaultErrorPage(const std::string& targetDir) const;
	bool IsValidHttpMethod(const std::string& targetDir, const std::string& method) const; // path에서 해당 http method가 허용되어 있는지 검사
	void PrintConfData() const;

private:
	void Parse(std::ifstream& ifs);
	bool ParseServerInfo(std::ifstream& ifs);
	bool ParseLocationInfo(std::ifstream& ifs, std::unordered_set<std::string>& hasLocation);
	size_t GetConfInfoIndexByTargetDirectory(const std::string &targetDir) const;

private:
	std::vector<ConfInfo> mConfInfos;
	bool mIsValid;
	std::string mServerName;
	std::vector<int> mPorts;
	std::string mDefaultErrorFile;
	int mClientBodySize;
	int mkeventSize;
	int mListenSize;
};

#endif
