/*
 * board_init.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __COM_INIT_H__
#define __COM_INIT_H__

#include "board_start.h"

extern bool board_diag_module_init(bool *falg);
extern void pwm_channel_init(uint8_t duty);
extern void cpt_channel_init(void);
extern int  cpt_start_thread(void *arg);
extern void sw_time_init(void);
extern void adc_channel_init(void);
extern void i2c_channel_init(void);
extern void comx_gpio_init(void);
extern void rtc_clk_init(void);
extern void can_bus_wakeup_init(void);
extern void i2cx_gpio_init(uint8_t state);
extern void remote_test_init(void);
extern void spread_spectrum_init(void);
#endif
