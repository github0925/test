/*
 * Copyright (c) 2020  Semidrive
 *
 */

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

#define ECHO_EPT_NAME		"rpmsg-echo"
#define ECHO_LINUX_EPT		(30)

int rpmsg_echo_loop(struct rpmsg_device *rdev)
{
    int recved = 0;
    int rproc;
    unsigned long src;
    char payload[RL_BUFFER_PAYLOAD_SIZE];
    struct rpmsg_channel *chn;

    rproc = rdev->config.remote_proc;
    chn = rpmsg_channel_create(rproc, ECHO_LINUX_EPT, ECHO_EPT_NAME);
    if (!chn)
        return ERR_NO_RESOURCES;

    rpmsg_channel_start(chn, NULL);

    while (1) {
        rpmsg_channel_recvfrom(chn, &src, payload,
                               RL_BUFFER_PAYLOAD_SIZE, &recved, RL_BLOCK);
        chn->rx_cnt++;
        chn->rx_bytes += recved;

#if LK_DEBUGLEVEL > 1
        printf("[echo]From %d:%lu received %d bytes: count:%d\n",
                        chn->rproc, src, recved, chn->rx_cnt);
        hexdump8(payload, MIN(32, recved));
#endif

        if (payload[0] == COMM_MSG_CCM_ECHO) {
            dprintf(2, "Echoing %d bytes to %d:%lu\n", recved, rproc, src);
            rpmsg_channel_sendto(chn, src, payload, recved, RL_BLOCK);
            chn->tx_cnt++;
            chn->tx_bytes += recved;
            continue;
        }

        if (payload[0] == COMM_MSG_CCM_ACK) {
            dprintf(2, "Acknowledge to %d:%lu\n", rproc, src);
            rpmsg_channel_sendto(chn, src, payload, 4, RL_BLOCK);
            continue;
        }

        dprintf(2, "drop %d bytes\n", recved);
        chn->drop_cnt++;

    }

    return 0;
}

