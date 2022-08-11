//*****************************************************************************
//
// hal_timer_weak.c - Driver for the timer hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <sys/types.h>
#include "timer_hal.h"

bool hal_timer_creat_handle(void **handle, uint32_t res_glb_idx)
{
    return true;
}

bool hal_timer_release_handle(void *handle)
{
    return true;
}

void hal_timer_global_init(void *handle, hal_timer_glb_cfg_t *cfg)
{
    return;
}

void hal_timer_ovf_init(void *handle, hal_timer_sub_t sub_cntr,
                        hal_timer_ovf_cfg_t *cfg)
{
    return;
}

void hal_timer_func_init(void *handle, hal_timer_func_ch_t sub_cntr,
                         hal_timer_fun_cfg_t *cfg)
{
    return;
}

void hal_timer_init(void *handle, hal_timer_cfg_t *cfg)
{
    return;
}

void hal_timer_cmp_force_out(void *handle, hal_timer_func_ch_t func_ch,
                             bool enable, bool level)
{
    return;
}

uint32_t hal_timer_timer_cpt_value_get(void *handle,
                                       hal_timer_func_ch_t func_ch)
{
    return 0;
}

void hal_timer_cmp_value_set(void *handle, hal_timer_func_ch_t func_ch,
                             uint32_t value0, uint32_t value1)
{
    return;
}

void hal_timer_func_cmp_enable(void *handle, hal_timer_func_ch_t func_ch)
{
    return;
}

void hal_timer_func_cmp_disable(void *handle, hal_timer_func_ch_t func_ch)
{
    return;
}

void hal_timer_func_cpt_enable(void *handle, hal_timer_func_ch_t func_ch)
{
    return;
}

void hal_timer_func_cpt_disable(void *handle, hal_timer_func_ch_t func_ch)
{
    return;
}

void hal_timer_int_src_enable(void *handle, hal_timer_int_src_t int_src)
{
    return;
}

void hal_timer_int_src_disable(void *handle, hal_timer_int_src_t int_src)
{
    return;
}

void hal_timer_int_sta_clear(void *handle, hal_timer_int_src_t int_src)
{
    return;
}

void hal_timer_int_cbk_register(void *handle, hal_timer_int_src_t int_src, hal_timer_int_cbk cbk)
{
    return;
}

uint64_t hal_timer_glb_cntr_get(void *handle)
{
    return 0;
}

uint64_t hal_timer_ms_to_cntr(void *handle, uint32_t ms)
{
    return 0;
}

uint32_t hal_timer_cpt_get_fifo_items_num(void *handle, hal_timer_func_ch_t func_ch)
{
    return 0;
}
uint32_t hal_timer_cntr_to_ms(void* handle, uint32_t cntr)
{
    return 0;
}
uint32_t hal_timer_cntr_to_us(void* handle, uint32_t cntr)
{
    return 0;
}
