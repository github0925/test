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

#include "ipcc_device.h"

#define IPCC_ECHO_CHN_NAME 	"ipcc-echo"

#if CONFIG_USE_IPCC_TTY
#define IPCC_TTY_CHN_NAME 	"ipcc-tty"
#endif

#if defined(CONFIG_IPCC_ECHO_EPT) && (CONFIG_IPCC_ECHO_EPT == 1)
void *ipcc_device_import_rpbuf(struct ipcc_device *dev, sd_rpbuf_t *rpbuf,
                    unsigned long *src, int *len);
int ipcc_device_release_rpbuf(struct ipcc_device *dev, sd_rpbuf_t *rpbuf);

void start_echo_loop(struct ipcc_device *dev)
{
    struct ipcc_channel *ichan;
    int recved = 0;
    char *payload;
    struct dcf_message *msg;
    unsigned long src = 0;
    sd_rpbuf_t *rpbuf;
    bool ns = false;

    if (dev->config.rproc == IPCC_RRPOC_AP1 || dev->config.rproc == IPCC_RRPOC_AP2)
        ns = true;

    ichan = ipcc_channel_create(dev, IPCC_ECHO_EPT, IPCC_ECHO_CHN_NAME, ns);
    if (!ichan)
        return;

    ipcc_channel_start(ichan, NULL);

    while(1) {
        rpbuf = ipcc_channel_recvbuf(ichan, INFINITE_TIME);
        if (!rpbuf) {
            ichan->err_cnt++;
            dprintf(0, "ipcc:fail to receive rpbuf\n");
            continue;
        }
        recved = ichan->mtu;
        payload = ipcc_device_import_rpbuf(dev, rpbuf, &src, &recved);
        if (!payload) {
            ichan->drop_cnt++;
            dprintf(0, "ipcc: invalid rpbuf=%p \n", rpbuf);
            continue;
        }

        ichan->rx_cnt++;
        ichan->rx_bytes += recved;
        dprintf(1, "[echo]From %d:%lu received %d bytes: count:%d\n",
                      ipcc_device_get_rproc(dev), src, recved, ichan->rx_cnt);

        msg = (struct dcf_message *) payload;
        if (msg->msg_type == COMM_MSG_CCM_ECHO) {
            dprintf(2, "[echo]Echoing %d bytes to %d:%lu\n", recved, ipcc_device_get_rproc(dev), src);
            ipcc_channel_sendto(ichan, src, payload, recved, 1000);
            ichan->tx_cnt++;
            ichan->tx_bytes += recved;
            ipcc_device_release_rpbuf(dev, rpbuf);
            continue;
        }

        if (msg->msg_type == COMM_MSG_CCM_ACK) {
            dprintf(2, "[echo]Acknowledge to %d:%lu\n", ipcc_device_get_rproc(dev), src);
            ipcc_channel_sendto(ichan, src, payload, 4, 1000);
            ipcc_device_release_rpbuf(dev, rpbuf);
            continue;
        }
        dprintf(1, "ipcc:drop %d bytes\n", recved);
        ipcc_device_release_rpbuf(dev, rpbuf);
        ichan->drop_cnt++;
    }

fail_out1:
    ipcc_channel_stop(ichan);
    ipcc_channel_destroy(ichan);
}
#else
void start_echo_loop(struct ipcc_device *dev) { }
#endif

#if CONFIG_USE_IPCC_TTY
static void ipcc_tty_handler(struct ipcc_channel *ichan, struct dcf_message *mssg, int len)
{
    dprintf(ALWAYS, "tty: Recv From: %lu %d bytes hexdump\n", len);
    hexdump8(mssg, len);

    return DCF_RELEASE;
}

void start_tty_device(struct ipcc_device *dev)
{
    struct ipcc_channel *ichan;

    ichan = ipcc_channel_create(dev, IPCC_TTY_EPT, IPCC_TTY_CHN_NAME, true);
    if (!ichan)
        reutrn;

    ipcc_channel_start(ichan, ipcc_tty_handler);
}
else
void start_tty_device(struct ipcc_device *dev) { }
#endif

