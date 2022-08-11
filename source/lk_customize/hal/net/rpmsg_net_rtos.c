/*
* Copyright (c) 2020 Semidrive Semiconductor
* All rights reserved.
*
* Description: RPMsg based ethernet network interface
*
*/

#if ENABLE_RPMSG_NET
#include <reg.h>
#include <err.h>
#include <debug.h>
#include <trace.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <lk/init.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <kernel/event.h>
#include <platform/interrupts.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <dev/class/netif.h>
#include <netif/etharp.h>
#include <dcf.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>
#include <rpmsg_queue.h>

/* Debug helper */
#define LOCAL_TRACE             (0)
#define LOCAL_DUMP_PAYLOAD          (0)

#define NET_EPT_NAME            "rpmsg-net"

struct platform_rpmsg_net_config {
    unsigned int vendor_id;
    unsigned int device_id;
    unsigned int index;
};

struct rpmsg_net_state {
    /* rpmsg related stuff */
    int rproc;
    int dst;
    struct rpmsg_lite_instance *rpmsg_dev;
    struct rpmsg_lite_endpoint *channel;
    rpmsg_queue_handle  channel_queue;

    /* netif related stuff */
    uint8_t dev_addr[6];
    struct netstack_state *netstack_state;
    struct pbuf **rx_buffers;
    struct pbuf **tx_buffers;
    int tx_pending;
    unsigned int mtu;

    /* synchronization stuff */
    mutex_t tx_lock;
    char *tx_buffer;
    unsigned int tx_buffer_size;

    /* bottom half state */
    event_t event;
    event_t initialized;
    bool done;

};

static status_t rpmsg_net_init(struct device *dev);
static int rpmsg_net_thread(void *arg);
static status_t rpmsg_net_set_state(struct device *dev, struct netstack_state *state);
static ssize_t rpmsg_net_get_hwaddr(struct device *dev, void *buf, size_t max_len);
static ssize_t rpmsg_net_get_mtu(struct device *dev);
static status_t rpmsg_net_output(struct device *dev, struct pbuf *p);

extern struct rpmsg_lite_instance *rpmsg_get_instance_timed(int remote, lk_time_t ms);

#ifndef ETH_ALEN
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#endif

static status_t rpmsg_net_init(struct device *dev)
{
    status_t res = NO_ERROR;
    const struct platform_rpmsg_net_config *config = dev->config;
    int i;

    LTRACE_ENTRY;

    if (!config)
        return ERR_NOT_CONFIGURED;

    struct rpmsg_net_state *state = calloc(1, sizeof(struct rpmsg_net_state));
    if (!state)
        return ERR_NO_MEMORY;

    dev->state = state;

    mutex_init(&state->tx_lock);

    state->done = false;
    event_init(&state->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&state->initialized, false, 0);

    for (i = 0; i < ETH_ALEN; i++)
        state->dev_addr[i] = 0;

    /* TODO: use configure for the remote processor */
    state->rproc = 3;
    state->dst = NET_LINUX_EPT;
    state->mtu = RL_BUFFER_PAYLOAD_SIZE - SIZEOF_ETH_HDR;

    state->dev_addr[0] = 0xAA;
    state->dev_addr[1] = 0xBB;
    state->dev_addr[2] = 0xCC;
    state->dev_addr[3] = dcf_get_this_proc();
    state->dev_addr[4] = state->rproc;
    state->dev_addr[5] = state->dst;

    state->rpmsg_dev = rpmsg_get_instance_timed(state->rproc, 3000);
    if (!state->rpmsg_dev) {
        return ERR_BAD_HANDLE;
    }

    state->channel_queue = rpmsg_queue_create(state->rpmsg_dev);
    state->channel = rpmsg_lite_create_ept(state->rpmsg_dev, state->dst,
                                     rpmsg_queue_rx_cb, state->channel_queue);
    if (!state->rpmsg_dev) {
        return ERR_NO_RESOURCES;
    }

    rpmsg_ns_announce(state->rpmsg_dev, state->channel, (char *)NET_EPT_NAME, RL_NS_CREATE);

    mutex_acquire(&state->tx_lock);

    state->tx_buffer_size = RL_BUFFER_PAYLOAD_SIZE;
    state->tx_buffer = malloc(state->tx_buffer_size);
    if (!state->tx_buffer) {
        mutex_release(&state->tx_lock);
        dprintf(0, "No memory\n");
        goto error;
    }

    mutex_release(&state->tx_lock);

    /* start up a thread to process packet activity */
    thread_resume(thread_create("rpmsg-net", rpmsg_net_thread, dev, DEFAULT_PRIORITY,
                                DEFAULT_STACK_SIZE));

    LTRACE_EXIT;
    return res;

error:
    LTRACEF("Error: %d\n", res);

    if (state->tx_buffer)
        free(state->tx_buffer);

    free(state);

    return res;
}

