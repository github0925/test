/*
 * per_sys.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: peripheral sys.
 *
 * Revision History:
 * -----------------
 */
#ifndef __PER_SYS_H__
#define __PER_SYS_H__

#include <module_helper_hal.h>

enum PER_SYS_STATE {
    SYS_S0, //closed, no clk, hold rst , no power?
    SYS_S1, //prepare. clk, power, hold rst,
    SYS_S2i1, //run. low performance,
    SYS_S2, //run. full performance
    SYS_UNINIT = SYS_S0,    /*s0*/
    SYS_INIT = SYS_S2,  /*s1*/
    MAX_SYS_STATE,
};
extern struct res_handle_item
    *per_sys_state_table[MAX_SYS_STATE][MAX_SYS_STATE];
#define PER_ITEM_SYS    \
    PER_ITEM(PER_ID_SYS, "per_sys", SYS_UNINIT, MAX_SYS_STATE, per_sys_state_table, NULL)


#endif /* __PER_SYS_H__ */