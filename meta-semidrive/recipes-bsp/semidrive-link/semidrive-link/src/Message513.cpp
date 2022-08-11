#include "Message513.hpp"

void Message_513::parse(unsigned char* buff,int len) {
    this->mode1 = buff[0] &BIT0;
    this->windspeed1 = (buff[0] >> 1) &0x7;
    this->temperature1 = (buff[0] >> 4) &0xf;
    this->rgb_r1 = buff[1];
    this->rgb_g1 = buff[2];
    this->rgb_b1 = buff[3];

    this->mode2 = buff[4] &BIT0;
    this->windspeed2 = (buff[4] >> 1) &0x7;
    this->temperature2 = (buff[4] >> 4) &0xf;
    this->rgb_r2 = buff[5];
    this->rgb_g2 = buff[6];
    this->rgb_b2 = buff[7];
}

void Message_513::reset() {
    this->mode1 = 0;
    this->windspeed1 = 0;
    this->temperature1 = 0;
    this->rgb_r1 = 0;
    this->rgb_g1 = 0;
    this->rgb_b1 = 0;

    this->mode2 = 0;
    this->windspeed2 = 0;
    this->temperature2 = 0;
    this->rgb_r2 = 0;
    this->rgb_g2 = 0;
    this->rgb_b2 = 0;
}

bool Message_513::onUpdate(Message *data,MessageUpdateListener *l) {
    Message_513 *orig = (Message_513 *)data;
    if(orig->mode1 != this->mode1) {
        this->mode1 = orig->mode1;
        l->onChanged(EVENT_AIR_MODE_1,orig->mode1);
    }

    if(orig->windspeed1 != this->windspeed1) {
        this->windspeed1 = orig->windspeed1;
        l->onChanged(EVENT_WIND_SPEED_1,orig->windspeed1);
    }

    if(orig->temperature1 != this->temperature1) {
        this->temperature1 = orig->temperature1;
        l->onChanged(EVENT_TEMPERATURE_1,orig->temperature1);
    }

    if(orig->rgb_r1 != this->rgb_r1) {
        this->rgb_r1 = orig->rgb_r1;
        l->onChanged(EVENT_AIR_RGB_R_1,orig->rgb_r1);
    }

    if(orig->rgb_g1 != this->rgb_g1) {
        this->rgb_g1 = orig->rgb_g1;
        l->onChanged(EVENT_AIR_RGB_G_1,orig->rgb_g1);
    }

    if(orig->rgb_b1 != this->rgb_b1) {
        this->rgb_b1 = orig->rgb_b1;
        l->onChanged(EVENT_AIR_RGB_B_1,orig->rgb_b1);
    }

    if(orig->mode2 != this->mode2) {
        this->mode2 = orig->mode2;
        l->onChanged(EVENT_AIR_MODE_2,orig->mode2);
    }

    if(orig->windspeed2 != this->windspeed2) {
        this->windspeed2 = orig->windspeed2;
        l->onChanged(EVENT_WIND_SPEED_2,orig->windspeed2);
    }

    if(orig->temperature2 != this->temperature2) {
        this->temperature2 = orig->temperature2;
        l->onChanged(EVENT_TEMPERATURE_2,orig->temperature2);
    }

    if(orig->rgb_r2 != this->rgb_r2) {
        this->rgb_r2 = orig->rgb_r2;
        l->onChanged(EVENT_AIR_RGB_R_2,orig->rgb_r2);
    }

    if(orig->rgb_g2 != this->rgb_g2) {
        this->rgb_g2 = orig->rgb_g2;
        l->onChanged(EVENT_AIR_RGB_G_2,orig->rgb_g2);
    }

    if(orig->rgb_b2 != this->rgb_b2) {
        this->rgb_b2 = orig->rgb_b2;
        l->onChanged(EVENT_AIR_RGB_B_2,orig->rgb_b2);
    }

    return true;
}

bool Message_513::onUpdate(int32_t event,int32_t value) {
    switch(event) {
        case EVENT_TEMPERATURE_1:
        this->temperature1 = value;
        return true;

        case EVENT_AIR_MODE_1:
        this->mode1 = value;
        return true;

        case EVENT_WIND_SPEED_1:
        this->windspeed1 = value;
        return true;

        case EVENT_AIR_RGB_R_1:
        this->rgb_r1 = value;
        return true;

        case EVENT_AIR_RGB_G_1:
        this->rgb_g1 = value;
        return true;

        case EVENT_AIR_RGB_B_1:
        this->rgb_b1 = value;
        return true;

        case EVENT_TEMPERATURE_2:
        this->temperature2 = value;
        return true;

        case EVENT_AIR_MODE_2:
        this->mode2 = value;
        return true;

        case EVENT_WIND_SPEED_2:
        this->windspeed2 = value;
        return true;

        case EVENT_AIR_RGB_R_2:
        this->rgb_r2 = value;
        return true;

        case EVENT_AIR_RGB_G_2:
        this->rgb_g2 = value;
        return true;

        case EVENT_AIR_RGB_B_2:
        this->rgb_b2 = value;
        return true;
    }

    return false;
}

int Message_513::compose(unsigned char *buff) {
    buff[0] = ((this->mode1 &BIT0)
                    |(((this->windspeed1) <<1) & (BIT1|BIT2|BIT3))
                    |((this->temperature1<<4) &(BIT7|BIT6|BIT5|BIT4)));
    buff[1] = this->rgb_r1;
    buff[2] = this->rgb_g1;
    buff[3] = this->rgb_b1;

    buff[4] = ((this->mode2 &BIT0)
                    |(((this->windspeed2) <<1) & (BIT1|BIT2|BIT3))
                    |((this->temperature2<<4) &(BIT7|BIT6|BIT5|BIT4)));
    buff[5] = this->rgb_r2;
    buff[6] = this->rgb_g2;
    buff[7] = this->rgb_b2;
    return 8;
}
