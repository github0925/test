/*
* app_rstgen.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
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
#include "rstgen_hal.h"
#include "res.h"
#include "chip_res.h"


static int cmd_rstgen(int argc, const cmd_args *argv)
{
    bool ret = true;
    static void *g_handle;
    ret = hal_rstgen_creat_handle(&g_handle,RES_GLOBAL_RST_SEC_RST_EN);

    if(!ret){
        return ret;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(g_handle);

    if(ret){
        ret = hal_rstgen_iso_enable(g_handle,RES_ISO_EN_SEC_CPU1);
    }else{
        hal_rstgen_release_handle(g_handle);
        return false;
    }

    if(ret){
        ret = hal_rstgen_module_reset(g_handle,RES_MODULE_RST_SEC_DC1);
    }else{
        hal_rstgen_release_handle(g_handle);
        return false;
    }

    if(ret){
        ret = hal_rstgen_core_reset(g_handle,RES_CORE_RST_SEC_CPU1_CORE_ALL_SW);//timeout 1500ms
    }else{
        hal_rstgen_release_handle(g_handle);
        return false;
    }

    return 0;
}

static void rstgen_entry(const struct app_descriptor *app, void *args)
{
    cmd_rstgen(0,0);
}
APP_START(rstgen_example)
.flags = 0,
.entry=rstgen_entry,
APP_END
