#include "Message512.hpp"
#include "Message.hpp"

void Message_512::parse(unsigned char* buff,int len) {
    this->enterBtn = (buff[0]&BIT6)>>6;
    this->prvBtn = (buff[0]&BIT5)>>5;
    this->nextBtn = (buff[0]&BIT4)>>4;
    this->menuBtn = (buff[0]&BIT3)>>3;
    this->volumnSubBtn = (buff[0]&BIT2)>>2;
    this->volumnPlusBtn = (buff[0]&BIT1)>>1;
    this->volumnSpeechCtrl = (buff[0]&BIT0);
    this->speedDown = (buff[1]&BIT6)>>6;
    this->speedUpAndRecover = (buff[1]&BIT5)>>5;
    this->followingDistenceSub = (buff[1]&BIT4)>>4;
    this->followingDistancePlus = (buff[1]&BIT3)>>3;
    this->cruiseOpen = (buff[1]&BIT2)>>2;
    this->switchFunPrev = (buff[1]&BIT1)>>1;
    this->switchFunNext = (buff[1]&BIT0);
    this->hazardLights = (buff[2]&BIT6)>>6;
    this->turnLeftRight = ((buff[2] >> 4) & 0x3);
    this->dippedHeadingBtn = (buff[2]&BIT3) >>3;
    this->hightBeanLightBtn = (buff[2]&BIT2)>>2;
    this->foglightBtn = (buff[2] &0x3);
    this->steeringWheelAngle = ( buff[4] <<8|buff[5]);
    printf("buff[4] is %x,buff[5] is %x \n",buff[4],buff[5]);
    printf("steering whellangle is %x \n",steeringWheelAngle);
}


void Message_512::reset() {
    this->enterBtn = 0;
    this->prvBtn = 0;
    this->nextBtn = 0;
    this->menuBtn = 0;
    this->volumnSubBtn = 0;
    this->volumnPlusBtn = 0;
    this->volumnSpeechCtrl = 0;
    this->speedDown = 0;
    this->speedUpAndRecover = 0;
    this->followingDistenceSub = 0;
    this->followingDistancePlus = 0;
    this->cruiseOpen = 0;
    this->switchFunPrev = 0;
    this->switchFunNext = 0;
    this->hazardLights = 0;
    this->turnLeftRight = 0;
    this->foglightBtn = 0;
    this->steeringWheelAngle = 0;
}

bool Message_512::onUpdate(Message *dest,MessageUpdateListener *l) {
    Message_512 *destination = (Message_512 *)dest;
    if(this->enterBtn != destination->enterBtn) {
        l->onChanged(EVENT_ENTER_BTN,destination->enterBtn);
        this->enterBtn = destination->enterBtn;
    }

    if(this->prvBtn != destination->prvBtn) {
        l->onChanged(EVENT_PRV_BTN,destination->prvBtn);
        this->prvBtn = destination->prvBtn;
    }

    if(this->nextBtn != destination->nextBtn) {
        l->onChanged(EVENT_NEXT_BTN,destination->nextBtn);
        this->nextBtn = destination->nextBtn;
    }

    if(this->menuBtn != destination->menuBtn) {
        l->onChanged(EVENT_MENU_BTN,destination->menuBtn);
        this->menuBtn = destination->menuBtn;
    }

    if(this->volumnSubBtn != destination->volumnSubBtn) {
        l->onChanged(EVENT_VOLUMN_SUB_BTN,destination->volumnSubBtn);
        this->volumnSubBtn = destination->volumnSubBtn;
    }

     if(this->volumnPlusBtn != destination->volumnPlusBtn) {
        l->onChanged(EVENT_VOLUMN_PLUS_BTN,destination->volumnPlusBtn);
        this->volumnPlusBtn = destination->volumnPlusBtn;
    }

    if(this->volumnSpeechCtrl != destination->volumnSpeechCtrl) {
        l->onChanged(EVENT_VOLUMN_SPEECH_CTRL,destination->volumnSpeechCtrl);
        this->volumnSpeechCtrl = destination->volumnSpeechCtrl ;
    }

    if(this->speedDown != destination->speedDown) {
        l->onChanged(EVENT_SPEED_DOWN,destination->speedDown);
        this->speedDown = destination->speedDown;
    }

    if(this->speedUpAndRecover != destination->speedUpAndRecover) {
        l->onChanged(EVENT_SPEED_UP_AND_RECOVER,destination->speedUpAndRecover);
        this->speedUpAndRecover = destination->speedUpAndRecover ;
    }

    if(this->followingDistenceSub != destination->followingDistenceSub) {
        l->onChanged(EVENT_FOLLOWING_DISTENCE_SUB,destination->followingDistenceSub);
        this->followingDistenceSub = destination->followingDistenceSub;
    }

    if(this->followingDistancePlus != destination->followingDistancePlus) {
        l->onChanged(EVENT_FOLLOWING_DISTENCE_PLUS,destination->followingDistancePlus);
        this->followingDistancePlus = destination->followingDistancePlus;
    }

     if(this->cruiseOpen != destination->cruiseOpen) {
        l->onChanged(EVENT_CRUISE_OPEN,destination->cruiseOpen);
        this->cruiseOpen = destination->cruiseOpen;
    }

    if(this->switchFunPrev != destination->switchFunPrev) {
        l->onChanged(EVENT_SWITCH_FUN_PREV,destination->switchFunPrev);
        this->switchFunPrev = destination->switchFunPrev;
    }

    if(this->switchFunNext != destination->switchFunNext) {
        l->onChanged(EVENT_SWITCH_FUN_NEXT,destination->switchFunNext);
        this->switchFunNext = destination->switchFunNext;
    }

    if(this->hazardLights != destination->hazardLights) {
        l->onChanged(EVENT_HAZARD_LIGHTS,destination->hazardLights);
        this->hazardLights = destination->hazardLights;
    }

    if(this->turnLeftRight != destination->turnLeftRight) {
        l->onChanged(EVENT_TURN_LEFT_RIGHT,destination->turnLeftRight);
        this->turnLeftRight = destination->turnLeftRight;
    }

    if(this->dippedHeadingBtn != destination->dippedHeadingBtn) {
        l->onChanged(EVENT_DIPPED_HEADING_BTN,destination->dippedHeadingBtn);
        this->dippedHeadingBtn = destination->dippedHeadingBtn;
    }

    if(this->hightBeanLightBtn != destination->hightBeanLightBtn) {
        l->onChanged(EVENT_HIGHT_BEAN_LIGHT_BTN,destination->hightBeanLightBtn);
        this->hightBeanLightBtn = destination->hightBeanLightBtn;
    }

    if(this->foglightBtn != destination->foglightBtn) {
        l->onChanged(EVENT_FOG_LIGHT_BTN,destination->foglightBtn);
        this->foglightBtn = destination->foglightBtn;
    }

    if(this->steeringWheelAngle != destination->steeringWheelAngle) {
        l->onChanged(EVENT_STEERING_WHEEL_ANGLE,destination->steeringWheelAngle);
        this->steeringWheelAngle = destination->steeringWheelAngle ;
    }

    return true;
}

