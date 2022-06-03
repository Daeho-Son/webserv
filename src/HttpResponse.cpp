#include "HttpResponse.hpp"

using namespace ft;

HttpResponse::HttpResponse()
{
	std::cerr << "You should not instantiate HttpResponse by default initializer!\n";
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


