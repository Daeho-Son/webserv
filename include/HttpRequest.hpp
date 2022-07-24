#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <algorithm>
# include <exception>
# include <string>
# include <sstream>
# include <utility>
# include <map>
# include <memory>
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
		CHUNKED,
		INVALID_TYPE
	};
	struct CgiInfo {
		size_t mCgiSentSize;
		int mPipeP2C[2];
		int mPipeC2P[2];
		int64_t mRemainedCgiMessage;
		int64_t mTotalReadSize;
	} mCgiInfo;

	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	virtual ~HttpRequest();

	bool Parse(const std::string& buf);
	std::string GetFieldByKey(const std::string& key) const;
	void GetCgiEnvVector(std::vector<std::string>& v) const;
	std::string GetMethodStringByEnum(eMethod e) const;
	void ShowHeader() const;
	void AppendResponseMessageBody(const std::string& responseMessageBody);

	// Getter
	eParseStatus GetParseStatus() const;
	eBodyType GetBodyType() const;
	eMethod GetMethod() const;
	const std::string& GetBody() const;
	ssize_t GetBodyLength();
	const std::string& GetHttpTarget() const;
	const std::string& GetResponseMessageBody() const;
	size_t GetBodyIndex() const { return mBodyIndex; }

	// Setter
	void IncrementBodyIndex(size_t size) { mBodyIndex += size; }

protected:

private:
	bool parseRequestLine();
	bool parseHeader();
	bool parseBody();
	bool parseChunked();
	bool parseContent();
private:
	eMethod mMethod;
	std::string mTarget;
	std::string mHttpVersion;
	std::string mCachedContent;
	std::string mContent;
	std::string mBufferCache;
	size_t mContentLength;
	ssize_t mBodyLength;
	eParseStatus mParseStatus;
	std::map<std::string, std::string> mHeader;
	eBodyType mBodyType;
	std::string mBody;
	std::string mResponseMessageBody;
	size_t mBodyIndex;

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
