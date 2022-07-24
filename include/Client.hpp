#ifndef FT_CLIENT_HPP
# define FT_CLIENT_HPP

# include <ctime>

class Client
{
public:
    enum eState {Request, Response, Done};

public:
    inline eState   GetState() const { return mState; }
    inline int      GetSocket() const { return mSocket; }
    inline int      GetServerSocket() const { return mServerSocket; }
    inline bool     IsCgiFd(int fd) const { return mCgiWriteFd == fd || mCgiReadFd == fd; }
	inline time_t	GetLastResponseTime() const { return mLastResponseTime; }
    inline int      GetCgiReadFd() const { return mCgiReadFd; }
    inline int      GetCgiWriteFd() const { return mCgiWriteFd; }
    
	inline void		SetState(eState newState) { mState = newState; }
	inline void		SetCgiFds(int writeFd, int readFd) { mCgiWriteFd = writeFd; mCgiReadFd = readFd; }
	inline void		SetLastResponseTime(time_t t) { mLastResponseTime = t; }
    Client();
    Client(int socket, int serverSocket);

private:
    eState      mState;
    int         mSocket;
    int         mServerSocket;
    int         mCgiWriteFd;
    int         mCgiReadFd;
    time_t      mLastResponseTime;
    // int         mResponseReadFd;
};

#endif