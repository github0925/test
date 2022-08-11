/*
 * sdpe_rpc_ipcc_transport.c
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
#include "ipcc_rpmsg.h"
#include "sdpe_rpc_transport.h"

typedef struct sdpe_rpc_ipcc_instance {
    uint8_t remote;
    uint32_t addr;
    struct rpmsg_dcf_instance *dcf;
    struct rpmsg_dcf_endpoint *ept;
} sdpc_rpc_ipcc_instance_t;

static int sdpe_rpc_ipcc_open(sdpe_rpc_transport_dev_t *dev, const char *name);
static int sdpe_rpc_ipcc_close(sdpe_rpc_transport_dev_t *dev);
static void *sdpe_rpc_ipcc_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size,
                                    uint32_t wait_time);
static int sdpe_rpc_ipcc_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time);
static int sdpe_rpc_ipcc_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len);
static int sdpe_rpc_ipcc_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len);

struct sdpe_rpc_transport_ops g_rpc_ipcc_ops = {
    .open                   = sdpe_rpc_ipcc_open,
    .close                  = sdpe_rpc_ipcc_close,
    .tx_alloc               = sdpe_rpc_ipcc_tx_alloc,
    .send                   = sdpe_rpc_ipcc_send,
    .send_nocopy            = sdpe_rpc_ipcc_send_nocopy,
    .recv                   = sdpe_rpc_ipcc_recv,
};

static int sdpe_rpc_ipcc_open(sdpe_rpc_transport_dev_t *dev, const char *name)
{
    struct sdpe_rpc_ipcc_instance *ipcc = dev->priv;

    ASSERT(ipcc != NULL);

    ipcc->ept = rpmsg_dcf_create_ept(ipcc->dcf, ipcc->addr, sdpe_rpc_rx_callback, (void*)ipcc);
    if (NULL == ipcc->ept) {
        return -1;
    }

    return 0;
}

static int sdpe_rpc_ipcc_close(sdpe_rpc_transport_dev_t *dev)
{
    struct sdpe_rpc_ipcc_instance *ipcc = dev->priv;

    ASSERT(ipcc != NULL);

    rpmsg_dcf_destroy_ept(ipcc->dcf, ipcc->ept);
    free(ipcc);

    return 0;
}

static void *sdpe_rpc_ipcc_tx_alloc(sdpe_rpc_transport_dev_t *dev, uint32_t *size, uint32_t wait_time)
{
    struct sdpe_rpc_ipcc_instance *ipcc = dev->priv;
    uint32_t remain_size = *size;
    void *buffer;

    buffer = rpmsg_dcf_alloc_tx_buffer(ipcc->dcf, &remain_size, wait_time);
    if (remain_size < *size) {
        /* need interface to free buffer */
        *size = 0;
        return NULL;
    }

    *size = remain_size;
    return buffer;
}

static int sdpe_rpc_ipcc_send(sdpe_rpc_transport_dev_t *dev, uint8_t *data,
                              uint32_t len, uint32_t wait_time)
{
    struct sdpe_rpc_ipcc_instance *ipcc = dev->priv;
    int ret;

    ASSERT(ipcc != NULL && data != NULL);

    ret = rpmsg_dcf_send(ipcc->dcf, ipcc->ept, ipcc->addr,
                         (char *)data, len, wait_time);

    return ret;
}

static int sdpe_rpc_ipcc_send_nocopy(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_ipcc_instance *ipcc = dev->priv;
    int ret;

    ASSERT(ipcc != NULL && data != NULL);

    ret = rpmsg_dcf_send_nocopy(ipcc->dcf, ipcc->ept, ipcc->addr, data, len);

    return ret;
}

static int sdpe_rpc_ipcc_recv(sdpe_rpc_transport_dev_t *dev, uint8_t *data, uint32_t len)
{
    return 0;
}

int sdpe_rpc_ipcc_init(sdpe_rpc_transport_dev_t *dev,
                       uint8_t remote, uint32_t addr)
{
    struct sdpe_rpc_ipcc_instance *ipcc;

    ipcc = malloc(sizeof(struct sdpe_rpc_ipcc_instance));
    if (ipcc == NULL) {
        dprintf(ALWAYS, "ipcc instance alloc fail\n");
        goto err_out;
    }

    ipcc_service_handle_t ipcc_service = find_service_handle(remote);
    if (ipcc_service == NULL || ipcc_service->rpmsg_dev == NULL) {
        dprintf(ALWAYS, "ipcc service get fail\n");
        goto err_with_free;
    }

    ipcc->remote = remote;
    ipcc->addr = addr;
    ipcc->dcf = ipcc_service->rpmsg_dev;
    dev->priv = ipcc;
    dev->ops = &g_rpc_ipcc_ops;
    dev->mtu = IPCC_MB_MTU;
    return 0;

err_with_free:
    free(ipcc);

err_out:
    return -1;
}
