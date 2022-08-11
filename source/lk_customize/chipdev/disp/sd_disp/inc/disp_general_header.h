/*
* disp_general_header.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/02/2019 BI create this file
*/

#ifndef __DISP_GENERAL_HEADER__
#define __DISP_GENERAL_HEADER__
#include <debug.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

#define udelay(x) spin(x)

#endif //__DISP_GENERAL_HEADER__