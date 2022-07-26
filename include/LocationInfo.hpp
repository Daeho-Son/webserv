#ifndef CONFINFO_HPP
# define CONFINFO_HPP

# include <iostream>
# include <string>
# include <vector>

struct LocationInfo
{
public:
	LocationInfo();

public:
	void SetLocation(const std::string& location);
	void SetAcceptedMethods(const std::vector<std::string>& acceptMethod);
	void SetRoot(const std::string& root);
	void SetDefaultFile(const std::string& defaultFile);
	void SetDefaultErrorfile(const std::string& defaultErrorFile);
	void SetClientBodySize(size_t clientBodySize);
	void SetCgi(const std::vector<std::string>& cgi);
	void SetAutoIndex(const std::string& autoIndex);
	void SetRedirect(const std::string& redirectLocation);

	const std::string& GetLocation() const;
	const std::vector<std::string>& GetAcceptedMethods() const;
	const std::string& GetRoot() const;
	const std::string& GetDefaultFile() const;
	const std::string& GetDefaultErrorFile() const;
	const std::string& GetRedirectLocation() const;
	size_t GetClientBodySize() const;
	const std::vector<std::string>& GetCgi() const;
	bool IsRedirected() const;
	bool IsAutoIndex() const;

private:
	std::vector<std::string> mAcceptedMethods;
	std::string mLocation;
	std::string mRoot;
	std::string mDefaultFile;
	std::string mDefaultErrorFile;
	bool mbIsRedirected;
	std::string mRedirectLocation;
	size_t mClientBodySize;
	std::vector<std::string> mCgi;
	bool mAutoIndex;
};

#endif