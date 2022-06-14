#include "HttpServer.hpp"

HttpServer::HttpServer(Conf& conf)
{
	this->mServerConf = conf;
}

int HttpServer::Run() // 서버를 실행합니다. Init()이 실행된 후여야 합니다.
{
	// make server sockets
	const std::vector<int>& serverPorts = this->mServerConf.GetPorts();
	std::vector<int> serverSockets;
	for (size_t i=0; i<serverPorts.size(); ++i)
	{
		serverSockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
		int& serverSocket = serverSockets[serverSockets.size()-1];
		std::cout << "server socket = " << serverSocket << ", server ports = " << serverPorts[i] << "\n";
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
		serverAddress.sin_port = htons(serverPorts[i]); // TODO: use Conf

		int bsize = 0;
		int rn;
		rn = sizeof(int);

		// 현재 전송 소켓 버퍼의 크기를 가져온다.
    	getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, &bsize, (socklen_t *)&rn);
		bsize *= 2;
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &bsize, (socklen_t)rn);

		if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		{
			std::cerr << "[ERROR] server socket bind() failed.\n";
			close(serverSocket);
			return 1;
		}

		if (listen(serverSocket, this->mServerConf.GetListenSize()) < 0) // TODO: use Conf
		{
			std::cerr << "[ERROR] server socket listen() failed.\n";
			close(serverSocket);
			return 1;
		}
	}
	std::cout << "All server sockets opened\n";
	std::cout << "Server is running.\n";

	// Prepare for multiflexing I/O
	int kq = kqueue();
	if (kq < 0)
	{
		assert(kq >= 0);
		std::cerr << "[ERROR] kqueue() failed.\n";
		for (size_t i=0; i<serverSockets.size(); ++i)
		{
			close(serverSockets[i]);
		}
		
		return 1;
	}

	std::unordered_set<int> clients;
	std::unordered_map<int, HttpResponse> responses;
	std::vector<struct kevent> changeList;
	for (size_t i=0; i<serverSockets.size(); ++i)
	{
		this->addEvent(changeList, serverSockets[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	
	struct kevent eventList[this->mServerConf.GetKeventsSize()];
	std::cout << "Server main loop started.\n";
	// Main loop
	while (1)
	{
		int newEventSize = kevent(kq, &changeList[0], changeList.size(), eventList, this->mServerConf.GetKeventsSize(), NULL);
		changeList.clear();

		for (int i=0; i<newEventSize; ++i)
		{
			struct kevent* newEvent = &eventList[i];
			std::cout << "new events! " << newEventSize << "\n";

			// 읽기 요청 이벤트
			if (newEvent->filter == EVFILT_READ)
			{
				// 새로운 Client
				if (this->IsServerSocket(serverSockets, newEvent->ident))
				{
					std::cout << "New client exists.\n";
					int newClientSocket;
					if ((newClientSocket = accept(newEvent->ident, NULL, NULL)) == -1)
					{
						std::cerr << "[ERROR] accept() failed.\n";
						continue;
					}
					clients.insert(newClientSocket);
					fcntl(newClientSocket, F_SETFL, O_NONBLOCK);
					this->addEvent(changeList, newClientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					this->addEvent(changeList, newClientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
				}
				// 기존 Client
				else
				{
					std::cout << "The client " << newEvent->ident << " sent a message.\n";
					std::unordered_set<int>::iterator clientIt = clients.find(newEvent->ident);
					if (clientIt == clients.end()) {
						std::cerr << "[ERROR] Request from Invalid client\n";
						continue;
					}
					char buffer[this->mServerConf.GetClientBodySize()];
					int readSize = read(*clientIt, buffer, this->mServerConf.GetClientBodySize());
					if (readSize <= 0)
					{
						if (readSize == -1)
							std::cerr << "[ERROR] Failed to read client message\n";
						close(*clientIt);
						clients.erase(*clientIt);
					}
					else
					{
						int statusCode = 400;
						std::string messageBody;
						HttpRequest httpRequest(buffer);
						std::string httpMethod = httpRequest.getFieldByKey("MethodToken");
						
						if (this->mServerConf.IsValidHttpMethod(httpRequest.getFieldByKey("RequestTarget"), httpMethod) == false)
						{
							statusCode = 405;
							messageBody = "";
						}
						else if (httpMethod == "GET")
						{
							// Check the target is directory or not.
							std::string rootedTarget = this->mServerConf.GetRootedLocation(httpRequest.getFieldByKey("RequestTarget"));
							struct stat myStat;
							bool isDirectory = false;
							bool isValid = (stat(rootedTarget.c_str(), &myStat) == 0);
							if (isValid)
							{
								isDirectory = (myStat.st_mode & S_IFDIR) != 0;
								// 폴더면 디폴트 페이지 받아오고
								if (isDirectory)
								{
									bool success = ReadFileAll(this->mServerConf.GetDefaultPage(httpRequest.getFieldByKey("RequestTarget")), messageBody); // TODO: 최적화
									if (success)
										statusCode = 200;
									else
									{
										statusCode = 404;
										messageBody = this->GetErrorPage(httpRequest.getFieldByKey("RequestTarget")); // TODO: targetDir->rootedTarget 최적화
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
										messageBody = this->GetErrorPage(httpRequest.getFieldByKey("RequestTarget")); // TODO: targetDir->rootedTarget 최적화
									}
								}
							}
							else
							{
								statusCode = 404;
								messageBody = this->GetErrorPage(httpRequest.getFieldByKey("RequestTarget")); // TODO: targetDir->rootedTarget 최적화
							}
							
						}
						else if (httpMethod == "POST") // TODO: check available method in this directory. use conf
						{
							statusCode = 204; // TODO: Remove literal
							messageBody = "";
						}
						else if (httpMethod == "DELETE")
						{
							statusCode = 204; // TODO: Remove literal
							messageBody = "";
						}
						else if (httpMethod == "PUT")
						{
							std::cout << "PUT request is pending..\n";
							std::string target = this->mServerConf.GetRootedLocation(httpRequest.getFieldByKey("RequestTarget"));
							if (target == "")
							{
								statusCode = 400; // TODO: Remove literal
								messageBody = GetErrorPage(httpRequest.getFieldByKey("RequestTarget"));
								std::cerr << "There is no RequestTarget.\n";
							}
							std::ifstream fin(target);
							bool isNewFile = !fin.is_open();
							fin.close();

							std::ofstream fout(target);
							fout << httpRequest.getFieldByKey("Body");
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
						responses[*clientIt] = HttpResponse(statusCode, messageBody);

						// 제대로된 HTTP Request를 받았다면 서버도 메세지를 보낼 준비를 한다.
						this->addEvent(changeList, newEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					}
				}
			} // 읽기 요청 이벤트 끝

			// 쓰기 요청 이벤트
			if (newEvent->filter == EVFILT_WRITE)
			{
				std::cout << "Pending message to " << newEvent->ident << ".\n";
				int clientSocket = newEvent->ident;
				int sendResult = -1;
				if (responses.find(clientSocket) != responses.end())
				{
					std::string message = responses[clientSocket].GetHttpMessage();
					sendResult = send(clientSocket, message.c_str(), message.length(), 0); // TODO: 2번 변환 없애기
					std::cout << "Message Sent.\n" << message.c_str() << "\n\n";
				}
				if (sendResult == -1) {
					std::cerr << "[ERROR] Failed to send message to client.\n";
					close(clientSocket);
					clients.erase(clientSocket);
				}
				this->addEvent(changeList, clientSocket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
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
	std::cout << "HttpServer is deleted successfully.\n";
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

// TODO: (의논) 외부 함수로 뺄까?
static std::string GetTargetFile(HttpRequest& httpRequest)
{
	std::string requestTarget = httpRequest.getFieldByKey("RequestTarget");
	std::string path = "./";
	std::string targetFile = "";
	if (requestTarget == "/")
		requestTarget = CONF_DEFAULT_TARGET; // TODO: use conf
	if (requestTarget.find(".html") != requestTarget.npos)
		path += "html";
	if (requestTarget.find(".ico") != requestTarget.npos)
		path += "ico";
	targetFile = path + requestTarget;
	return targetFile;
}

int HttpServer::GetStatusCode(HttpRequest& httpRequest)
{
	// 폴더인지 파일인지 확인
	// 해당 장소에 폴더가 없으면 파일이라고 생각
	std::string rootedTarget = this->mServerConf.GetRootedLocation(httpRequest.getFieldByKey("RequestTarget"));
	struct stat myStat;
	bool isValid = (stat(rootedTarget.c_str(), &myStat) == 0);
	if (isValid == false)
		return 404;
	return 200;
}

std::string HttpServer::GetMessageBody(HttpRequest& httpRequest, int statusCode) const
{
	std::stringstream ss;
	std::string targetFile = "";
	if (statusCode != 200)
		targetFile = "./html/404.html";
	else
		targetFile = GetTargetFile(httpRequest);
	std::ifstream fin(targetFile);
	if (fin.is_open() == false)
	{
		std::cerr << "Could not open " << targetFile << "\n";
		fin.close();
		return NULL;
	}
	std::string buf;
	while (getline(fin, buf))
		ss << buf;
	fin.close();
	return ss.str();
}

std::string HttpServer::GetErrorPage(const std::string& targetDir) const
{
	std::stringstream ss;
	std::string errorPagePath = this->mServerConf.GetDefaultErrorPage(targetDir); // TODO: use conf
	std::ifstream fin(errorPagePath);
	if (fin.is_open() == false)
	{
		std::cerr << "Could not open " << errorPagePath << "\n";
		fin.close();
		return NULL;
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
