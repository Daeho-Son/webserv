#include "HttpServer.hpp"

HttpServer::HttpServer(Conf& conf)
{
	this->mServerConf = conf;
}

int HttpServer::Run() // 서버를 실행합니다. Init()이 실행된 후여야 합니다.
{
	// make server sockets
	std::vector<int> serverSockets;
	std::vector<ServerInfo> serverInfos = this->mServerConf.GetServerInfos();
	// <server socket, port index>
	std::map<int, int> getServerIndexBySocket;
	for (size_t i=0; i<serverInfos.size(); ++i)
	{
		int serverPort = serverInfos[i].GetPort();
		serverSockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
		int& serverSocket = serverSockets[serverSockets.size()-1];
		getServerIndexBySocket[serverSocket] = i;
		if (serverSocket < 0)
		{
			assert(serverSocket >= 0);
			return 1;
		}

		// bind socket
		sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof(sockaddr_in));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddress.sin_port = htons(serverPort); // TODO: use Conf

		int bsize = 0;
		int rn;
		rn = sizeof(int);

		// 현재 전송 소켓 버퍼의 크기를 가져온다.
    	getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, &bsize, (socklen_t *)&rn);
		bsize *= 2;
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &bsize, (socklen_t)rn);

		if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		{
			std::cerr << RED << "Server: Error: server socket bind() failed.\n" << NM;
			close(serverSocket);
			return 1;
		}

		if (listen(serverSocket, this->mServerConf.GetListenSize()) < 0) // TODO: use Conf
		{
			std::cerr << RED << "Server: Error: server socket listen() failed.\n" << NM;
			close(serverSocket);
			return 1;
		}
	}
	std::cout << GRN << "Server: Notice: All server sockets opened\n" << NM;
	std::cout << GRN << "Server: Notice: Server is running.\n" << NM;

	// Prepare for multiflexing I/O
	int kq = kqueue();
	if (kq < 0)
	{
		assert(kq >= 0);
		std::cerr << RED << "Server: Error: kqueue() failed.\n" << NM;
		for (size_t i=0; i<serverSockets.size(); ++i)
		{
			close(serverSockets[i]);
		}

		return 1;
	}

	// <client socket, server socket>
	std::map<int, int> getServerSocketByClientSocket;
	std::set<int> clients;
	std::map<uintptr_t, HttpRequest> cachedRequests;
	std::map<int, HttpResponse> responses;
	std::vector<struct kevent> changeList;
	for (size_t i=0; i<serverSockets.size(); ++i)
	{
		this->addEvent(changeList, serverSockets[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}

	struct kevent eventList[this->mServerConf.GetKeventsSize()];
	std::cout << GRN << "Server: Notice: Server main loop started.\n" << NM;
	// Main loop
	while (1)
	{
		int newEventSize = kevent(kq, &changeList[0], changeList.size(), eventList, this->mServerConf.GetKeventsSize(), NULL);
		changeList.clear();

		for (int i=0; i<newEventSize; ++i)
		{
			struct kevent* newEvent = &eventList[i];
			
			// 읽기 요청 이벤트
			if (newEvent->filter == EVFILT_READ)
			{
				// CGI Read
				std::map<int, int>::iterator it;
				if ((it = mPipeFds.find(newEvent->ident)) != mPipeFds.end())
				{
					HttpRequest& httpRequest = cachedRequests[(*it).second];
					char readBuffer[MAX_READ_SIZE];
					memset(readBuffer, 0, MAX_READ_SIZE);
					int64_t readSize = 0;
					while ((readSize = read(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], readBuffer, MAX_READ_SIZE - 1)) > 0)
					{
						httpRequest.mCgiInfo.mTotalReadSize += readSize;
						httpRequest.AppendResponseMessageBody(readBuffer);
						memset(readBuffer, 0, MAX_READ_SIZE);
					}
					// 다 읽었으면 Response 만들어서 보낼 준비
					
					if (readSize == 0)
					{
						responses.insert(std::make_pair((*it).second, HttpResponse(httpRequest.GetResponseMessageBody())));
						cachedRequests.erase((*it).second);
						addEvent(changeList, newEvent->ident, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
						addEvent(changeList, (*it).second, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						mPipeFds.erase(newEvent->ident);
						close(newEvent->ident);
					}
					if (readSize == -1)
					{
						// std::cerr << "Server: Error: Read Failed\n";
					}
				}
				// 새로운 Client
				else if (IsServerSocket(serverSockets, newEvent->ident))
				{
					int newClientSocket;
					if ((newClientSocket = accept(newEvent->ident, NULL, NULL)) == -1)
					{
						std::cerr << "Server: Error: accept() failed.\n";
						continue;
					}
					std::cout << "Server: Notice: new client " << newClientSocket << " added.\n";
					getServerSocketByClientSocket[newClientSocket] = newEvent->ident;
					clients.insert(newClientSocket);
					fcntl(newClientSocket, F_SETFL, O_NONBLOCK);
					this->addEvent(changeList, newClientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					this->addEvent(changeList, newClientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
				}
				// 기존 Client
				else
				{
					std::cout << "Server: Notice: The client " << newEvent->ident << " sent a message.\n";
					std::set<int>::iterator clientIt = clients.find(newEvent->ident);
					if (clientIt == clients.end()) {
						std::cerr << "Server: Warning: Request from Invalid client\n";
						continue;
					}
					size_t serverIndex = getServerIndexBySocket[getServerSocketByClientSocket[newEvent->ident]];
					ServerInfo serverInfo = mServerConf.GetServerInfos()[serverIndex];
					int port = serverInfo.GetPort();

					/*
					STEP 1. fd에 있는 모든 데이터를 읽는다.
					*/
					char readBuffer[MAX_READ_SIZE];
					memset(readBuffer, 0, MAX_READ_SIZE);
					std::string buffer;
					int readSize;
					while ((readSize = recv(*clientIt, readBuffer, MAX_READ_SIZE-1, MSG_DONTWAIT)) > 0)
					{
						buffer.append(readBuffer);
						memset(readBuffer, 0, MAX_READ_SIZE);
					}
					if (readSize == 0)
					{
						close(newEvent->ident);
						std::cout << "Server: Notice: client " << newEvent->ident << " left.\n";
						clients.erase(newEvent->ident);
						addEvent(changeList, newEvent->ident, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
						continue;
					}
					/*
					STEP 2: 파싱한다. 없으면 생성 후, 파싱.
					*/
					if (cachedRequests.find(newEvent->ident) == cachedRequests.end())
					{
						cachedRequests.insert(std::make_pair(newEvent->ident, HttpRequest()));
					}
					try {
						cachedRequests[newEvent->ident].Parse(buffer);
					}
					catch(std::exception& e)
					{
						std::cerr << "Server: Error: Request Parse Error ==> " << e.what() << "\n";
					}
					catch(...)
					{
						// TODO: 이대로 가나?
						std::cerr << "Server: Error: Request Parse Error\n";
					}
					/*
					STEP 3: HTTP Request가 적절히 변환됐다면 올바른 Response를 구성해서 저장한다.
					*/
					if (cachedRequests[newEvent->ident].GetParseStatus() != HttpRequest::DONE)
						continue;
					int statusCode = 417;
					std::string messageBody = "";
				    HttpRequest& httpRequest = cachedRequests[newEvent->ident];
					const HttpRequest::eMethod httpMethod = httpRequest.GetMethod();
					if (httpRequest.GetBody().length() > mServerConf.GetClientBodySize(httpRequest.GetHttpTarget(), port))
					{
						statusCode = 413;
						messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port);
					}
					else if (httpRequest.GetMethod() == HttpRequest::NOT_VALID)
					{
						statusCode = 400;
						messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port);
					}
					else if (this->mServerConf.IsValidHttpMethod(httpRequest.GetHttpTarget(), port, httpRequest.GetMethodStringByEnum(httpMethod)) == false)
					{
						statusCode = 405;
						messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port);
					}
					// CGI Process
					else if (IsCGIRequest(httpRequest, port))
					{
						// httpRequest.ShowHeader();
						// 환경변수 세팅
						char* const argv[] = {
							(char*)"cgi_tester",
							(char*)0
						};
						char* envp[] = {
							(char*)"REQUEST_METHOD=GET",
							(char*)"SERVER_PROTOCOL=HTTP/1.1",
							(char*)"PATH_INFO=\"/index.test\"",
							(char*)0
						};
						char* envp2[] = {
							(char*)"REQUEST_METHOD=GET",
							(char*)"SERVER_PROTOCOL=HTTP/1.1",
							(char*)"PATH_INFO=\"/index.test\"",
							(char*)"HTTP_X_SECRET_HEADER_FOR_TEST=1",
							(char*)0
						};

						// 파이프 세팅
						if (pipe(httpRequest.mCgiInfo.mPipeP2C) < 0 || pipe(httpRequest.mCgiInfo.mPipeC2P) < 0)
						{
							std::cerr << "Server: Error: pipe() failed.\n";
							continue;
						}
						httpRequest.mCgiInfo.mCgiSentSize = 0;
						httpRequest.mCgiInfo.mRemainedCgiMessage = httpRequest.GetBodyLength();
						httpRequest.mCgiInfo.mTotalReadSize = 0;
						fcntl(httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD], F_SETFL, O_NONBLOCK);
						fcntl(httpRequest.mCgiInfo.mPipeP2C[PIPE_READ_FD], F_SETFL, O_NONBLOCK);
						fcntl(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], F_SETFL, O_NONBLOCK);

						// fork와 CGI 프로세스 실행
						pid_t pid = fork();
						if (pid < 0)
						{
							std::cerr << "Server: Error: fork() failed.\n";
							continue;
						}

						if (pid > 0) // 부모 프로세스
						{
							mPipeFds.insert(std::make_pair(httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD], newEvent->ident));
							mPipeFds.insert(std::make_pair(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], newEvent->ident));
							close(httpRequest.mCgiInfo.mPipeP2C[PIPE_READ_FD]); // 부모는 p2c 파이프에서 쓰기만 한다.
							close(httpRequest.mCgiInfo.mPipeC2P[PIPE_WRITE_FD]); // 부모는 c2p 파이프에서 읽기만 한다.
							addEvent(changeList, httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
							addEvent(changeList, httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							continue;
						}
						else // 자식 프로세스 (= CGI 프로세스)
						{
							close(httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD]); // 자식은 p2c 파이프에 읽기만 한다.
							close(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD]); // 자식은 c2p 파이프에 쓰기만 한다.
							dup2(httpRequest.mCgiInfo.mPipeP2C[PIPE_READ_FD], STDIN);
							dup2(httpRequest.mCgiInfo.mPipeC2P[PIPE_WRITE_FD], STDOUT);
							if (httpRequest.GetFieldByKey("X-Secret-Header-For-Test") == "1")
								execve("./html/cgi-bin/cgi_tester", argv, envp2);
							else
								execve("./html/cgi-bin/cgi_tester", argv, envp);
						}
					}
					else if (httpMethod == HttpRequest::GET)
					{
						// Check the target is directory or not.
						std::string rootedTarget = mServerConf.GetRootedLocation(httpRequest.GetHttpTarget(), port);
						struct stat myStat;
						bool isDirectory = false;
						bool isValid = (stat(rootedTarget.c_str(), &myStat) == 0);
						if (isValid)
						{
							isDirectory = (myStat.st_mode & S_IFDIR) != 0;
							// 폴더면 디폴트 페이지 받아오고
								// 폴더인데 루트 폴더가 아닌 경우 404
							if (isDirectory && mServerConf.IsAutoIndex(httpRequest.GetHttpTarget(), port))
							{
								statusCode = 200;
								bool success = GetDirectoryList(httpRequest.GetHttpTarget(), port, messageBody);
								if (success)
									statusCode = 200;
								else
								{
									statusCode = 404;
									messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port); // TODO: targetDir->rootedTarget 최적화
								}
							}
							else if (isDirectory)
							{
								bool success = ReadFileAll(mServerConf.GetDefaultPage(httpRequest.GetHttpTarget(), port), messageBody); // TODO: 최적화
								if (success)
									statusCode = 200;
								else
								{
									statusCode = 404;
									messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port); // TODO: targetDir->rootedTarget 최적화
								}
							}
							else
							{
								// 파일이면 그 파일 받아온다
								statusCode = 200;
								bool success = ReadFileAll(rootedTarget, messageBody);
								if (success)
									statusCode = 200;
								else
								{
									statusCode = 404;
									messageBody = this->GetErrorPage(httpRequest.GetHttpTarget(), port); // TODO: targetDir->rootedTarget 최적화
								}
							}
						}
						else
						{
							statusCode = 404;
							messageBody = this->GetErrorPage(httpRequest.GetHttpTarget(), port); // TODO: targetDir->rootedTarget 최적화
						}

					}
					else if (httpMethod == HttpRequest::POST) // TODO: check available method in this directory. use conf
					{
						if (httpRequest.GetParseStatus() == HttpRequest::DONE && httpRequest.GetBodyType() == HttpRequest::INVALID_TYPE)
						{
							statusCode = 411;
							messageBody = this->GetErrorPage(httpRequest.GetHttpTarget(), port);
						}
						else
						{
							statusCode = 204; // TODO: Remove literal
							messageBody = "";
						}
					}
					else if (httpMethod == HttpRequest::DELETE)
					{
						statusCode = 204; // TODO: Remove literal
						messageBody = "";
					}
					else if (httpMethod == HttpRequest::PUT)
					{
						if (httpRequest.GetParseStatus() == HttpRequest::DONE && httpRequest.GetBodyType() == HttpRequest::INVALID_TYPE)
						{
							statusCode = 411;
							messageBody = this->GetErrorPage(httpRequest.GetHttpTarget(), port);
						}
						else
						{
							std::string target = this->mServerConf.GetRootedLocation(httpRequest.GetHttpTarget(), port);
							if (target == "")
							{
								statusCode = 400; // TODO: Remove literal
								messageBody = GetErrorPage(httpRequest.GetHttpTarget(), port);
							}
							std::ifstream fin(target);
							bool isNewFile = !fin.is_open();
							fin.close();

							std::ofstream fout(target);
							fout << httpRequest.GetBody();
							// 해당 루트에 파일이 없으면 생성한다 -> 성공시 201, Created
							if (isNewFile)
							{
								statusCode = 201;
								messageBody = "";
							}
							// 해당 루트에 파일이 이미 있으면 수정한다 -> 성공시 204, No Content
							else
							{
								statusCode = 204;
								messageBody = "";
							}
						}
					}
					else if (httpMethod == HttpRequest::HEAD)
					{
						// Check the target is directory or not.
						std::string rootedTarget = this->mServerConf.GetRootedLocation(httpRequest.GetHttpTarget(), port);
						struct stat myStat;
						bool isDirectory = false;
						bool isValid = (stat(rootedTarget.c_str(), &myStat) == 0);
						if (isValid)
						{
							isDirectory = (myStat.st_mode & S_IFDIR) != 0;
							// 폴더면 디폴트 페이지 받아오고
							if (isDirectory)
							{
								bool success = ReadFileAll(this->mServerConf.GetDefaultPage(httpRequest.GetHttpTarget(), port), messageBody); // TODO: 최적화
								if (success)
									statusCode = 200;
								else
									statusCode = 404;
							}
							else
							{
								// 파일이면 그 파일 받아온다
								bool success = ReadFileAll(rootedTarget, messageBody);
								if (success)
									statusCode = 200;
								else
									statusCode = 404;
							}
							messageBody = "";
						}
						else
						{
							statusCode = 404;
							messageBody = this->GetErrorPage(httpRequest.GetHttpTarget(), port); // TODO: targetDir->rootedTarget 최적화
						}
					}
					responses.insert(std::make_pair(*clientIt, HttpResponse(statusCode, messageBody)));
					cachedRequests.erase(newEvent->ident);
					// 제대로된 HTTP Request를 받았다면 서버도 메세지를 보낼 준비를 한다.
					this->addEvent(changeList, newEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				}
			} // 읽기 요청 이벤트 끝

			// 쓰기 요청 이벤트
			if (newEvent->filter == EVFILT_WRITE)
			{
				// CGI Write
				std::map<int, int>::iterator it = mPipeFds.find(newEvent->ident);
				if (it != mPipeFds.end())
				{
					HttpRequest& httpRequest = cachedRequests[(*it).second];

					if (httpRequest.GetBodyLength() > 0)
					{
						int writeSize = 0;
						const std::string &writeMessage = httpRequest.GetBody().substr(
							httpRequest.GetBodyLength() - httpRequest.mCgiInfo.mRemainedCgiMessage,
							MAX_WRITE_SIZE);

						writeSize = write(newEvent->ident, writeMessage.c_str(), writeMessage.length());
						if (writeSize != -1)
							httpRequest.mCgiInfo.mRemainedCgiMessage -= writeSize;
						else
							std::cerr << "Server: Error: Write Failed\n";
					}
					if (httpRequest.mCgiInfo.mRemainedCgiMessage <= 0)
					{
						addEvent(changeList, newEvent->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
						close(newEvent->ident);
						mPipeFds.erase(newEvent->ident);
					}
				}
				else if (clients.find(newEvent->ident) != clients.end())
				{
					std::cout << "Server: Notice: Pending message to " << newEvent->ident << ".\n";
					int clientSocket = newEvent->ident;
					int sendResult = -1;
					std::map<int, HttpResponse>::iterator it;
					if ((it = responses.find(clientSocket)) != responses.end())
					{
						HttpResponse& res = (*it).second;
						const std::string& message = res.GetHttpMessage(MAX_READ_SIZE);
						sendResult = send(clientSocket, message.c_str(), message.length(), MSG_DONTWAIT); // TODO: 2번 변환 없애기
						if (sendResult > 0) {
							res.IncrementSendIndex(sendResult);
						}
					}
					if (sendResult == -1 || sendResult == 0) {
						std::cerr << "Server: Error: Failed to send message to client.\n";
						return 0;
					}

					if (responses[clientSocket].GetIsSendDone() == true)
					{
						responses.erase(clientSocket);
						this->addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
					}
				}
			}
		} // end of for
	} // end of main loop

	return 0;
}

