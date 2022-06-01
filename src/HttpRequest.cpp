#include "HttpRequest.hpp"
#include <iostream>

HttpRequest::HttpRequest()
{
}

HttpRequest::HttpRequest(const std::string& httpMessage)
{
	if (parseHttpMessageToMap(mHttpMessageMap, httpMessage) == false)
		throw parseException();

}

HttpRequest::HttpRequest(const HttpRequest& other)
{
	this->mHttpMessageMap = other.mHttpMessageMap;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	this->mHttpMessageMap = other.mHttpMessageMap;
	return *this;
}

HttpRequest::~HttpRequest()
{
}

static std::vector<std::string> split(const std::string& string, char delimiter)
{
	std::vector<std::string> split_str;
	std::stringstream ss(string);
	std::string temp;
	while (getline(ss, temp, delimiter))
		split_str.push_back(temp);
	return split_str;
}

bool HttpRequest::parseHttpMessageToMap(std::unordered_map<std::string, std::string>& mHttpMessageMap, const std::string& httpMessage)
{
	std::vector<std::string> splitMessage = split(httpMessage, '\n');
	if (splitMessage.size() == 0)
	{
		std::cout << "Start line이 존재하지 않습니다." << std::endl;
		return (false);
	}
	std::vector<std::string> startLine = split(splitMessage[0], ' ');
	if (startLine.size() != 3)
	{
		std::cout << "Start line에 잘못된 값이 입력되었습니다." << std::endl;
		return (false);
	}
	mHttpMessageMap["HttpMethod"] = startLine[0];
	mHttpMessageMap["RequestTarget"] = startLine[1];
	mHttpMessageMap["HttpVersion"] = startLine[2];
	for (unsigned int i = 1; i < splitMessage.size(); i++)
	{
		if (splitMessage[i].length() == 1)
			break;
		std::vector<std::string> data = split(splitMessage[i], ':');
		std::string key = "";
		unsigned int j;
		for (j = 1; j < data.size() - 1; j++)
			key = key + data[j] + ":";
		key = key + data[j];
		mHttpMessageMap[data[0]] = key.substr(1, key.size() - 1);
	}

	if (httpMessage.find("\n\n") == httpMessage.npos)
	{
		mHttpMessageMap["Body"] = "null";
	}
	else
	{
		mHttpMessageMap["Body"] = httpMessage.substr(httpMessage.find("\n\n") + 2, httpMessage.size() - httpMessage.find("\n\n") - 2);
	}
	return (true);
}

std::string HttpRequest::getFieldByKey(const std::string &key)
{
	std::unordered_map<std::string, std::string>::iterator fieldIt = this->mHttpMessageMap.find(key);
	if (fieldIt == this->mHttpMessageMap.end())
		return ("null");
	std::cout << (*fieldIt).second;
	return (*fieldIt).second;
}
