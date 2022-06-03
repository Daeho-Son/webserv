#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <ctime>
# include <iostream>
# include <sstream>
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
		HttpResponse(int statusCode, const std::string& body);
		virtual ~HttpResponse();
		HttpResponse& operator= (const HttpResponse& other);

		std::string GetHttpMessage() const;

		// Getters
		std::string GetHttpVersion() const;
		int			GetStatusCode() const;
		std::string GetStatusText() const;
		std::string GetDate() const;
		std::string GetConnection() const;
		int			GetContentLength() const;
		std::string GetContentType() const;
		std::string GetBody() const;

	protected:

	private:

	// Variables
	private:
		std::string mHttpVersion;
		int 		mStatusCode;
		std::string mDate;
		std::string mConnection;
		std::string mContentType;
		std::string mBody;
	};
}

#endif