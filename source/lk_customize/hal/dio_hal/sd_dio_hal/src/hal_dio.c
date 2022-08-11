/*
 * hal_dio.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dio/gpio driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 11/15/2019 init version
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <trace.h>

#include "hal_dio.h"

spin_lock_t dio_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static struct dio_handle s_dio_handle;

bool hal_dio_creat_handle(void **handle, uint32_t dio_res_glb_idx)
{
    struct dio_handle *p_handle;
    int8_t ret = 0;
    paddr_t phy_addr = 0;
    int32_t real_idx = 0;
    spin_lock_saved_state_t states;

    ret = res_get_info_by_id(dio_res_glb_idx, &phy_addr, &real_idx);

    if (ret != -1) {
        dprintf(INFO, "hal_dio_creat_handle: phy_addr[0x%lx], real_idx[%d]\n", phy_addr,
               real_idx);
    }
    else {
        printf("hal_dio_creat_handle: res_get_info_by_id failed! 02\n");
        return false;
    }

    p_handle = &s_dio_handle;
    spin_lock_irqsave(&dio_spin_lock, states);
    p_handle->phy_addr = phy_addr;
    p_handle->real_idx = real_idx;
    *handle = p_handle;
    Dio_SetHandle((void *)p_handle);
    spin_unlock_irqrestore(&dio_spin_lock, states);

    return true;
}

bool hal_dio_release_handle(void **handle)
{
    ASSERT(handle);
    struct dio_handle *dio = *handle;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&dio_spin_lock, states);
    dio->phy_addr = 0;
    dio->real_idx = -1;
    *handle = NULL;
    spin_unlock_irqrestore(&dio_spin_lock, states);
    return true;
}

void hal_dio_enable_irq(const Dio_ChannelType ChannelId)
{
    Dio_enable_irq(ChannelId);
}

void hal_dio_disable_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    Dio_disable_irq(ChannelId, irqflag);
}

bool hal_dio_config_irq(const Dio_ChannelType ChannelId, int irqflag)
{
    if (irqflag == IRQ_TYPE_EDGE_RISING ||
            irqflag == IRQ_TYPE_EDGE_FALLING ||
            irqflag == IRQ_TYPE_EDGE_BOTH ||
            irqflag == IRQ_TYPE_LEVEL_HIGH ||
            irqflag == IRQ_TYPE_LEVEL_LOW) {
        Dio_config_irq(ChannelId, irqflag);
        return true;
    }

    return false;
}

bool hal_dio_get_irq_status(const Dio_ChannelType ChannelId)
{
    return Dio_get_irq_status(ChannelId);
}

Dio_LevelType hal_dio_read_channel(void *handle,
                                   const Dio_ChannelType ChannelId)
{
    ASSERT(handle);
    return Dio_ReadChannel(ChannelId);
}

void hal_dio_write_channel(void *handle, const Dio_ChannelType ChannelId,
                           const Dio_LevelType Level)
{
    ASSERT(handle);
    Dio_WriteChannel(ChannelId, Level);
    return;
}

Dio_PortLevelType hal_dio_read_port(void *handle,
                                    const Dio_PortType PortId)
{
    ASSERT(handle);
    return Dio_ReadPort(PortId);
}

void hal_dio_write_port(void *handle, const Dio_PortType PortId,
                        const Dio_PortLevelType Level)
{
    ASSERT(handle);
    Dio_WritePort(PortId, Level);
}

Dio_PortLevelType hal_dio_read_channel_group(void *handle,
        const Dio_ChannelGroupType *const ChannelGroupIdPtr)
{
    ASSERT(handle);
    return Dio_ReadChannelGroup(ChannelGroupIdPtr);
}

void hal_dio_write_channel_group(void *handle,
                                 const Dio_ChannelGroupType  *const ChannelGroupIdPtr,
                                 const Dio_PortLevelType Level)
{
    ASSERT(handle);
    Dio_WriteChannelGroup(ChannelGroupIdPtr, Level);
}

Dio_LevelType hal_dio_flip_channel(void *handle,
                                   const Dio_ChannelType ChannelId)
{
    ASSERT(handle);
    return Dio_FlipChannel(ChannelId);
}

void hal_dio_masked_write_port(void *handle, Dio_PortType PortId,
                               Dio_PortLevelType Level, Dio_PortLevelType Mask)
{
    ASSERT(handle);
    Dio_MaskedWritePort(PortId, Level, Mask);
}

int hal_dio_set_channel_direction(void *handle, const Dio_ChannelType ChannelId,
                               const Dio_ChannelDirectionType direction)
{
    ASSERT(handle);
    Dio_SetChannelDirection(ChannelId, direction);

    return true;
}

