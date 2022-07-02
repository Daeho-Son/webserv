#include "LocationInfo.hpp"

void LocationInfo::SetLocation(const std::string& location)
{
	this->mLocation = location;
}

void LocationInfo::SetAcceptedMethods(const std::vector<std::string>& acceptedMethod)
{
	this->mAcceptedMethods = acceptedMethod;
}

void LocationInfo::SetRoot(const std::string& root)
{
	this->mRoot = root;
}

void LocationInfo::SetDefaultFile(const std::string& deaultFile)
{
	this->mDefaultFile = deaultFile;
}

void LocationInfo::SetDefaultErrorfile(const std::string& defaultErrorFile)
{
	this->mDefaultErrorFile = defaultErrorFile;
}

void LocationInfo::SetClientBodySize(int clientBodySize)
{
	this->mClientBodySize = clientBodySize;
}

void LocationInfo::SetCgi(const std::vector<std::string>& cgi)
{
	this->mCgi = cgi;
}

std::string LocationInfo::GetLocation() const
{
	return this->mLocation;
}

std::vector<std::string> LocationInfo::GetAcceptedMethods() const
{
	return this->mAcceptedMethods;
}

const std::string& LocationInfo::GetRoot() const
{
	return this->mRoot;
}

const std::string& LocationInfo::GetDefaultFile() const
{
	return this->mDefaultFile;
}

const std::string& LocationInfo::GetDefaultErrorFile() const
{
	return this->mDefaultErrorFile;
}

int LocationInfo::GetClientBodySize() const
{
	return this->mClientBodySize;
}

std::vector<std::string> LocationInfo::GetCgi() const
{
	return this->mCgi;
}