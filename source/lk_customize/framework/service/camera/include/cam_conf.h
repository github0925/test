#ifndef __CAM_CONF_HEAD__
#define __CAM_CONF_HEAD__

#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <event.h>


#define MBOX_INDEX_CAMERA_CSI0  0x80
#define MBOX_INDEX_CAMERA_CSI0_PARAM    0x8000
#define MBOX_INDEX_CAMERA_CSI0_0    0x81
#define MBOX_INDEX_CAMERA_CSI0_0_PARAM  0x8100
#define MBOX_INDEX_CAMERA_CSI0_1    0x82
#define MBOX_INDEX_CAMERA_CSI0_1_PARAM  0x8200
#define MBOX_INDEX_CAMERA_CSI0_2    0x83
#define MBOX_INDEX_CAMERA_CSI0_2_PARAM  0x8300
#define MBOX_INDEX_CAMERA_CSI0_3    0x84
#define MBOX_INDEX_CAMERA_CSI0_3_PARAM  0x8400

#define MBOX_INDEX_CAMERA_CSI1  0x86
#define MBOX_INDEX_CAMERA_CSI1_PARAM    0x8600
#define MBOX_INDEX_CAMERA_CSI1_0    0x87
#define MBOX_INDEX_CAMERA_CSI1_0_PARAM  0x8700
#define MBOX_INDEX_CAMERA_CSI1_1    0x88
#define MBOX_INDEX_CAMERA_CSI1_1_PARAM  0x8800
#define MBOX_INDEX_CAMERA_CSI1_2    0x89
#define MBOX_INDEX_CAMERA_CSI1_2_PARAM  0x8900
#define MBOX_INDEX_CAMERA_CSI1_3    0x8a
#define MBOX_INDEX_CAMERA_CSI1_3_PARAM  0x8a00

#define MBOX_INDEX_CAMERA_CSI2  0x8c
#define MBOX_INDEX_CAMERA_CSI2_PARAM    0x8c00
#define MBOX_INDEX_CAMERA_CSI2_0    0x8d
#define MBOX_INDEX_CAMERA_CSI2_0_PARAM  0x8d00
#define MBOX_INDEX_CAMERA_CSI2_1    0x8e
#define MBOX_INDEX_CAMERA_CSI2_1_PARAM  0x8e00



struct cam_config {
    bool enabled;
    u32 rproc;
    u16 mbox_addr;
};

#endif //__CAM_CONF_HEAD__
