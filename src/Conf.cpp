#include "Conf.hpp"

Conf::Conf()
{
	mIsValid = true;
	mListenSize = DEFAULT_LISTEN_SIZE;
	mkeventSize = DEFAULT_KEVENT_SIZE;
}

Conf::Conf(const std::string &confFile)
	:	mIsValid(true),
		mkeventSize(DEFAULT_KEVENT_SIZE),
		mListenSize(DEFAULT_LISTEN_SIZE)
{
	Parse(confFile);
}

Conf::Conf(const Conf &other)
{
	mServerInfos = other.GetServerInfos();
	mIsValid = other.IsValid();
	mkeventSize = other.GetKeventsSize();
	mListenSize = other.GetListenSize();
}

Conf& Conf::operator=(const Conf &other)
{
	mServerInfos = other.GetServerInfos();
	mIsValid = other.IsValid();
	mkeventSize = other.GetKeventsSize();
	mListenSize = other.GetListenSize();
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
		mIsValid = false;
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
			mIsValid = ParseServerInfo(confInfo, hasPort);
			if (mIsValid == false)
				return ;
			confInfo.clear();
		}
		else
			confInfo.push_back(buf);
	}
	if (mServerInfos.size() == 0)
		mIsValid = false;
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
	mServerInfos.push_back(ServerInfo());
	ServerInfo* serverInfo = &(mServerInfos[mServerInfos.size() - 1]);

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
			mIsValid = false;
			return false;
		}
		if (splitString[0] == "server_name")
			serverInfo->SetServerName(splitString[1]);
		else if (splitString[0] == "port")
		{
			if (hasPort.find(splitString[1]) != hasPort.end())
			{
				mIsValid = false;
				return false;
			}
			hasPort.insert(splitString[1]);
			serverInfo->SetPort(static_cast<int>(strtod(splitString[1].c_str(), NULL)));
		}
		else
		{
			mIsValid = false;
			return false;
		}
	}
	std::set<std::string> hasLocation;
	for (; confInfoIndex < confInfo.size(); confInfoIndex++)
	{
		mIsValid = ParseLocationInfo(serverInfo, confInfo, confInfoIndex, hasLocation);
		if (mIsValid == false)
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
		mIsValid = false;
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
			mIsValid = false;
			return false;
		}
		if (splitString[0] == "accepted_method")
		{
			if (locationInfo.GetAcceptedMethods().size() != 0)
				mIsValid = false;
			std::vector<std::string> methods;
			split(methods, splitString[1], '|');
			locationInfo.SetAcceptedMethods(methods);
		}
		else if (splitString[0] == "root")
		{
			if (locationInfo.GetRoot() != "")
			{
				mIsValid = false;
				return false;
			}
			locationInfo.SetRoot(splitString[1]);
		}
		else if (splitString[0] == "default_file")
		{
			if (locationInfo.GetDefaultFile() != "")
			{
				mIsValid = false;
				return false;
			}
			locationInfo.SetDefaultFile(splitString[1]);
		}
		else if (splitString[0] == "default_error_file")
			locationInfo.SetDefaultErrorfile(splitString[1]);
		else if (splitString[0] == "client_body_size")
			locationInfo.SetClientBodySize(static_cast<size_t>(strtod(splitString[1].c_str(), NULL)));
		else if (splitString[0] == "cgi")
		{
			if (locationInfo.GetCgi().size() != 0)
			{
				mIsValid = false;
				return false;
			}
			std::vector<std::string> cgis;
			split(cgis, splitString[1], '|');
			locationInfo.SetCgi(cgis);
		}
		else if (splitString[0] == "auto_index")
		{
			locationInfo.SetAutoIndex(splitString[1]);
		}
		else if (splitString[0] == "redirect")
		{
			locationInfo.SetRedirect(splitString[1]);
		}
		else // Unknown keyword
		{
			mIsValid = false;
			return false;
		}
	}
	if (locationInfo.GetAcceptedMethods().size() == 0 ||
		locationInfo.GetRoot() == "" ||
		locationInfo.GetDefaultFile() == "" ||
		locationInfo.GetClientBodySize() <= 0)
	{
		mIsValid = false;
		return false;
	}
	std::string root = locationInfo.GetRoot();
	std::ifstream fIn(root + "/" + locationInfo.GetDefaultFile());
	if (fIn.is_open() == false)
		mIsValid = false;
	fIn.open(root + "/" + locationInfo.GetDefaultErrorFile());
	if (fIn.is_open() == false)
		mIsValid = false;
	fIn.close();
	serverInfo->AddLocationInfo(locationInfo);
	return true;
}

const std::vector<ServerInfo>& Conf::GetServerInfos() const
{
	return mServerInfos;
}

bool Conf::IsValid() const
{
	return mIsValid;
}

int Conf::GetKeventsSize() const
{
	return mkeventSize;
}

int Conf::GetListenSize() const
{
	return mListenSize;
}

const std::string& Conf::GetRedirectTarget(const std::string& target, int port) const
{
	const LocationInfo& locationInfo = GetLocationInfo(target, port);
	return locationInfo.GetRedirectLocation();
}

// 에러면 confinfos.size() 리턴
size_t Conf::GetLocationInfoIndexByTargetDirectory(const std::string& targetDir, size_t serverInfoIndex) const
{
	if (mServerInfos.size() <= serverInfoIndex)
		return SIZE_T_MAX;
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	for (size_t i = locationInfos.size() - 1; i >= 0; i--)
	{
		const std::string& location = locationInfos[i].GetLocation();
		size_t targetIndex = targetDir.find(location);
		if (targetIndex == 0)
			return i;
	}
	return locationInfos.size();
}

