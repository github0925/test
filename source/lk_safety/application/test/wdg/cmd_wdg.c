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

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

static void *g_handle;
uint32_t wdg_test_timeout = 6;//1000;//ms

#if defined (watchdog_res_def)
static wdg_src_t g_watchdog_test_resource= watchdog_res_def;
#else
static wdg_src_t g_watchdog_test_resource= {.version = 0x10000,
                                                                            .res_category = "watchdog",
                                                                            .res_num = 2,
                                                                            .res_info[0].res_glb_idx = RES_WATCHDOG_WDT5,
                                                                            .res_info[0].res_describe = "cpu1",
                                                                            .res_info[0].phy_addr = 0x30a20000,
                                                                            .res_info[0].addr_range = 0x10000,
                                                                            .res_info[1].res_glb_idx = RES_WATCHDOG_WDT2,
                                                                            .res_info[1].res_describe = "vdsp",
                                                                            .res_info[1].phy_addr = 0x301d0000,
                                                                            .res_info[1].addr_range = 0x10000,
                                                                            };
#endif

char wdg_read_only_reg_test_help[]= {
    "do_wdg_read_only_reg_test:wdg really number\n" \
    "" \
};
int do_wdg_read_only_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t watchdog_number = 0;
    watchdog_number = argv[1].u;
    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    printf("do_wdg_read_only_reg_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);

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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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

    if(watchdog_number >= DEFAULT_WDG_MAX_NUM){
        ret = false;
        printf("do_wdg_read_only_reg_test ret:%d:\n",ret);
        return ret;
    }

    ret = hal_wdg_test_creat_handle(&g_handle,g_watchdog_test_resource.res_info[watchdog_number].res_glb_idx);
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
