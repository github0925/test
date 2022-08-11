#ifndef __SEMIDRIVE_MESSAGE_514_HPP__
#define  __SEMIDRIVE_MESSAGE_514_HPP__
#include <stdint.h>
#include <stdio.h>

#include "Message.hpp"

#define GEAR_P 0x1
#define GEAR_R 0x2
#define GEAR_N 0x3
#define GEAR_D 0x4

class Message_514:public Message{
public:
    uint8_t frontAirCondition;
    uint8_t sync;
    uint8_t recircudatingAi;
    uint8_t frontWindshieldGlassWarm;
    uint8_t rearWindshieldGlassWarm;
    uint8_t rearAirConditioning;
    uint8_t gears;
    uint8_t rotaryKnobLeft;
    uint8_t enterBtn;
    uint8_t rotaryKnobRight;
    void reset();
    
public:
    void parse(unsigned char* buff,int len) ; 
    int compose(unsigned char *buff);
    bool onUpdate(int32_t event,int32_t value);
    bool onUpdate(Message *data,MessageUpdateListener *l) ;
};

#endif