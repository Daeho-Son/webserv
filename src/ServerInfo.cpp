#include "ServerInfo.hpp"

void ServerInfo::SetServerName(const std::string &serverName)
{
	mServerName = serverName;
}

void ServerInfo::SetPort(const int &port)
{
	mPort = port;
}

const std::string& ServerInfo::GetServerName() const
{
	return mServerName;
}

const int& ServerInfo::GetPort() const
{
	return mPort;
}

void ServerInfo::AddLocationInfo(const LocationInfo &locationInfo)
{
	mLocationInfos.push_back(locationInfo);
}

const std::vector<LocationInfo>& ServerInfo::GetLocationInfos() const
{
	return mLocationInfos;
}
