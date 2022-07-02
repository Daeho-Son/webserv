#ifndef SERVERINFO_HPP
# define SERVERINFO_HPP

# include <string>
# include <vector>

# include "LocationInfo.hpp"

struct ServerInfo
{
public:
	void SetServerName(const std::string& serverName);
	void SetPort(const int& port);
	const std::string& GetServerName() const;
	const int& GetPort() const;

	void AddLocationInfo(const LocationInfo& locationInfo);
	const std::vector<LocationInfo>& GetLocationInfos() const;


private:
	std::string mServerName;
	int mPort;

	std::vector<LocationInfo> mLocationInfos;
};

#endif