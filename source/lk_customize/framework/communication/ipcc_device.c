/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: default inter-processor communication messaging service
*
*/
#include <reg.h>
#include <stdlib.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <platform/debug.h>
#include <mbox_hal.h>

#include "rpmsg_common.h"
#include "ipcc_device.h"

#define MAX_DEVICE_NUM   (6)
static struct ipcc_device ipcc_devices[MAX_DEVICE_NUM];

int dcf_get_this_proc(void);
void start_echo_loop(struct ipcc_device *dev);
static int recvfrom_timed(struct ipcc_channel *ichan, unsigned long *src,
                            char *data, int *len, lk_time_t timeout);

static void ipcc_new_ept_cb(unsigned int new_ept,
                                    const char *new_ept_name,
                                    unsigned long flags,
                                    void *user_data)
{
    struct ipcc_device *dev;
    struct ipcc_channel *ichan;

    dev = (struct ipcc_device *) user_data;
    dprintf(INFO, "%s: receive new ept %d:%d %s flags %ld\n", __func__,
           dev->config.rproc, new_ept, new_ept_name, flags);

    list_for_every_entry(&dev->channels, ichan,
                         struct ipcc_channel, node) {
        if (ichan->endpoint && ichan->announce && !ichan->last_announced) {
            ichan->last_announced = 1;
            rpmsg_dcf_ns_announce(dev->rpmsg_dev, ichan->endpoint, ichan->name, DCF_NS_CREATE);
        }
    }
}

static int looper_thread(void *arg)
{
    struct ipcc_channel *ichan;
    int recved = 0;
    unsigned long src;
    status_t ret;
    unsigned char *rxbuf = NULL;

    ichan = (struct ipcc_channel *) arg;
    if (!ichan) {
        return ERR_BAD_STATE;
    }

    rxbuf = malloc(ichan->mtu);
    if (!rxbuf) {
        dprintf(0, "No memory for ipcc channel\n");
        return ERR_NO_MEMORY;
    }

    ichan->looper = get_current_thread();
    event_signal(&ichan->initialized, true);
    ichan->state = DCF_STATE_Connected;

    while (ichan->state == DCF_STATE_Connected) {
        recved = ichan->mtu;
        ret = recvfrom_timed(ichan, &src, (char*) rxbuf, &recved, DCF_BLOCK);
        if (ret < 0) {
            ichan->err_cnt++;
            dprintf(0, ":%s: fail to read data, ret: %d\n", __func__, ret);
            continue;
        }
        ichan->rx_cnt++;
        ichan->rx_bytes += recved;

        /* data processing start from here */
        dprintf(1, "%s: rx %d bytes from %ld\n", __func__, recved, src);

        if (ichan->msg_handler)
            ichan->msg_handler(ichan, (struct dcf_message *)rxbuf, recved, src);
        /* data processing end */
    }

    ichan->state = DCF_STATE_Closed;
    if (rxbuf)
        free(rxbuf);

    return 0;
}

static int attach_channel(struct ipcc_device *dev, struct ipcc_channel *ichan)
{
    mutex_acquire(&dev->lock);
    list_add_tail(&dev->channels, &ichan->node);
    mutex_release(&dev->lock);
    return 0;
}

static int detach_channel(struct ipcc_device *dev, struct ipcc_channel *ichan)
{
    mutex_acquire(&dev->lock);
    list_delete(&ichan->node);
    mutex_release(&dev->lock);
    return 0;
}

static struct ipcc_device *alloc_device(struct ipcc_dev_config *conf)
{
    int i;
    struct ipcc_device *dev = NULL;

    if (!conf)
        return NULL;

    for (i = 0;i < MAX_DEVICE_NUM;i++) {
        dev = &ipcc_devices[i];
        if (dev->used == false) {
            memset(dev, 0, sizeof(*dev));
            memcpy(&dev->config, conf, sizeof(rpmsg_dev_config_t));
            dev->used = true;
            return dev;
        }
    }

    return NULL;
}

static void free_device(struct ipcc_device *dev)
{
    if (dev->used != 0) {
        memset(&dev->config, 0, sizeof(rpmsg_dev_config_t));
        dev->used = false;
    }
}

static struct ipcc_device *find_device_by_rproc(int rproc)
{
    int i;
    struct ipcc_device *dev = NULL;

    for (i = 0;i < MAX_DEVICE_NUM;i++) {
        dev = &ipcc_devices[i];
        if (dev->used && rproc == dev->config.rproc) {
            return dev;
        }
    }

