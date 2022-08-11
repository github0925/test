//*****************************************************************************
//
// dma_weak_hal.c - Driver for the Watchdog hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include "dma_hal.h"
#include <assert.h>
#include <platform.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <string.h>
#include <sys/types.h>
#include <trace.h>
/**
 *  A dma init function for dma controller hardware
 */
void hal_dma_init(void) { return; }
void hal_dma_deinit(void) { return; }
/**
 *  Allocate a DMA slave channel
 * return a available dma_chan
 */
struct dma_chan *hal_dma_chan_req(enum dma_chan_tr_type ch_type)
{
    return NULL;
}

/**
 *  Allocate a DMA slave channel with id number
 * return a available dma_chan
 */
struct dma_chan *hal_dma_chan_req_with_ch(u32 id) { return NULL; }

/*
    hal_dma_dev_config: Set device and controller specific parameters
    @dma_chan the channel of dma_chan_req result
    @dma_dev_cfg  dma device configuration
    //return 0 is success.
    */
void hal_dma_dev_config(struct dma_chan *chan, struct dma_dev_cfg *cfg)
{
    return;
}

/*
    dw_prep_dma_memcpy: Get a descriptor for memory to memory transaction
    @dest the destination of memory copy
    @src  the source of memory copy
    @len  the length of memory copy
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_memcpy(struct dma_chan *chan, void *dest,
                                     void *src, size_t len, unsigned long flags)
{
    return NULL;
}

/*
    hal_prep_dma_memset: Get a descriptor for memset function.
    @val the value of memset unsigned char
    @buf_addr  the target of memory set
    @count  the number of memory set
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_memset(struct dma_chan *chan, u8 val,
                                     void *buf_addr, size_t count,
                                     unsigned long flags)
{
    return NULL;
}

/*
    hal_prep_dma_dev: Get a descriptor for memory to peripheral transaction
    @buf_addr the destination of memory
    @buf_len  the length of transfer
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_dev(struct dma_chan *chan, void *buf_addr,
                                  size_t buf_len, unsigned long flags)
{
    return NULL;
}

/*
    hal_prep_dma_cyclic: Get a descriptor for memory to peripheral transaction
   in cyclic mode
    @buf_addr the destination of memory
    @buf_len  the length of transfer
    @period_len the length of period, buf_len must be a multiple of period_len.
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_cyclic(struct dma_chan *chan, void *buf_addr,
                                     size_t buf_len, size_t period_len,
                                     unsigned long flags)
{
    return NULL;
}

/*
    dma_submit: Submit a transfer
    @desc the descriptor of a dma transfer
    */
void hal_dma_submit(struct dma_desc *desc) { return; }

/*
    hal_dma_sync_wait: Wait for dma transfer result.
    @desc the descriptor of a dma transfer
    @timeout ms if timeout this function will return a error status.
    */
enum dma_status hal_dma_sync_wait(struct dma_desc *desc, int timeout)
{
    return 0;
}

/*
    dma_terminate: terminate a dma channel.
    @chan dma transfer channel.
    */
bool hal_dma_terminate(struct dma_desc *desc) { return NULL; }

/*
    hal_dma_free_desc: free a dma descriptor.
    Must free desc when you don't use it.
    @desc dma transfer descriptor.
    */
void hal_dma_free_desc(struct dma_desc *desc) { return; }
