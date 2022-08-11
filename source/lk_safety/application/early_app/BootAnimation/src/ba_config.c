#include "ba_config.h"
#include "stdio.h"

#ifndef RES_WIDTH
#define RES_WIDTH   0X1680
#endif
#ifndef RES_HEIGHT
#define RES_HEIGHT  0X2D0
#endif


prect_t g_srcres[] = {
    {0,0,1920,720},
    {1920,0,1920,720},
    {3840,0,1920,720}
};
#if defined (TARGET_X9H_ICL02)
prect_t g_dstdisp[] = {
    {0,0,1920,720},
    {344,0,1920,720}, 
    {0,0,1920,720},
    {0,0,1920,720}
};
#else
prect_t g_dstdisp[] = {
    {0,0,1920,720},
    {0,0,1920,720},
    {0,0,1920,720},
    {0,0,1920,720}
};
#endif


uint32_t getsrcCnt(void)
{
    return sizeof(g_srcres) / sizeof(g_srcres[0]);
}
uint32_t getdstCnt(void)
{
    return sizeof(g_dstdisp) / sizeof(g_dstdisp[0]);
}