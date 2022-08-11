/*
* erpc_test.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: erpc_test samplecode.
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
#include "Can.h"

extern const Can_ConfigType gCan_Config;
extern void virCanIf_Init(void);


int vircan_init_test(int argc, const cmd_args *argv)
{
    //printf("%s() start\n", __func__);
    //Can_Init(&gCan_Config);
    Can_DeInit();
    //printf("%s() end\n", __func__);
    return 0;
}

static void erpc_test_init(const struct app_descriptor *app)
{
    virCanIf_Init();
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("test_vircan", "vircan init test", (console_cmd)&vircan_init_test)
STATIC_COMMAND_END(erpc_test);
#endif

APP_START(erpc_test)
.init = erpc_test_init,
.flags = 0,
APP_END
