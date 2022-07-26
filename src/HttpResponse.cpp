#include "HttpResponse.hpp"

using namespace ft;

HttpResponse::HttpResponse()
{
	this->mHttpVersion = "HTTP/1.1";
	this->mStatusCode = 0;
	this->mDate = GetHttpFormDate();
	this->mConnection = "Keep-Alive";
	this->mContentType = "text/html";
	this->mBody = "";
	this->mSendIndex = 0;
	this->mIsSendDone = false;
	this->mHasMessage = false;
	this->mMessageLength = 0;
	this->mRedirectLocation = "";
}

HttpResponse::HttpResponse(int statusCode, const std::string& body, const std::string& connection)
{
	this->mHttpVersion = "HTTP/1.1";
	this->mStatusCode = statusCode;
	this->mDate = GetHttpFormDate();
	if (connection == "close")
		this->mConnection = "close";
	else
		this->mConnection = "Keep-Alive";
	this->mContentType = "text/html";
	this->mBody = body;
	this->mSendIndex = 0;
	this->mIsSendDone = false;
	this->mHasMessage = false;
	this->mMessageLength = 0;
	this->mRedirectLocation = "";
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

	this->mHttpVersion = "HTTP/1.1";
	this->mDate = GetHttpFormDate();
	this->mConnection = "Keep-Alive";
	this->mRedirectLocation = "";
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
			this->mContentType = splitData[i].substr(delimPos+2);
		}
		else if (key == "Connection")
		{
			this->mConnection = splitData[i].substr(delimPos+2);
		}
	}

	// Get Body
	size_t bodyPos = cgiData.find("\r\n\r\n") + 4;
	if (bodyPos < cgiData.length())
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
	this->mSendIndex = 0;
	this->mIsSendDone = false;
	this->mHasMessage = false;
	this->mMessageLength = 0;
	this->mRedirectLocation = other.mRedirectLocation;

	return *this;
}

// Setter
void HttpResponse::SetHttpMessage()
{
	std::stringstream ss;

	ss << this->GetHttpVersion() << " " << this->GetStatusCode() << " " << this->GetStatusText() << "\n";
	ss << "Content-Length: " << this->GetContentLength() << "\r\n";
	ss << "Content-Type: " << this->GetContentType() << "; charset=UTF-8\r\n";
	std::cout << "mRed: " << mRedirectLocation << std::endl;
	if (mRedirectLocation != "")
	{
		std::cout << "Location: " << mRedirectLocation << "\n";
		ss << "Location: " << mRedirectLocation << "\r\n\r\n";
	}
	else if (this->GetConnection() == "close")
	{
		ss << "Date: " << this->GetDate() << "\r\n\r\n";
	}
	else
	{
		ss << "Connection: " << this->GetConnection() << "\r\n";
		ss << "Date: " << this->GetDate() << "\r\n";
		ss << "Keep-Alive: timeout=3, max=1000" << "\r\n\r\n";
	}
	ss << this->GetBody();
	mMessage = ss.str();
	mMessageLength = mMessage.length();
}

void HttpResponse::SetRedirectLocation(const std::string& location)
{
	mRedirectLocation = location;
}

void HttpResponse::IncrementSendIndex(size_t amount)
{
	mSendIndex += amount;
	if (mSendIndex >= mMessageLength) mIsSendDone = true;
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

size_t HttpResponse::GetSendIndex() const
{
	return mSendIndex;
}

bool HttpResponse::GetIsSendDone() const
{
	return mIsSendDone;
}

bool HttpResponse::GetHasMessage() const
{
	return mHasMessage;
}

size_t HttpResponse::GetMessageLength() const
{
	return mMessageLength;
}

const std::string& HttpResponse::GetHttpMessage(size_t size)
{
	if (mHasMessage == false)
	{
		SetHttpMessage();
		mHasMessage = true;
	}
	mSendMessage = mMessage.substr(mSendIndex, size); // substr
	std::cout << "Response: Notice: Send Percent: " << (int)(100 * mSendIndex / mMessageLength) << "%\n";
	return mSendMessage;
}

void HttpResponse::AppendBody(const std::string& str)
{
	mBody.append(str);
}

HttpResponse::~HttpResponse()
{

}


