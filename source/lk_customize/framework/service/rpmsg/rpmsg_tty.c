/*
 * Copyright (c) 2020  Semidrive
 *
 */
#if CONFIG_RPMSG_TTY
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <kernel/semaphore.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <kernel/event.h>
#include <dcf.h>
#include "rpmsg_rtos.h"

#define TTY_EPT_NAME		    "rpmsg-tty-raw"
static struct rpmsg_channel *rpmsg_tty;

int rpmsg_tty_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    char *ptr = payload;

    ptr[payload_len-1] = '\0';
    dprintf(ALWAYS, "vtty: Recv From: %lu %d bytes hexdump\n", src, payload_len);
    fputs(ptr, stdout);

    return RL_RELEASE;
}

void tty_msg_handler(struct dcf_message *msg, void *ctx)
{

}

int rpmsg_tty_probe(struct rpmsg_device *rdev)
{
    if (rdev->config.remote_proc == DP_CA_AP1) {
        rpmsg_tty = rpmsg_channel_create(rdev->config.remote_proc,
                                         TTY_LINUX_EPT, TTY_EPT_NAME);
        if (!rpmsg_tty) {
            dprintf(1, "tty: no rpmsg channel\n");
            return -1;
        }

        rpmsg_channel_start(rpmsg_tty, tty_msg_handler);
    }
    return 0;
}

#endif