static int rpmsg_net_thread(void *arg)
{
    struct device *dev;
    struct rpmsg_net_state *state;
    int recved = 0;
    unsigned long src;
    int count = 0;
    struct pbuf *p;
    status_t ret;

    DEBUG_ASSERT(arg);
    dev = (struct device *)arg;
    state = (struct rpmsg_net_state *) dev->state;
    if (!state) {
        return ERR_BAD_STATE;
    }

    dprintf(0, "vnet MAC: %x:%x:%x:%x:%x:%x started\n",
             state->dev_addr[0], state->dev_addr[1], state->dev_addr[2],
             state->dev_addr[3], state->dev_addr[4], state->dev_addr[5]);

    while (1) {
        p = pbuf_alloc(PBUF_RAW, RL_BUFFER_PAYLOAD_SIZE, PBUF_RAM);
        if (p != NULL) {
            ret = rpmsg_queue_recv(state->rpmsg_dev, state->channel_queue, &src,
                         (char*)p->payload, p->len, &recved, RL_BLOCK);
            if (ret < 0) {
                dprintf(0, "Netif No data available, ret: %d\n", ret);
                continue;
            }
            count++;

            dprintf(2, "Netif %s receiving %d bytes count:%d\n",
                            dev->name, (int)recved, count);

            ret = class_netstack_input(dev, state->netstack_state, p);
            if (ret < 0) {
                dprintf(0, "Netif %s failed to input ret: %d\n",
                                dev->name, ret);
                continue;
            }
            LINK_STATS_INC(link.recv);

#if LOCAL_DUMP_PAYLOAD
            hexdump8(p->payload, 34);
            dprintf(2, "RX by netif %s\n", dev->name);
#endif
        } else {
            dprintf(0, "No pbuf, dropped\n");
            LINK_STATS_INC(link.memerr);
            LINK_STATS_INC(link.drop);
        }
    }

    return 0;
}

static status_t rpmsg_net_set_state(struct device *dev, struct netstack_state *netstack_state)
{
    if (!dev)
        return ERR_INVALID_ARGS;

    if (!dev->state)
        return ERR_NOT_CONFIGURED;

    struct rpmsg_net_state *state = dev->state;

    state->netstack_state = netstack_state;

    return NO_ERROR;
}

static ssize_t rpmsg_net_get_hwaddr(struct device *dev, void *buf, size_t max_len)
{
    struct rpmsg_net_state *state;

    if (!dev || !buf)
        return ERR_INVALID_ARGS;

    if (!dev->state)
        return ERR_NOT_CONFIGURED;

    state = dev->state;

    memcpy(buf, state->dev_addr, MIN(sizeof(state->dev_addr), max_len));

    return sizeof(state->dev_addr);
}

static ssize_t rpmsg_net_get_mtu(struct device *dev)
{
    struct rpmsg_net_state *state;

    if (!dev)
        return ERR_INVALID_ARGS;

    if (!dev->state)
        return ERR_NOT_CONFIGURED;

    state = dev->state;

    return state->mtu;
}

static status_t rpmsg_net_output(struct device *dev, struct pbuf *p)
{
    struct rpmsg_net_state *state;
    status_t ret;
    struct pbuf *q;
    char *ptr;

    LTRACE_ENTRY;
    if (!dev || !p)
        return ERR_INVALID_ARGS;

    if (!dev->state)
        return ERR_NOT_CONFIGURED;

    state = dev->state;
    dprintf(INFO, "vnet: start to TX dat: %p tot_len:%d\n", p->payload, p->tot_len);

    mutex_acquire(&state->tx_lock);

    ptr = state->tx_buffer;
    if (p->tot_len > state->tx_buffer_size) {
        mutex_release(&state->tx_lock);
        dprintf(WARN, "vnet: send tot_len: %d exceed tx buf:%d\n",
                    p->tot_len, state->tx_buffer_size);
        return ERR_BUF;
    }

    for (q = p; q != NULL; q = q->next) {
        dprintf(SPEW, "vnet: TX data %p len: %d\n", q->payload, q->len);
        memcpy(ptr, q->payload, q->len);
        ptr += q->len;
    }

    ret = rpmsg_lite_send(state->rpmsg_dev, state->channel, state->dst, state->tx_buffer, p->tot_len, 0);
    if (ret < 0) {
        mutex_release(&state->tx_lock);
        dprintf(WARN, "vnet: Failed to TX tot_len:%d\n", p->tot_len);
        return ret;
    }

    mutex_release(&state->tx_lock);

    LINK_STATS_INC(link.xmit);

    LTRACE_EXIT;
    return ERR_OK;
}

static const struct netif_ops rpmsg_net_ops = {
    .std = {
        .init = rpmsg_net_init,
    },

    .set_state = rpmsg_net_set_state,
    .get_hwaddr = rpmsg_net_get_hwaddr,
    .get_mtu = rpmsg_net_get_mtu,
    .output = rpmsg_net_output,
};

DRIVER_EXPORT(netif, &rpmsg_net_ops.std);

static const struct platform_rpmsg_net_config rpmsg_net_config = {
    .vendor_id = 0x1022,
    .device_id = 0x2000,
    .index = 0,
};

DEVICE_INSTANCE(netif, rpmsg_net, &rpmsg_net_config);


#include "lwip/apps/lwiperf.h"
void rpmsg_net_init_hook(void)
{
    device_init(device_get_by_name(netif, rpmsg_net));
    class_netif_add(device_get_by_name(netif, rpmsg_net));

    dprintf(0, "rpmsg_net device added\n");
    lwiperf_start_tcp_server_default(NULL, NULL);
}
#else
void rpmsg_net_init_hook(void)
{
}
#endif  //ENABLE_RPMSG_NET
