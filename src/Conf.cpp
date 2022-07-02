#include "Conf.hpp"

Conf::Conf()
{
	this->mIsValid = true;
	this->mListenSize = DEFAULT_LISTEN_SIZE;
	this->mkeventSize = DEFAULT_KEVENT_SIZE;
}

Conf::Conf(const std::string &confFile)
{
	this->mIsValid = true;
	this->mkeventSize = DEFAULT_KEVENT_SIZE;
	this->mListenSize = DEFAULT_LISTEN_SIZE;
	Parse(confFile);
}

Conf::Conf(const Conf &other)
{
	this->mServerInfos = other.GetServerInfos();
	this->mIsValid = other.IsValid();
	this->mkeventSize = other.GetKeventsSize();
	this->mListenSize = other.GetListenSize();
}

Conf& Conf::operator=(const Conf &other)
{
	this->mServerInfos = other.GetServerInfos();
	this->mIsValid = other.IsValid();
	this->mkeventSize = other.GetKeventsSize();
	this->mListenSize = other.GetListenSize();
	return *this;
}

Conf::~Conf()
{

}

void Conf::Parse(const std::string& confFile)
{
	std::ifstream ifs;

	ifs.open(confFile);
	if (ifs.is_open() == false)
	{
		this->mIsValid = false;
		return ;
	}
	std::set<std::string> hasPort;
	std::vector<std::string> confInfo;
	std::string buf;
	while (ifs.eof() == false)
	{
		getline(ifs, buf);
		if (buf == "::eosb")
		{
			this->mIsValid = ParseServerInfo(confInfo, hasPort);
			confInfo.clear();
		}
		else
			confInfo.push_back(buf);
	}
	if (this->mServerInfos.size() == 0)
		this->mIsValid = false;
	ifs.close();
}

static bool split(std::vector<std::string>& splitString, const std::string& string, const char& delimiter)
{
	std::stringstream ss(string);
	std::string buf;

	while (getline(ss, buf, delimiter))
		splitString.push_back(buf);
	return true;
}

bool Conf::ParseServerInfo(std::vector<std::string> confInfo, std::set<std::string> hasPort)
{
	this->mServerInfos.push_back(ServerInfo());
	ServerInfo* serverInfo = &(this->mServerInfos[this->mServerInfos.size() - 1]);

	size_t confInfoIndex = 0;
	for (; confInfoIndex < confInfo.size(); confInfoIndex++)
	{
		if (confInfo[confInfoIndex] == "")
		{
			confInfoIndex++;
			break;
		}
		std::vector<std::string> splitString;
		split(splitString, confInfo[confInfoIndex], ' ');
		if (splitString.size() != 2)
		{
			this->mIsValid = false;
			return false;
		}
		if (splitString[0] == "server_name")
			serverInfo->SetServerName(splitString[1]);
		else if (splitString[0] == "port")
		{
			if (hasPort.find(splitString[1]) != hasPort.end())
			{
				this->mIsValid = false;
				return false;
			}
			hasPort.insert(splitString[1]);
			serverInfo->SetPort(static_cast<int>(strtod(splitString[1].c_str(), NULL)));
		}
		else
		{
			this->mIsValid = false;
			return false;
		}
	}
	std::set<std::string> hasLocation;
	for (; confInfoIndex < confInfo.size(); confInfoIndex++)
	{
		this->mIsValid = ParseLocationInfo(serverInfo, confInfo, confInfoIndex, hasLocation);
		if (this->mIsValid == false)
			return false;
	}
	return true;
}

bool Conf::ParseLocationInfo(ServerInfo* serverInfo, const std::vector<std::string>& confInfo, size_t& confInfoIndex, std::set<std::string>& hasLocation)
{
	// Parse location path
	LocationInfo locationInfo;
	std::vector<std::string> splitLocation;
	split(splitLocation, confInfo[confInfoIndex++], ' ');
	if (splitLocation.size() != 2 ||
		splitLocation[0] != "location" ||
		hasLocation.find(splitLocation[1]) != hasLocation.end())
	{
		this->mIsValid = false;
		return false;
	}
	hasLocation.insert(splitLocation[1]);
	locationInfo.SetLocation(splitLocation[1]);
	// Parse location details
	for (; confInfoIndex < confInfo.size(); confInfoIndex++)
	{
		if (confInfo[confInfoIndex] == "")
		{
			break;
		}
		std::vector<std::string> splitString;
		split(splitString, confInfo[confInfoIndex], ' ');
		if (splitString.size() != 2)
		{
			this->mIsValid = false;
			return false;
		}
		if (splitString[0] == "accepted_method")
		{
			if (locationInfo.GetAcceptedMethods().size() != 0)
				this->mIsValid = false;
			std::vector<std::string> methods;
			split(methods, splitString[1], '|');
			locationInfo.SetAcceptedMethods(methods);
		}
		else if (splitString[0] == "root")
		{
			if (locationInfo.GetRoot() != "")
			{
				this->mIsValid = false;
				return false;
			}
			locationInfo.SetRoot(splitString[1]);
		}
		else if (splitString[0] == "default_file")
		{
			if (locationInfo.GetDefaultFile() != "")
			{
				this->mIsValid = false;
				return false;
			}
			locationInfo.SetDefaultFile(splitString[1]);
		}
		else if (splitString[0] == "default_error_file")
			locationInfo.SetDefaultErrorfile(splitString[1]);
		else if (splitString[0] == "client_body_size")
			locationInfo.SetClientBodySize(static_cast<int>(strtod(splitString[1].c_str(), NULL)));
		else if (splitString[0] == "cgi")
		{
			if (locationInfo.GetCgi().size() != 0)
			{
				this->mIsValid = false;
				return false;
			}
			std::vector<std::string> cgis;
			split(cgis, splitString[1], '|');
			locationInfo.SetCgi(cgis);
		}
		else // Unknown keyword
		{
			this->mIsValid = false;
			return false;
		}
	}
	if (locationInfo.GetAcceptedMethods().size() == 0 ||
		locationInfo.GetRoot() == "" ||
		locationInfo.GetDefaultFile() == "" ||
		locationInfo.GetClientBodySize() <= 0)
	{
		this->mIsValid = false;
		return false;
	}
	std::string root = locationInfo.GetRoot();
	std::ifstream fIn(root + "/" + locationInfo.GetDefaultFile());
	if (fIn.is_open() == false)
		this->mIsValid = false;
	fIn.open(root + "/" + locationInfo.GetDefaultErrorFile());
	if (fIn.is_open() == false)
		this->mIsValid = false;
	fIn.close();
	serverInfo->AddLocationInfo(locationInfo);
	return true;
}