    return NULL;
}

static int ipcc_device_thread(void *data)
{
    struct ipcc_device *dev = data;

    dprintf(1, "rpmsg_mb:%d starting\n", dev->config.rproc);

    dev->rpmsg_dev = rpmsg_dcf_device_init(dev->config.rproc, dev->config.cfg.init_flags);
    if (!dev->rpmsg_dev) {
        dprintf(ALWAYS, "init rpmsg device %d failed\n", dev->config.rproc);
        return -1;
    }

    dev->rpmsg_ns = rpmsg_dcf_ns_bind(dev->rpmsg_dev, ipcc_new_ept_cb, (void *) dev);
    if (!dev->rpmsg_ns) {
        dprintf(ALWAYS, "bind rpmsg ns failed\n");
        return -1;
    }

    dev->state = 1;
    dprintf(1, "ipcc device %s started\n", dev->config.devname);
    event_signal(&dev->initialized, false);

    start_echo_loop(dev);

    return 0;
}

#define RETRY_INTERNAL    (20)
struct ipcc_device *ipcc_device_gethandle(int rproc, lk_time_t ms)
{
    struct ipcc_device *dev = find_device_by_rproc(rproc);
    if (!dev) {
        dprintf(1, "ipcc device to rproc%d not found \n", rproc);
        return NULL;
    }

    while(!dev->state) {
        if (ms < RETRY_INTERNAL) {
            return NULL;
        }

        thread_sleep(RETRY_INTERNAL);
        ms -= RETRY_INTERNAL;
    };

    return dev;
}

int ipcc_device_probe(struct ipcc_dev_config *conf)
{
    char thread_name[16];
    struct ipcc_device *dev = find_device_by_rproc(conf->rproc);

    if (dev && dev->main_thread) {
        dprintf(0, "ipcc dev%d->%d already started \n",
                dcf_get_this_proc(), conf->rproc);
        return ERR_ALREADY_EXISTS;
    }

    dev = alloc_device(conf);
    if (!dev) {
        dprintf(ALWAYS, "failed to allocate device rproc-%d \n", conf->rproc);
        return ERR_NO_RESOURCES;
    }

    list_initialize(&dev->channels);
    mutex_init(&dev->lock);
    event_init(&dev->initialized, false, EVENT_FLAG_AUTOUNSIGNAL);

    sprintf(thread_name, "ipcc-echod/%d", dev->config.rproc);
    dev->main_thread = thread_create(thread_name, ipcc_device_thread, dev,
                                     THREAD_PRI_IPCC_CHN, CONFIG_ECHO_STACK_SIZE);
    thread_resume(dev->main_thread);

    event_wait(&dev->initialized);

    return 0;
}

static void list_channel(struct ipcc_device *dev)
{
    struct ipcc_channel *rpchn;
    printf("\tchannel list:\n");

    printf("\tName\t\tAddr\tStatus\tTxBytes\tRxBytes\n");
    list_for_every_entry(&dev->channels, rpchn,
                         struct ipcc_channel, node) {
        printf("\t%s\t%d\t%d\t%d\t%d\n", rpchn->name, rpchn->addr, rpchn->state,
                            rpchn->tx_bytes, rpchn->rx_bytes);
    }

    printf("\tend of list\n");
}

void ipcc_device_show(void)
{
    int i;
    struct ipcc_device *dev;

    printf("ipcc devices:\n");
    printf("\tRproc\tState\n");
    for (i = 0;i < MAX_DEVICE_NUM;i++) {
        dev = &ipcc_devices[i];
        if (dev->used) {

            printf("%d:\t%d\t%d\n", i, dev->config.rproc, dev->state);

            list_channel(dev);
            printf("\n\n");
        }
    }
    printf("End of ipcc devices\n");
}

static void reset_channels(struct ipcc_device *dev)
{
    struct ipcc_channel *rpchn;

    list_for_every_entry(&dev->channels, rpchn,
                         struct ipcc_channel, node) {
        if (rpchn->announce)
            rpchn->last_announced = 0;
    }
}

#include <sys_diagnosis.h>

int ipcc_device_reset_cb(int signal, void *args)
{
    int i;
    struct ipcc_device *dev;
    int rp = IPCC_RRPOC_AP1;

    if (signal == WDT6____ovflow_int)
        rp = IPCC_RRPOC_AP2;

    dprintf(INFO, "ipcc: sys signal %d recieved\n", signal);
    for (i = 0;i < MAX_DEVICE_NUM;i++) {
        dev = &ipcc_devices[i];
        if (dev->used && dev->config.rproc == rp) {
            reset_channels(dev);
        }
    }

    dprintf(INFO, "ipcc devices recovered\n");

    return 0;
}

