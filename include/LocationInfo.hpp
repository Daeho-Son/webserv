#ifndef CONFINFO_HPP
# define CONFINFO_HPP

# include <iostream>
# include <string>
# include <vector>

struct LocationInfo
{
public:
	void SetLocation(const std::string& location);
	void SetAcceptedMethods(const std::vector<std::string>& acceptMethod);
	void SetRoot(const std::string& root);
	void SetDefaultFile(const std::string& defaultFile);
	void SetDefaultErrorfile(const std::string& defaultErrorFile);
	void SetClientBodySize(int clientBodySize);
	void SetCgi(const std::vector<std::string>& cgi);

	std::string GetLocation() const;
	std::vector<std::string> GetAcceptedMethods() const;
	const std::string& GetRoot() const;
	const std::string& GetDefaultFile() const;
	const std::string& GetDefaultErrorFile() const;
	int GetClientBodySize() const;
	std::vector<std::string> GetCgi() const;

private:
	std::vector<std::string> mAcceptedMethods;
	std::string mLocation;
	std::string mRoot;
	std::string mDefaultFile;
	std::string mDefaultErrorFile;
	int mClientBodySize;
	std::vector<std::string> mCgi;
};

#endif