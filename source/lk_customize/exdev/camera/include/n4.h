/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#ifndef __N4_H__
#define __N4_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>


struct v4l2_device *n4_init(int i2c_bus, u8 addr);
int n4_set_sync_mode(struct v4l2_device *vdev, bool sync);
int n4_vc_channel_enable(struct v4l2_device *vdev, bool en,
                              uint8_t ch);
int n4_deinit(struct v4l2_device *vdev);

#endif //#ifndef __N4_H__