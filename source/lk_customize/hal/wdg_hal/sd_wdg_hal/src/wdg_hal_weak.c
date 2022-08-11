//*****************************************************************************
//
// wdg_weak_hal.c - Driver for the Watchdog hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include "wdg_hal.h"

 bool hal_wdg_creat_handle(void **handle,uint32_t wdg_res_glb_idx)
{
    return true;
}

 bool hal_wdg_release_handle(void *handle)
{
    return true;
}

 bool hal_wdg_init(void *handle,wdg_app_config_t *wdg_app_cfg)
{
    return true;
}

 bool hal_wdg_deinit(void *handle)
{
    return true;
}

 bool hal_wdg_enable_selftest(void *handle,bool enable_selftest)
{
    return true;
}

 bool hal_wdg_enable(void *handle)
{
    return true;
}

 bool hal_wdg_disable(void *handle)
{
    return true;
}

 bool hal_wdg_enable_interrupts(void *handle)
{
    return true;
}

 bool hal_wdg_disable_interrupts(void *handle)
{
    return true;
}

 bool hal_wdg_clear_status_flags(void *handle)
{
    return true;
}

 bool hal_wdg_set_timeout(void *handle,uint32_t timeout_ms)
{
    return true;
}

 bool hal_wdg_set_windowvalue(void *handle,uint32_t window_timeout_ms)
{
    return true;
}

 bool hal_wdg_set_seqdelta(void *handle,uint32_t seq_delta_timeout_ms)
{
    return true;
}

 bool hal_wdg_refresh(void *handle)
{
    return true;
}

 bool hal_wdg_clear_reset_count(void *handle)
{
    return true;
}

 bool hal_wdg_clear_ext_reset_count(void *handle)
{
    return true;
}

 bool hal_wdg_int_register(void *handle,int_handler call_func,bool overflow_int)
{
    return true;
}

 bool hal_wdg_int_unregister(void *handle,bool overflow_int)
{
    return true;
}

 bool hal_wdg_int_clear(void *handle)
{
    return true;
}

 bool hal_wdg_halt_enable(void *handle)
{
    return true;
}

 bool hal_wdg_halt_disable(void *handle)
{
    return true;
}

 uint32_t hal_wdg_get_status_flags(void *handle)
{
    return 0;
}

 uint32_t hal_wdg_get_reset_count(void *handle)
{
    return 0;
}

 uint32_t hal_wdg_get_ext_reset_count(void *handle)
{
    return 0;
}

