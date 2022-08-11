//*****************************************************************************
//
// i2s_candence_mc.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __I2S_CADENCE_MC_H__
#define __I2S_CADENCE_MC_H__

#include "i2s_hal.h"

#define I2S_MC_FIFO_OFFSET  0x3c

typedef struct {
    uint8_t  bus;
    addr_t base_addr;
    uint32_t clock;
    uint8_t interrupt_num;
    uint8_t is_added;
    i2s_mc_init_t cfg_info;
} i2s_mc_config_info;

bool i2s_mc_register(i2s_mc_config_info *cfg);
bool i2s_mc_config(i2s_mc_init_t *cfg_info, void *handle);
bool i2s_mc_start(void *handle);
bool i2s_mc_stop(void *dev);
enum handler_return i2s_mc_transmit_intmode(void *dev);
void i2s_mc_reg_cur_setting(i2s_mc_config_info *dev);

#endif
