/*
 * hal_dio_weak.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dio/iomux driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <trace.h>

#include "hal_dio.h"

bool hal_dio_creat_handle(void **handle, uint32_t dio_res_glb_idx)
{
    return true;
}

bool hal_dio_release_handle(void **handle)
{
    return true;
}

void hal_dio_enable_irq(const Dio_ChannelType ChannelId)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return;
}

void hal_dio_disable_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return;
}

bool hal_dio_config_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return false;
}

bool hal_dio_get_irq_status(const Dio_ChannelType ChannelId)
{
    dprintf(ALWAYS, "%s, not implement\n", __func__);
    return false;
}

Dio_LevelType hal_dio_read_channel(void *handle,
                                   const Dio_ChannelType ChannelId)
{
    return 0;
}

void hal_dio_write_channel(void *handle, const Dio_ChannelType ChannelId,
                           const Dio_LevelType Level)
{
    return;
}

Dio_PortLevelType hal_dio_read_port(void *handle,
                                    const Dio_PortType PortId)
{
    return 0;
}

void hal_dio_write_port(void *handle, const Dio_PortType PortId,
                        const Dio_PortLevelType Level)
{
    return;
}

Dio_PortLevelType hal_dio_read_channel_group(void *handle,
        const Dio_ChannelGroupType *const ChannelGroupIdPtr)
{
    return 0;
}

void hal_dio_write_channel_group(void *handle,
                                 const Dio_ChannelGroupType  *const ChannelGroupIdPtr,
                                 const Dio_PortLevelType Level)
{
    return;
}

Dio_LevelType hal_dio_flip_channel(void *handle,
                                   const Dio_ChannelType ChannelId)
{
    return 0;
}

void hal_dio_masked_write_port(void *handle, Dio_PortType PortId,
                               Dio_PortLevelType Level, Dio_PortLevelType Mask)
{
    return;
}
