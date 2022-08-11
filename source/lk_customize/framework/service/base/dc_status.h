/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication common define for kunlun
*
*/

#ifndef _DC_STATUS_H_
#define _DC_STATUS_H_

#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <mbox_hal.h>

/*
 * Display controller status value
 */
typedef enum {
	DC_STAT_NOTINIT     = 0,	/* not initilized by remote cpu */
	DC_STAT_INITING     = 1,	/* on initilizing */
	DC_STAT_INITED      = 2,	/* initilize compilete, ready for display */
	DC_STAT_BOOTING     = 3,	/* during boot time splash screen */
	DC_STAT_CLOSING     = 4,	/* DC is going to close */
	DC_STAT_CLOSED      = 5,	/* DC is closed safely */
	DC_STAT_NORMAL      = 6,	/* DC is used by DRM */
	DC_STAT_MAX         = DC_STAT_NORMAL,
} dc_state_t;

#endif //_DC_STATUS_H_