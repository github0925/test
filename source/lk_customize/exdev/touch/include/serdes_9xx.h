//*****************************************************************************
//
// serdes.h
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SERDES_9XX_TP_H__
#define __SERDES_9XX_TP_H__

#include <touch_device.h>

extern int du90ub941_or_947_enable_port(void *i2c_handle, uint8_t addr, enum ts_serdes_type port_flag);
extern int du90ub941_or_947_enable_i2c_passthrough(void *i2c_handle, uint8_t addr);
extern int du90ub941_or_947_enable_int(void *i2c_handle, uint8_t addr);
extern int du90ub948_gpio_output(void *i2c_handle, uint8_t addr, int gpio, int val);
extern int du90ub948_gpio_input(void *i2c_handle, uint8_t addr, int gpio);
extern int du90ub941_or_947_gpio_input(void *i2c_handle, uint8_t addr, int gpio);

#endif

