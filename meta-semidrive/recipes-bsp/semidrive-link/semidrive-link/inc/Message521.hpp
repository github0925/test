#ifndef __SEMIDRIVE_MESSAGE_521_HPP__
#define  __SEMIDRIVE_MESSAGE_521_HPP__
#include <stdint.h>
#include <stdio.h>
#include "Message.hpp"
#define SEAT_AHEAD                                           1
#define SEAT_BACK                                              1<<1
#define SEAT_UP                                                    2<<1
#define SEAT_DOWN                                            3<<1
#define SEAT_LONG                                              4<<1
#define SEAT_SHORT                                            5<<1
#define BACK_SEAT_BOW                                  6<<1
#define BACK_SEAT_BEND                                7<<1
#define SEAT_BOW                                                8<<1
#define SEAT_BEND                                              9<<1

class Message_521:public Message {
public:
    uint16_t driverSeat;
    uint8_t hazardLights;
    uint8_t rightRearSeatPanelPower;
    uint8_t leftRearSeatPanelPower;
    uint8_t airConditionPanelPower;
    uint8_t frontPassengerSeatPanelPower;
    uint8_t centerPanelPower;
    uint8_t instrumentPanelPower;
    uint16_t passengreSeat;
    uint8_t flidarUpAndDown;
    uint8_t rildarUpAndDown;
    uint8_t gearUpAndDown;
    void reset();

    void parse(unsigned char* buff,int len) ; 
    int compose(unsigned char *buff);
    bool onUpdate(int32_t event,int32_t value);
    bool onUpdate(Message *data,MessageUpdateListener *l) ;
};


#endif