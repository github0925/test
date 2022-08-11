//*****************************************************************************
//
// wdg_ip_test_weak_hal.c - Driver for the Watchdog ip test hal Module.
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
#include "wdg_hal_ip_test.h"

 bool hal_wdg_test_creat_handle(void **handle,uint32_t wdg_res_glb_idx)
{
    return true;
}

 bool hal_wdg_test_delete_handle(void *handle)
{
    return true;
}

 bool hal_wdg_read_only_reg_test(void *handle)
{
    return true;
}

 bool hal_wdg_rw_reg_test(void *handle)
{
    return true;
}

 bool hal_wdg_self_test(void *handle)
{
    return true;
}

 bool hal_wdg_terminal_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_terminal_from_fuse_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_reset_control_restart_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_reset_control_not_restart_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode1_refresh_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode2_refresh_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode3_refresh_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode2_window_reset_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode2_1_refresh_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode3_2_1_refresh_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_mode1_overflow_intcheck_test(void *handle,uint32_t timeout)
{
    return true;
}

 bool hal_wdg_debug_mode_test(void *handle,uint32_t timeout)
{
    return true;
}
