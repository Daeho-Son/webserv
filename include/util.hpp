#ifndef FT_UTIL_HPP
# define FT_UTIL_HPP

# include <iostream>
# include <string>
# include <sstream>

	std::string GetStatusTextByStatusCode(int statusCode);
	std::string GetWeekdayTextByNum(int w);
	std::string GetMonthTextByNum(int m);
	std::string GetHttpFormDate();


#endif
