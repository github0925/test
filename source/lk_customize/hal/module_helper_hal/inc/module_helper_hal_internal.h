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
#ifndef _MODULE_HELPER_HAL_INTERNAL_H
#define _MODULE_HELPER_HAL_INTERNAL_H
#include <module_helper_hal.h>
#include <res/res_clk.h>
#include <res/res_rstgen.h>

enum res_type {
    RES_CLK,
    RES_RSTGEN,
    RES_REGULATOR,
    RES_PINCTRL,
};

struct res_handle_item {
    //enum res_type restype;
    unsigned long resid;    //not global res id
    unsigned long resparams;
};
struct module_per_item {
    enum module_per_id per_id;
    char name[32];//for debug
    int cur_state;
    int state_num;
    void *state_table;
    const char *(*get_state_name)(int state);
    int (*init)(struct module_per_item *cur);
    //int (*uninit)(struct module_res_init *cur);

};

#define PER_ITEM(perid, _name, _init_state, _state_num,_state_table, _get_state_name)    \
[perid] = {\
.per_id =   perid,  \
.name = _name,  \
.cur_state = _init_state,   \
.state_num = _state_num,    \
.state_table = _state_table,    \
.get_state_name = _get_state_name, \
.init = NULL,   \
}

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif
#define NULL_RES_HANDLE_ITEM    \
    {0,0}

char *module_get_per_name_by_id(int per_id);

#endif /* _MODULE_HELPER_HAL_INTERNAL_H */

