/*
* app_dio.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: sample code for gpio/dio
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
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

const Dio_ChannelType channel50 = DIO_CHANNEL_50;
const Dio_LevelType channel50Level = 1;

const Dio_PortType portId = 0;
const Dio_PortLevelType portLevel = 0x50505050;

const Dio_ChannelGroupType channelGrp = {0x01, 0x10, 0x00};

static int cmd_dio(int argc, const cmd_args *argv)
{
    bool ret = true;
    static void *g_handle;
    Dio_LevelType channelLevel;
    Dio_PortLevelType portLevel;
    Dio_PortLevelType mask = 0x0f;

    printf("+++++dio sample cmd_dio \n");
    ret = hal_dio_creat_handle(&g_handle, RES_GPIO_GPIO2);  // GPIO2 for secure

    channelLevel = hal_dio_read_channel(g_handle, channel50);
    printf("hal_dio_read_channel: %d\n", channelLevel);

    hal_dio_write_channel(g_handle, channel50, channel50Level);
    printf("hal_dio_write_channel: %d\n", channel50Level);

    portLevel = hal_dio_read_port(g_handle, portId);
    printf("hal_dio_read_channel: 0x%lx\n", portLevel);

    hal_dio_write_port(g_handle, portId, portLevel);
    printf("hal_dio_write_port: %d\n", portLevel);

    portLevel = hal_dio_read_channel_group(g_handle, &channelGrp);
    printf("hal_dio_read_channel_group: %d\n", portLevel);

    hal_dio_write_channel_group(g_handle, &channelGrp, portLevel);
    printf("hal_dio_write_channel: %d\n", portLevel);

    channelLevel = hal_dio_flip_channel(g_handle, channel50);
    printf("hal_dio_flip_channel: %d\n", channelLevel);

    hal_dio_masked_write_port(g_handle, portId, portLevel, mask);
    printf("hal_dio_masked_write_port: %d\n", portLevel);

    hal_dio_release_handle(&g_handle);
    return ret;
}

static void dio_entry(const struct app_descriptor *app, void *args)
{
    printf("++++ dio sample dio_entry\n");
    cmd_dio(0, 0);
}

APP_START(dio_example)
.flags = 0,
.entry = dio_entry,
APP_END
