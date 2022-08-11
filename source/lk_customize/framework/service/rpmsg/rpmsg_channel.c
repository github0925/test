/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <list.h>
#include <kernel/event.h>

#include <dcf.h>
#include <rpmsg_lite.h>
#include <rpmsg_ns.h>
#include <rpmsg_queue.h>
#include "rpmsg_rtos.h"

#define RPMC_INIT_TIMEOUT             (8000)

struct rpmsg_lite_instance *rpmsg_get_instance_timed(int remote, lk_time_t ms);
struct rpmsg_lite_instance *rpmsg_get_instance(int remote);

struct rpmsg_device *rpmsg_device_get_handle(int remote, lk_time_t ms);
int rpmsg_device_add_channel(struct rpmsg_device *dev, struct rpmsg_channel *rpchn);
int rpmsg_device_rm_channel(struct rpmsg_device *dev, struct rpmsg_channel *rpchn);

static int rpmsg_looper_thread(void *arg)
{
    struct rpmsg_channel *rpchn;
    int recved = 0;
    unsigned long src;
    status_t ret;
    unsigned char *rxbuf = NULL;
    struct dcf_message *msg = (struct dcf_message *)rxbuf;

    rpchn = (struct rpmsg_channel *) arg;
    if (!rpchn) {
        return ERR_BAD_STATE;
    }

    rxbuf = malloc(rpchn->mtu);
    if (!rxbuf) {
        dprintf(0, "No memory for rpmsg channel\n");
        return ERR_NO_MEMORY;
    }

    rpchn->looper = get_current_thread();
    event_signal(&rpchn->initialized, true);
    rpchn->state = DCF_STATE_Connected;

    while (rpchn->state == DCF_STATE_Connected) {
        ret = rpmsg_queue_recv(rpchn->rpmsg_dev, rpchn->msg_queue, &src,
                     (char*) rxbuf, rpchn->mtu, &recved, RL_BLOCK);
        if (ret < 0) {
            rpchn->err_cnt++;
            dprintf(0, ":%s: fail to read data, ret: %d\n", __func__, ret);
            continue;
        }
        rpchn->rx_cnt++;
        rpchn->rx_bytes += recved;

        /* data processing start from here */
        dprintf(1, "%s: rx %d bytes from %ld\n", __func__, recved, src);

        if (rpchn->msg_handler)
            rpchn->msg_handler(rpchn, (struct dcf_message *)rxbuf, recved);
        /* data processing end */
    }

    rpchn->state = DCF_STATE_Closed;
    if (rxbuf)
        free(rxbuf);

    return 0;
}

struct rpmsg_channel *rpmsg_channel_create(int rproc, int endpoint, const char *name)
{
    struct rpmsg_channel *rpchn = calloc(1, sizeof(struct rpmsg_channel));
    int ret;

    if (!rpchn) {
        ret = ERR_NO_MEMORY;
        goto error;
    }

    if (rproc == dcf_get_this_proc()) {
        ret = ERR_NOT_ALLOWED;
        goto error;
    }
    event_init(&rpchn->initialized, false, EVENT_FLAG_AUTOUNSIGNAL);

    /* TODO: use configure for the remote processor */
    rpchn->rproc = rproc;
    rpchn->addr = endpoint;
    rpchn->state = DCF_STATE_Initializing;
    rpchn->mtu = DCF_MSG_MAX_LEN;

    rpchn->parent = rpmsg_device_get_handle(rpchn->rproc, RPMC_INIT_TIMEOUT);
    if (!rpchn->parent) {
        ret = ERR_BAD_HANDLE;
        dprintf(0, "Fail to get rpmsg device rp%d\n", rpchn->rproc);
        goto error;
    }
    rpchn->rpmsg_dev = rpchn->parent->rl_instance;

    rpchn->msg_queue = rpmsg_queue_create(rpchn->rpmsg_dev);
    if (!rpchn->msg_queue) {
        ret = ERR_NOT_ENOUGH_BUFFER;
        goto error;
    }

    rpchn->endpoint = rpmsg_lite_create_ept(rpchn->rpmsg_dev, rpchn->addr,
                                     rpmsg_queue_rx_cb, rpchn->msg_queue);
    if (!rpchn->endpoint) {
        ret = ERR_NO_RESOURCES;
        goto error;
    }

    rpmsg_device_add_channel(rpchn->parent, rpchn);

    strncpy(rpchn->name, name, DCF_NAM_MAX_LEN);
    rpchn->state = DCF_STATE_Initialized;

    return rpchn;

error:
    if (rpchn)
        free(rpchn);

    dprintf(0, "%s: ret=%d\n", __func__, ret);

    return NULL;
}

int rpmsg_channel_set_mtu(struct rpmsg_channel *rpchn, unsigned int mtu)
{
    rpchn->mtu = MIN(mtu, RL_BUFFER_PAYLOAD_SIZE);
    return rpchn->mtu;
}

