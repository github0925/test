/*
* g2dlite_common.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/28/2020 BI create this file
*/

#ifndef __G2DLITE_DEBUG_H__
#define __G2DLITE_DEBUG_H__

#include <stdio.h>
#include <stdint.h>

#define PRINT(fmt, args...) printf("g2dlite: [%20s]: " fmt, __FUNCTION__, ##args)
#define LOGD(fmt, args...) PRINT(fmt, ##args)
#define LOGE(fmt, args...) PRINT("Error " fmt, ##args)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // __G2DLITE_DEBUG_H__

