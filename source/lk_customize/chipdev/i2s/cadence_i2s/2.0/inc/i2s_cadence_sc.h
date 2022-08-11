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

#define I2S_SC_FIFO_OFFSET  0x040

bool sdrv_i2s_sc_startup(struct dev_controller_info dev,
                         pcm_params_t pcm_info);
bool sdrv_i2s_sc_set_format(struct dev_controller_info dev,
                            pcm_params_t pcm_info);
bool sdrv_i2s_sc_set_hw_parameters(struct dev_controller_info dev,
                                   pcm_params_t pcm_info);
bool sdrv_i2s_sc_trigger(struct dev_controller_info dev, int cmd);
bool sdrv_i2s_sc_shutdown(struct dev_controller_info dev);
void sdrv_i2s_sc_reg_cur_setting(struct dev_controller_info *dev);

#endif
