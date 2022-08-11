/*
* wdg_init.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg system init .
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
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

void *watchdog_init_handle;

static wdg_src_t g_watchdog_app_resource= watchdog_res_def;

static void wdg_system_entry(const struct app_descriptor *app, void *args)
{
    uint32_t wdg_number =0;
    uint8_t i = 0;
    wdg_app_config_t wdg_app_cfg = {.workMode = wdg_mode1,
                                                            .seqDeltaValue = 100, //ms
                                                            .timeoutValue = 1000, //ms
                                                            .windowLimitValue = 1000,//ms
                                                            };

    for(i = 0; i<g_watchdog_app_resource.res_num;i++){
        hal_wdg_creat_handle(&watchdog_init_handle,g_watchdog_app_resource.res_info[i].res_glb_idx);
        /*get handle ok and enable wdg is true*/
        if(watchdog_init_handle){
            hal_wdg_init(watchdog_init_handle,&wdg_app_cfg);
        }else{
            printf("watchdog gei handle failed:watchdog number =%d \n",i);
        }
        hal_wdg_release_handle(watchdog_init_handle);
    }
    return;
}

APP_START(wdg_system_init)
.flags = 0,
.entry=wdg_system_entry,
APP_END
