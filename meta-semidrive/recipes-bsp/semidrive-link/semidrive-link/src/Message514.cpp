#include "Message514.hpp"
#include "Message.hpp"

void Message_514::parse(unsigned char* buff,int len) {
    this->frontAirCondition = (buff[0] &BIT5)>>5;
    this->sync = (buff[0] &BIT4)>>4;
    this->recircudatingAi = (buff[0] &BIT3)>>3;
    this->frontWindshieldGlassWarm = (buff[0] &BIT2)>>2;
    this->rearWindshieldGlassWarm = (buff[0] &BIT1)>>1;
    this->rearAirConditioning = (buff[0] &BIT0);
    this->gears = (buff[1] &(BIT0|BIT1|BIT2));
    this->rotaryKnobRight = (buff[2] &(BIT0|BIT1));
    this->enterBtn = ( buff[2]&BIT2)>>2;
    this->rotaryKnobLeft = ((buff[2]>>3) &(BIT0|BIT1));
}

int Message_514::compose(unsigned char *buff) {
    buff[0] = ((this->rearAirConditioning&BIT0)
                    |(this->rearWindshieldGlassWarm<<1)
                    |(this->frontWindshieldGlassWarm<<2)
                    |(this->recircudatingAi<<3)
                    |(this->sync<<4)
                    |(this->frontAirCondition<<5));
    buff[1] = this->gears;
    buff[2] = ((this->rotaryKnobRight &(BIT0|BIT1))
                    |((this->enterBtn<<2))
                    |((this->rotaryKnobLeft<<3)&(BIT3|BIT4)));
    buff[3] = 0;
    buff[4] = 0;
    buff[5] = 0;
    buff[6] = 0;
    buff[7] = 0;
    return 8;
}

void Message_514::reset() {
    this->frontAirCondition = 0;
    this->sync = 0;
    this->recircudatingAi = 0;
    this->frontWindshieldGlassWarm = 0;
    this->rearWindshieldGlassWarm = 0;
    this->rearAirConditioning = 0;
    this->gears = 0;
    this->rotaryKnobRight = 0;
    this->enterBtn = 0;
    this->rotaryKnobRight = 0;
}

bool Message_514::onUpdate(int32_t event,int32_t value) {
    switch(event) {
        case EVENT_FRONT_AIR_CONDITION:
            this->frontAirCondition = value;
            return true;

        case EVENT_SYNC:
            this->sync = value;
            return true;

        case EVENT_RECIRCUDATING_AI:
            this->recircudatingAi = value;
            return true;

        case EVENT_FRONT_WINSHIELD_GLASS_WARM:
            this->frontWindshieldGlassWarm = value;
            return  true;

        case EVENT_REAR_WINSHIELD_GLASS_WARM:
            this->rearWindshieldGlassWarm = value;
            return true;

        case EVENT_REAR_AIR_CONDITIONING:
            this->rearAirConditioning = value;
            return true;

        case EVENT_GEARS:
            this->gears = value;
            return true;

        case EVENT_ROTARY_KNOB_LEFT:
            this->rotaryKnobLeft = value;
            return true;

        case EVENT_ENTER_BTN_2:
            this->enterBtn = value;
            return true;

        case EVENT_ROTARY_KNOB_RIGHT:
            this->rotaryKnobRight = value;
            return true;
    }
    return false;
}

bool Message_514::onUpdate(Message *data,MessageUpdateListener *l)  {
    Message_514 *orig = (Message_514 *)data;
    if(orig->frontAirCondition != this->frontAirCondition) {
        l->onChanged(EVENT_FRONT_AIR_CONDITION,orig->frontAirCondition);
        this->frontAirCondition = orig->frontAirCondition;
    }

    if(orig->sync != this->sync) {
        l->onChanged(EVENT_SYNC,orig->sync);
        this->sync = orig->sync;
    }

    if(orig->recircudatingAi != this->recircudatingAi) {
        l->onChanged(EVENT_RECIRCUDATING_AI,orig->recircudatingAi);
        this->recircudatingAi = orig->recircudatingAi;
    }

    if(orig->frontWindshieldGlassWarm != this->frontWindshieldGlassWarm) {
        l->onChanged(EVENT_FRONT_WINSHIELD_GLASS_WARM,orig->frontWindshieldGlassWarm);
        this->frontWindshieldGlassWarm = orig->frontWindshieldGlassWarm;
    }

    if(orig->rearWindshieldGlassWarm != this->rearWindshieldGlassWarm) {
        l->onChanged(EVENT_REAR_WINSHIELD_GLASS_WARM,orig->rearWindshieldGlassWarm);
        this->rearWindshieldGlassWarm = orig->rearWindshieldGlassWarm;
    }

    if(orig->rearAirConditioning != this->rearAirConditioning) {
        l->onChanged(EVENT_REAR_AIR_CONDITIONING,orig->rearAirConditioning);
        this->rearAirConditioning = orig->rearAirConditioning;
    }

    if(orig->gears != this->gears) {
        l->onChanged(EVENT_GEARS,orig->gears);
        this->gears = orig->gears;
    }

    if(orig->rotaryKnobRight != this->rotaryKnobRight) {
        l->onChanged(EVENT_ROTARY_KNOB_RIGHT,orig->rotaryKnobRight);
        this->rotaryKnobRight = orig->rotaryKnobRight;
    }

    if(orig->enterBtn != this->enterBtn) {
        l->onChanged(EVENT_ENTER_BTN_2,orig->enterBtn);
        this->enterBtn = orig->enterBtn;
    }

    if(orig->rotaryKnobLeft!= this->rotaryKnobLeft) {
        l->onChanged(EVENT_ROTARY_KNOB_LEFT,orig->rotaryKnobLeft);
        this->rotaryKnobLeft = orig->rotaryKnobLeft;
    }

    return true;
}