bool Message_512::onUpdate(int32_t event,int32_t value) {
    switch(event) {
        case EVENT_ENTER_BTN:
            this->enterBtn = value;
            return true;

        case EVENT_PRV_BTN:
            this->prvBtn = value;
            return true;

        case EVENT_NEXT_BTN:
            this->nextBtn = value;
            return true;

        case EVENT_MENU_BTN:
            this->menuBtn = value;
            return true;

        case EVENT_VOLUMN_SUB_BTN:
            this->volumnSubBtn = value;
            return true;

        case EVENT_VOLUMN_PLUS_BTN:
            this->volumnPlusBtn = value;
            return true;

        case EVENT_VOLUMN_SPEECH_CTRL:
            this->volumnSpeechCtrl = value;
            return true;

        case EVENT_SPEED_DOWN:
            this->speedDown = value;
            return true;

        case EVENT_SPEED_UP_AND_RECOVER:
            this->speedUpAndRecover = value;
            return true;

        case EVENT_FOLLOWING_DISTENCE_SUB:
            this->followingDistenceSub = value;
            return true;

        case EVENT_FOLLOWING_DISTENCE_PLUS:
            this->followingDistancePlus = value;
            return true;

        case EVENT_CRUISE_OPEN:
            this->cruiseOpen = value;
            return true;

        case EVENT_SWITCH_FUN_PREV:
            this->switchFunPrev = value;
            return true;

        case EVENT_SWITCH_FUN_NEXT:
            this->switchFunNext = value;
            return true;

        case EVENT_HAZARD_LIGHTS:
            this->hazardLights = value;
            return true;

        case EVENT_TURN_LEFT_RIGHT:
            this->turnLeftRight = value;
            return true;

        case EVENT_DIPPED_HEADING_BTN:
            this->dippedHeadingBtn = value;
            return true;

        case EVENT_HIGHT_BEAN_LIGHT_BTN:
            this->hightBeanLightBtn = value;
            return true;

        case EVENT_FOG_LIGHT_BTN:
            this->foglightBtn = value;
            return true;

        case EVENT_STEERING_WHEEL_ANGLE:
            this->steeringWheelAngle = value;
            return true;
    }

    return false;
}

int Message_512::compose(unsigned char *buff) {
    buff[0] = ((this->volumnSpeechCtrl)
                    |(this->volumnPlusBtn<<1)
                    |(this->volumnSubBtn<<2)
                    |(this->menuBtn<<3)
                    |(this->nextBtn<<4)
                    |(this->prvBtn<<5)
                    |(this->enterBtn<<6));

    buff[1] = ((this->switchFunNext &BIT0)
                    |((this->switchFunNext<<1) &BIT1)
                    |((this->cruiseOpen<<2) &BIT2)
                    |((this->followingDistancePlus<<3)&BIT3)
                    |((this->followingDistenceSub<<4)&BIT4)
                    |((this->speedUpAndRecover<<5)&BIT5)
                    |((this->speedDown<<6)&BIT6));

    buff[2] =  ((this->foglightBtn &(BIT0|BIT1))
                    |((this->hightBeanLightBtn<<2) &BIT2)
                    |((this->dippedHeadingBtn<<3) &BIT3)
                    |((this->turnLeftRight<<4)&(BIT4|BIT5))
                    |((this->hazardLights<<6)&BIT6));

    buff[3] = 0;

    buff[4] = (((this->steeringWheelAngle) >> 8) &0xFF);
    buff[5] = (this->steeringWheelAngle &0xFF);
    buff[6] = 0;
    buff[7] = 0;
#ifdef MESSAGE_DEBUG
    printf("message512 buff[0] is %x \n",buff[0]);
    printf("message512 buff[1] is %x \n",buff[1]);
    printf("message512 buff[2] is %x \n",buff[2]);
    printf("message512 buff[3] is %x \n",buff[3]);
    printf("message512 buff[4] is %x \n",buff[4]);
    printf("message512 buff[5] is %x \n",buff[5]);
    printf("message512 buff[6] is %x \n",buff[6]);
    printf("message512 buff[7] is %x \n",buff[7]);
#endif
    return 8;
}
