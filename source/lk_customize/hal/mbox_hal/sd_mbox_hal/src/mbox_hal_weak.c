//*****************************************************************************
//
// mbox_hal.c - Driver for the rstgen hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <stdlib.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <platform.h>
#include <sys/types.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <kernel/event.h>
#include "res.h"
#include "chip_res.h"
#include "target_res.h"
#include "mbox_hal.h"


/*mbox HAL interface*/
hal_mb_client_t hal_mb_get_client(void)
{
    return NULL;
}

hal_mb_client_t hal_mb_get_client_with_addr(u8 myaddr)
{
    return NULL;
}

void hal_mb_put_client(hal_mb_client_t cl)
{
    return;
}

hal_mb_chan_t* hal_mb_request_channel(hal_mb_client_t cl, bool low_latency,
        hal_mb_rx_cb cb, hal_mb_proc_t remote)
{
    return NULL;
}

hal_mb_chan_t* hal_mb_request_channel_with_addr(hal_mb_client_t cl,
        bool low_latency, hal_mb_rx_cb cb, hal_mb_proc_t remote, int address)
{
    return NULL;
}

void hal_mb_free_channel(hal_mb_chan_t *chan)
{
    return;
}

int hal_mb_send_data(hal_mb_chan_t *chan, u8* data, u16 len, u32 timeoutMs)
{
    return 0;
}

int hal_mb_send_data_rom(hal_mb_chan_t *chan, u8 *data, u16 len)
{
    return 0;
}

int hal_mb_send_data_dsp(hal_mb_chan_t *chan, u8 *data, u16 len)
{
    return 0;
}

int hal_mb_alloc_txbuf(hal_mb_chan_t *chan, u8 **data, u16 *len)
{
    return 0;
}

int hal_mb_send_data_nocopy(hal_mb_chan_t *chan, hal_mb_buf_id idx, u32 timeoutMs)
{
    return 0;
}

int hal_mb_cancel_lastsend(hal_mb_chan_t *chan)
{
    return 0;
}

int hal_mb_recv_msg(hal_mb_chan_t *chan, void *msg)
{
    return NO_ERROR;
}

static enum handler_return mb_irq_handler(void *arg)
{
    return 0;
}

void hal_mb_init(void *cl, hal_mb_cfg_t *cfg)
{
    return;
}

void hal_mb_deinit(void *cl)
{
    return;
}

bool hal_mb_create_handle(void **handle, uint32_t res_glb_idx)
{
    return false;
}

bool hal_mb_release_handle(void *handle)
{
    return false;
}

int hal_mb_set_user(hal_mb_client_t cl, void *data)
{
    return 0;
}

void *hal_mb_get_user(hal_mb_client_t cl)
{
    return NULL;
}