const std::vector<ServerInfo>& Conf::GetServerInfos() const
{
	return this->mServerInfos;
}

bool Conf::IsValid() const
{
	return this->mIsValid;
}

int Conf::GetKeventsSize() const
{
	return mkeventSize;
}

int Conf::GetListenSize() const
{
	return mListenSize;
}

// 에러면 confinfos.size() 리턴
size_t Conf::GetLocationInfoIndexByTargetDirectory(const std::string& targetDir, size_t serverInfoIndex) const
{
	std::vector<LocationInfo> locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	for (size_t i = locationInfos.size() - 1; i >= 0; i--)
	{
		std::string location = locationInfos[i].GetLocation();
		size_t targetIndex = targetDir.find(location);
		if (targetIndex == 0)
			return i;
	}
	return locationInfos.size();
}

size_t Conf::GetServerInfoIndexByPort(int port) const
{
	size_t serverInfoIndex = 0;
	for (; serverInfoIndex < this->mServerInfos.size(); serverInfoIndex++)
	{
		if (this->mServerInfos[serverInfoIndex].GetPort() == port)
			return serverInfoIndex;
	}
	return this->mServerInfos.size();
}

std::string Conf::GetRootedLocation(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = this->GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	std::vector<LocationInfo> locationInfos = this->mServerInfos[serverInfoIndex].GetLocationInfos();
	if (confInfoIndex == locationInfos.size())
		return "";

	std::string location = locationInfos[confInfoIndex].GetLocation();
	size_t targetIndex = targetDir.find(location);
	std::string root = locationInfos[confInfoIndex].GetRoot();
	root.append("/");
	return root.append(targetDir.substr(targetIndex + location.size(), targetDir.size()));
}

std::string Conf::GetDefaultPage(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = this->GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	std::vector<LocationInfo> locationInfos = this->mServerInfos[serverInfoIndex].GetLocationInfos();
	if (confInfoIndex == locationInfos.size())
		return "";
	std::string result = locationInfos[confInfoIndex].GetRoot();
	result.append("/");
	result.append(locationInfos[confInfoIndex].GetDefaultFile());
	return result;
}

std::string Conf::GetDefaultErrorPage(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = this->GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	std::vector<LocationInfo> locationInfos = this->mServerInfos[serverInfoIndex].GetLocationInfos();
	std::string result = locationInfos[confInfoIndex].GetRoot();
	result.append("/");
	result.append(locationInfos[confInfoIndex].GetDefaultErrorFile());
	return result;
}

bool Conf::IsValidHttpMethod(const std::string& targetDir, int port, const std::string& method) const // path에서 해당 http method가 허용되어 있는지 검사
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = this->GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	std::vector<LocationInfo> locationInfos = this->mServerInfos[serverInfoIndex].GetLocationInfos();
	if (confInfoIndex == locationInfos.size())
		return false;
	std::vector<std::string> acceptedMethods = locationInfos[confInfoIndex].GetAcceptedMethods();
	for (size_t i = 0; i < acceptedMethods.size(); i++)
	{
		if (acceptedMethods[i] == method) //TODO: Method를 string이 아니라 enum으로 저장하기
			return true;
	}
	return false;
}

void Conf::PrintConfData() const
{
	std::cout << "# is valid: " << IsValid() << std::endl;
	for (size_t j = 0; j < this->mServerInfos.size(); j++)
	{
		std::cout << "# server name: " << this->mServerInfos[j].GetServerName() << std::endl;
		std::cout << "# port: " << this->mServerInfos[j].GetPort() << std::endl;
		std::vector<LocationInfo> locationInfos = mServerInfos[j].GetLocationInfos();
		for (size_t i = 0; i < locationInfos.size(); i++)
		{
			std::cout << "## location: " << locationInfos[i].GetLocation() << std::endl;
			std::cout << "## accepted method: " << std::endl;
			std::vector<std::string> acceptedMethods = locationInfos[i].GetAcceptedMethods();
			for (size_t i = 0; i < acceptedMethods.size(); i++)
			{
				std::cout << " - " << acceptedMethods[i] << std::endl;
			}
			std::cout << "## root: " << locationInfos[i].GetRoot() << std::endl;
			std::cout << "## default file: " << locationInfos[i].GetDefaultFile() << std::endl;
			std::cout << "## default error file: " << locationInfos[i].GetDefaultErrorFile() << std::endl;
			std::cout << "## client body size: " << locationInfos[i].GetClientBodySize() << std::endl;
			std::cout << "## cgi: " << std::endl;
			std::vector<std::string> cgis = locationInfos[i].GetCgi();
			for (size_t i = 0; i < cgis.size(); i++)
			{
				std::cout << " - " << cgis[i] << std::endl;
			}
			std::cout << std::endl;
		}
		std::cout << "----------"<< std::endl;
	}
}