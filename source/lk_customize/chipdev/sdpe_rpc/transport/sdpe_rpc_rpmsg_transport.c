/*
 * sdpe_rpc_rpmsg_transport.c
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
#include <kernel/thread.h>
#include <assert.h>

#include "dcf_common.h"
#include "rpmsg_rtos.h"
#include "sdpe_rpc_transport.h"

typedef struct sdpe_rpc_rpmsg_instance {
    struct rpmsg_channel *rpmsg_channel;
    uint32_t remote;
    uint32_t addr;
    uint8_t rx_buf[RL_BUFFER_PAYLOAD_SIZE];
} sdpe_rpc_rpmsg_instance_t;

extern struct rpmsg_lite_instance *rpmsg_get_instance(int remote);
static int sdpe_rpc_rpmsg_open(sdpe_rpc_transport_dev_t *dev, const char *name);
static int sdpe_rpc_rpmsg_close(sdpe_rpc_transport_dev_t *dev);
static void *sdpe_rpc_rpmsg_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size,
                                     uint32_t wait_time);
static int sdpe_rpc_rpmsg_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t len, uint32_t wait_time);
static int sdpe_rpc_rpmsg_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                                      uint32_t len);
static int sdpe_rpc_rpmsg_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t *len, bool block);

struct sdpe_rpc_transport_ops g_rpc_rpmsg_ops = {
    .open                   = sdpe_rpc_rpmsg_open,
    .close                  = sdpe_rpc_rpmsg_close,
    .tx_alloc               = sdpe_rpc_rpmsg_tx_alloc,
    .send                   = sdpe_rpc_rpmsg_send,
    .send_nocopy            = sdpe_rpc_rpmsg_send_nocopy,
    .recv                   = sdpe_rpc_rpmsg_recv,
};

static int sdpe_rpc_rpmsg_open(sdpe_rpc_transport_dev_t *dev, const char *name)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;
    struct rpmsg_channel *chn;

    ASSERT(rpmsg != NULL);

    chn = rpmsg_channel_create(rpmsg->remote, rpmsg->addr, name);
    if (!chn)
        goto err_out;

    rpmsg->rpmsg_channel = chn;

    if (rpmsg_channel_start(rpmsg->rpmsg_channel, NULL) < 0) {
        dprintf(ALWAYS, "rpmsg_channel_start fail\n");
        goto err_out_with_ept;
    }

    return 0;

err_out_with_ept:
    rpmsg_channel_destroy(rpmsg->rpmsg_channel);

err_out:
    return -1;
}

static int sdpe_rpc_rpmsg_close(sdpe_rpc_transport_dev_t *dev)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;

    ASSERT(rpmsg != NULL);

    if (rpmsg->rpmsg_channel) {
        rpmsg_channel_stop(rpmsg->rpmsg_channel);
        rpmsg_channel_destroy(rpmsg->rpmsg_channel);
    }

    return 0;
}

static void *sdpe_rpc_rpmsg_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;
    uint32_t remain_size = 0;
    void *buffer;

    buffer = rpmsg_channel_alloc_tx_buf(rpmsg->rpmsg_channel, (unsigned long *)&remain_size, wait_time);
    *size = remain_size;
    return buffer;
}

static int sdpe_rpc_rpmsg_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t len, uint32_t wait_time)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;
    int ret;

    ASSERT(rpmsg != NULL && data != NULL);

    ret = rpmsg_channel_sendto(rpmsg->rpmsg_channel, rpmsg->addr,
                          (char *)data, len, wait_time);

    return ret;
}

static int sdpe_rpc_rpmsg_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;
    int ret;

    ASSERT(rpmsg != NULL && data != NULL);

    ret = rpmsg_channel_sendto_nocopy(rpmsg->rpmsg_channel, rpmsg->addr, (char *)data, len);

    return ret;
}

static int sdpe_rpc_rpmsg_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                               uint32_t *len, bool block)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg = dev->priv;
    unsigned long src;
    unsigned long wait_time = 0;

    ASSERT(rpmsg != NULL && data != NULL);

    if (block) {
        wait_time = SDPE_RPC_WAIT_FOREVER;
    }

    rpmsg_channel_recvfrom(rpmsg->rpmsg_channel, &src, (char *)data,
                           RL_BUFFER_PAYLOAD_SIZE, (int *)len, wait_time);

    return 0;
}

int sdpe_rpc_rpmsg_init(sdpe_rpc_transport_dev_t *dev,
                        uint8_t remote, uint32_t addr)
{
    struct sdpe_rpc_rpmsg_instance *rpmsg;

    rpmsg = malloc(sizeof(struct sdpe_rpc_rpmsg_instance));
    if (rpmsg == NULL) {
        dprintf(ALWAYS, "sdpe_rpc_rpmsg_init malloc fail\n");
        goto err_out;
    }

    rpmsg->remote = remote;
    rpmsg->addr = addr;

    dev->priv = rpmsg;
    dev->ops = &g_rpc_rpmsg_ops;
    dev->mtu = RL_BUFFER_PAYLOAD_SIZE;

    return 0;

err_with_free:
    free(rpmsg);

err_out:
    return -1;
}
