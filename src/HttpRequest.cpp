#include "HttpRequest.hpp"
#include <iostream>

static size_t hexToInt(const std::string& hex);
static inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v");
static inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v");
static inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");
static HttpRequest::eMethod GetMethodByString(const std::string& str);

HttpRequest::HttpRequest()
{
	this->mParseStatus = REQUEST_LINE;
}

HttpRequest::HttpRequest(const HttpRequest& other)
{
	*this = other;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	this->mParseStatus = other.mParseStatus;
	this->mBufferCache = other.mBufferCache;
	this->mHeader = other.mHeader;
	return *this;
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::Parse(std::string& buf)
{
	// Load cached data
	buf = mBufferCache + buf;

	if (mParseStatus == DONE) return false;
	if (mParseStatus == REQUEST_LINE) parseRequestLine(buf);
	if (mParseStatus == HEADER) parseHeader(buf);
	if (mParseStatus == BODY) parseBody(buf);
	// Cache remained buffer
	if (mParseStatus != DONE) mBufferCache = buf;
	return true;
}

bool HttpRequest::parseRequestLine(std::string& buf)
{
	if (mParseStatus != REQUEST_LINE) throw InvalidParseStatus();
	size_t newlinePos = buf.find("\r\n");
	if (newlinePos == buf.npos) return false;// request line이 완성되지 못한채 들어왔을 때

	std::string requestLine = buf.substr(0, newlinePos);
	size_t firstSpace = requestLine.find(' ');
	size_t lastSpace = requestLine.rfind(' ');
	if (firstSpace == lastSpace)
	{
		std::cerr << "Request: Error: InvalidRequestLine: " << requestLine << std::endl;
		throw InvalidRequestLine();
	}

	mMethod = GetMethodByString(requestLine.substr(0, firstSpace));
	mTarget = requestLine.substr(firstSpace+1, lastSpace-firstSpace-1);
	mHttpVersion = requestLine.substr(lastSpace+1);

	// Caching...
	buf.erase(0, newlinePos+2);

	mParseStatus = HEADER;
	return true;
}

// If parsing failed, return false
bool HttpRequest::parseHeader(std::string& buf)
{
	if (mParseStatus != HEADER) throw InvalidParseStatus();
	size_t delimPos = buf.find("\r\n\r\n");
	if (delimPos == buf.npos) return false;

	std::stringstream ss(buf.substr(0, delimPos));

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
	else if ((it = mHeader.find("Content-Length")) != mHeader.end() && (*it).second != "0")
	{
		this->mParseStatus = BODY;
		mBodyType = CONTENT;
		mContentLength = strtod((*it).second.c_str(), NULL);
		if (mContentLength > 0) throw BadContentLength();
	}
	else
	{
		this->mParseStatus = DONE;
	}

	buf.erase(0, delimPos+4);
	return true;
}

bool HttpRequest::parseBody(std::string& buf)
{
	if (mParseStatus != BODY) throw InvalidParseStatus();

	if (mBodyType == CHUNKED) parseChunked(buf);
	else if (mBodyType == CONTENT) parseContent(buf);
	else throw InvalidParseStatus();

	return true;
}

bool HttpRequest::parseChunked(std::string& buf)
{
	while (true)
	{
		size_t chunkedLengthPos = buf.find("\r\n");
		if (chunkedLengthPos == buf.npos) return false;
		size_t chunkedLength = hexToInt(buf.substr(0, chunkedLengthPos));
		if (chunkedLength == 0)
		{
			if (buf.find("\r\n\r\n") == buf.npos)
				return false;
			mParseStatus = DONE;
			break;
		}
		if ((buf.length() - chunkedLengthPos) >= chunkedLength + 4) // 청크 세트가 전부 들어온 경우
		{
			mBody.append(buf.substr(chunkedLengthPos+2, chunkedLength));
			buf.erase(0, chunkedLengthPos+chunkedLength+4);
		}
		else // 청크 세트가 다 들어오지 않았다면 파싱을 하지 않는다.
		{
			return false;
		}
	}
	return true;
}

bool HttpRequest::parseContent(std::string& buf)
{
	if (buf.length() >= mContentLength+4)
	{
		mBody = buf.substr(0, mContentLength);
		mParseStatus = DONE;
		return true;
	}
	return false;
}

std::string HttpRequest::GetFieldByKey(const std::string &key) const // TODO: ref
{
	std::map<std::string, std::string>::const_iterator fieldIt = this->mHeader.find(key);
	if (fieldIt == this->mHeader.end())
		return ("null");
	return (*fieldIt).second;
}

HttpRequest::eParseStatus HttpRequest::GetParseStatus() const
{
	return mParseStatus;
}

HttpRequest::eMethod HttpRequest::GetMethod() const
{
	return mMethod;
}

const std::string& HttpRequest::GetBody() const
{
	return mBody;
}

void HttpRequest::ShowHeader() const
{
	std::map<std::string, std::string>::const_iterator it = mHeader.begin();
	for (; it != mHeader.end(); ++it) {
		std::cout << (*it).first << ": " << (*it).second << std::endl;
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

const std::string& HttpRequest::GetHttpTarget() const
{
	return mTarget;
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