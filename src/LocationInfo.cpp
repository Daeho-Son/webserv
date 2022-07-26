#include "LocationInfo.hpp"

LocationInfo::LocationInfo()
	:	mClientBodySize(SIZE_T_MAX),
		mAutoIndex(false) {}

void LocationInfo::SetLocation(const std::string& location)
{
	mLocation = location;
}

void LocationInfo::SetAcceptedMethods(const std::vector<std::string>& acceptedMethod)
{
	mAcceptedMethods = acceptedMethod;
}

void LocationInfo::SetRoot(const std::string& root)
{
	mRoot = root;
}

void LocationInfo::SetDefaultFile(const std::string& defaultFile)
{
	mDefaultFile = defaultFile;
}

void LocationInfo::SetDefaultErrorfile(const std::string& defaultErrorFile)
{
	mDefaultErrorFile = defaultErrorFile;
}

void LocationInfo::SetClientBodySize(size_t clientBodySize)
{
	mClientBodySize = clientBodySize;
}

void LocationInfo::SetCgi(const std::vector<std::string>& cgi)
{
	mCgi = cgi;
}

void LocationInfo::SetAutoIndex(const std::string& autoIndex)
{
	if (autoIndex == "on")
		mAutoIndex = true;
	else
		mAutoIndex = false;
}

const std::string& LocationInfo::GetLocation() const
{
	return mLocation;
}

const std::vector<std::string>& LocationInfo::GetAcceptedMethods() const
{
	return mAcceptedMethods;
}

const std::string& LocationInfo::GetRoot() const
{
	return mRoot;
}

const std::string& LocationInfo::GetDefaultFile() const
{
	return mDefaultFile;
}

const std::string& LocationInfo::GetDefaultErrorFile() const
{
	return mDefaultErrorFile;
}

size_t LocationInfo::GetClientBodySize() const
{
	return mClientBodySize;
}

const std::vector<std::string>& LocationInfo::GetCgi() const
{
	return mCgi;
}

bool LocationInfo::IsAutoIndex() const
{
	return mAutoIndex;
}