int rpmsg_channel_start(struct rpmsg_channel *rpchn, rpmsg_msg_handler handler)
{
    struct rpmsg_device *dev = rpchn->parent;

    /* connecting with remote side */
    if (!dev->config.is_master) {
        rpmsg_ns_announce(rpchn->rpmsg_dev, rpchn->endpoint,
                          rpchn->name, RL_NS_CREATE);
    }

    /* if specify msg handler, spawn a thread to receive msg and call handler */
    if (handler) {
        rpchn->msg_handler = handler;
        /* start up a thread to process message queue */
        thread_resume(thread_create(rpchn->name, rpmsg_looper_thread, rpchn,
                            THREAD_PRI_RPMSG_CHN, CONFIG_RPMSG_STACK_SIZE));
        event_wait(&rpchn->initialized);
    } else
        rpchn->state = DCF_STATE_Connected;

    dprintf(INFO, "rpmsg channel %s:%d->%d connected\n",
            rpchn->name, dcf_get_this_proc(), rpchn->rproc);

    return 0;
}

void rpmsg_channel_destroy(struct rpmsg_channel *rpchn)
{
    rpmsg_device_rm_channel(rpchn->parent, rpchn);

    if (rpchn) {
        if (rpchn->msg_queue)
            rpmsg_queue_destroy(rpchn->rpmsg_dev, rpchn->msg_queue);

        if (rpchn->endpoint)
            rpmsg_lite_destroy_ept(rpchn->rpmsg_dev, rpchn->endpoint);

        free(rpchn);
    }
}

status_t rpmsg_channel_stop(struct rpmsg_channel *rpchn)
{
    struct rpmsg_device *dev;

    if (rpchn) {
        rpchn->state = DCF_STATE_Closing;
        dev = rpchn->parent;
        if (!dev->config.is_master) {
            rpmsg_ns_announce(rpchn->rpmsg_dev, rpchn->endpoint,
                              rpchn->name, RL_NS_DESTROY);
        }

        if (rpchn->looper) {
            thread_sleep(100);
            /* insert a null message to indicate stop */
            rpmsg_queue_rx_cb(NULL, 0, RL_ADDR_ANY, rpchn->msg_queue);
            thread_join(rpchn->looper, NULL, 1000);
            rpchn->looper = NULL;
        }
        dprintf(0, "rpmsg channel %s disconnected\n", rpchn->name);
    }

    return 0;
}

void rpmsg_channel_listall(struct rpmsg_device *dev)
{
    struct rpmsg_channel *rpchn;
    printf("\tchannel list:\n");

    printf("\tName\t\tAddr\tStatus\tTxBytes\tRxBytes\n");
    list_for_every_entry(&dev->channels, rpchn,
                         struct rpmsg_channel, node) {
        printf("\t%s\t%d\t%d\t%d\t%d\n", rpchn->name, rpchn->addr, rpchn->state,
                            rpchn->tx_bytes, rpchn->rx_bytes);
    }

    printf("\tend of list\n");
}

status_t rpmsg_channel_sendmsg(struct rpmsg_channel *rpchn,
                struct dcf_message *msg, int len, lk_time_t timeout)
{
    return rpmsg_lite_send(rpchn->rpmsg_dev, rpchn->endpoint, rpchn->addr,
                    (char *)msg, len, timeout);
}

status_t rpmsg_channel_recvmsg(struct rpmsg_channel *rpchn,
                struct dcf_message *msg, int msglen, lk_time_t timeout)
{
    unsigned long src;
    int len;
    int ret;

    ret = rpmsg_queue_recv(rpchn->rpmsg_dev, rpchn->msg_queue, &src,
                    (char *)msg, msglen, &len, timeout);
    if (ret < 0 || msglen != len) {
        dprintf(0, "Fail to read rpmsg or bad len\n");
        return ret;
    }

    return 0;
}

/* user rx by itself, no internal loop reciever */
status_t rpmsg_channel_recvfrom(struct rpmsg_channel *rpchn,
                     unsigned long *src,
                     char *data,
                     int maxlen,
                     int *len,
                     unsigned long timeout)
{
    return rpmsg_queue_recv(rpchn->rpmsg_dev, rpchn->msg_queue, src,
                    data, maxlen, len, timeout);
}

status_t rpmsg_channel_sendto(struct rpmsg_channel *rpchn, unsigned long dst,
                    char *data,
                    unsigned long size,
                    unsigned long timeout)
{
    return rpmsg_lite_send(rpchn->rpmsg_dev, rpchn->endpoint, dst,
                    data, size, timeout);
}

void *rpmsg_channel_alloc_tx_buf(struct rpmsg_channel *rpchn,
                    unsigned long *size,
                    unsigned long timeout)
{
    return rpmsg_lite_alloc_tx_buffer(rpchn->rpmsg_dev, size, timeout);
}

status_t rpmsg_channel_sendto_nocopy(struct rpmsg_channel *rpchn, unsigned long dst,
                    char *data,
                    unsigned long size)
{
    return rpmsg_lite_send_nocopy(rpchn->rpmsg_dev, rpchn->endpoint, dst,
                    data, size);
}

