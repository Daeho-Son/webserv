#include "Conf.hpp"

Conf::Conf()
{
	this->mIsValid = true;
	this->mServerName = "webserv";
	this->mClientBodySize = 4086;
	this->mListenSize = DEFAULT_LISTEN_SIZE;
	this->mkeventSize = DEFAULT_KEVENT_SIZE;
}

Conf::Conf(const std::string &confFile)
	:	mIsValid(true),
		mClientBodySize(0),
		mkeventSize(DEFAULT_KEVENT_SIZE),
		mListenSize(DEFAULT_LISTEN_SIZE)
{
	std::ifstream ifs;
	ifs.open(confFile);
	if (ifs.is_open() == false)
	{
		this->mIsValid = false;
		return ;
	}
	Parse(ifs);
}

Conf::Conf(const Conf &other)
{
	this->mConfInfos = other.GetConfInfos();
	this->mIsValid = other.IsValid();
	this->mServerName = other.GetServerName();
	this->mPorts = other.GetPorts();
	this->mDefaultErrorFile = other.GetDefaultErrorFile();
	this->mClientBodySize = other.GetClientBodySize();
	this->mkeventSize = other.GetKeventsSize();
	this->mListenSize = other.GetListenSize();
}

Conf& Conf::operator=(const Conf &other)
{
	this->mConfInfos = other.GetConfInfos();
	this->mIsValid = other.IsValid();
	this->mServerName = other.GetServerName();
	this->mPorts = other.GetPorts();
	this->mDefaultErrorFile = other.GetDefaultErrorFile();
	this->mClientBodySize = other.GetClientBodySize();
	this->mkeventSize = other.GetKeventsSize();
	this->mListenSize = other.GetListenSize();
	return *this;
}

Conf::~Conf()
{

}

static bool split(std::vector<std::string>& splitString, const std::string& string, const char& delimiter)
{
	std::stringstream ss(string);
	std::string buf;

	while (getline(ss, buf, delimiter))
	{
		splitString.push_back(buf);
	}
	return true;
}

void Conf::Parse(std::ifstream& ifs)
{
	std::unordered_set<std::string> hasLocation;
	if (ParseServerInfo(ifs) == false)
	{
		this->mIsValid = false;
		return ;
	}
	// TODO: location 정보들을 ConfInfo에 담고 ConfInfos에 push_back() 하기
	while (ifs.eof() == false)
	{
		if (ParseLocationInfo(ifs, hasLocation) == false)
		{
			this->mIsValid = false;
			return ;
		}
	}
	if (this->mConfInfos.size() == 0)
	{
		this->mIsValid = false;
	}
	ifs.close();
}

bool Conf::ParseServerInfo(std::ifstream& ifs)
{
	std::string buf;

	while (getline(ifs, buf))
	{
		if (buf == "")
			break ;
		std::vector<std::string> splitString;
		split(splitString, buf, ' ');
		if (splitString.size() != 2)
		{
			this->mIsValid = false;
			return false;
		}
		if (splitString[0] == "server_name")
			this->mServerName = splitString[1];
		else if (splitString[0] == "ports")
		{
			std::vector<std::string> ports;
			split(ports, splitString[1], '|');
			for (size_t i = 0; i < ports.size(); i++)
			{
				char* end;
				this->mPorts.push_back(static_cast<int>(strtod(ports[i].c_str(), &end))); // string을 int로 변환
			}
		}
		else if (splitString[0] == "default_error_file")
		{
			this->mDefaultErrorFile = splitString[1];
		}
		else if (splitString[0] == "client_body_size")
		{
			char *end;
			this->mClientBodySize = static_cast<int>(strtod(splitString[1].c_str(), &end));
		}
		else
		{
			this->mIsValid = false;
			return false;
		}
	}

	if (this->mClientBodySize <= 0)
	{
		this->mIsValid = false;
		return false;
	}
	return true;
}

// confInfo가 지역변수라서 사라지나?

bool Conf::ParseLocationInfo(std::ifstream& ifs, std::unordered_set<std::string>& hasLocation)
{
	this->mConfInfos.push_back(ConfInfo());
	ConfInfo* confInfo = &(this->mConfInfos[this->mConfInfos.size() - 1]);

	// Parse location path
	std::string location;
	getline(ifs, location);
	std::vector<std::string> splitLocation;
	split(splitLocation, location, ' ');
	if (splitLocation.size() != 2 ||
		splitLocation[0] != "location" ||
		hasLocation.find(splitLocation[1]) != hasLocation.end())
	{
		this->mIsValid = false;
		return false;
	}
	hasLocation.insert(splitLocation[1]);
	confInfo->SetLocation(splitLocation[1]);

	// Parse location details
	std::string buf;
	while (getline(ifs, buf))
	{
		if (buf == "")
			break ;
		std::vector<std::string> splitString;
		split(splitString, buf, ' ');
		if (splitString.size() != 2)
		{
			this->mIsValid = false;
			return false;
		}
		if (splitString[0] == "accepted_method")
		{
			if (confInfo->GetAcceptedMethods().size() != 0)
				this->mIsValid = false;
			std::vector<std::string> methods;
			split(methods, splitString[1], '|');
			confInfo->SetAcceptedMethods(methods);
		}
		else if (splitString[0] == "root")
		{
			if (confInfo->GetRoot() != "")
			{
				this->mIsValid = false;
				return false;
			}
			confInfo->SetRoot(splitString[1]);
		}
		else if (splitString[0] == "default_file")
		{
			if (confInfo->GetDefaultFile() != "")
			{
				this->mIsValid = false;
				return false;
			}
			confInfo->SetDefaultFile(splitString[1]);
		}
		else // Unknown keyword
		{
			this->mIsValid = false;
			return false;
		}
	}
	if (confInfo->GetAcceptedMethods().size() == 0 ||
		confInfo->GetRoot() == "" ||
		confInfo->GetDefaultFile() == "")
	{
		this->mIsValid = false;
		return false;
	}
	std::string root = confInfo->GetRoot();
	std::ifstream fIn(root + "/" + this->GetDefaultErrorFile());
	if (fIn.is_open() == false)
		this->mIsValid = false;
	fIn.open(root + "/" + confInfo->GetDefaultFile());
	if (fIn.is_open() == false)
		this->mIsValid = false;
	fIn.close();
	return true;
}

