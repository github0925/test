//*****************************************************************************
//
// cmd_i2c.c - app for the i2c test Module.
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
#include "i2c_hal_ip_test.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include <sdunittest.h>

static void *g_handle;
uint32_t i2c_test_timeout = 6;//1000;//ms

struct i2c_bus_index {
    uint32_t i2c_bus_idx;
    uint32_t res_glb_idx;
};

#define DEFAULT_I2C_MAX_NUM 16
struct i2c_bus_index g_table[DEFAULT_I2C_MAX_NUM] = {
    {16, RES_I2C_I2C16},
    {15, RES_I2C_I2C15},
    {14, RES_I2C_I2C14},
    {13, RES_I2C_I2C13},
    {12, RES_I2C_I2C12},
    {11, RES_I2C_I2C11},
    {10, RES_I2C_I2C10},
    {9, RES_I2C_I2C9},
    {8, RES_I2C_I2C8},
    {7, RES_I2C_I2C7},
    {6, RES_I2C_I2C6},
    {5, RES_I2C_I2C5},
    {4, RES_I2C_I2C4},
    {3, RES_I2C_I2C3},
    {2, RES_I2C_I2C2},
    {1, RES_I2C_I2C1},
};


char i2c_dump_all_reg_test_help[] = {
    "do_i2c_dump_all_reg_test: i2c bus really number\n" \
    "" \
};
int do_i2c_dump_all_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t bus_number = 0;
    uint32_t i2c_res_glb_idx = 0;
    uint32_t i = 0;

    bus_number = argv[1].u;

    if (bus_number >= DEFAULT_I2C_MAX_NUM) {
        ret = false;
        printf("do_i2c_dump_all_reg_test ret:%d:\n", ret);
        return ret;
    }

    for (i = 0; i < DEFAULT_I2C_MAX_NUM; i++) {
        if (g_table[i].i2c_bus_idx == bus_number) {
            i2c_res_glb_idx = g_table[i].res_glb_idx;
            break;
        }
    }

    printf("do_i2c_dump_all_reg_test str:%s   u:%d\n", argv[1].str, argv[1].u);
    ret = hal_i2c_test_creat_handle(&g_handle, i2c_res_glb_idx);

    if (ret) {
        ret = hal_i2c_dump_all_reg_test(g_handle);
    }

    hal_i2c_test_release_handle(g_handle);
    printf("do_i2c_dump_all_reg_test ret:%d:\n", ret);
    return ret;
}

char i2c_read_only_reg_test_help[] = {
    "do_i2c_read_only_reg_test: i2c bus really number\n" \
    "" \
};
int do_i2c_read_only_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t bus_number = 0;
    uint32_t i2c_res_glb_idx = 0;
    uint32_t i = 0;

    bus_number = argv[1].u;

    if (bus_number >= DEFAULT_I2C_MAX_NUM) {
        ret = false;
        printf("do_i2c_read_only_reg_test ret:%d:\n", ret);
        return ret;
    }

    for (i = 0; i < 12; i++) {
        if (g_table[i].i2c_bus_idx == bus_number) {
            i2c_res_glb_idx = g_table[i].res_glb_idx;
            break;
        }
    }

    printf("do_i2c_read_only_reg_test str:%s   u:%d\n", argv[1].str,
           argv[1].u);
    ret = hal_i2c_test_creat_handle(&g_handle, i2c_res_glb_idx);

    if (ret) {
        ret = hal_i2c_read_only_reg_test(g_handle);
    }

    hal_i2c_test_release_handle(g_handle);
    printf("do_i2c_read_only_reg_test ret:%d:\n", ret);
    return ret;
}

char i2c_rw_reg_test_help[] = {
    "do_i2c_rw_reg_test: i2c bus really number\n" \
    "" \
};
int do_i2c_rw_reg_test(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint8_t bus_number = 0;
    uint32_t i2c_res_glb_idx = 0;
    uint32_t i = 0;

    bus_number = argv[1].u;

    if (bus_number >= DEFAULT_I2C_MAX_NUM) {
        ret = false;
        printf("do_i2c_rw_reg_test ret:%d:\n", ret);
        return ret;
    }

    for (i = 0; i < 12; i++) {
        if (g_table[i].i2c_bus_idx == bus_number) {
            i2c_res_glb_idx = g_table[i].res_glb_idx;
            break;
        }
    }

    printf("do_i2c_rw_reg_test str:%s   u:%d\n", argv[1].str, argv[1].u);
    ret = hal_i2c_test_creat_handle(&g_handle, i2c_res_glb_idx);

    if (ret) {
        ret = hal_i2c_rw_reg_test(g_handle);
    }

    hal_i2c_test_release_handle(g_handle);
    printf("do_i2c_rw_reg_test ret:%d:\n", ret);
    return ret;
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("i2c_test1", i2c_dump_all_reg_test_help,
               (console_cmd)&do_i2c_dump_all_reg_test)
STATIC_COMMAND("i2c_test2", i2c_read_only_reg_test_help,
               (console_cmd)&do_i2c_read_only_reg_test)
STATIC_COMMAND("i2c_test3", i2c_rw_reg_test_help,
               (console_cmd)&do_i2c_rw_reg_test)
STATIC_COMMAND_END(i2ctest);

DEFINE_REGISTER_TEST_COMMAND(i2c_test1, i2c, i2c_test1 6);
DEFINE_REGISTER_TEST_COMMAND(i2c_test2, i2c, i2c_test2 6);
DEFINE_REGISTER_TEST_COMMAND(i2c_test3, i2c, i2c_test3 6);

#endif

APP_START(i2c)
.flags = 0
         APP_END
