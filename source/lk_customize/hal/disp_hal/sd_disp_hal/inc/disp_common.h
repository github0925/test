//*****************************************************************************
//
// disp_debug.h - Prototypes for the display debug
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DISP_DEBUG_H__
#define __DISP_DEBUG_H__

#include <stdio.h>
#include <stdint.h>
#include <debug.h>

#define DISP_DEBUG 1

#define PRINT(level, fmt, args...) dprintf(level, "disp: [%20s]: " fmt, __FUNCTION__, ##args)
#define LOGD(fmt, args...) PRINT(DISP_DEBUG, fmt, ##args)
#define LOGE(fmt, args...) PRINT(0, "Error " fmt, ##args)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // __DISP_DEBUG_H__

