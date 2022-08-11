/*
* init_port.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: port system init .
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
#include "hal_port.h"
#include "res.h"
#include "chip_res.h"

void *port_init_handle;

static void port_system_entry(const struct app_descriptor *app, void *args)
{
    hal_port_creat_handle(&port_init_handle, RES_IOMUXC_RTC_IOMUXC_RTC);
    printf("ssystem: port_system_entry\n");

    if (port_init_handle) {
        hal_port_init(port_init_handle);
    }
    else {
        printf("port get handle failed! \n");
    }

    hal_port_release_handle(&port_init_handle);

    return;
}

APP_START(port_system_init)
.flags = 0,
.entry = port_system_entry,
APP_END
