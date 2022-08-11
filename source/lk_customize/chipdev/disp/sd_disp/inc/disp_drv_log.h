/*
* disp_drv_log.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef _DISP_DRV_LOG_H__
#define _DISP_DRV_LOG_H__
#include <debug.h>

#define DISP_DEBUG_BUILD 1

#if DISP_DEBUG_BUILD
#define DISP_DFINFO     (1)
#else
#define DISP_DFINFO     (0)
#endif

#define DISP_LOG_PRINT(level, sub_module, fmt, arg...)      \
    do {                                                    \
        dprintf(DISP_DFINFO,"DISP/"fmt, ##arg);                          \
    }while(0)

#define LOG_PRINT(level, module, fmt, arg...)               \
    do {                                                    \
        dprintf(DISP_DFINFO,fmt, ##arg);                                 \
    }while(0)

#if ENABLE_DISP_LINK
#define DISPMSG(string, args...)
#define DISPDBG(string, args...)
#define DISPERR(string, args...) printf("[DISP]ERROR:"string, ##args)  //default on, err msg
#define DISPFUNC()

#define DISPCHECK(string, args...)
#else
#define DISPMSG(string, args...)
#define DISPDBG(string, args...)
#define DISPERR(string, args...) dprintf(DISP_DFINFO,"[DISP]ERROR:"string, ##args)  //default on, err msg
#define DISPFUNC()

#define DISPCHECK(string, args...)
#endif

#endif //_DISP_DRV_LOG_H__
