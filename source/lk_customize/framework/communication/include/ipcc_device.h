/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __IPCC_DEVICE_H__
#define __IPCC_DEVICE_H__

#include <kernel/semaphore.h>
#include "rpmsg_common.h"
#include "ipcc_rpmsg.h"
#include "ipcc_rpmsg_ns.h"
#include "ipcc_queue.h"

typedef enum ipcc_event {
  IPCC_EVENT_RECVED,
  IPCC_EVENT_TXDONE,
  IPCC_EVENT_ERROR
}ipcc_event_t;

struct ipcc_dev_config {
    char        devname[DCF_NAM_MAX_LEN];
    int         rproc;
    rpmsg_cfg_extend_t  cfg;
    unsigned int features;
};

struct ipcc_channel;
typedef void (*ipcc_msg_handler)(struct ipcc_channel *, struct dcf_message *mssg, int len, int src);
typedef void (*ipcc_event_cb_func)(void *ctx, ipcc_event_t e, u32 band);

struct ipcc_device {
    struct ipcc_dev_config config;
    bool        used;
    int         state;

    thread_t    *main_thread;
    struct list_node channels;
    mutex_t     lock;
    event_t     initialized;

    /* rpmsg resources */
    struct rpmsg_dcf_instance *rpmsg_dev;
    rpmsg_dcf_ns_handle rpmsg_ns;
};

struct ipcc_channel {

    char        name[DCF_NAM_MAX_LEN];
    int         rproc;
    int         addr;
    u32         mtu;
    event_t     initialized;
    dcf_state_t state;
    thread_t    *looper;
    struct list_node node;
    u32         rx_bytes;
    u32         tx_bytes;
    u32         rx_cnt;
    u32         tx_cnt;
    u32         err_cnt;
    u32         drop_cnt;

    struct ipcc_device  *parent;
    struct rpmsg_dcf_endpoint *endpoint;
    sd_rpbuf_queue_t rxq;
    semaphore_t rxq_wait;
    ipcc_msg_handler    msg_handler;
    void        *user_context;
    bool        announce;

    ipcc_event_cb_func event_cb;
    void *event_cb_ctx;
    bool        last_announced;
};

inline static void ipcc_channel_set_context(struct ipcc_channel *ichan, void *context)
{
    ichan->user_context = context;
}

inline static void *ipcc_channel_get_context(struct ipcc_channel *ichan)
{
    return ichan->user_context;
}

inline static int ipcc_device_get_rproc(struct ipcc_device *dev)
{
    return dev->config.rproc;
}

int ipcc_device_probe(struct ipcc_dev_config *cfg);
struct ipcc_device *ipcc_device_gethandle(int rproc, lk_time_t ms);
void ipcc_device_show(void);
int ipcc_device_reset_cb(int signal, void *args);

struct ipcc_channel *
ipcc_channel_create(struct ipcc_device *dev, int endpoint, const char *name, bool announce);
status_t ipcc_channel_start(struct ipcc_channel *rpchn, ipcc_msg_handler handler);
void ipcc_channel_destroy(struct ipcc_channel *rpchn);
status_t ipcc_channel_stop(struct ipcc_channel *rpchn);

status_t ipcc_channel_sendmsg(struct ipcc_channel *rpchn,
                    struct dcf_message *msg, int len, lk_time_t timeout);

status_t ipcc_channel_recvmsg(struct ipcc_channel *rpchn,
                              struct dcf_message *msg,
                              int msglen,
                              lk_time_t timeout);

status_t ipcc_channel_recvfrom(struct ipcc_channel *rpchn,
                               unsigned long *src,
                               char *data,
                               int *len,
                               lk_time_t timeout);

sd_rpbuf_t *ipcc_channel_recvbuf(struct ipcc_channel *ichan, lk_time_t timeout);

status_t ipcc_channel_sendto(struct ipcc_channel *ichan,
                             unsigned long dst, char * data,
                             int size, lk_time_t timeout);

bool ipcc_channel_inq_avail(struct ipcc_channel *ichan);

static inline
int ipcc_channel_register_event_cb(struct ipcc_channel *ichan, ipcc_event_cb_func cb, void *ctx)
{
    ichan->event_cb = cb;
    ichan->event_cb_ctx = ctx;

    return 0;
}

#endif //__IPCC_DEVICE_H__

