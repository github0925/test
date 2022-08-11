/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: display service API prototype
 *
 */

#ifndef __DISPLAY_SERVICE_H__
#define __DISPLAY_SERVICE_H__

#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>


/* Remote IOC helper */
#define RP_IOC_DATA(ctl)		&ctl->u.data[0]

/* Define common data */
struct display_server {
	mutex_t dev_lock;
	u32 features;
	bool binitialized;

	thread_t *disp_thread;
	event_t disp_event;
	thread_t *disp_vsync_thread;
	event_t disp_vsync_event;
	int on_screen;
	u16 mbox_addr;
	int rproc;
	/* communication level stuff */
	hal_mb_client_t client;
	hal_mb_chan_t *mchan;
};

struct disp_frame_info {
	u32 addr_l;
	u32 addr_h;
	u32 format;
	u16 width;
	u16 height;
	u16 pos_x;
	u16 pos_y;
	u16 pitch;
	u16 mask_id;
}__attribute__ ((packed));

/* Do not exceed 32 bytes so far */
struct disp_ioctl_cmd {
	u32 op;
	union {
		u8 data[28];
		struct disp_frame_info fi;
	} u;
};

struct disp_ioctl_ret {
	u32 op;
	union {
		u8 data[8];
	} u;
};

#endif //__DISPLAY_SERVICE_H__
