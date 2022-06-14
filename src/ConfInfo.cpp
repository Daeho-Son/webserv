#include "ConfInfo.hpp"

void ConfInfo::SetLocation(const std::string& location)
{
	this->mLocation = location;
}

void ConfInfo::SetAcceptedMethods(const std::vector<std::string>& acceptedMethod)
{
	this->mAcceptedMethods = acceptedMethod;
}

void ConfInfo::SetRoot(const std::string& root)
{
	this->mRoot = root;
}

void ConfInfo::SetDefaultFile(const std::string& deaultFile)
{
	this->mDefaultFile = deaultFile;
}

std::string ConfInfo::GetLocation() const
{
	return this->mLocation;
}

std::vector<std::string> ConfInfo::GetAcceptedMethods() const
{
	return this->mAcceptedMethods;
}

std::string ConfInfo::GetRoot() const
{
	return this->mRoot;
}

std::string ConfInfo::GetDefaultFile() const
{
	return this->mDefaultFile;
}