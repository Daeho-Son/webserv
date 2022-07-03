#include "HttpResponse.hpp"

using namespace ft;

HttpResponse::HttpResponse()
{
}

HttpResponse::HttpResponse(int statusCode, const std::string& body)
{
	this->mHttpVersion = "HTTP/1.1";
	this->mStatusCode = statusCode;
	this->mDate = GetHttpFormDate();
	this->mConnection = "close";
	this->mContentType = "text/html";
	this->mBody = body;
}

HttpResponse::HttpResponse(const std::string& cgiData)
{
	std::stringstream ss;
	std::string buffer;
	ss << cgiData;
	std::vector<std::string> splitData;
	while (true)
	{
		getline(ss, buffer);
		splitData.push_back(buffer);
		if (ss.eof() || buffer.length() <= 1)
			break;
	}

	// Set Http Header
	std::stringstream tss;
	for (size_t i=0; i<splitData.size(); ++i)
	{
		size_t delimPos = splitData[i].find(':');
		if (delimPos == splitData[i].npos)
			break;
		std::string key = splitData[i].substr(0, delimPos);
		if (key == "Status")
		{
			this->mStatusCode = strtod(splitData[i].substr(delimPos+2, 3).c_str(), NULL);
		}
		else if (key == "Content-Type")
		{
			this->mContentType =  splitData[i].substr(delimPos+2);
		}
	}
	this->mHttpVersion = "HTTP/1.1";
	this->mDate = GetHttpFormDate();
	this->mConnection = "close";

	// Get Body
	size_t bodyPos = cgiData.find("\r\n\r\n") + 4;
	if (bodyPos > cgiData.length())
	{
		this->mBody = cgiData.substr(bodyPos);
	}
}

HttpResponse::HttpResponse(const HttpResponse& other)
{
	*this = other;
}

HttpResponse& HttpResponse::operator=(const HttpResponse& other)
{
	this->mHttpVersion = other.GetHttpVersion();
	this->mStatusCode = other.GetStatusCode();
	this->mDate = other.GetDate();
	this->mConnection = other.GetConnection();
	this->mContentType = other.GetContentType();
	this->mBody = other.GetBody();

	return *this;
}

// Getters
std::string HttpResponse::GetHttpVersion() const
{
	return this->mHttpVersion;
}

int HttpResponse::GetStatusCode() const
{
	return this->mStatusCode;
}

std::string HttpResponse::GetStatusText() const
{
	return GetStatusTextByStatusCode(this->mStatusCode);
}

std::string HttpResponse::GetConnection() const
{
	return this->mConnection;
}

int HttpResponse::GetContentLength() const
{
	return (this->mBody).length();
}

std::string HttpResponse::GetContentType() const
{
	return this->mContentType;
}

std::string HttpResponse::GetDate() const
{
	return this->mDate;
}

std::string HttpResponse::GetBody() const
{
	return this->mBody;
}

std::string HttpResponse::GetHttpMessage() const
{
	std::stringstream ss;

	ss << this->GetHttpVersion() << " " << this->GetStatusCode() << " " << this->GetStatusText() << "\n";
	ss << "Connection: " << this->GetConnection() << "\n";
	ss << "Content-Length: " << this->GetContentLength() << "\n";
	ss << "Content-Type: " << this->GetContentType() << "; charset=UTF-8\n";
	ss << "Date: " << this->GetDate() << "\n\n";
	ss << this->GetBody();
	return ss.str();
}

HttpResponse::~HttpResponse()
{

}


