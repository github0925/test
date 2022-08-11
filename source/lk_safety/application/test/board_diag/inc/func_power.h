/*
 * func_power.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __FUNC_POWER_H_
#define __FUNC_POWER_H_
#include "board_start.h"

extern void system_wake_up_trigger(void);

extern void set_system_wakeup(void);

extern int set_system_powerdown(void);

extern int set_soc_internal_wakeup(void);

extern int set_soc_external_wakeup(void);

#endif