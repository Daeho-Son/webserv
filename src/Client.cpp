#include "Client.hpp"

Client::Client()
    :   mState(Request),
        mSocket(-1),
        mServerSocket(-1),
        mCgiWriteFd(-1),
        mCgiReadFd(-1),
        mLastResponseTime(time(NULL)) {}
        // mResponseReadFd(-1),

Client::Client(int socket, int serverSocket)
    :   mState(Request),
        mSocket(socket),
        mServerSocket(serverSocket),
        mCgiWriteFd(-1),
        mCgiReadFd(-1),
        mLastResponseTime(time(NULL)) {}
        // mResponseReadFd(-1),