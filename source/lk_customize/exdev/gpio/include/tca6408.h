/*
 * tca6408.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TCA6408_H__
#define __TCA6408_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>


enum tca6408_port {
    TCA6408_P0 = 0,
    TCA6408_P1 = 1,
    TCA6408_P2 = 2,
    TCA6408_P3 = 3,
    TCA6408_P4 = 4,
    TCA6408_P5 = 5,
    TCA6408_P6 = 6,
    TCA6408_P7 = 7,
    TCA6408_PORT_NUM = 8,
};

struct tca6408_device;
#if 0
struct tca6408_dev_ops {
    int (*output_enable)(struct tca6408_device *dev, int port_index);
    int (*output_val)(struct tca6408_device *dev, int port_index, int val);
    int (*input_val)(struct tca6408_device *dev, int port_index);
    int (*input_enable)(struct tca6408_device *dev, int port_index);
};
#else
int tca6408_output_enable(struct tca6408_device *dev,
                          enum tca6408_port port);
int tca6408_output_val(struct tca6408_device *dev, enum tca6408_port port,
                       int value);
int tca6408_input_val(struct tca6408_device *dev, enum tca6408_port port);
int tca6408_input_enable(struct tca6408_device *dev,
                         enum tca6408_port port);
#endif

struct tca6408_device {
    int count;
    int i2c_bus;
    u8 device_address;
    void *i2c_handle;
    //struct tca6408_dev_ops ops;
};

struct tca6408_device *tca6408_init(int i2c_bus, u8 addr);
void tca6408_deinit(struct tca6408_device *dev);

#endif
