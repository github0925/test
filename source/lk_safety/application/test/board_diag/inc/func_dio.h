/*
 * func_dio.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __FUNC_DIO_H_
#define __FUNC_DIO_H_

#include "board_start.h"
#include "hal_port.h"
#include "hal_dio.h"

#define MASTER_SLAVE_PORT GPIO_D11
#define KL15_PORT         GPIO_E9
extern uint8_t get_kl15_gpio_state(void);
extern void monitor_master_slave_mode(uint8_t timers);
extern void com_gpio_set_system_power_mgr(uint8_t STATE);
extern Dio_LevelType dio_read_channel_saf(uint8_t ChannelId);
#endif