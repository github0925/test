#ifndef __SEMIDRIVE_MESSAGE_512_HPP__
#define  __SEMIDRIVE_MESSAGE_512_HPP__
#include <stdint.h>
#include <stdio.h>

#include "Message.hpp"

#define FRONT_FOG_LIGHT 0x2
#define BACK_FOG_LIGHT 1

class  Message_512:public Message {
public:
    uint8_t enterBtn;
    uint8_t prvBtn;
    uint8_t nextBtn;
    uint8_t menuBtn;
    uint8_t volumnSubBtn;
    uint8_t volumnPlusBtn;
    uint8_t volumnSpeechCtrl;
    uint8_t speedDown;
    uint8_t speedUpAndRecover;
    uint8_t followingDistenceSub;
    uint8_t followingDistancePlus;
    uint8_t cruiseOpen;
    uint8_t switchFunPrev;
    uint8_t switchFunNext;
    uint8_t hazardLights;
    uint8_t turnLeftRight;
    uint8_t dippedHeadingBtn;
    uint8_t hightBeanLightBtn;
    uint8_t foglightBtn;
    uint32_t steeringWheelAngle;

    void reset();
public:
    void parse(unsigned char* buff,int len) ; 
    int compose(unsigned char *buff);
    bool onUpdate(int32_t event,int32_t value);
    bool onUpdate(Message *data,MessageUpdateListener *l) ;
};

#endif