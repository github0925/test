/*
 * Copyright (C) Semidrive Semiconductor Ltd.
 * All rights reserved.
 */

#ifndef __TOUCH_DEVICE_H__
#define __TOUCH_DEVICE_H__

#include "hal_port.h"
#include <disp_hal.h>

#define TS_ENABLE true
#define TS_DISABLE false

enum ts_domain_type {
    TS_DO_ANRDOID_MAIN,
    TS_DO_ANRDOID_AUX1,
    TS_DO_ANRDOID_AUX2,
    TS_DO_ANRDOID_AUX3,
    TS_DO_CTRLPANEL_MAIN,
    TS_DO_CTRLPANEL_AUX1,
    TS_DO_MAX,
};

#define TS_SUPPORT_ANRDOID_MAIN   (1<<TS_DO_ANRDOID_MAIN)
#define TS_SUPPORT_ANRDOID_AUX1   (1<<TS_DO_ANRDOID_AUX1)
#define TS_SUPPORT_ANRDOID_AUX2   (1<<TS_DO_ANRDOID_AUX2)
#define TS_SUPPORT_ANRDOID_AUX3   (1<<TS_DO_ANRDOID_AUX3)
#define TS_SUPPORT_CTRLPANEL_MAIN (1<<TS_DO_CTRLPANEL_MAIN)
#define TS_SUPPORT_CTRLPANEL_AUX1 (1<<TS_DO_CTRLPANEL_AUX1)

struct ts_coord_config {
    u16 x_max;
    u16 y_max;
    u8 max_touch_num;
    u8 swapped_x_y;
    u8 inverted_x;
    u8 inverted_y;
};

struct port_expand_config {
    bool enable;
    u32 port_i2c_bus;
    u16 port_addr;
    u16 port_id;
};

enum ts_serdes_type {
    TI941_SINGLE,
    TI941_SECOND,
    TI941_DUAL,
    TI947_SINGLE,
    TI947_SECOND,
    TI947_DUAL,
};

struct ts_serdes_config {
    bool enable;
    enum ts_serdes_type serdes_type;
    u16 ser_addr;
    u16 des_addr;
    u16 irq_channel;
    u16 reset_channel;
};

struct ts_pin_config {
    Port_PinType pin_num;
    Port_PinModeType pin_mode;
};

struct ts_board_config {
    bool enable;
    const char *device_name;
    u32 res_id;
    u16 i2c_addr;
    u16 ts_domain_support;
    enum DISPLAY_SCREEN screen_id;
    struct ts_coord_config coord_config;
    struct port_expand_config port_config;
    struct ts_serdes_config serdes_config;
    struct ts_pin_config reset_pin;
    struct ts_pin_config irq_pin;
};

extern void get_touch_device(struct ts_board_config **_tsc, int *_tsc_num);

#endif //__TOUCH_DEVICE_H__

