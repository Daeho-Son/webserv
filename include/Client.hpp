#ifndef FT_CLIENT_HPP
#define FT_CLIENT_HPP

class Client
{
enum eState {Request, Response, Done}

public:
    inline eState   GetState() { return mState; }
    inline int      GetSocket() { return mSocket; }
    inline bool     IsCgiFd(int fd) { return mCgiWriteFd == fd || mCgiReadFd == fd; }
	inline time_t	GetLastResponseTime(time_t t) { return mLastResponseTime; }
    
	inline void		SetState(eState newState) { mState = newState; }
	inline void		SetSocket(int socket) { mSocket = socket; }
	inline void		SetCgiFds(int writeFd, int readFd) { mCgiWriteFd = writeFd; mCgiReadFd = readFd; }
	inline void		SetLastResponseTime(time_t t) { mLastResponseTime = t; }

private:
    eState  mState;
    int     mSocket;
    int     mCgiWriteFd;
    int     mCgiReadRd;
    int     mResponseReadFd;
    time_t  mLastResponseTime;
};

#endif