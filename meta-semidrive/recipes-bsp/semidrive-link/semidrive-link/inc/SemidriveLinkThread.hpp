#ifndef __SEMIDRIVE_LINK_THREAD_HPP__
#define  __SEMIDRIVE_LINK_THREAD_HPP__
#include <thread>

class SemidriveLinkThreadListener {
public:
    virtual void onData(unsigned char *,int) = 0;
    virtual ~SemidriveLinkThreadListener(){}
};

class SemidriveLinkThread {
public:
    SemidriveLinkThread(int listenFd,SemidriveLinkThreadListener *);
    void start();
    void stop();
    void run();

private:
    SemidriveLinkThreadListener *listener;
    std::thread mWorkThread;
    int mEpollFd;
    int mDeviceFd;
    bool isRun;
};

#endif