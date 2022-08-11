/*
 * gpio-ops.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __GPIO_OPS_H__
#define __GPIO_OPS_H__
#include "board_diag.h"

void gpio_init(void);
void gpio_deinit(void);
bool gpio_read(uint8_t pin, uint8_t *out_val);
bool gpio_write(uint8_t pin, uint8_t val);
#endif
