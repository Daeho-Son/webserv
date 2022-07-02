#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <algorithm>
# include <exception>
# include <string>
# include <sstream>
# include <map>
# include <vector>

class HttpRequest
{
public:
	HttpRequest(const std::string& httpMessage);
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	virtual ~HttpRequest();
	std::string getFieldByKey(const std::string& key);
	void showAllFieldByKey();

protected:

private:
	HttpRequest();
	bool parseHttpMessageToMap(std::map<std::string, std::string> &mHttpMessageMap, const std::string& httpMessage);
	std::map<std::string, std::string> mHttpMessageMap;

	class parseException : public std::exception
	{
		const char* what() const throw()
		{
			return "Failed to make Http Request while parsing.";
		}
	};
};
#endif
