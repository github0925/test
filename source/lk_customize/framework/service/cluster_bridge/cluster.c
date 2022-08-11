/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <dcf.h>

/* Check DCF endpoint management document for allocation */
#define CLUSTER_APP_EPT_NAME           "rpmsg-cluster"
#ifndef CLUSTER_APP_EPT
#define CLUSTER_APP_EPT                (70)
#endif

#define IVI_APP_EPT_NAME               "rpmsg-ivi"
#ifndef IVI_APP_EPT
#define IVI_APP_EPT                    (71)
#endif

#define WAIT_RPMSG_DEV_TIMEOUT         (10000)
struct clusterapp_server_info {
    bool binitialized;

    /* communication level stuff */
    struct ipcc_channel *rpchan_ivi;
    struct ipcc_channel *rpchan_cluster;

    /* TODO: operation stuff */
};

struct clusterapp_cmd_msg {
    struct dcf_message head;

    /* to extend cmd body */
    u32 subcmd;
    u32 arg1;
    u32 arg2;
} __attribute__((__packed__));

static struct clusterapp_server_info clusterapp_server;

static void process_ivi_message(struct ipcc_channel *rpchan, struct dcf_message *msg, int len, int src)
{
    struct clusterapp_server_info *s = &clusterapp_server;

    dprintf(1, "clusterapp forward %d bytes to cluster\n", len);
    ipcc_channel_sendto(s->rpchan_cluster, IVI_APP_EPT, (char *)msg, len, 1000);
}

static void process_cluster_message(struct ipcc_channel *rpchan, struct dcf_message *msg, int len, int src)
{
    struct clusterapp_server_info *s = &clusterapp_server;

    dprintf(1, "clusterapp forward %d bytes to ivi\n", len);
    ipcc_channel_sendto(s->rpchan_ivi, CLUSTER_APP_EPT, (char *)msg, len, 1000);
}

void clusterapp_bridge_init(void)
{
    struct clusterapp_server_info *s = &clusterapp_server;
    struct ipcc_device *dev;

    if (s->binitialized)
        return;

    dev = ipcc_device_gethandle(DP_CA_AP1, WAIT_RPMSG_DEV_TIMEOUT);
    if (!dev)
        return;

    /* daemon thread */
    s->rpchan_ivi = ipcc_channel_create(dev, CLUSTER_APP_EPT, CLUSTER_APP_EPT_NAME, true);
    if (s->rpchan_ivi)
        ipcc_channel_start(s->rpchan_ivi, process_ivi_message);

    dev = ipcc_device_gethandle(DP_CA_AP2, WAIT_RPMSG_DEV_TIMEOUT);
    if (!dev)
        return;

    s->rpchan_cluster = ipcc_channel_create(dev, IVI_APP_EPT, IVI_APP_EPT_NAME, true);
    if (s->rpchan_cluster)
        ipcc_channel_start(s->rpchan_cluster, process_cluster_message);

    s->binitialized = true;
}

void cluster_app(const struct app_descriptor *app, void *args)
{
    clusterapp_bridge_init();
}

APP_START(cluster_app)
 .entry = (app_entry)cluster_app,
 .stack_size = 512,
APP_END

