#include "util.hpp"

std::string GetStatusTextByStatusCode(int statusCode)
{
	std::string statusText;
	switch (statusCode)
	{
	case 100:
		statusText = "Continue";
		break;
	case 101:
		statusText = "Switching Protocols";
		break;
	case 103:
		statusText = "Early Hints";
		break;
	case 200:
		statusText = "OK";
		break;
	case 201:
		statusText = "Created";
		break;
	case 202:
		statusText = "Accepted";
		break;
	case 203:
		statusText = "Non-Authoritative Information";
		break;
	case 204:
		statusText = "No Content";
		break;
	case 205:
		statusText = "Reset Content";
		break;
	case 206:
		statusText = "Partial Content";
		break;
	case 300:
		statusText = "Multiple Choices";
		break;
	case 301:
		statusText = "Moved Permanently";
		break;
	case 302:
		statusText = "Found";
		break;
	case 303:
		statusText = "See Other";
		break;
	case 304:
		statusText = "Not Modified";
		break;
	case 307:
		statusText = "Temporary Redirect";
		break;
	case 308:
		statusText = "Permanent Redirect";
		break;
	case 400:
		statusText = "Bad Request";
		break;
	case 401:
		statusText = "Unauthorized";
		break;
	case 402:
		statusText = "Payment Required";
		break;
	case 403:
		statusText = "Forbidden";
		break;
	case 404:
		statusText = "Not Found";
		break;
	case 405:
		statusText = "Method Not Allowed";
		break;
	case 406:
		statusText = "Not Acceptable";
		break;
	case 407:
		statusText = "Proxy Authentication Required";
		break;
	case 408:
		statusText = "Request Timeout";
		break;
	case 409:
		statusText = "Conflict";
		break;
	case 410:
		statusText = "Gone";
		break;
	case 411:
		statusText = "Length Required";
		break;
	case 412:
		statusText = "Precondition Failed";
		break;
	case 413:
		statusText = "Request Too Large";
		break;
	case 414:
		statusText = "Request-URI Too Long";
		break;
	case 415:
		statusText = "Unsupported Media Type";
		break;
	case 416:
		statusText = "Range Not Satisfiable";
		break;
	case 417:
		statusText = "Expectation Failed";
		break;
	case 500:
		statusText = "Internal Server Error";
		break;
	case 501:
		statusText = "Not Implemented";
		break;
	case 502:
		statusText = "Bad Gateway";
		break;
	case 503:
		statusText = "Service Unavailable";
		break;
	case 504:
		statusText = "Gateway Timeout";
		break;
	case 505:
		statusText = "HTTP Version Not Supported";
		break;
	case 511:
		statusText = "Network Authentication Required";
		break;
	default:
		std::cerr << "[ERROR] Bad Status Code.\n";
		statusText = "Not Implemented";
		break;
	}

	return statusText;
}

std::string GetWeekdayTextByNum(int w)
{
	switch (w)
	{
	case 0:
		return "Mon";
	case 1:
		return "Tue";
	case 2:
		return "Wed";
	case 3:
		return "Thu";
	case 4:
		return "Fri";
	case 5:
		return "Sat";
	case 6:
		return "Sun";
	default:
		return "ERR";
	}
}

std::string GetMonthTextByNum(int m)
{
	switch (m)
	{
	case 0:
		return "Jan";
	case 1:
		return "Feb";
	case 2:
		return "Mar";
	case 3:
		return "Apr";
	case 4:
		return "May";
	case 5:
		return "Jun";
	case 6:
		return "Jul";
	case 7:
		return "Aug";
	case 8:
		return "Sep";
	case 9:
		return "Oct";
	case 10:
		return "Nov";
	case 11:
		return "Dec";
	default:
		return "ERR";
	}
}

std::string GetHttpFormDate()
{
	std::stringstream ss;
	std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);
	ss	<< GetWeekdayTextByNum(now->tm_wday) << ", "
		<< now->tm_mday << " "
		<< GetMonthTextByNum(now->tm_mon) << " "
		<< now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << " "
		<< "GMT";
	return ss.str();
}
