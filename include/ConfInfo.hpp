#ifndef CONFINFO_HPP
# define CONFINFO_HPP

# include <iostream>
# include <string>
# include <vector>

class ConfInfo
{
public:
	void SetLocation(const std::string& location);
	void SetAcceptedMethods(const std::vector<std::string>& acceptMethod);
	void SetRoot(const std::string& root);
	void SetDefaultFile(const std::string& defaultFile);
	std::string GetLocation() const;
	std::vector<std::string> GetAcceptedMethods() const;
	std::string GetRoot() const;
	std::string GetDefaultFile() const;

private:
	std::string mLocation;
	std::vector<std::string> mAcceptedMethods;
	std::string mRoot;
	std::string mDefaultFile;
};

#endif