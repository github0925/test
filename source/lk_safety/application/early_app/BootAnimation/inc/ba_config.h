#ifndef _BA_CONFIG_H
#define _BA_CONFIG_H

#include "stdlib.h"

typedef struct _rect_t{
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
}prect_t;
extern prect_t g_srcres[];
extern prect_t g_dstdisp[];
#define RES_WIDTH   0X1680  //1920*3
#define RES_HEIGHT  0X2D0   //720

uint32_t getsrcCnt(void);
uint32_t getdstCnt(void);
#endif /*_BA_CONFIG_H*/