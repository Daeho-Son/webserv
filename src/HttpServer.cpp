#include "HttpServer.hpp"

HttpServer::HttpServer(Conf& conf)
{
	mServerConf = conf;
}

int HttpServer::Run() // 서버를 실행합니다. Init()이 실행된 후여야 합니다.
{
	// make server sockets
	std::vector<int> serverSockets;
	std::vector<ServerInfo> serverInfos = mServerConf.GetServerInfos();
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
		serverAddress.sin_port = htons(serverPort);

		int sock_opt = 1;

		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));

		if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		{
			std::cerr << RED << "Server: Error: server socket bind() failed.\n" << NM;
			close(serverSocket);
			return 1;
		}
		if (listen(serverSocket, mServerConf.GetListenSize()) < 0)
		{
			std::cerr << RED << "Server: Error: server socket listen() failed.\n" << NM;
			close(serverSocket);
			return 1;
		}

		fcntl(serverSocket, F_SETFL, O_NONBLOCK);
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

	std::map<int, HttpResponse> responses;
	std::vector<struct kevent> changeList;
	for (size_t i=0; i<serverSockets.size(); ++i)
	{
		addEvent(changeList, serverSockets[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}

	struct kevent eventList[mServerConf.GetKeventsSize()];
	std::cout << GRN << "Server: Notice: Server main loop started.\n" << NM;
	// Main loop
	while (1)
	{
		int newEventSize = kevent(kq, &changeList[0], changeList.size(), eventList, mServerConf.GetKeventsSize(), NULL);
		changeList.clear();

		std::map<int, Client>::iterator it;
		std::stack<int> timeoutClients;
		for (it = mClients.begin(); it != mClients.end(); ++it)
		{
			if ((*it).second.GetState() == Client::Done && IsTimeoutSocket((*it).second))
			{
				timeoutClients.push((*it).first);
			}
		}

		while (timeoutClients.empty() == false)
		{
			DisconnectClient(timeoutClients.top(), changeList);
			timeoutClients.pop();
		}
		// end of timeout

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
					int clientSocket = (*it).second;
					HttpRequest& httpRequest = mCachedRequests[clientSocket];
					char readBuffer[MAX_READ_SIZE];
					memset(readBuffer, 0, MAX_READ_SIZE);
					int64_t readSize = 0;
					if ((readSize = read(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], readBuffer, MAX_READ_SIZE - 1)) > 0)
					{
						httpRequest.mCgiInfo.mTotalReadSize += readSize;
						httpRequest.AppendResponseMessageBody(readBuffer);
					}
					// 다 읽었으면 Response 만들어서 보낼 준비
					if (readSize == 0)
					{
						responses.insert(std::make_pair(clientSocket, HttpResponse(httpRequest.GetResponseMessageBody())));
						mCachedRequests.erase(clientSocket);
						addEvent(changeList, newEvent->ident, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						mPipeFds.erase(newEvent->ident);
						close(newEvent->ident);
						wait(NULL);
					}
					else if (readSize == -1)
					{
						std::cerr << "Server: Error: Read Failed\n";
					}
				}
				else if (IsFileFd(newEvent->ident))
				{
					uintptr_t& fileFd = newEvent->ident;
					int& clientSocket = mFileFds[fileFd];
					
					// Read file fd
					char readBuffer[MAX_READ_SIZE];
					memset(readBuffer, 0, MAX_READ_SIZE);
					int readSize = read(fileFd, readBuffer, MAX_READ_SIZE-1);

					if (readSize > 0)
					{
						if (responses.find(clientSocket) != responses.end())
							responses[clientSocket].AppendBody(readBuffer);
					}

					if (readSize == -1)
					{
						std::cout << "Read size -1\n";
						continue;
					}
					else if (readSize == 0 || readSize < MAX_READ_SIZE-1)
					{
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
						mCachedRequests.erase(clientSocket);
						mFileFds.erase(fileFd);
						close(fileFd);
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
					ConnectClient(newClientSocket, newEvent->ident, changeList);
				}
				// 기존 Client
				else
				{
					std::cout << "Server: Notice: The client " << newEvent->ident << " sent a message.\n";
					std::map<int, Client>::iterator clientIt = mClients.find(newEvent->ident);
					if (clientIt == mClients.end()) {
						std::cerr << "Server: Warning: Request from Invalid client\n";
						continue;
					}
					size_t serverIndex = getServerIndexBySocket[(*clientIt).second.GetServerSocket()];
					int clientSocket = (*clientIt).second.GetSocket();
					ServerInfo serverInfo = mServerConf.GetServerInfos()[serverIndex];
					int port = serverInfo.GetPort();

					/*
					STEP 1. fd에 있는 데이터를 읽는다.
					*/
					char readBuffer[MAX_READ_SIZE];
					memset(readBuffer, 0, MAX_READ_SIZE);
					int readSize = read(clientSocket, readBuffer, MAX_READ_SIZE-1);
					if (readSize == 0)
					{
						std::cerr<< "Server: Error: Recv read size 0\n";
						DisconnectClient(clientSocket, changeList);
						continue;
					}
					else if (readSize == -1)
					{
						std::cerr << "Server: Error: Recv read size: -1\n";
						continue;
					}
					/*
					STEP 2: 파싱한다. 없으면 생성 후, 파싱.
					*/
					(*clientIt).second.SetState(Client::Request);
					if (mCachedRequests.find(clientSocket) == mCachedRequests.end())
					{
						mCachedRequests.insert(std::make_pair(clientSocket, HttpRequest()));
					}
					try {
						mCachedRequests[clientSocket].Parse(std::string(readBuffer));
					}
					catch(std::exception& e)
					{
						std::cerr << "Server: Error: Request Parse Error ==> " << e.what() << "\n";
					}
					catch(...)
					{
						std::cerr << "Server: Error: Request Parse Error\n";
					}
					/*
					STEP 3: HTTP Request가 적절히 변환됐다면 올바른 Response를 구성해서 저장한다.
					*/
					if (mCachedRequests[clientSocket].GetParseStatus() != HttpRequest::DONE)
						continue;

					int statusCode = 417;
					std::string messageBody = "";
				    HttpRequest& httpRequest = mCachedRequests[clientSocket];
					const HttpRequest::eMethod httpMethod = httpRequest.GetMethod();
					int fileFd = -1;
					
					if (httpRequest.GetBody().length() > mServerConf.GetClientBodySize(httpRequest.GetHttpTarget(), port))
					{
						statusCode = 413;
						if (httpRequest.GetMethod() == HttpRequest::HEAD)
							messageBody = "";
						else
						{
							fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
							mFileFds.insert(std::make_pair(fileFd, clientSocket));
							addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
					else if (mServerConf.IsRedirectedTarget(httpRequest.GetHttpTarget(), port))
					{
						statusCode = 301;
						mClients[clientSocket].SetState(Client::Response);
						HttpResponse response(statusCode, messageBody, httpRequest.GetFieldByKey("Connection"));
						response.SetRedirectLocation(mServerConf.GetRedirectTarget(httpRequest.GetHttpTarget(), port));
						responses.insert(std::make_pair(clientSocket, response));
						mCachedRequests.erase(clientSocket);
						// 제대로된 HTTP Request를 받았다면 서버도 메세지를 보낼 준비를 한다.
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						continue;
					}
					else if (httpRequest.GetMethod() == HttpRequest::NOT_VALID)
					{
						statusCode = 400;
						fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
						mFileFds.insert(std::make_pair(fileFd, clientSocket));
						addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					}
					else if (mServerConf.IsValidHttpMethod(httpRequest.GetHttpTarget(), port, httpRequest.GetMethodStringByEnum(httpMethod)) == false)
					{
						statusCode = 405;
						if (httpRequest.GetMethod() == HttpRequest::HEAD)
							messageBody = "";
						else
						{
							fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
							mFileFds.insert(std::make_pair(fileFd, clientSocket));
							addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
					// CGI Process
					else if (IsCGIRequest(httpRequest, port))
					{
						char* const argv[] = {
							(char*)"cgi_tester",
							(char*)0
						};
						std::vector<std::string> envpVec;
						httpRequest.GetCgiEnvVector(envpVec);
						
						char** envp = (char**)malloc(sizeof(char*) * MAX_ENVP_SIZE);
						for (int i=0; i<MAX_ENVP_SIZE; ++i){
							envp[i] = 0;
							if (envpVec[i].length() > 0) {
								envp[i] = (char*)malloc(envpVec[i].length() + 1);
								memset(envp[i], 0, envpVec[i].length() + 1);
								strcpy(envp[i], envpVec[i].c_str());
							}
						}

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
						fcntl(httpRequest.mCgiInfo.mPipeC2P[PIPE_WRITE_FD], F_SETFL, O_NONBLOCK);
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
							(*clientIt).second.SetCgiFds(
									httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD],
									httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD]
							);
							close(httpRequest.mCgiInfo.mPipeP2C[PIPE_READ_FD]); // 부모는 p2c 파이프에서 쓰기만 한다.
							close(httpRequest.mCgiInfo.mPipeC2P[PIPE_WRITE_FD]); // 부모는 c2p 파이프에서 읽기만 한다.
							addEvent(changeList, httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
							addEvent(changeList, httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							
							// free envp array
							for (int i=0; i<MAX_ENVP_SIZE; ++i) {
								if (envp[i] != 0) free(envp[i]);
							}
							free(envp);
							continue;
						}
						else // 자식 프로세스 (= CGI 프로세스)
						{
							close(httpRequest.mCgiInfo.mPipeP2C[PIPE_WRITE_FD]); // 자식은 p2c 파이프에 읽기만 한다.
							close(httpRequest.mCgiInfo.mPipeC2P[PIPE_READ_FD]); // 자식은 c2p 파이프에 쓰기만 한다.
							dup2(httpRequest.mCgiInfo.mPipeP2C[PIPE_READ_FD], STDIN);
							dup2(httpRequest.mCgiInfo.mPipeC2P[PIPE_WRITE_FD], STDOUT);
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
								bool success = GetDirectoryList(httpRequest.GetHttpTarget(), port, messageBody);
								if (success)
								{
									statusCode = 200;
								}
								else
								{
									statusCode = 404;
									fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
								}
							}
							else if (isDirectory)
							{
								fileFd = OpenFile(mServerConf.GetDefaultPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
								if (fileFd != -1)
								{
									statusCode = 200;
								}
								else
								{
									statusCode = 404;
									fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
								}
							}
							else
							{
								// 파일이면 그 파일 받아온다
								fileFd = OpenFile(rootedTarget, O_RDONLY);
								if (fileFd != -1)
								{
									statusCode = 200;
								}
								else
								{
									statusCode = 404;
									fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
								}
							}
						}
						else
						{
							statusCode = 404;
							fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
						}
						if (fileFd != -1)
						{
							mFileFds.insert(std::make_pair(fileFd, clientSocket));
							addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
					else if (httpMethod == HttpRequest::POST)
					{
						if (httpRequest.GetParseStatus() == HttpRequest::DONE && httpRequest.GetBodyType() == HttpRequest::INVALID_TYPE)
						{
							statusCode = 411;
							fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
							addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							mFileFds.insert(std::make_pair(fileFd, clientSocket));
						}
						else
						{
							statusCode = 204;
							messageBody = "";
						}
					}
					else if (httpMethod == HttpRequest::DELETE)
					{
						statusCode = 204;
						messageBody = "";
					}
					else if (httpMethod == HttpRequest::PUT)
					{
						if (httpRequest.GetParseStatus() == HttpRequest::DONE && httpRequest.GetBodyType() == HttpRequest::INVALID_TYPE)
						{
							statusCode = 411;
							fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
						}
						else
						{
							std::string target = mServerConf.GetRootedLocation(httpRequest.GetHttpTarget(), port);
							if (target == "")
							{
								statusCode = 400;
								fileFd = OpenFile(mServerConf.GetDefaultErrorPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
							}
							else
							{
								// Set status code
								std::ifstream fin(target);
								statusCode = fin.is_open() ? 204 : 201;
								fin.close();
								messageBody = "";

								// Write Multiflexing
								fileFd = OpenFile(target, O_WRONLY | O_CREAT | O_TRUNC);
								addEvent(changeList, fileFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
							}
						}
						if (statusCode >= 400)
						{
							addEvent(changeList, fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
						mFileFds.insert(std::make_pair(fileFd, clientSocket));
					}
					else if (httpMethod == HttpRequest::HEAD)
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
							if (isDirectory)
							{
								fileFd = OpenFile(mServerConf.GetDefaultPage(httpRequest.GetHttpTarget(), port), O_RDONLY);
								statusCode = fileFd == -1 ? 404 : 200;
							}
							else
							{
								// 파일이면 그 파일 받아온다
								fileFd = OpenFile(rootedTarget, O_RDONLY);
								statusCode = fileFd == -1 ? 404 : 200;
							}
						}
						else
						{
							statusCode = 404;
						}
						if (fileFd > 0)
							close(fileFd);
						fileFd = -1; // reset fd for inserting response immediately
					} // end of method ifs
					mClients[clientSocket].SetState(Client::Response);
					responses.insert(std::make_pair(clientSocket, HttpResponse(statusCode, messageBody, httpRequest.GetFieldByKey("Connection"))));
					if (fileFd == -1)
					{
						mCachedRequests.erase(clientSocket);
						// 제대로된 HTTP Request를 받았다면 서버도 메세지를 보낼 준비를 한다.
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					}
				}
			} // 읽기 요청 이벤트 끝
	
			// 쓰기 요청 이벤트
			if (newEvent->filter == EVFILT_WRITE)
			{
				// CGI Write
				std::map<int, int>::iterator it = mPipeFds.find(newEvent->ident);
				if (it != mPipeFds.end())
				{
					HttpRequest& httpRequest = mCachedRequests[(*it).second];

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
				else if (IsFileFd(newEvent->ident))
				{
					uintptr_t& fileFd = newEvent->ident;
					int& clientSocket = mFileFds[fileFd];
					HttpRequest& httpRequest = mCachedRequests[clientSocket];
					std::string buffer = httpRequest.GetBody().substr(httpRequest.GetBodyIndex(), MAX_WRITE_SIZE);
					int writeResult = write(fileFd, buffer.c_str(), buffer.length());
					if (writeResult > 0)
					{
						httpRequest.IncrementBodyIndex(writeResult);
					}
					if (writeResult == -1) continue;
					else if (writeResult < MAX_WRITE_SIZE)
					{
						addEvent(changeList, fileFd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						mCachedRequests.erase(clientSocket);
						mFileFds.erase(fileFd);
						close(fileFd);
					}
				}
				else if (mClients.find(newEvent->ident) != mClients.end())
				{
					std::cout << "Server: Notice: Pending message to " << newEvent->ident << ".\n";
					int clientSocket = newEvent->ident;
					int sendResult = -1;
					std::map<int, HttpResponse>::iterator it;
					if ((it = responses.find(clientSocket)) != responses.end())
					{
						HttpResponse& res = (*it).second;
						const std::string& message = res.GetHttpMessage(MAX_WRITE_SIZE);
						sendResult = send(clientSocket, message.c_str(), message.length(), MSG_DONTWAIT);
						if (sendResult > 0) {
							res.IncrementSendIndex(sendResult);
						}
					}
					if (sendResult == -1) {
						std::cerr << "Server: Error: Failed to send message to client.\n";
						continue;
					}
					
					if (sendResult == 0) {
						DisconnectClient(clientSocket, changeList);
						continue;
					}

					if (it != responses.end() && responses[clientSocket].GetIsSendDone() == true)
					{
						addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
						mClients[clientSocket].SetState(Client::Done);
						if (responses[clientSocket].GetConnection() != "close")
							UpdateTimeout(clientSocket);
						else
						{
							DisconnectClient(clientSocket, changeList);
						}
						responses.erase(clientSocket);
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

bool HttpServer::IsTimeoutSocket(const Client& client)
{
	return difftime(time(NULL), client.GetLastResponseTime()) > TIMEOUT_LIMIT;
}

bool HttpServer::IsDeadSocket(const Client& client)
{
	return difftime(time(NULL), client.GetLastResponseTime()) > (TIMEOUT_LIMIT * 10);
}

bool HttpServer::DisconnectClient(int clientSocket, std::vector<struct kevent>& changeList)
{
	int result = 0;
	if (mClients.find(clientSocket) != mClients.end())
	{
		result = close(clientSocket);
		std::cout << RED << "Server: Notice: client " << clientSocket << " left.\n" << NM;
		addEvent(changeList, clientSocket, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
		mCachedRequests.erase(clientSocket);
		if (mClients[clientSocket].GetCgiReadFd() != -1) close(mClients[clientSocket].GetCgiReadFd());
		if (mClients[clientSocket].GetCgiWriteFd() != -1) close(mClients[clientSocket].GetCgiWriteFd());
		mClients.erase(clientSocket);
	}

	return result == 0;
}

bool HttpServer::ConnectClient(int newClientSocket, int serverSocket, std::vector<struct kevent>& changeList)
{
	std::cout << "Server: Notice: new client " << newClientSocket << " added.\n";
	mClients.insert(std::make_pair(newClientSocket, Client(newClientSocket, serverSocket)));
	fcntl(newClientSocket, F_SETFL, O_NONBLOCK);
	addEvent(changeList, newClientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	addEvent(changeList, newClientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
	return true;
}

bool HttpServer::UpdateTimeout(int clientSocket)
{
	mClients[clientSocket].SetLastResponseTime(time(NULL));
	return true;
}

int HttpServer::OpenFile(const std::string& target, int fileMode)
{
	int fileFd = open(target.c_str(), fileMode, 0755);
	if (fileFd < 0)
		return -1;
	return fileFd;
}