#include "Message521.hpp"
#include "Message.hpp"

void Message_521::parse(unsigned char* buff,int len) {
    //driverSeat
    this->driverSeat =  buff[0]|(buff[1]<<8);
    this->hazardLights = (buff[3] &BIT7)>>7;
    this->rightRearSeatPanelPower = (buff[3] &BIT5)>>5;
    this->leftRearSeatPanelPower = (buff[3] &BIT4)>>4;
    this->airConditionPanelPower = (buff[3] &BIT3)>>3;
    this->frontPassengerSeatPanelPower = (buff[3] &BIT2)>>2;
    this->centerPanelPower = (buff[3] &BIT1)>>1;
    this->instrumentPanelPower = (buff[3] &BIT0);

    //passengerSeat
    this->passengreSeat =  buff[6]|((buff[5]<<8));
    this->flidarUpAndDown = (buff[7]&BIT2)>>2;
    this->rildarUpAndDown = (buff[7]&BIT1)>>1;
    this->gearUpAndDown = (buff[7]&BIT0);
}

bool Message_521::onUpdate(Message *data,MessageUpdateListener *l) {
    Message_521 *orig = (Message_521 *)data;
    if(orig->driverSeat != this->driverSeat) {
        l->onChanged(EVENT_DRIVER_SEAT,orig->driverSeat);
        this->driverSeat = orig->driverSeat;
    }

    if(orig->hazardLights != this->hazardLights) {
        l->onChanged(EVENT_HAZARD_LIGHTS_2,orig->hazardLights);
        this->hazardLights = orig->hazardLights;
    }

    if(orig->rightRearSeatPanelPower != this->rightRearSeatPanelPower) {
        l->onChanged(EVENT_RIGHT_REAR_SEAT_PANEL_POWER,orig->rightRearSeatPanelPower);
        this->rightRearSeatPanelPower = orig->rightRearSeatPanelPower;
    }

    if(orig->leftRearSeatPanelPower != this->leftRearSeatPanelPower) {
        l->onChanged(EVENT_LEFT_REAR_SEAT_PANEL_POWER,orig->leftRearSeatPanelPower);
        this->leftRearSeatPanelPower = orig->leftRearSeatPanelPower;
    }

    if(orig->airConditionPanelPower != this->airConditionPanelPower) {
        l->onChanged(EVENT_AIR_POWER,orig->airConditionPanelPower);
        this->airConditionPanelPower = orig->airConditionPanelPower;
    }

    if(orig->frontPassengerSeatPanelPower != this->frontPassengerSeatPanelPower) {
        l->onChanged(EVENT_FRONT_PASSENGER_SEAT_PANEL_POWER,orig->airConditionPanelPower);
        this->frontPassengerSeatPanelPower = orig->frontPassengerSeatPanelPower;
    }

     if(orig->centerPanelPower != this->centerPanelPower) {
        l->onChanged(EVENT_CENTER_PANEL_POWER,orig->centerPanelPower);
        this->centerPanelPower = orig->centerPanelPower;
    }

    if(orig->instrumentPanelPower != this->instrumentPanelPower) {
        l->onChanged(EVENT_INSTRUMENT_PANEL_POWER,orig->instrumentPanelPower);
        this->instrumentPanelPower = orig->instrumentPanelPower;
    }

    if(orig->passengreSeat != this->passengreSeat) {
        l->onChanged(EVENT_PASSENGER_SEAT,orig->passengreSeat);
        this->passengreSeat = orig->passengreSeat;
    }

    if(orig->flidarUpAndDown != this->flidarUpAndDown) {
        l->onChanged(EVENT_FLIDAR_UP_AND_DOWN,orig->flidarUpAndDown);
        this->flidarUpAndDown = orig->flidarUpAndDown;
    }

    if(orig->rildarUpAndDown != this->rildarUpAndDown) {
        l->onChanged(EVENT_RLIDAR_UP_AND_DOWN,orig->rildarUpAndDown);
        this->rildarUpAndDown = orig->rildarUpAndDown;
    }

    if(orig->gearUpAndDown != this->gearUpAndDown) {
        l->onChanged(EVENT_GEAR_UP_AND_DOWN,orig->gearUpAndDown);
        this->gearUpAndDown = orig->gearUpAndDown;
    }
    return true;
}

bool Message_521::onUpdate(int32_t event,int32_t value) {
    switch(event) {
        case EVENT_DRIVER_SEAT:
        this->driverSeat = value;
        return true;

        case EVENT_HAZARD_LIGHTS_2:
        this->hazardLights = value;
        return true;

        case EVENT_RIGHT_REAR_SEAT_PANEL_POWER:
        this->rightRearSeatPanelPower = value;
        return true;

        case EVENT_LEFT_REAR_SEAT_PANEL_POWER:
        this->leftRearSeatPanelPower = value;
        return true;

        case EVENT_AIR_POWER:
        this->airConditionPanelPower = value;
        return true;

        case EVENT_FRONT_PASSENGER_SEAT_PANEL_POWER:
        this->frontPassengerSeatPanelPower = value;
        return true;

        case EVENT_CENTER_PANEL_POWER:
        this->centerPanelPower = value;
        return true;

        case EVENT_INSTRUMENT_PANEL_POWER:
        this->instrumentPanelPower = value;
        return true;
    }

    return false;
}

int Message_521::compose(unsigned char *buff) {
    buff[0] = ((this->driverSeat>>2)&(0xF));
    buff[1] = ((this->driverSeat &(BIT0|BIT1))<<6);
    buff[2] = 0;
    buff[3] = ((this->instrumentPanelPower &BIT0)
                        |((this->centerPanelPower<<1)&BIT1)
                        |((this->frontPassengerSeatPanelPower<<2)&BIT2)
                        |((this->airConditionPanelPower<<3)&BIT3)
                        |((this->leftRearSeatPanelPower<<4)&BIT4)
                        |((this->rightRearSeatPanelPower<<5)&BIT5)
                        |((this->hazardLights<<7)&BIT7));

    buff[4] = 0;
    buff[5] = ((this->passengreSeat>>2)&(0xF));
    buff[6] = ((this->passengreSeat &(BIT0|BIT1))<<6);
    buff[7] = ((this->gearUpAndDown &BIT0)
                    |((this->rildarUpAndDown<<1)&BIT1)
                    |((this->flidarUpAndDown<<2)&BIT2));
    return 8;
}

void Message_521::reset() {
    this->driverSeat =  0;

    this->hazardLights = 0;
    this->rightRearSeatPanelPower = 0;
    this->leftRearSeatPanelPower = 0;
    this->airConditionPanelPower = 0;
    this->frontPassengerSeatPanelPower = 0;
    this->centerPanelPower = 0;
    this->instrumentPanelPower = 0;
    this->passengreSeat =  0;
    this->flidarUpAndDown = 0;
    this->rildarUpAndDown = 0;
    this->gearUpAndDown = 0;
}
