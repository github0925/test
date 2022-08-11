/*
* app_i2c.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
* 001, 10/20/2019 henglei create this file
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include <platform/interrupts.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <platform/debug.h>
#include <trace.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "i2c_hal.h"

#define LOG_LEVEL 0
#define WR_SIZE 16
#define WR_SIZE2 16
#define MEM_SIZE 1024

static int i2c_thread_1(void *arg)
{
    dprintf(LOG_LEVEL, "app:[t1] %s():.\n", __func__);
    i2c_app_config_t config;
    void *i2c_handle;

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);

    uint8_t addr = 0x50;

    uint8_t reg[2] = {0x00, 0x00};
    uint8_t *data = NULL;
    int i, j;
    uint8_t *rdata;
    data = malloc(MEM_SIZE);
    rdata = malloc(WR_SIZE);

    for (i = 0; i < MEM_SIZE / 4;) {
        *(data + i) = i;
        *(data + i + 1) = i + 1;
        *(data + i + 2) = i + 2;
        *(data + i + 3) = i + 3;
        i = i + 4;
    }

    dprintf(LOG_LEVEL,
            "[t1]: i2c11 transmit: address: 0x50, read reg: 0x0000, WR_SIZE=%d.\n",
            WR_SIZE);

    for (;;) {
        dprintf(LOG_LEVEL, "[t1]: restart\n");

        for (j = 0; j < 4; j++) {
            for (i = 0; i < 16; i++) {
                dprintf(LOG_LEVEL, "[t1]write block-%d, page %d, time=%d.\n", j, i,
                        current_time());
                reg[0] = WR_SIZE * i;
                hal_i2c_write_reg_data(i2c_handle, addr + j, reg, 1, data + (WR_SIZE * i),
                                       WR_SIZE);

                dprintf(LOG_LEVEL, "[t1]read block-%d, page %d, time=%d.\n", j, i,
                        current_time());
                memset(rdata, 0x00, WR_SIZE);
                hal_i2c_read_reg_data(i2c_handle, addr + j, reg, 1, rdata, WR_SIZE);

                if (memcmp(data + (WR_SIZE * i), rdata, WR_SIZE) == 0) {
                    dprintf(LOG_LEVEL, "[t1]w/r check done, time=%d\n", current_time());
                }
                else {
                    dprintf(LOG_LEVEL, "[t1]w/r error, time=%d\n", current_time());
                }
            }
        }

        dprintf(LOG_LEVEL, "[t1]: done\n");
        thread_sleep(20);
    }

    hal_i2c_release_handle(i2c_handle);
    dprintf(LOG_LEVEL, "app: [t1] end.\n");
    free(data);
    return 0;
}

static int i2c_thread_2(void *arg)
{
    dprintf(LOG_LEVEL, "app:[t2] %s():.\n", __func__);
    i2c_app_config_t config;
    void *i2c_handle;

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);

    uint8_t addr = 0x57;

    uint8_t reg[2] = {0x00, 0x00};
    uint8_t *data = NULL;
    int i, j;
    int ret = 0;
    uint8_t *rdata;
    data = malloc(MEM_SIZE);
    rdata = malloc(WR_SIZE);

    for (i = 0; i < MEM_SIZE / 4;) {
        *(data + i) = i;
        *(data + i + 1) = i + 1;
        *(data + i + 2) = i + 2;
        *(data + i + 3) = i + 3;
        i = i + 4;
    }

    dprintf(LOG_LEVEL,
            "[t2]: i2c11 transmit: address: 0x57, read reg: 0x0000, WR_SIZE2=%d.\n",
            WR_SIZE2);

    while (1) {
        //thread_sleep(1);
        dprintf(LOG_LEVEL, "[t2]: restart\n");

        for (j = 0; j < 4; j++) {
            for (i = 0; i < 16; i++) {
                dprintf(LOG_LEVEL, "[t2]write block-%d, page %d, time=%d.\n", j, i,
                        current_time());
                reg[0] = WR_SIZE * i;
                hal_i2c_write_reg_data(i2c_handle, addr + j, reg, 1, data + (WR_SIZE * i),
                                       WR_SIZE);

                dprintf(LOG_LEVEL, "[t2]read block-%d, page %d, time=%d.\n", j, i,
                        current_time());
                memset(rdata, 0x00, WR_SIZE);
                hal_i2c_read_reg_data(i2c_handle, addr + j, reg, 1, rdata, WR_SIZE);

                if (memcmp(data + (WR_SIZE * i), rdata, WR_SIZE) == 0) {
                    dprintf(LOG_LEVEL, "[t1]w/r check done, time=%d\n", current_time());
                }
                else {
                    dprintf(LOG_LEVEL, "[t1]w/r error, time=%d\n", current_time());
                }
            }
        }

        dprintf(LOG_LEVEL, "[t2]: done\n");
        thread_sleep(10);
    }

    hal_i2c_release_handle(i2c_handle);
    free(data);
    dprintf(LOG_LEVEL, "app: [t2] end.\n");
    return 0;
}

static void i2c_entry(const struct app_descriptor *app, void *args)
{
    dprintf(LOG_LEVEL, "app: i2c_entry():.\n");

    thread_t *t1 = NULL;
    thread_t *t2 = NULL;
    t1 = thread_create("i2c thread 1 daemon",
                       i2c_thread_1,
                       NULL,
                       DEFAULT_PRIORITY,
                       DEFAULT_STACK_SIZE);
    t2 = thread_create("i2c thread 2 daemon",
                       i2c_thread_2,
                       NULL,
                       HIGH_PRIORITY,
                       DEFAULT_STACK_SIZE);
    thread_resume(t1);
    thread_resume(t2);

    dprintf(LOG_LEVEL, "app: i2c_entry(): end.\n");
}


char i2c_tool_scan_help[] = {
    "do_i2c_tool_scan: i2c bus really number\n" \
    "" \
};
int do_i2c_tool_scan(int argc, const cmd_args *argv)
{
    bool ret = true;
    int i;
    void *i2c_handle;

    dprintf(LOG_LEVEL, "%s(): start\n", __func__);
    ret = hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);

    if (ret == false) {
        dprintf(LOG_LEVEL, "%s(): fail to create handle\n", __func__);
    }

    dprintf(LOG_LEVEL, "scan start\n");

    for (i = 0; i < 0x80; i++) {
        if (i == 0x4f || i == 0x50 || i == 0x51)
            ret = hal_i2c_scan(i2c_handle, i);
    }

    dprintf(LOG_LEVEL, "scan end.\n");
    hal_i2c_release_handle(i2c_handle);
    dprintf(LOG_LEVEL, "%s(): end\n", __func__);
    return ret;
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("i2c_tool_scan", i2c_tool_scan_help,
               (console_cmd)&do_i2c_tool_scan)
STATIC_COMMAND_END(i2ctool);
#endif

APP_START(i2c_example)
.flags = 0,
.entry = i2c_entry,
APP_END
