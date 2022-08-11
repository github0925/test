/*
 * Copyright (C) Semidrive Semiconductor Ltd.
 * All rights reserved.
 */

#ifndef __TOUCH_CONFIG_H__
#define __TOUCH_CONFIG_H__

#include "target_port.h"
#include "hal_port.h"


//touch device slave address
#define GT9XX_DEFAULT_SLAVE_ID  (0x5d)
#define GT9XX_DEFAULT_SLAVE_ID_14  (0x14)

#define TS_MBOX_ADDR        (0x70)

enum semidrive_tp_itf {
    TP_LVDS1,
    TP_LVDS2,
    TP_LVDS3,
    TP_LVDS4,
    TP_DSI1,
    TP_DSI2,
};

enum semidrive_tp_type {
    TP_GOODIX_9XX,
    TP_CYPRESS_T,
};

//use for android 2 touches
enum semidrive_dev_type {
    DEV_TYPE_MAIN,
    DEV_TYPE_AUX,
};

enum semidrive_serdes_type {
    TI941_SINGLE,
    TI941_DUAL,
    TI947_SINGLE,
    TI947_DUAL,
};

struct ts_board_config {
    enum semidrive_tp_itf itf;
    enum semidrive_tp_type type;
    bool enabled;
    bool serdes;
    enum semidrive_serdes_type serdes_type;
    bool virtual;
    u32 res_id;
    u16 i2c_addr;
    u16 mbox_addr;
    u16 rproc;
    u16 dev_type;
    u16 offset;
    u16 port_id;
    u16 ser_addr;
    u16 des_addr;
    u16 irq_channel;
    u16 reset_channel;
    u16 x_max;
    u16 y_max;
    u16 vir_x_offset;
    u16 vir_y_offset;
    u16 vir_x_max;
    u16 vir_y_max;
    Port_PinType pin;
    Port_PinModeType mode;
};

bool target_config_ts_enabled(int instance);
struct ts_board_config *target_config_ts_acquire(int instance);

#endif //__TOUCH_CONFIG_H__
