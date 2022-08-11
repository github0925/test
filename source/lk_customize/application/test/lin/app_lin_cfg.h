#ifndef __APP_LIN_CFG_H_
#define __APP_LIN_CFG_H_

#include "Lin.h"
#include <stdint.h>


#define LIN_SEND_DATA_LEN  8

typedef enum {
	False = 0,
	True  = 1
}Bool;


extern Lin_PduType linPduInfo[];

extern const char *chn[5];


Bool get_Lin_PduType(uint8 typedNums,Lin_PduType **pduType);

#endif
