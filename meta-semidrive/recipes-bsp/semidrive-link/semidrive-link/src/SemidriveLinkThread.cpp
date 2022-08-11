#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include<sys/epoll.h>
#include <fcntl.h>

#ifdef ANDORID
#include <utils/Log.h>
#define TAG semidrivelink
#define LOGD(fmt,args...) ALOGD(fmt,##args)
#define LOGE(fmt,args...) ALOGE(fmt,##args)
#else
#define LOGD(fmt,args...) printf(fmt,##args)
#define LOGE(fmt,args...) printf(fmt,##args)
#endif

#include "SemidriveLinkThread.hpp"
#include "Log.hpp"
#include "CanData.hpp"
#include "Message.hpp"

#define MAX_EPOLL_EVENTS 16
#define BUFFER_SIZE  32

SemidriveLinkThread::SemidriveLinkThread(int fd,SemidriveLinkThreadListener *l) {
    mEpollFd = 0;
    mDeviceFd = fd;
    listener = l;
#if 0
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
    mDeviceFd = fd;
    listener  = l;
    //create epoll
    mEpollFd = epoll_create(32);
    if(mEpollFd < 0) {
        LOGE("create epoll failed \n");
        return;
    }

    struct epoll_event event;
    event.data.fd = mDeviceFd;
    printf("mDeviceid is %d \n",mDeviceFd);
    event.events = EPOLLIN;
    int ret = epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mDeviceFd,&event);

    if(ret < 0) {
        LOGE("add epoll failed reason is %m\n",errno);
        return;
    }
#endif
    isRun = false;
}

void SemidriveLinkThread::start() {
    isRun = true;
    mWorkThread = std::thread(&SemidriveLinkThread::run,this);
}

void SemidriveLinkThread::stop() {
    isRun = false;
    mWorkThread.join();
}

#ifdef USE_SOCKET
int clientfd;
#endif

void SemidriveLinkThread::run(void) {
    epoll_event events[MAX_EPOLL_EVENTS];
    //unsigned char buff[BUFFER_SIZE];
    CanData data;


    while(isRun) {
//        int eventsize = epoll_wait(mEpollFd, events, MAX_EPOLL_EVENTS, -1);
//        printf("start run1,i get data \n");
//        for(int i = 0; i < eventsize; ++i){
//            int fd = events[i].data.fd;
#ifdef USE_SOCKET
            if(fd == mDeviceFd) {
                struct sockaddr client_address;
                socklen_t length = 0;
                printf("one client connect \n");
                int connfd = accept(mDeviceFd, (struct sockaddr *)&client_address, &length);
                struct epoll_event connectEvent;
                connectEvent.events = EPOLLIN|EPOLLET;
                connectEvent.data.fd = connfd;
                epoll_ctl(mEpollFd, EPOLL_CTL_ADD, connfd, &connectEvent);
                clientfd = connfd;
                continue;
            }
#endif
//            uint32_t recvEvents = events[i].events;
            //printf("recvEvents is %d \n",recvEvents);
//            if((recvEvents &EPOLLRDHUP) != 0) {
                //TODO(disconnect msg with data???)
                //LOGD("client fd[%ld] disconnect \n",recvEvents);
//                continue;
//            }
            if(mDeviceFd == -1) {
                LOGE("mDeviceFd is %d \n",mDeviceFd);
                return;
            }
            int len = read(mDeviceFd,&data,sizeof(CanData));
            if(len> 0 && this->listener != nullptr) {
                listener->onData(data.payload,len);
            }
        }
//    }
}
