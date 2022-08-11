/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#ifndef __MAX9286_H__
#define __MAX9286_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>


struct v4l2_device *max9286_init(int i2c_bus, u8 addr);
int max9286_set_sync_mode(struct v4l2_device *vdev, bool sync);
int max9286_vc_channel_enable(struct v4l2_device *vdev, bool en,
                              uint8_t ch);
int max9286_deinit(struct v4l2_device *vdev);

#endif
