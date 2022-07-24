#ifndef HTTP_SERVER_HPP
# define HTTP_SERVER_HPP

# include <exception>
# include <fcntl.h>
# include <fstream>
# include <iostream>
# include <netinet/in.h> // sockaddr_in
# include <string>
# include <string.h>
# include <sys/event.h> // kqueue()
# include <sys/socket.h>
# include <sys/stat.h>
# include <unistd.h> // close(), execve()
# include <utility>
# include <map>
# include <set>
# include <vector>
# include <cstdio>
# include <dirent.h>

# include "Conf.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Client.hpp"

# define RED "\033[1;31m"
# define GRN "\033[1;32m"
# define BLU "\033[1;33m"
# define NM "\033[0m"
# define CONF_DEFAULT_TARGET "/index.html"
# define PIPE_READ_FD 0
# define PIPE_WRITE_FD 1
# define STDIN 0
# define STDOUT 1
# define MAX_ENVP_SIZE 5
# define TIMEOUT_LIMIT 3

using namespace ft;

class HttpServer
{
	enum {
		MAX_READ_SIZE = 65527,
		MAX_WRITE_SIZE = 65527
	};
public:
	HttpServer(Conf& conf);
	virtual ~HttpServer();

	int Run(); // 서버를 실행합니다.
	int Stop(); // 서버를 종료합니다.

private:
	HttpServer();
	HttpServer(const HttpServer& other);
	HttpServer& operator=(const HttpServer& other);

	int addEvent(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
	HttpResponse GetResponseByRequest(HttpRequest& request);
	bool IsServerSocket(const std::vector<int>& serverSockets, uintptr_t ident) const;
	bool IsCGIRequest(const HttpRequest& request, int port) const;
	bool GetDirectoryList(const std::string& targetDit, int port, std::string& result) const;
	bool IsTimeoutSocket(const Client& client);
	bool IsDeadSocket(const Client& client);
	bool DisconnectClient(int clientSocket, std::vector<struct kevent>& changeList);
	bool ConnectClient(int newClientSocket, int ServerSocket, std::vector<struct kevent> &changeList);
	bool UpdateTimeout(int clientSocket);
	int OpenFile(const std::string &target, int fileMode);

	inline bool IsFileFd(int fd) { return mFileFds.find(fd) != mFileFds.end(); }
	
private:
	Conf mServerConf;
	std::map<int, Client> mClients;
	std::map<int, int> mPipeFds;
	std::map<int, int> mFileFds;
	std::map<uintptr_t, HttpRequest> mCachedRequests;
};

#endif
