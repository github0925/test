/*
 * tmp411.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TMP411_H__
#define __TMP411_H__

#include <sys/types.h>

struct tmp411_device {
    int i2c_bus;
    u8 device_address;
    void *i2c_handle;
};

bool check_extended_binary(struct tmp411_device *dev);
void extended_range_config(struct tmp411_device *dev);
void alert_pin_config(struct tmp411_device *dev);

u8 tmp411_get_manufacture_id(struct tmp411_device *dev);
u8 tmp411_get_local_temp_low_limit(struct tmp411_device *dev);
u8 tmp411_get_local_temp_high_limit(struct tmp411_device *dev);
u8 tmp411_get_local_temp_therm_limit(struct tmp411_device *dev);

float tmp411_get_result(struct tmp411_device *dev);

struct tmp411_device *tmp411_init(void);
void tmp411_deinit(struct tmp411_device *dev);

#endif