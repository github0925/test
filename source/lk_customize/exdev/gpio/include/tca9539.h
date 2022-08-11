/*
 * tca9539.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TCA9539_H__
#define __TCA9539_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

#define TCA9539_P00 1
#define TCA9539_P01 2
#define TCA9539_P02 3
#define TCA9539_P03 4
#define TCA9539_P04 5
#define TCA9539_P05 6
#define TCA9539_P06 7
#define TCA9539_P07 8
#define TCA9539_P10 9
#define TCA9539_P11 10
#define TCA9539_P12 11
#define TCA9539_P13 12
#define TCA9539_P14 13
#define TCA9539_P15 14
#define TCA9539_P16 15
#define TCA9539_P17 16

struct tca9539_device;

struct tca9539_dev_ops {
    int (*output_enable)(struct tca9539_device *dev, int port_index);
    int (*output_val)(struct tca9539_device *dev, int port_index, int val);
    int (*input_val)(struct tca9539_device *dev, int port_index);
    int (*input_enable)(struct tca9539_device *dev, int port_index);
};

struct tca9539_device {
    int count;
    int i2c_bus;
    u8 device_address;
    void *i2c_handle;
    struct tca9539_dev_ops ops;
};

struct tca9539_device *tca9539_init(int i2c_bus, u8 addr);
int tca9539_enable_i2cpoll(struct tca9539_device *dev);

void tca9539_deinit(struct tca9539_device *dev);

#endif