int HttpServer::Stop() // 서버를 종료합니다.
{
	return 0;
}

HttpServer::~HttpServer()
{
	std::cout << "Server: Notice: HttpServer is deleted successfully.\n";
}

int HttpServer::addEvent(std::vector<struct kevent>& changeList,
			uintptr_t ident,
			int16_t filter,
			uint16_t flags,
			uint32_t fflags,
			intptr_t data,
			void* udata)
{
	struct kevent newEvent;

	EV_SET(&newEvent, ident, filter, flags, fflags, data, udata);
	changeList.push_back(newEvent);
	return 0;
}

// 실행되면 안되는 Canonical form methods
HttpServer::HttpServer()
{

}

HttpServer::HttpServer(const HttpServer& other)
{
	(void)other;
}

HttpServer& HttpServer::operator=(const HttpServer& other)
{
	(void)other;
	return *this;
}

std::string HttpServer::GetErrorPage(const std::string& targetDir, int port) const
{
	std::stringstream ss;
	std::string errorPagePath = this->mServerConf.GetDefaultErrorPage(targetDir, port); // TODO: use conf
	std::ifstream fin(errorPagePath);
	if (fin.is_open() == false)
	{
		std::cerr << "Server: Error: Could not open " << errorPagePath << "\n";
		fin.close();
		return "";
	}
	std::string buf;
	while (getline(fin, buf))
		ss << buf;
	fin.close();
	return ss.str();
}

