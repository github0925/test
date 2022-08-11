#ifndef __SEMIDRIVE_CAN_MESSAGE_HPP__
#define __SEMIDRIVE_CAN_MESSAGE_HPP__
#include <stdint.h>

struct CanData {
    uint32_t bindid;
	uint32_t len; 
    unsigned char payload[32];
};
//__attribute__((__packed__))
#endif