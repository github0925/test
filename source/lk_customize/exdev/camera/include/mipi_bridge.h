/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#ifndef __MIPI_BRIDGE_H__
#define __MIPI_BRIDGE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

#include "v4l2.h"

struct v4l2_device *mipi_bridge_init(int id);

#endif
