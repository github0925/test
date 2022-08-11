/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: inter domain notification API
*
*/
#include <reg.h>
#include <stdio.h>
#include <trace.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <platform/debug.h>
#include "dcf.h"

#if defined(CONFIG_SUPPORT_DCF) && (CONFIG_SUPPORT_DCF == 1)

void dcf_notify_attr_init(struct dcf_notifier_attribute *attr, int rproc, int addr)
{
    memset(attr, 0, sizeof(*attr));
    attr->addr = addr;
    attr->rproc = rproc;
    attr->n_flags = 0; //reserved for future
}

struct dcf_notifier *dcf_create_notifier(struct dcf_notifier_attribute *attr)
{
    struct dcf_notifier *ntf;

    ntf = malloc(sizeof(struct dcf_notifier));
    if (!ntf) {
        dprintf(0, "No memory for notifier\n");
        posix_set_errno(-ENOMEM);
        return NULL;
    }

    ntf->client = hal_mb_get_client_with_addr(attr->addr);
    if (!ntf->client) {
        printf("get ipi failed failed\n");
        posix_set_errno(-ENODEV);
        return NULL;
    }

    ntf->mchan = hal_mb_request_channel_with_addr(ntf->client, true,
                                  NULL, attr->rproc, attr->addr);
    if (!ntf->mchan) {
        printf("request ipi channel failed\n");
        posix_set_errno(-ENOENT);
        return NULL;
    }

    return ntf;
}

void dcf_destroy_notifier(struct dcf_notifier *ntf)
{
    if (!ntf)
        return;

    if (ntf->mchan) {
        hal_mb_free_channel(ntf->mchan);
    }

    if (ntf->client)
        hal_mb_put_client(ntf->client);

    free(ntf);
}

int dcf_do_notify(struct dcf_notifier *ntf, u8 *data, u16 len)
{
    return hal_mb_send_data(ntf->mchan, data, len, 1000);
}

#endif
