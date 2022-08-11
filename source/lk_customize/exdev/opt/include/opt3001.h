/*
 * opt3001.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __OPT3001_H__
#define __OPT3001_H__

#include <sys/types.h>

struct opt3001_device {
    int i2c_bus;
    u8 device_address;
    void *i2c_handle;
};

enum opt3001_config_mode {
    OPT3001_CONFIGURATION_SHUTDOWN   = 0 << 9,
    OPT3001_CONFIGURATION_SINGLE     = 1 << 9,
    OPT3001_CONFIGURATION_CONTINUOUS = 2 << 9,
};

u16 opt3001_get_id(struct opt3001_device *dev);
void opt3001_config(struct opt3001_device *dev, enum opt3001_config_mode mode);
float opt3001_get_result(struct opt3001_device *dev);
void opt3001_shutdown(struct opt3001_device *dev);

struct opt3001_device *opt3001_init(void);
void opt3001_deinit(struct opt3001_device *dev);

#endif
