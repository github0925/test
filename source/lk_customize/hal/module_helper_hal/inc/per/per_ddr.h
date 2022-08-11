/*
 * per_ddr.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: peripheral ddr.
 *
 * Revision History:
 * -----------------
 */
#ifndef __PER_DDR_H__
#define __PER_DDR_H__

#include <module_helper_hal.h>
#define DDR_S2i1 DDR_200M   //run. low performance,
#define DDR_S2 DDR_1066M    //run. full performance
#define DDR_UNINIT DDR_S0
enum PER_DDR_STATE {
    DDR_S0,    //closed, no clk, hold rst , no power?
    DDR_S1,    //prepare. clk, power, hold rst,
    DDR_200M,
    DDR_400M,
    DDR_532M,
    DDR_600M,
    DDR_800M,
    DDR_1066M,
    MAX_DDR_STATE
};
extern struct res_handle_item
    *per_ddr_state_table[MAX_DDR_STATE][MAX_DDR_STATE];
extern const char *ddr_get_state_name(int state);
#define PER_ITEM_DDR   \
    PER_ITEM(PER_ID_DDR, "per_ddr", DDR_UNINIT, MAX_DDR_STATE, per_ddr_state_table, ddr_get_state_name)


#endif /* __PER_DDR_H__ */