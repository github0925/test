/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#ifndef __OV5640_H__
#define __OV5640_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

#include "v4l2.h"

struct v4l2_device *ov5640_init(int i2c_bus);
int ov5640_exit(struct v4l2_device *vdev);

#endif
