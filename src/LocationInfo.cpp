#include "LocationInfo.hpp"

LocationInfo::LocationInfo()
	:	mbIsRedirected(false),
		mRedirectLocation(""),
		mClientBodySize(SIZE_T_MAX),
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

void LocationInfo::SetRedirect(const std::string& redirectLocation)
{
	mbIsRedirected = true;
	mRedirectLocation = redirectLocation;
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

const std::string& LocationInfo::GetRedirectLocation() const
{
	return mRedirectLocation;
}

bool LocationInfo::IsAutoIndex() const
{
	return mAutoIndex;
}

bool LocationInfo::IsRedirected() const 
{
	return mbIsRedirected;
}