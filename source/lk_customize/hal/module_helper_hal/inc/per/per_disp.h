/*
 * per_disp.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: peripheral display.
 *
 * Revision History:
 * -----------------
 */
#ifndef __PER_DISP_H__
#define __PER_DISP_H__

#include <module_helper_hal.h>

enum PER_DISP_STATE {
    DISP_S0,    //closed, no clk, hold rst , no power?
    DISP_S1,    //prepare. clk, power, hold rst,
    DISP_S2i1, //run. low performance,
    DISP_S2, //run. full performance
    DISP_UNINIT = DISP_S0,  /*s0*/
    DISP_INIT = DISP_S2,    /*s1*/
    MAX_DISP_STATE,
};
extern struct res_handle_item
    *per_disp_state_table[MAX_DISP_STATE][MAX_DISP_STATE];
#define PER_ITEM_DISP   \
    PER_ITEM(PER_ID_DISP, "per_disp", DISP_UNINIT, MAX_DISP_STATE, per_disp_state_table, NULL)


#endif /* __PER_DISP_H__ */