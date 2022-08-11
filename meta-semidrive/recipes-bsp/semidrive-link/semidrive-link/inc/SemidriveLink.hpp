#ifndef __SEMIDRIVE_LINK_HPP__
#define __SEMIDRIVE_LINK_HPP__

#include <map>
#include <mutex>

#include "SemidriveLinkThread.hpp"
#include "Message.hpp"
#include "Log.hpp"
#include "Message512.hpp"
#include "Message514.hpp"
#include "Message521.hpp"
#include "Message513.hpp"


enum LinkType {
    LINK_IVI,
    LINK_CLUSTER,
    LINK_CONTROL_PANEL,
    LINK_TEST,
    LINK_MAX_TYPE
};

class SemidriveLinkListener {
public:
    virtual void onEvent(int32_t event,int32_t value) = 0;
    virtual int getType() = 0;
    virtual ~SemidriveLinkListener(){}
};

class SemidriveLink :public SemidriveLinkThreadListener,public MessageUpdateListener{
public:
    SemidriveLink(SemidriveLinkListener *l);
    void updateEvent(std::map<int32_t,int32_t> &);
    void updateEvent(int32_t,int32_t);
    
private:
    void onData(unsigned char *,int);
    void onChanged(int32_t event,int32_t value) ;
    void openFd(int);
    void updateEventLocked(int32_t,int32_t);
    SemidriveLinkThread *mThread;
    int mCanfd;
    SemidriveLinkListener *listener;
    std::mutex mUpdateMutex;

    Message_512 m512data;
    Message_514 m514data;
    Message_521 m521data;
    Message_513 m513data;
};

#endif