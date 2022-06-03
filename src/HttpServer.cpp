#include "HttpServer.hpp"

HttpServer::HttpServer(Conf& conf)
{
	this->mServerConf = conf;


}

int HttpServer::Run() // 서버를 실행합니다. Init()이 실행된 후여야 합니다.
{
	// make socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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
	serverAddress.sin_port = htons(8080); // TODO: use Conf

	if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		std::cerr << "[ERROR] server socket bind() failed.\n";
		close(serverSocket);
		return 1;
	}

	if (listen(serverSocket, 1024) < 0) // TODO: use Conf
	{
		std::cerr << "[ERROR] server socket listen() failed.\n";
		close(serverSocket);
		return 1;
	}
	std::cout << "Server is running.\n";

	// Prepare for multiflexing I/O
	int kq = kqueue();
	if (kq < 0)
	{
		assert(kq >= 0);
		std::cerr << "[ERROR] kqueue() failed.\n";
		close(serverSocket);
		return 1;
	}

	std::unordered_set<int> clients;
	std::unordered_map<int, HttpResponse> responses;
	std::vector<struct kevent> changeList;
	this->addEvent(changeList, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	struct kevent eventList[1024]; // TODO: use Conf object

	// Main loop
	while (1)
	{
		int newEventSize = kevent(kq, &changeList[0], changeList.size(), eventList, 1024, NULL); // TODO: use Conf
		changeList.clear();

		std::cout << "New event size: " << newEventSize << std::endl;

		for (int i=0; i<newEventSize; ++i)
		{
			struct kevent* newEvent = &eventList[i];

			// 읽기 요청 이벤트
			if (newEvent->filter == EVFILT_READ)
			{
				// 새로운 Client
				if (newEvent->ident == (uintptr_t)serverSocket)
				{
					std::cout << "New client exists.\n";
					int newClientSocket;
					if ((newClientSocket = accept(serverSocket, NULL, NULL)) == -1)
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
					char buffer[4096]; // TODO: use Conf
					int readSize = read(*clientIt, buffer, 4096); // TODO: use Conf
					if (readSize <= 0)
					{
						if (readSize == -1)
							std::cerr << "[ERROR] Failed to read client message\n";
						close(*clientIt);
						clients.erase(*clientIt);
					}
					else
					{
						std::cout << buffer << std::endl;
						HttpRequest httpRequest(buffer);
						int statusCode = GetStatusCode(httpRequest);
						std::cout << "Status Code: " << statusCode << std::endl;
						std::string messageBody = GetMessageBody(httpRequest);
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

static bool IsValidStatus404(HttpRequest& httpRequest)
{
	std::string targetFile = GetTargetFile(httpRequest);
	std::ifstream readFile;
	readFile.open(targetFile);
	// TODO: (search) 파일은 존재하지만 파일에 읽기 권한이 없을 때, 다른 처리를 해야하는가?
	if (!readFile.is_open())
		return false;
	return true;
}

static bool IsValidStatus505(HttpRequest& httpRequest)
{
	std::string protocolVersion = httpRequest.getFieldByKey("ProtocolVersion");
	if (protocolVersion != "HTTP/1.1")
		return false;
	return true;
}
int HttpServer::GetStatusCode(HttpRequest& httpRequest)
{
	if (IsValidStatus404(httpRequest) == false)
		return 404;
	if (IsValidStatus505(httpRequest) == false)
		return 505;
	return 200;
}

std::string HttpServer::GetMessageBody(HttpRequest& httpRequest)
{
	std::string messageBody = "";
	std::string targetFile = GetTargetFile(httpRequest);
	std::ifstream readFile;
	std::string buff;

	readFile.open(targetFile);
	// TODO: (의논) messageBody에 \n도 붙여야하는지?
	while (getline(readFile, buff))
		messageBody += buff ;
	readFile.close();
	std::cout << "Message Body: " << messageBody << std::endl;
	return messageBody;
}
