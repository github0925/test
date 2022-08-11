//*****************************************************************************
//
// cmd_wdg.c - app for the Watchdog test Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include "wdg_hal_ip_test.h"
#include "res.h"
#include "chip_res.h"


static void *g_handle;
uint32_t wdg_test_timeout = 6;//1000;//ms

const wdg_glb_idx_to_id g_wdg_test_glb_idx_to_id[DEFAULT_WDG_MAX_NUM] = {
    {RES_WATCHDOG_WDT1, wdg_really_num1},
    {RES_WATCHDOG_WDT2, wdg_really_num2},
    {RES_WATCHDOG_WDT3, wdg_really_num3},
    {RES_WATCHDOG_WDT4, wdg_really_num4},
    {RES_WATCHDOG_WDT5, wdg_really_num5},
    {RES_WATCHDOG_WDT6, wdg_really_num6},
    {RES_WATCHDOG_WDT7, wdg_really_num7},
    {RES_WATCHDOG_WDT8, wdg_really_num8},
};


char wdg_read_only_reg_test_help[]= {
    "do_wdg_read_only_reg_test:wdg really number\n" \
    "" \
};
int do_wdg_read_only_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;
    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    printf("do_wdg_read_only_reg_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);

    if(ret){
        ret = hal_wdg_read_only_reg_test(g_handle);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
    return ret;
}

char wdg_rw_only_reg_test_help[]= {
    "do_wdg_rw_only_reg_test:wdg really number\n" \
    "" \
};
int do_wdg_rw_only_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_rw_reg_test(g_handle);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
    return ret;
}

char wdg_self_test_help[]= {
    "wdg_self_test_help:wdg really number\n" \
    "" \
};
int do_wdg_self_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_self_test(g_handle);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_self_test ret:%d:\n",ret);
    return ret;
}

char wdg_terninal_test_help[]= {
    "wdg_terninal_test_help:wdg really number\n" \
    "" \
};
int do_wdg_terninal_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_terminal_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_terninal_test ret:%d:\n",ret);
    return ret;
}

char wdg_do_wdg_terninal_from_fuse_test_help[]= {
    "wdg_do_wdg_terninal_from_fuse_test_help:wdg really number\n" \
    "" \
};
int do_wdg_terninal_from_fuse_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_terminal_from_fuse_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_terninal_from_fuse_test ret:%d:\n",ret);
    return ret;
}

char wdg_reset_control_restart_test_help[]= {
    "wdg_reset_control_restart_test_help:wdg really number\n" \
    "" \
};
int do_wdg_reset_control_restart_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_reset_control_restart_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_reset_control_restart_test ret:%d:\n",ret);
    return ret;
}

char wdg_reset_control_not_restart_test_help[]= {
    "wdg_reset_control_not_restart_test_help:wdg really number\n" \
    "" \
};
int do_wdg_reset_control_not_restart_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_reset_control_not_restart_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_reset_control_not_restart_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode1_refresh_test_help[]= {
    "wdg_mode1_refresh_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode1_refresh_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode1_refresh_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode1_refresh_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode2_refresh_test_help[]= {
    "wdg_mode2_refresh_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode2_refresh_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode2_refresh_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode2_refresh_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode3_refresh_test_help[]= {
    "wdg_mode3_refresh_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode3_refresh_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode3_refresh_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode3_refresh_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode2_window_reset_test_help[]= {
    "wdg_mode2_window_reset_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode2_window_reset_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode2_window_reset_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode2_window_reset_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode2_1_refresh_test_help[]= {
    "wdg_mode2_1_refresh_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode2_1_refresh_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode2_1_refresh_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode2_1_refresh_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode3_2_1_refresh_test_help[]= {
    "wdg_mode3_2_1_refresh_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode3_2_1_refresh_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode3_2_1_refresh_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode3_2_1_refresh_test ret:%d:\n",ret);
    return ret;
}

char wdg_mode1_overflow_intcheck_test_help[]= {
    "wdg_mode1_overflow_intcheck_test_help:wdg really number\n" \
    "" \
};
int do_wdg_mode1_overflow_intcheck_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_mode1_overflow_intcheck_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_wdg_mode1_overflow_intcheck_test ret:%d:\n",ret);
    return ret;
}

char hal_wdg_debug_mode_test_help[]= {
    "hal_wdg_debug_mode_test_help:wdg really number\n" \
    "" \
};
int do_hal_wdg_debug_mode_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;

    if(watchdog_number > DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_wdg_test_glb_idx_to_id[watchdog_number-1].res_glb_idx);
    if(ret){
        ret = hal_wdg_debug_mode_test(g_handle,wdg_test_timeout);
    }

    hal_wdg_test_delete_handle(g_handle);
    printf("do_hal_wdg_debug_mode_test ret:%d:\n",ret);
    return ret;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("wdg_test1", wdg_read_only_reg_test_help, (console_cmd)&do_wdg_read_only_reg_test)
STATIC_COMMAND("wdg_test2", wdg_rw_only_reg_test_help, (console_cmd)&do_wdg_rw_only_reg_test)
STATIC_COMMAND("wdg_test3", wdg_self_test_help, (console_cmd)&do_wdg_self_test)
STATIC_COMMAND("wdg_test4", wdg_terninal_test_help, (console_cmd)&do_wdg_terninal_test)
STATIC_COMMAND("wdg_test5", wdg_do_wdg_terninal_from_fuse_test_help, (console_cmd)&do_wdg_terninal_from_fuse_test)
STATIC_COMMAND("wdg_test6", wdg_reset_control_restart_test_help, (console_cmd)&do_wdg_reset_control_restart_test)
STATIC_COMMAND("wdg_test7", wdg_reset_control_not_restart_test_help, (console_cmd)&do_wdg_reset_control_not_restart_test)
STATIC_COMMAND("wdg_test8", wdg_mode1_refresh_test_help, (console_cmd)&do_wdg_mode1_refresh_test)
STATIC_COMMAND("wdg_test9", wdg_mode2_refresh_test_help, (console_cmd)&do_wdg_mode2_refresh_test)
STATIC_COMMAND("wdg_test10", wdg_mode3_refresh_test_help, (console_cmd)&do_wdg_mode3_refresh_test)
STATIC_COMMAND("wdg_test11", wdg_mode2_window_reset_test_help, (console_cmd)&do_wdg_mode2_window_reset_test)
STATIC_COMMAND("wdg_test12", wdg_mode2_1_refresh_test_help, (console_cmd)&do_wdg_mode2_1_refresh_test)
STATIC_COMMAND("wdg_test13", wdg_mode3_2_1_refresh_test_help, (console_cmd)&do_wdg_mode3_2_1_refresh_test)
STATIC_COMMAND("wdg_test14", wdg_mode1_overflow_intcheck_test_help, (console_cmd)&do_wdg_mode1_overflow_intcheck_test)
STATIC_COMMAND("wdg_test15", hal_wdg_debug_mode_test_help, (console_cmd)&do_hal_wdg_debug_mode_test)
STATIC_COMMAND_END(wdgtest);
#endif
APP_START(wdg)
.flags = 0
         APP_END
