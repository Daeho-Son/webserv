#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>

class HttpRequest
{
	enum eHttpMethod
	{
		GET,
		POST,
		PUT,
		DELETE
	};
public:
	HttpRequest();
	HttpRequest(const std::string& str);
	HttpRequest(const HttpRequest& httprequest);
	HttpRequest& operator=(const HttpRequest& httprequest);
	~HttpRequest();

	eHttpMethod GetHttpMethod();

protected:
	virtual ~HttpRequest();

private:
	eHttpMethod mHttpMethod;
};

#endif
