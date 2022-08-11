/*
 * sdpe_rpc_mbox_transport.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <kernel/event.h>
#include <kernel/semaphore.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <assert.h>
#include <list.h>
#include <mbox_hal.h>

#include "sdpe_rpc_transport.h"
#include "sdpe_rpc_cbuf.h"

#define SDPE_RPC_MBOX_RX_BUF        600
#define SDPE_RPC_MBOX_MAGIC         0xfefe

typedef struct sdpe_rpc_mbox_instance {
    uint8_t remote;
    uint32_t addr;
    hal_mb_client_t mb_cl;
    hal_mb_chan_t *mb_chan;
    sdpe_rpc_cbuf_t rx_buf;
} sdpc_rpc_mbox_instance_t;

static int sdpe_rpc_mbox_open(sdpe_rpc_transport_dev_t *dev, const char *name);
static int sdpe_rpc_mbox_close(sdpe_rpc_transport_dev_t *dev);
static void *sdpe_rpc_mbox_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size,
                                    uint32_t wait_time);
static int sdpe_rpc_mbox_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time);
static int sdpe_rpc_mbox_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                                     uint32_t len);
static int sdpe_rpc_mbox_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t *len, bool block);

struct sdpe_rpc_transport_ops g_rpc_mbox_ops = {
    .open                   = sdpe_rpc_mbox_open,
    .close                  = sdpe_rpc_mbox_close,
    .tx_alloc               = sdpe_rpc_mbox_tx_alloc,
    .send                   = sdpe_rpc_mbox_send,
    .send_nocopy            = sdpe_rpc_mbox_send_nocopy,
    .recv                   = sdpe_rpc_mbox_recv,
};

static void sdpe_rpc_mb_rx_callback(hal_mb_client_t cl, void *data, uint16_t len)
{
    struct sdpe_rpc_mbox_instance *mbox = hal_mb_get_user(cl);

    ASSERT(mbox != NULL);
    ASSERT(data != NULL);

    sdpe_rpc_cbuf_write(&mbox->rx_buf, data, len);
}

static int sdpe_rpc_mbox_open(sdpe_rpc_transport_dev_t *dev, const char *name)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;

    ASSERT(mbox != NULL);

    mbox->mb_cl = hal_mb_get_client_with_addr(mbox->addr);
    if (!mbox->mb_cl) {
        dprintf(ALWAYS, "mbox client get fail\n");
        goto err_out;
    }

    hal_mb_set_user(mbox->mb_cl, (void *)mbox);

    mbox->mb_chan = hal_mb_request_channel_with_addr(mbox->mb_cl, false,
                                                    sdpe_rpc_mb_rx_callback,
                                                    mbox->remote, mbox->addr);
    if (!mbox->mb_chan) {
        dprintf(ALWAYS, "mbox channel get fail\n");
        goto err_out;
    }

    return 0;

err_out:
    if (mbox->mb_chan)
        hal_mb_free_channel(mbox->mb_chan);

    if (mbox->mb_cl)
        hal_mb_put_client(mbox->mb_cl);

    return -1;
}

static int sdpe_rpc_mbox_close(sdpe_rpc_transport_dev_t *dev)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;

    ASSERT(mbox != NULL);

    hal_mb_free_channel(mbox->mb_chan);
    hal_mb_put_client(mbox->mb_cl);
    free(mbox);

    return 0;
}

static void *sdpe_rpc_mbox_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;
    uint16_t remain_size = *size + 2;
    uint16_t index;
    void *buffer;
    uint16_t *idx_ptr;

    index = hal_mb_alloc_txbuf(mbox->mb_chan, (uint8_t **)&buffer, &remain_size);
    if (index < 0) {
        /* need interface to free buffer */
        return NULL;
    }

    idx_ptr = (uint16_t*)((uint8_t*)buffer + *size);
    *idx_ptr = index;
    return buffer;
}

static int sdpe_rpc_mbox_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;
    int ret;

    ASSERT(mbox != NULL && data != NULL);
    ret = hal_mb_send_data(mbox->mb_chan, data, len, wait_time);

    return ret;
}

static int sdpe_rpc_mbox_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;
    uint16_t *idx_ptr;
    int ret;

    ASSERT(mbox != NULL && data != NULL);

    idx_ptr = (uint16_t*)(data + len);
    ret = hal_mb_send_data_nocopy(mbox->mb_chan, *idx_ptr, SDPE_RPC_MAX_WAIT);

    return ret;
}

static int sdpe_rpc_mbox_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t *len, bool block)
{
    struct sdpe_rpc_mbox_instance *mbox = dev->priv;
    uint32_t data_len = sdpe_rpc_cbuf_read(&mbox->rx_buf, data);

    if (data_len > *len) {
        dprintf(ALWAYS, "mbox buf read len error:%d\n", data_len);
        return -1;
    }

    *len = data_len;
    return 0;
}

int sdpe_rpc_mbox_init(sdpe_rpc_transport_dev_t *dev,
                       uint8_t remote, uint32_t addr)
{
    struct sdpe_rpc_mbox_instance *mbox;

    mbox = malloc(sizeof(struct sdpe_rpc_mbox_instance));
    if (mbox == NULL) {
        dprintf(ALWAYS, "mbox instance alloc fail\n");
        goto err_out;
    }

    mbox->remote = remote;
    mbox->addr = addr;
    dev->priv = mbox;
    dev->ops = &g_rpc_mbox_ops;
    dev->mtu = HAL_MB_MTU;

    sdpe_rpc_cbuf_init(&mbox->rx_buf, SDPE_RPC_MBOX_RX_BUF);

    return 0;

err_out:
    return -1;
}
