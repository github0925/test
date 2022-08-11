#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "CanData.hpp"
#include "SemidriveLink.hpp"

#define CANFD "/dev/vircan"

#ifdef USE_SOCKET
extern int clientfd;
#endif

SemidriveLink::SemidriveLink(SemidriveLinkListener *l) {
    //int fd = open(CANFD,O_RDONLY);
    m512data.reset();
    openFd(l->getType());
    mThread = new SemidriveLinkThread(mCanfd,this);
    this->listener = l;
    mThread->start();
}

void SemidriveLink::openFd(int type) {
    switch(type) {
        case LINK_IVI:
            //TODO
            mCanfd = open(CANFD,O_RDWR);
			printf("open canfd is %d \n",mCanfd);
        break;

        case LINK_CLUSTER:
            //TODO
            mCanfd = open(CANFD,O_RDWR);
        break;

        case LINK_CONTROL_PANEL:
            //TODO
            mCanfd = open(CANFD,O_RDWR);
        break;

        case LINK_TEST: {
            printf("this is a test \n");
            struct sockaddr_in serverAddr;
            serverAddr.sin_family = PF_INET;
            serverAddr.sin_port = htons(1315);
            serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            int opt = 1;

            if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
                printf("fail1 \n");
            }

            if(bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
                printf("fail2 \n");
            }

            int ret = listen(sock, 16);
            if(ret < 0) {
                printf("fail3 \n");
            }

            mCanfd = sock;
        }
        break;
    }
}

void SemidriveLink::onData(unsigned char *buff,int len) {
    uint64_t value = buff[3]<<24|buff[2]<<16|buff[1]<<8|buff[0];
    //printf("value is %p \n",value);
    uint32_t id = (value&0x7FF);

    switch(id) {
        case 0x512: {
            Message_512 msg;
            unsigned char *data = &buff[5];
            msg.parse(data,len);
            m512data.onUpdate(&msg,this);
            //TOTEST
#ifdef MESSAGE_DEBUG
            unsigned char buff[32];
            buff[3] = (id>>24&0XFF);
            buff[2] = (id>>16&0XFF);
            buff[1]= (id>>8&0XFF);
            buff[0] = (id&0XFF);
            buff[4] = m512data.compose((unsigned char * )&buff[5]);
            printf("compose buff[0]is %x \n",buff[0]);
            printf("compose buff[1]is %x\n",buff[1]);
            printf("compose buff[2]is %x\n",buff[2]);
            printf("compose buff[3]is %x\n",buff[3]);
            printf("compose buff[4]is %x\n",buff[4]);
            printf("compose buff[5]is %x\n",buff[5]);
            printf("compose buff[6]is %x\n",buff[6]);
            printf("compose buff[7]is %x\n",buff[7]);
            printf("compose buff[8]is %x\n",buff[8]);
            printf("compose buff[9]is %x\n",buff[9]);
#endif
        }
        break;

        case 0x514:{
            Message_514 msg;
            unsigned char *data = &buff[5];
            msg.parse(data,len);
            m514data.onUpdate(&msg,this);
            //TOTEST
#ifdef MESSAGE_DEBUG
            unsigned char buff[32];
            buff[3] = (id>>24&0XFF);
            buff[2] = (id>>16&0XFF);
            buff[1]= (id>>8&0XFF);
            buff[0] = (id&0XFF);
            buff[4] = m514data.compose((unsigned char * )&buff[5]);
            printf("compose buff[0]is %x \n",buff[0]);
            printf("compose buff[1]is %x\n",buff[1]);
            printf("compose buff[2]is %x\n",buff[2]);
            printf("compose buff[3]is %x\n",buff[3]);
            printf("compose buff[4]is %x\n",buff[4]);
            printf("compose buff[5]is %x\n",buff[5]);
            printf("compose buff[6]is %x\n",buff[6]);
            printf("compose buff[7]is %x\n",buff[7]);
            printf("compose buff[8]is %x\n",buff[8]);
            printf("compose buff[9]is %x\n",buff[9]);
#endif
        }
        break;

        case 0x513:{
            Message_513 msg;
            unsigned char *data = &buff[5];
            msg.parse(data,len);
            m513data.onUpdate(&msg,this);
            //TOTEST
#ifdef MESSAGE_DEBUG
            unsigned char buff[32];
            buff[3] = (id>>24&0XFF);
            buff[2] = (id>>16&0XFF);
            buff[1]= (id>>8&0XFF);
            buff[0] = (id&0XFF);
            buff[4] = m513data.compose((unsigned char * )&buff[5]);
            printf("compose buff[0]is %x \n",buff[0]);
            printf("compose buff[1]is %x\n",buff[1]);
            printf("compose buff[2]is %x\n",buff[2]);
            printf("compose buff[3]is %x\n",buff[3]);
            printf("compose buff[4]is %x\n",buff[4]);
            printf("compose buff[5]is %x\n",buff[5]);
            printf("compose buff[6]is %x\n",buff[6]);
            printf("compose buff[7]is %x\n",buff[7]);
            printf("compose buff[8]is %x\n",buff[8]);
            printf("compose buff[9]is %x\n",buff[9]);
            printf("compose buff[10]is %x\n",buff[10]);
            printf("compose buff[11]is %x\n",buff[11]);
            printf("compose buff[12]is %x\n",buff[12]);
#endif
        }
        break;

        case 0x521:{
            Message_521 msg;
            unsigned char *data = &buff[5];
            msg.parse(data,len);
            m521data.onUpdate(&msg,this);
            //TOTEST
#ifdef MESSAGE_DEBUG
            unsigned char buff[32];
            buff[3] = (id>>24&0XFF);
            buff[2] = (id>>16&0XFF);
            buff[1]= (id>>8&0XFF);
            buff[0] = (id&0XFF);
            buff[4] = m521data.compose((unsigned char * )&buff[5]);
            printf("compose buff[0]is %x \n",buff[0]);
            printf("compose buff[1]is %x\n",buff[1]);
            printf("compose buff[2]is %x\n",buff[2]);
            printf("compose buff[3]is %x\n",buff[3]);
            printf("compose buff[4]is %x\n",buff[4]);
            printf("compose buff[5]is %x\n",buff[5]);
            printf("compose buff[6]is %x\n",buff[6]);
            printf("compose buff[7]is %x\n",buff[7]);
            printf("compose buff[8]is %x\n",buff[8]);
            printf("compose buff[9]is %x\n",buff[9]);
            printf("compose buff[10]is %x\n",buff[10]);
            printf("compose buff[11]is %x\n",buff[11]);
            printf("compose buff[12]is %x\n",buff[12]);
#endif
        }

        break;
    }
}

