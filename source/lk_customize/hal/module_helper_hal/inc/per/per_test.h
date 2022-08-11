/*
 * per_test.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: peripheral test.
 *
 * Revision History:
 * -----------------
 */
#ifndef __PER_TEST_H__
#define __PER_TEST_H__

#include <module_helper_hal.h>

enum PER_TEST_STATE {
    TEST_S0,    //closed, no clk, hold rst , no power?
    TEST_S1,    //prepare. clk, power, hold rst,
    TEST_S2i1, //run. low performance,
    TEST_S2, //run. full performance
    TEST_UNINIT = TEST_S0,  /*s0*/
    TEST_INIT = TEST_S2,    /*s1*/
    MAX_TEST_STATE,
};
extern struct res_handle_item
    *per_test_state_table[MAX_TEST_STATE][MAX_TEST_STATE];
#define PER_ITEM_TEST   \
    PER_ITEM(PER_ID_TEST, "per_test", TEST_UNINIT, MAX_TEST_STATE, per_test_state_table, NULL)


#endif /* __PER_TEST_H__ */