static void release_channel(struct ipcc_device *dev, struct ipcc_channel *ichan)
{
    if (ichan) {
        sd_rpbuf_t *rpbuf = NULL;

        while (!sd_rpbuf_queue_empty(&ichan->rxq)) {
            rpbuf = sd_rpbuf_dequeue(&ichan->rxq);
            dprintf(0, "ipcc: release rpbuf=%d\n", rpbuf->buf_id);
            rpmsg_dcf_release_rpbuf(dev->rpmsg_dev, rpbuf);
        };

        sd_rpbuf_remove_queue(&ichan->rxq);

        sem_destroy(&ichan->rxq_wait);

        if (ichan->endpoint)
            rpmsg_dcf_destroy_ept(dev->rpmsg_dev, ichan->endpoint);

        free(ichan);
    }
}

int ipcc_channel_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    struct ipcc_channel *ichan = priv;
    struct rpmsg_dcf_instance *rpmsg_dev = ichan->parent->rpmsg_dev;
    sd_rpbuf_t *rpbuf;

    if (payload) {
        rpbuf = rpmsg_dcf_get_rpbuf(rpmsg_dev, payload);
        if (!rpbuf) {
            if (ichan->event_cb)
                ichan->event_cb(ichan->event_cb_ctx, IPCC_EVENT_ERROR, 0);

            dprintf(0, "Not found rpbuf with ptr=%p\n", payload);
            return DCF_RELEASE;
        }

        dprintf(2, "%s ptr=%p\n", __func__, payload);

        /* hold the rx buffer */
        sd_rpbuf_enqueue(&ichan->rxq, rpbuf);
    }
    sem_post(&ichan->rxq_wait, true);

    if (ichan->event_cb)
        ichan->event_cb(ichan->event_cb_ctx, IPCC_EVENT_RECVED, 0);

    return DCF_HOLD;
}

struct ipcc_channel *
ipcc_channel_create(struct ipcc_device *dev, int endpoint, const char *name, bool announce)
{
    struct ipcc_channel *ichan;

    if (!dev || !name) {
        return NULL;
    }

    ichan = calloc(1, sizeof(struct ipcc_channel));
    if (!ichan) {
        return NULL;
    }

    event_init(&ichan->initialized, false, EVENT_FLAG_AUTOUNSIGNAL);
    sd_rpbuf_init_queue(&ichan->rxq);
    sem_init(&ichan->rxq_wait, 0);

    /* TODO: use configure for the remote processor */
    ichan->rproc = dev->config.rproc;
    ichan->addr = endpoint;
    ichan->state = DCF_STATE_Initializing;
    ichan->mtu = DCF_MSG_MAX_LEN;
    ichan->parent = dev;
    ichan->announce = announce;
    ichan->last_announced = 0;
    ichan->endpoint = rpmsg_dcf_create_ept(dev->rpmsg_dev, ichan->addr,
                                     ipcc_channel_rx_cb, ichan);
    if (!ichan->endpoint) {
        goto error;
    }

    attach_channel(dev, ichan);

    strncpy(ichan->name, name, 16);
    ichan->state = DCF_STATE_Initialized;

    return ichan;

error:
    release_channel(dev, ichan);

    return NULL;
}

void ipcc_channel_destroy(struct ipcc_channel *ichan)
{
    struct ipcc_device *dev = ichan->parent;

    detach_channel(dev, ichan);
    release_channel(dev, ichan);
}

int ipcc_channel_set_mtu(struct ipcc_channel *ichan, unsigned int mtu)
{
    ichan->mtu = MIN(mtu, DCF_BUFFER_PAYLOAD_SIZE);
    return ichan->mtu;
}

int ipcc_channel_start(struct ipcc_channel *ichan, ipcc_msg_handler handler)
{
    struct ipcc_device *dev = ichan->parent;

    /* if specify msg handler, spawn a thread to receive msg and call handler */
    if (handler) {
        ichan->msg_handler = handler;
        /* start up a thread to process message queue */
        thread_resume(thread_create(ichan->name, looper_thread, ichan,
                            THREAD_PRI_IPCC_CHN, CONFIG_RPMSG_STACK_SIZE));
        event_wait(&ichan->initialized);
    } else
        ichan->state = DCF_STATE_Connected;

    if (ichan->announce)
        rpmsg_dcf_ns_announce(dev->rpmsg_dev, ichan->endpoint, ichan->name, DCF_NS_CREATE);

    dprintf(1, "ipcc channel %s:%d->%d started\n",
            ichan->name, dcf_get_this_proc(), ichan->rproc);

    return 0;
}

