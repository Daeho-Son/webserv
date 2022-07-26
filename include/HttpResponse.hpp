#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <ctime>
# include <iostream>
# include <sstream>
# include <stdlib.h>
# include <vector>

# include "util.hpp"

namespace ft
{
	class HttpResponse
	{
	// Methods
	public:
		HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse(int statusCode, const std::string& body, const std::string& connection);
		HttpResponse(const std::string& rawData);
		virtual ~HttpResponse();
		HttpResponse& operator= (const HttpResponse& other);

		// Setters
		void SetHttpMessage();
		void SetRedirectLocation(const std::string& location);

		void IncrementSendIndex(size_t amount);

		// Getters
		const std::string& GetHttpMessage(size_t size);
		std::string GetHttpVersion() const;
		int			GetStatusCode() const;
		std::string GetStatusText() const;
		std::string GetDate() const;
		std::string GetConnection() const;
		int			GetContentLength() const;
		std::string GetContentType() const;
		std::string GetBody() const;
		size_t		GetSendIndex() const;
		bool		GetIsSendDone() const;
		bool		GetHasMessage() const;
		size_t		GetMessageLength() const;

		void 		AppendBody(const std::string& str);

	protected:

	private:

	// Variables
	private:
		std::string mHttpVersion;
		int 		mStatusCode;
		std::string mDate;
		std::string mConnection;
		std::string mContentType;
		std::string mRedirectLocation;
		std::string mBody;
		std::string mMessage;
		std::string	mSendMessage;
		size_t		mMessageLength;
		size_t		mSendIndex;
		bool		mIsSendDone;
		bool		mHasMessage;
	};
}

#endif