const std::vector<ConfInfo>& Conf::GetConfInfos() const
{
	return this->mConfInfos;
}

bool Conf::IsValid() const
{
	return this->mIsValid;
}

const std::string& Conf::GetServerName() const
{
	return this->mServerName;
}

const std::vector<int>& Conf::GetPorts() const
{
	return this->mPorts;
}

const std::string& Conf::GetDefaultErrorFile() const
{
	return this->mDefaultErrorFile;
}

int Conf::GetClientBodySize() const
{
	return this->mClientBodySize;
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
size_t Conf::GetConfInfoIndexByTargetDirectory(const std::string& targetDir) const
{
	for (size_t i = mConfInfos.size() - 1; i >= 0; i--)
	{
		std::string location = mConfInfos[i].GetLocation();
		size_t targetIndex = targetDir.find(location);
		if (targetIndex == 0)
			return i;
	}
	return mConfInfos.size();
}

std::string Conf::GetRootedLocation(const std::string& targetDir) const
{
	size_t confInfoIndex = this->GetConfInfoIndexByTargetDirectory(targetDir);
	if (confInfoIndex == this->mConfInfos.size())
		return "";

	std::string location = mConfInfos[confInfoIndex].GetLocation();
	size_t targetIndex = targetDir.find(location);
	std::string root = mConfInfos[confInfoIndex].GetRoot();
	root.append("/");
	return root.append(targetDir.substr(targetIndex + location.size(), targetDir.size()));
}

bool Conf::IsRootFolder(const std::string& targetDir) const
{
	size_t confInfoIndex = this->GetConfInfoIndexByTargetDirectory(targetDir);
	if (confInfoIndex == this->mConfInfos.size())
		return false;

	return mConfInfos[confInfoIndex].GetLocation() == targetDir;
}

std::string Conf::GetDefaultPage(const std::string& targetDir) const
{
	int confInfoIndex = GetConfInfoIndexByTargetDirectory(targetDir);
	std::string result = GetRootedLocation(targetDir);
	result.append("/");
	result.append(mConfInfos[confInfoIndex].GetDefaultFile());
	return result;
}

std::string Conf::GetDefaultErrorPage(const std::string& targetDir) const
{
	size_t confInfoIndex = this->GetConfInfoIndexByTargetDirectory(targetDir);
	std::string result = this->mConfInfos[confInfoIndex].GetRoot();
	result.append("/");
	result.append(this->GetDefaultErrorFile());
	return result;
}

bool Conf::IsValidHttpMethod(const std::string& targetDir, const std::string& method) const // path에서 해당 http method가 허용되어 있는지 검사
{
	size_t confInfoIndex = this->GetConfInfoIndexByTargetDirectory(targetDir);
	if (confInfoIndex == this->mConfInfos.size())
		return false;
	std::vector<std::string> acceptedMethods = mConfInfos[confInfoIndex].GetAcceptedMethods();
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
	std::cout << "# server name: " << GetServerName() << std::endl;
	std::cout << "# ports: " << std::endl;
	std::vector<int> ports = GetPorts();
	for (size_t i = 0; i < ports.size(); i++)
	{
		std::cout << " - " << ports[i] << std::endl;
	}
	std::cout << "# default error file: " << GetDefaultErrorFile() << std::endl;
	std::cout << "# client body size: " << GetClientBodySize() << std::endl;
	std::cout << std::endl;
	for (size_t i = 0; i < mConfInfos.size(); i++)
	{
		std::cout << "## location: " << mConfInfos[i].GetLocation() << std::endl;
		std::cout << "## accepted method: " << std::endl;
		std::vector<std::string> acceptedMethods = mConfInfos[i].GetAcceptedMethods();
		for (size_t i = 0; i < acceptedMethods.size(); i++)
		{
			std::cout << " - " << acceptedMethods[i] << std::endl;
		}
		std::cout << "## root: " << mConfInfos[i].GetRoot() << std::endl;
		std::cout << "## default file: " << mConfInfos[i].GetDefaultFile() << std::endl;
		std::cout << std::endl;
	}

}
