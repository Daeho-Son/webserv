#include "HttpRequest.hpp"
#include <iostream>

static size_t hexToInt(const std::string& hex);
static inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v");
static inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v");
static inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");
static HttpRequest::eMethod GetMethodByString(const std::string& str);

HttpRequest::HttpRequest()
	:	mBodyLength(0),
		mResponseMessageBody(""),
		mBodyIndex(0)
{
	this->mParseStatus = REQUEST_LINE;
}

HttpRequest::HttpRequest(const HttpRequest& other)
	:	mBodyLength(0),
		mResponseMessageBody(""),
		mBodyIndex(0)
{
	*this = other;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	this->mParseStatus = other.mParseStatus;
	this->mBufferCache = other.mBufferCache;
	this->mHeader = other.mHeader;
	this->mResponseMessageBody = other.mResponseMessageBody;
	this->mBodyIndex = other.mBodyIndex;
	return *this;
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::Parse(const std::string& buf)
{
	// Load cached data
	mBufferCache.append(buf);

	if (mParseStatus == DONE) return false;
	if (mParseStatus == REQUEST_LINE) parseRequestLine();
	if (mParseStatus == HEADER) parseHeader();
	if (mParseStatus == BODY) parseBody();
	return true;
}

bool HttpRequest::parseRequestLine()
{
	if (mParseStatus != REQUEST_LINE) throw InvalidParseStatus();
	size_t newlinePos = mBufferCache.find("\r\n");
	if (newlinePos == mBufferCache.npos) return false;// request line이 완성되지 못한채 들어왔을 때

	std::string requestLine = mBufferCache.substr(0, newlinePos);
	size_t firstSpace = requestLine.find(' ');
	size_t lastSpace = requestLine.rfind(' ');
	if (firstSpace == lastSpace)
	{
		std::cerr << "Request: Error: InvalidRequestLine: " << requestLine << "\n";
		throw InvalidRequestLine();
	}

	mMethod = GetMethodByString(requestLine.substr(0, firstSpace));
	mTarget = requestLine.substr(firstSpace+1, lastSpace-firstSpace-1);
	mHttpVersion = requestLine.substr(lastSpace+1);
	// Caching...
	mBufferCache.erase(0, newlinePos+2);

	mParseStatus = HEADER;
	return true;
}

// If parsing failed, return false
bool HttpRequest::parseHeader()
{
	if (mParseStatus != HEADER) throw InvalidParseStatus();
	size_t delimPos = mBufferCache.find("\r\n\r\n");
	if (delimPos == mBufferCache.npos) return false;

	std::stringstream ss(mBufferCache.substr(0, delimPos));

	std::string line;
	while (getline(ss, line))
	{
		if (line.length() == 1) continue; // if line is "\r", continue
		size_t colonPos = line.find(":");
		if (colonPos == line.npos || colonPos == line.length()-2)
			continue;
		std::string value = line.substr(colonPos+1);
		mHeader.insert(make_pair(line.substr(0, colonPos), trim(value)));
	}
	std::map<std::string, std::string>::iterator it;
	if ((it = mHeader.find("Transfer-Encoding")) != mHeader.end() && (*it).second == "chunked")
	{
		this->mParseStatus = BODY;
		mBodyType = CHUNKED;
	}
	else if ((it = mHeader.find("Content-Length")) != mHeader.end())
	{
		this->mParseStatus = BODY;
		mBodyType = CONTENT;
		mContentLength = strtod((*it).second.c_str(), NULL);
		if (mContentLength < 0) throw BadContentLength();
	}
	else
	{
		mBodyType = INVALID_TYPE;
		this->mParseStatus = DONE;
	}

	mBufferCache.erase(0, delimPos+4);
	return true;
}

bool HttpRequest::parseBody()
{
	if (mParseStatus != BODY) throw InvalidParseStatus();

	if (mBodyType == CHUNKED) parseChunked();
	else if (mBodyType == CONTENT) parseContent();
	else if (mBodyType == INVALID_TYPE) mParseStatus = DONE;
	else throw InvalidParseStatus();

	return true;
}

bool HttpRequest::parseChunked()
{
	while (true)
	{
		size_t chunkedLengthPos = mBufferCache.find("\r\n");
		if (chunkedLengthPos == mBufferCache.npos) return false;
		size_t chunkedLength = hexToInt(mBufferCache.substr(0, chunkedLengthPos));
		if (chunkedLength == 0)
		{
			if (mBufferCache.find("\r\n\r\n") == mBufferCache.npos)
				return false;
			mParseStatus = DONE;
			break;
		}
		if ((mBufferCache.length() - chunkedLengthPos) >= chunkedLength + 4) // 청크 세트가 전부 들어온 경우
		{
			mBody.append(mBufferCache.substr(chunkedLengthPos+2, chunkedLength));
			mBufferCache.erase(0, chunkedLengthPos+chunkedLength+4);
			mBodyLength = mBody.length(); // TEST
		}
		else // 청크 세트가 다 들어오지 않았다면 파싱을 하지 않는다.
		{
			return false;
		}
	}
	return true;
}

bool HttpRequest::parseContent()
{
	if (mBufferCache.length() >= mContentLength)
	{
		mBody = mBufferCache.substr(0, mContentLength);
		mParseStatus = DONE;
		return true;
	}
	return false;
}

std::string HttpRequest::GetFieldByKey(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator fieldIt = this->mHeader.find(key);
	if (fieldIt == this->mHeader.end())
		return ("null");
	return (*fieldIt).second;
}

void HttpRequest::GetCgiEnvVector(std::vector<std::string>& v) const
{
	v.push_back(std::string("REQUEST_METHOD=").append(GetMethodStringByEnum(mMethod)));
	v.push_back(std::string("SERVER_PROTOCOL=HTTP/1.1"));
	v.push_back(std::string("PATH_INFO=").append(mTarget));
	if (GetFieldByKey("X-Secret-Header-For-Test") == "1")
	{
		v.push_back(std::string("HTTP_X_SECRET_HEADER_FOR_TEST=1"));
		v.push_back("");
	}
	else
	{
		v.push_back("");
		v.push_back("");
	}
}

std::string HttpRequest::GetMethodStringByEnum(HttpRequest::eMethod e) const
{
	if (e == HttpRequest::GET)
		return std::string("GET");
	if (e == HttpRequest::POST)
		return std::string("POST");
	if (e == HttpRequest::PUT)
		return std::string("PUT");
	if (e == HttpRequest::DELETE)
		return std::string("DELETE");
	if (e == HttpRequest::HEAD)
		return std::string("HEAD");
	return std::string("NOT_VALID");
}

void HttpRequest::ShowHeader() const
{
	std::map<std::string, std::string>::const_iterator it = mHeader.begin();
	for (; it != mHeader.end(); ++it) {
		std::cout << (*it).first << ": " << (*it).second << "\n";
	}
}

void HttpRequest::AppendResponseMessageBody(const std::string& newResponseMessage)
{
	mResponseMessageBody.append(newResponseMessage);
}

HttpRequest::eParseStatus HttpRequest::GetParseStatus() const
{
	return mParseStatus;
}

HttpRequest::eBodyType HttpRequest::GetBodyType() const
{
	return mBodyType;
}

HttpRequest::eMethod HttpRequest::GetMethod() const
{
	return mMethod;
}

const std::string& HttpRequest::GetBody() const
{
	return mBody;
}

ssize_t HttpRequest::GetBodyLength()
{
	if (mBodyLength == -1) mBodyLength = mBody.length();
	return mBodyLength;
}

const std::string& HttpRequest::GetHttpTarget() const
{
	return mTarget;
}

const std::string& HttpRequest::GetResponseMessageBody() const
{
	return mResponseMessageBody;
}

static size_t hexToInt(const std::string& hex)
{
    size_t result;
    std::istringstream iss(hex);
    iss >> std::hex >> result;

    return result;
}

// trim from left
static inline std::string& ltrim(std::string& s, const char* t)
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}
// trim from right
static inline std::string& rtrim(std::string& s, const char* t)
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}
// trim from left & right
static inline std::string& trim(std::string& s, const char* t)
{
	return ltrim(rtrim(s, t), t);
}

static HttpRequest::eMethod GetMethodByString(const std::string& str)
{
	if (str == "GET")
		return HttpRequest::GET;
	if (str == "POST")
		return HttpRequest::POST;
	if (str == "PUT")
		return HttpRequest::PUT;
	if (str == "DELETE")
		return HttpRequest::DELETE;
	if (str == "HEAD")
		return HttpRequest::HEAD;
	else
		return HttpRequest::NOT_VALID;
}
