//*****************************************************************************
//
// i2s_cadence_sc.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __I2S_CADENCE_SC_H__
#define __I2S_CADENCE_SC_H__

#include "i2s_hal.h"
#include <kernel/event.h>

#define I2S_SC_FIFO_OFFSET  0x040

typedef struct {
    uint8_t  bus;
    addr_t base_addr;
    uint32_t clock;
    uint8_t interrupt_num;
    uint8_t is_added;
    i2s_sc_init_t cfg_info;
    event_t tx_comp;
    event_t rx_comp;
} i2s_sc_config_info;

bool i2s_sc_register(i2s_sc_config_info *cfg);
bool i2s_sc_config(i2s_sc_config_info *dev, i2s_sc_init_t *cfg);
bool i2s_sc_start(void *dev);
bool i2s_sc_stop(void *dev);
enum handler_return i2s_sc_transmit_intmode(void *dev);
void i2s_sc_reg_cur_setting(i2s_sc_config_info *dev);

#endif
