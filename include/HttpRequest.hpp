#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <algorithm>
# include <exception>
# include <string>
# include <sstream>
# include <utility>
# include <map>
# include <vector>

class HttpRequest
{
public:
	enum eMethod{
		GET,
		POST,
		PUT,
		DELETE,
		HEAD,
		NOT_VALID
	};
	enum eParseStatus{
		REQUEST_LINE,
		HEADER,
		BODY,
		DONE
	};
	enum eBodyType{
		CONTENT,
		CHUNKED
	};

	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	virtual ~HttpRequest();

	bool Parse(std::string& buf);
	std::string GetFieldByKey(const std::string& key) const;
	void ShowHeader() const;
	eParseStatus GetParseStatus() const;
	eMethod GetMethod() const;
	const std::string& GetBody() const;
	std::string GetMethodStringByEnum(eMethod e) const;
	const std::string& GetHttpTarget() const;

protected:

private:
	bool parseRequestLine(std::string& buf);
	bool parseHeader(std::string& buf);
	bool parseBody(std::string& buf);
	bool parseChunked(std::string& buf);
	bool parseContent(std::string& buf);
private:
	eMethod mMethod;
	std::string mTarget;
	std::string mHttpVersion;
	std::string mCachedContent;
	std::string mContent;
	std::string mBufferCache;
	size_t mContentLength;
	eParseStatus mParseStatus;
	std::map<std::string, std::string> mHeader;
	eBodyType mBodyType;
	std::string mBody;

	// exceptions
	class InvalidParseStatus : std::exception
	{
		const char* what() const throw() { return "Request: Exception: Invalid Parse Status"; }
	};

	class InvalidRequestLine : std::exception
	{
		const char* what() const throw() { return "Request: Exception: Received invalid request line"; }
	};

	class InvalidHeader: std::exception
	{
		const char* what() const throw() { return "Request: Exception: Invalid Header"; }
	};

	class BadContentLength: std::exception
	{
		const char* what() const throw() { return "Request: Exception: Bad Content Length"; }
	};
};
#endif
