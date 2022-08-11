/*
 * module_helper_hal.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER HAL.
 *
 * Revision History:
 * -----------------
 */
#ifndef _MODULE_HELPER_HAL_H
#define _MODULE_HELPER_HAL_H

#define RES_INVALID 0
#include <stdio.h>
#include <debug.h>
#if MODULE_HELPER_PER_SYS
#include <per/per_sys.h>
#endif
#if MODULE_HELPER_PER_TEST
#include <per/per_test.h>
#endif
#if MODULE_HELPER_PER_DDR
#include <per/per_ddr.h>
#endif
#if MODULE_HELPER_PER_DISP
#include <per/per_disp.h>
#endif
#ifndef DBGV
#define DBGV 4
#endif

enum module_per_id {
#if MODULE_HELPER_PER_SYS
    PER_ID_SYS,
#endif
#if MODULE_HELPER_PER_TEST
    PER_ID_TEST,
#endif
#if MODULE_HELPER_PER_DDR
    PER_ID_DDR,
#endif
#if MODULE_HELPER_PER_DISP
    PER_ID_DISP,
#endif
    MAX_PER_ID,
    INVALID_PER_ID = MAX_PER_ID,
};

int module_get_cur_state(enum module_per_id per_id);
int module_set_state(enum module_per_id per_id, int state);
int module_helper_init(void);
#include <res/res_clk.h>

#endif /* _MODULE_HELPER_HAL_H */

