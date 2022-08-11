/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#ifndef __MAX20086_H__
#define __MAX20086_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

struct max20086_device;

struct max20086_dev_ops {
    int (*dump_register)(struct max20086_device *dev);
};

struct max20086_device {
    u8 device_address;
    void *i2c_handle;
    struct max20086_dev_ops ops;
};

struct max20086_device *max20086_init(int i2c_bus, u8 addr);
int max20086_deinit(struct max20086_device *dev);

#endif