status_t ipcc_channel_stop(struct ipcc_channel *ichan)
{
    struct ipcc_device *dev = ichan->parent;

    if (ichan->announce)
        rpmsg_dcf_ns_announce(dev->rpmsg_dev, ichan->endpoint, ichan->name, DCF_NS_DESTROY);

    if (ichan) {
        ichan->state = DCF_STATE_Closing;

        if (ichan->looper) {
            thread_sleep(100);
            /* insert a null message to indicate stop */
            ipcc_channel_rx_cb(NULL, 0, DCF_ADDR_ANY, ichan);
            thread_join(ichan->looper, NULL, 1000);
            ichan->looper = NULL;
        }
        dprintf(1, "ipcc channel %s stopped\n", ichan->name);
    }

    return 0;
}

int ipcc_channel_sendto(struct ipcc_channel *ichan, unsigned long dst,
                            char * data, int size, lk_time_t timeout)
{
    if (!ichan)
        return DCF_ERR_DEV_ID;

    if (ichan->parent && ichan->parent->rpmsg_dev)
        return rpmsg_dcf_send(ichan->parent->rpmsg_dev, ichan->endpoint,
                              dst, data, size, (unsigned long)timeout);

    dprintf(0, "ipcc: channel %s sendto failed\n", ichan->name);

    return DCF_NOT_READY;
}

static sd_rpbuf_t *recv_rpbuf_timed(struct ipcc_channel *ichan, lk_time_t timeout)
{
    status_t ret = 0;

    /* FIXME: a bug in sem_timedwait of freertos lk-wrapper*/
    if (timeout != INFINITE_TIME)
        ret = sem_timedwait(&ichan->rxq_wait, timeout);
    else
        ret = sem_wait(&ichan->rxq_wait);

    if (ret ==  NO_ERROR) {
        dprintf(2, "%s\n", __func__);
        return sd_rpbuf_dequeue(&ichan->rxq);
    }

    return NULL;
}

static int recvfrom_timed(struct ipcc_channel *ichan, unsigned long *src,
                            char *data, int *len, lk_time_t timeout)
{
    struct rpmsg_dcf_instance *rpmsg_dev = ichan->parent->rpmsg_dev;
    sd_rpbuf_t *rpbuf = NULL;

    rpbuf = recv_rpbuf_timed(ichan, timeout);
    return rpmsg_dcf_copy_payload(rpmsg_dev, rpbuf, src, data, len);
}

int ipcc_channel_recvfrom(struct ipcc_channel *ichan, unsigned long *src,
                            char *data, int *len, lk_time_t timeout)
{
    status_t ret = 0;

    if (!ichan || !ichan->parent)
        return DCF_ERR_DEV_ID;

    if (!len || (*len == 0)) {
        return DCF_ERR_PARAM;
    }

    ret = recvfrom_timed(ichan, src, data, len, timeout);

    dprintf(INFO, "%s rx data, cbuf %d\n", __func__, (uint32_t)*len);
    return ret;
}

sd_rpbuf_t *ipcc_channel_recvbuf(struct ipcc_channel *ichan, lk_time_t timeout)
{
    if (!ichan || !ichan->parent)
        return NULL;

    return recv_rpbuf_timed(ichan, timeout);
}

bool ipcc_channel_inq_avail(struct ipcc_channel *ichan)
{
    return !sd_rpbuf_queue_empty(&ichan->rxq);
}

void *ipcc_device_import_rpbuf(struct ipcc_device *dev, sd_rpbuf_t *rpbuf,
                    unsigned long *src, int *len)
{
    dprintf(2, "%s ptr=%p\n", __func__, rpbuf);
    return rpmsg_dcf_import_buffer(dev->rpmsg_dev, rpbuf, src, len);
}

int ipcc_device_release_rpbuf(struct ipcc_device *dev, sd_rpbuf_t *rpbuf)
{
    dprintf(2, "%s ptr=%p\n", __func__, rpbuf);
    return rpmsg_dcf_release_rpbuf(dev->rpmsg_dev, rpbuf);
}

