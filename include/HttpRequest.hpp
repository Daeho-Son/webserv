#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <vector>
# include <sstream>
# include <unordered_map>
# include <algorithm>

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
	void parseHttpMessageToMap(std::unordered_map<std::string, std::string> &mHttpMessageMap, const std::string& httpMessage);
	std::unordered_map<std::string, std::string> mHttpMessageMap;
};
#endif