void SemidriveLink::updateEvent(std::map<int,int> &event) {
    mUpdateMutex.lock();
    std::map<int, int>::iterator iter = event.begin();
    while(iter != event.end()) {
        updateEventLocked(iter->first,iter->second);
        //printf("event is %d,value is %d \n",iter->first,iter->second);
        iter++;
    }
    mUpdateMutex.unlock();
}

void SemidriveLink::updateEvent(int32_t event,int32_t value) {
    mUpdateMutex.lock();
    updateEventLocked(event,value);
    mUpdateMutex.unlock();
}

void SemidriveLink::updateEventLocked(int32_t event,int32_t value) {
    //unsigned char buff[32];
    CanData data;
    data.bindid = 0; //TODO
	  data.len = 13;//TODO
    unsigned char *buff = data.payload;
    printf("mCanfd is %d \n",mCanfd);
#ifdef USE_SOCKET
    int fd =  clientfd;
#else
    int fd = mCanfd;
#endif

    if(m512data.onUpdate(event,value)) {
        uint32_t id= 0x512;
        buff[3] = (id>>24&0XFF);
        buff[2] = (id>>16&0XFF);
        buff[1]= (id>>8&0XFF);
        buff[0] = (id&0XFF);
        buff[4] = m512data.compose(&buff[5]);
        write(fd,&data,sizeof(CanData));
        return;
    }

    if(m513data.onUpdate(event,value)) {
        uint32_t id= 0x513;
        buff[3] = (id>>24&0XFF);
        buff[2] = (id>>16&0XFF);
        buff[1]= (id>>8&0XFF);
        buff[0] = (id&0XFF);
        buff[4]  = m513data.compose(&buff[5]);
        write(fd,&data,sizeof(CanData));
        return;
    }

    if(m514data.onUpdate(event,value)) {
        uint32_t id= 0x514;
        buff[3] = (id>>24&0XFF);
        buff[2] = (id>>16&0XFF);
        buff[1]= (id>>8&0XFF);
        buff[0] = (id&0XFF);
        buff[4] = m514data.compose(&buff[5]);
        write(fd,&data,sizeof(CanData));
        return;
    }

    if(m521data.onUpdate(event,value)) {
        uint32_t id= 0x532;
        buff[3] = (id>>24&0XFF);
        buff[2] = (id>>16&0XFF);
        buff[1]= (id>>8&0XFF);
        buff[0] = (id&0XFF);
        buff[4]  = m521data.compose(&buff[5]);
        write(fd,&data,sizeof(CanData));
        return;
    }

}

void SemidriveLink::onChanged(int event,int value)  {
#ifdef MESSAGE_DEBUG
    printf("onChanged event is %d ,value is %d \n",event,value);
#endif
    if(this->listener != nullptr) {
        listener->onEvent(event,value);
    }
}