size_t Conf::GetServerInfoIndexByPort(int port) const
{
	size_t serverInfoIndex = 0;
	for (; serverInfoIndex < mServerInfos.size(); serverInfoIndex++)
	{
		if (mServerInfos[serverInfoIndex].GetPort() == port)
			return serverInfoIndex;
	}
	return mServerInfos.size();
}

bool Conf::IsValidCgiExtension(const std::string &targetDir, int port, const std::string& cgiExtension) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		return "";
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	if (locationInfoIndex == locationInfos.size() || locationInfoIndex == SIZE_T_MAX)
		return "";
	const std::vector<std::string>& cgiExtensions = locationInfos[locationInfoIndex].GetCgi();
	std::vector<std::string>::const_iterator it = cgiExtensions.begin();
	for (; it != cgiExtensions.end(); ++it)
	{
		if (*it == cgiExtension) return true;
	}
	return false;
}

bool Conf::IsRedirectedTarget(const std::string& target, int port) const 
{
	return GetLocationInfo(target, port).IsRedirected();
}

std::string Conf::GetRootedLocation(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex= GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		return "";
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	if (locationInfoIndex == locationInfos.size() || locationInfoIndex == SIZE_T_MAX)
		return "";
	const std::string& location = locationInfos[locationInfoIndex].GetLocation();
	size_t targetIndex = targetDir.find(location);
	std::string root = locationInfos[locationInfoIndex].GetRoot();
	root.append("/");
	return root.append(targetDir.substr(targetIndex + location.size(), targetDir.size()));
}

const LocationInfo& Conf::GetLocationInfo(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex= GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		throw new std::out_of_range("No Location Info");
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	if (locationInfoIndex == locationInfos.size() || locationInfoIndex == SIZE_T_MAX)
		throw new std::out_of_range("No Location Info");
	return locationInfos[locationInfoIndex];
}

bool Conf::IsRootFolder(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		return false;
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	if (locationInfoIndex == locationInfos.size() || locationInfoIndex == SIZE_T_MAX)
		return false;
	const std::string& location = locationInfos[locationInfoIndex].GetLocation();
	return location == targetDir;
}

bool Conf::IsAutoIndex(const std::string &targetDir, int port) const
{
	size_t serverInfoIndex= GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		return "";
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	if (locationInfoIndex == locationInfos.size() || locationInfoIndex == SIZE_T_MAX)
		return "";
	return locationInfos[locationInfoIndex].IsAutoIndex();
}

std::string Conf::GetDefaultPage(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	if (serverInfoIndex == mServerInfos.size())
		return "";
	size_t locationInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	if (locationInfoIndex == locationInfos.size())
		return "";
	std::string result = GetRootedLocation(targetDir, port);
	result.append("/");
	result.append(locationInfos[locationInfoIndex].GetDefaultFile());
	return result;
}

std::string Conf::GetDefaultErrorPage(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	std::string result = locationInfos[confInfoIndex].GetRoot();
	result.append("/");
	result.append(locationInfos[confInfoIndex].GetDefaultErrorFile());
	return result;
}

size_t Conf::GetClientBodySize(const std::string& targetDir, int port) const
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	return locationInfos[confInfoIndex].GetClientBodySize();
}

bool Conf::IsValidHttpMethod(const std::string& targetDir, int port, const std::string& method) const // path에서 해당 http method가 허용되어 있는지 검사
{
	size_t serverInfoIndex = GetServerInfoIndexByPort(port);
	size_t confInfoIndex = GetLocationInfoIndexByTargetDirectory(targetDir, serverInfoIndex);
	const std::vector<LocationInfo>& locationInfos = mServerInfos[serverInfoIndex].GetLocationInfos();
	if (confInfoIndex == locationInfos.size())
		return false;
	const std::vector<std::string>& acceptedMethods = locationInfos[confInfoIndex].GetAcceptedMethods();
	for (size_t i = 0; i < acceptedMethods.size(); i++)
	{
		if (acceptedMethods[i] == method)
			return true;
	}
	return false;
}

void Conf::PrintConfData() const
{
	std::cout << "# is valid: " << IsValid() << "\n";
	for (size_t j = 0; j < mServerInfos.size(); j++)
	{
		std::cout << "# server name: " << mServerInfos[j].GetServerName() << "\n";
		std::cout << "# port: " << mServerInfos[j].GetPort() << "\n";
		std::vector<LocationInfo> locationInfos = mServerInfos[j].GetLocationInfos();
		for (size_t i = 0; i < locationInfos.size(); i++)
		{
			std::cout << "## location: " << locationInfos[i].GetLocation() << "\n";
			std::cout << "## accepted method: " << std::endl;
			std::vector<std::string> acceptedMethods = locationInfos[i].GetAcceptedMethods();
			for (size_t i = 0; i < acceptedMethods.size(); i++)
			{
				std::cout << " - " << acceptedMethods[i] << "\n";
			}
			std::cout << "## root: " << locationInfos[i].GetRoot() << "\n";
			std::cout << "## default file: " << locationInfos[i].GetDefaultFile() << "\n";
			std::cout << "## default error file: " << locationInfos[i].GetDefaultErrorFile() << "\n";
			std::cout << "## client body size: " << locationInfos[i].GetClientBodySize() << "\n";
			std::cout << "## cgi: " << std::endl;
			std::cout << "## auto index: " << locationInfos[i].IsAutoIndex() << "\n";
			std::vector<std::string> cgis = locationInfos[i].GetCgi();
			for (size_t i = 0; i < cgis.size(); i++)
			{
				std::cout << " - " << cgis[i] << "\n";
			}
			std::cout << std::endl;
		}
		std::cout << "----------\n";
	}
}
