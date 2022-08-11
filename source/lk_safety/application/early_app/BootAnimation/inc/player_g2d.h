#ifndef _PLAYER_G2D_H
#define  _PLAYER_G2D_H
#include <stdlib.h>
#include <stdint.h>

typedef struct _fmt_yuv420p_t{
    void* bufY;
    void* bufCb;
    void* bufCr;
    prect_t rect;
}fmt_yuv420p_t;

void free_g2dm(fmt_yuv420p_t* dst);
bool setG2dHandle(void);
void scale_g2d(fmt_yuv420p_t src,fmt_yuv420p_t* dst,uint32_t stride);
#endif