bool HttpServer::ReadFileAll(const std::string& filePath, std::string& result) const
{
	std::ifstream fin(filePath);
	if (fin.is_open() == false)
		return false;

	std::stringstream ss;
	std::string buf;
	while (getline(fin, buf))
		ss << buf;
	fin.close();
	result = ss.str();
	return true;
}

bool HttpServer::IsServerSocket(const std::vector<int>& serverSockets, uintptr_t ident) const
{
	for (size_t i=0; i<serverSockets.size(); ++i)
		if (static_cast<uintptr_t>(serverSockets[i]) == ident)
			return true;

	return false;
}

bool HttpServer::IsCGIRequest(const HttpRequest& request, int port) const
{
	std::string rootedTarget = mServerConf.GetRootedLocation(request.GetHttpTarget(), port);
	size_t extensionPos = rootedTarget.rfind(".");
	if (extensionPos != rootedTarget.npos)
	{
		std::string fileExtension = rootedTarget.substr(extensionPos);
		return mServerConf.IsValidCgiExtension(request.GetHttpTarget(), port, fileExtension);
	}
	else
	{
		return false;
	}
}

bool HttpServer::GetDirectoryList(const std::string& targetDir, int port, std::string& result) const
{
	struct dirent *diread;
	DIR *dir;

	std::string rootedLocation = mServerConf.GetRootedLocation(targetDir, port);
    if ((dir = opendir(rootedLocation.c_str())) != nullptr)
	{
		result.append("<h1>");
		result.append(targetDir);
		result.append("</h1><pre>");
        while ((diread = readdir(dir)) != nullptr)
		{
			if (std::string(diread->d_name) == "." || std::string(diread->d_name) == "..")
				continue;
            result.append(diread->d_name);
            result.append("\n");
        }
		result.append("</pre>");
        closedir (dir);
		return true;
    }
	else
	{
        return false;
    }
}