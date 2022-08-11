/*
 * pll_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: PFPLL HAL.
 *
 * Revision History:
 * -----------------
 */
#ifndef _PLL_HAL_H
#define _PLL_HAL_H

#include <stdint.h>
#include "pll.h"
#define PLL_ROOT 0
#define PLL_DIVA 1
#define PLL_DIVB 2
#define PLL_DIVC 3
#define PLL_DIVD 4
#define PLL_DUMMY_ROOT 5

typedef uint32_t pll_handle_t;

pll_handle_t hal_pll_create_handle(uint32_t resid);
void hal_pll_delete_handle(pll_handle_t handle);
void hal_pll_config(pll_handle_t handle, const pll_config_t *config);
int hal_pll_get_config(pll_handle_t handle, pll_config_t *config);
unsigned long hal_pll_calcurate(unsigned long freq, int plldiv,
                                pll_config_t *config);
unsigned long hal_pll_set_rate(pll_handle_t handle, unsigned long freq,
                               int plltype);

#endif /* _PLL_HAL_H */
