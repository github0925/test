#ifndef __SEMIDRIVE_MESSAGE_513_HPP__
#define  __SEMIDRIVE_MESSAGE_513_HPP__
#include <stdint.h>
#include <stdio.h>
#include "Message.hpp"

class Message_513:public Message {
public:
    uint8_t temperature1;
    uint8_t windspeed1;
    uint8_t mode1;
    uint8_t rgb_r1;
    uint8_t rgb_g1;
    uint8_t rgb_b1;
    uint8_t temperature2;
    uint8_t windspeed2;
    uint8_t mode2;
    uint8_t rgb_r2;
    uint8_t rgb_g2;
    uint8_t rgb_b2;

    void reset();

    void parse(unsigned char* buff,int len) ; 
    int compose(unsigned char *buff);
    bool onUpdate(int32_t event,int32_t value);
    bool onUpdate(Message *data,MessageUpdateListener *l) ;
};


#endif