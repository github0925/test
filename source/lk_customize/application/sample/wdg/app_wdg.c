/*
* app_wdg.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include "wdg_hal.h"
#include "res.h"
#include "chip_res.h"

void *watchdog_handle;

const wdg_glb_idx_to_id g_wdg_glb_idx_to_id[DEFAULT_WDG_MAX_NUM] = {
    {RES_WATCHDOG_WDT1, wdg_really_num1},
    {RES_WATCHDOG_WDT2, wdg_really_num2},
    {RES_WATCHDOG_WDT3, wdg_really_num3},
    {RES_WATCHDOG_WDT4, wdg_really_num4},
    {RES_WATCHDOG_WDT5, wdg_really_num5},
    {RES_WATCHDOG_WDT6, wdg_really_num6},
    {RES_WATCHDOG_WDT7, wdg_really_num7},
    {RES_WATCHDOG_WDT8, wdg_really_num8},
};

enum handler_return wdg_irq_handle(void *arg)
{
    printf("wdg_irq_handle instance\n");
    //add reset or mendump func
    hal_wdg_disable_interrupts(arg);
    hal_wdg_int_clear(arg);
    platform_halt(HALT_ACTION_REBOOT, HALT_REASON_HW_WATCHDOG);
    /* We should never get here; watchdog handlers should always be fatal. */
    //DEBUG_ASSERT(false);
    return INT_NO_RESCHEDULE;
}

static int cmd_wdg(int argc, const cmd_args *argv)
{
    wdg_app_config_t wdg_app_cfg;
    bool ret = false;
    uint32_t wdg_refresh_time =100;
    uint32_t wdg_number =0;
    uint8_t i = 0;

    if (argc < 3) {
        printf("not enough arguments\n");
        printf("%s <kernel addr> <dtb addr>\n", argv[0].str);
        return -1;
    }

    wdg_refresh_time = 100;
    wdg_number =(uint32_t)argv[i].i;
    if(wdg_number ==0 || wdg_number >8){
        return 0;
    }

    hal_wdg_creat_handle(&watchdog_handle,g_wdg_glb_idx_to_id[wdg_number-1].res_glb_idx);
    /*get handle ok and enable wdg is true*/
    if(watchdog_handle && argv[i].b){
        wdg_app_cfg.workMode = wdg_mode1;
        wdg_app_cfg.seqDeltaValue = 100; //ms
        wdg_app_cfg.timeoutValue = 500; //ms
        wdg_app_cfg.windowLimitValue = 1000;//ms
        ret = hal_wdg_init(watchdog_handle,&wdg_app_cfg);
        if(ret == true){
            ret = hal_wdg_int_register(watchdog_handle,wdg_irq_handle,true);
        }

        if(ret == true){
            ret = hal_wdg_set_timeout(watchdog_handle,1500);//timeout 1500ms
        }else{
            hal_wdg_deinit(watchdog_handle);
        }

        if(ret == true){
            ret = hal_wdg_enable_interrupts(watchdog_handle);
        }else{
            hal_wdg_deinit(watchdog_handle);
        }

        if(ret == true){
            ret = hal_wdg_enable(watchdog_handle);
        }else{
            hal_wdg_deinit(watchdog_handle);
        }

        if(ret != true){
            ret = hal_wdg_release_handle(watchdog_handle);
        }

        while(wdg_refresh_time > 0){
            spin(200*1000);
            hal_wdg_refresh(watchdog_handle);
            wdg_refresh_time --;
        }

    }else if(watchdog_handle && !argv[i].b){
        /*disable watchdog*/
        if(watchdog_handle){
            ret = hal_wdg_disable(watchdog_handle);
            if(ret == true){
                hal_wdg_deinit(watchdog_handle);
                hal_wdg_release_handle(watchdog_handle);
            }
        }
    }else{
        printf("watchdog gei handle failed:watchdog number =%d \n",argv[i].i);
    }

    return 0;
}

static void wdg_entry(const struct app_descriptor *app, void *args)
{
    cmd_args argv[3];
    /*watchdog1*/
    argv[0].u=1000; //timeout ms
    argv[0].i=0;    //watchdog really number
    argv[0].b=true; //enable or disable

    cmd_wdg(4,&argv[0]);
}
APP_START(wdg_example)
.flags = 0,
.entry=wdg_entry,
APP_END
