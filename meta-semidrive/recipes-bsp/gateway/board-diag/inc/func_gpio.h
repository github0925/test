/*
 * func_gpio.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _FUNC_GPIO_H
#define _FUNC_GPIO_H
#include "sdrv_types.h"

bool gpio_read_reply_deal(test_exec_t *exec, test_state_e state);
bool gpio_write_reply_deal(test_exec_t *exec,test_state_e state);
#endif

