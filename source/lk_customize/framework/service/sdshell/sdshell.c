/*
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include <dcf.h>
#include <ipcc_device.h>
#include <lib/cbuf.h>
#include <lk/init.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include "sdshell.h"

static int tx_len = 0;
static cbuf_t vuart_cbuf;
static char vuart_tx_buf[LINE_TXBUF_SIZE];

int ipcc_channel_set_mtu(struct ipcc_channel *ichan, unsigned int mtu);

typedef struct sdshell_service_control {
    bool xcon_en;
    bool logbuf_en;
    bool thread_running;
    uint32_t core_id;
    uint32_t bk_len;
    char *txbuf;
    struct ipcc_device *dev;
    struct ipcc_channel *chan;
    thread_t *thread;
    event_t event;
    sdshell_msg_t msg;
} sdshell_ctl_t;

static sdshell_ctl_t sdshell_ctl;

static sdshell_ctl_t *sdshell_ctl_get(void)
{
    return &sdshell_ctl;
}

static int sdshell_save_char(char *buf, int len)
{
    int i;
    cbuf_t *cbuf = &vuart_cbuf;

    for (i = 0; i < len; i++)
        cbuf_write_char(cbuf, buf[i], false);

    cbuf_write_char(cbuf, '\n', false);
    return 0;
}

static int sdshell_send_msg(char *buf, int len)
{
    sdshell_ctl_t *ctl = sdshell_ctl_get();
    int slen = 0;
    int pos = 0;
    int ret;

    while (pos < len) {
        if (MAX_IPCC_BUFF_SIZE + pos < len)
            slen = MAX_IPCC_BUFF_SIZE;
        else
            slen = len - pos;

        ret = ipcc_channel_sendto(ctl->chan, SDSHELL_SERVICE_EPT, (buf + pos),
                                  slen, 500);

        if (ret < 0)
            return ret;

        pos += slen;
    }

    return 0;
}

cbuf_t *sdshell_get_rx_cbuf(void)
{
    return &vuart_cbuf;
}

int sdshell_wake_thread(void)
{
    sdshell_ctl_t *ctl = sdshell_ctl_get();

    if (ctl->xcon_en && (!ctl->thread_running) && ctl->logbuf_en)
        event_signal(&ctl->event, false);

    return 0;
}

static int sdshell_resource_free(sdshell_ctl_t *ctl)
{
    ctl->core_id = SDRV_CORE_MAX;
    ctl->xcon_en = 0;
    return 0;
}

static void sdshell_event_handle(struct ipcc_channel *chan,
                                 struct dcf_message *dmsg, int len, int src)
{
    int read_len;
    char *cmdbuf;
    sdshell_ctl_t *ctl = sdshell_ctl_get();
    memcpy(&ctl->msg, (char *)dmsg, sizeof(ctl->msg));

    if (ctl->msg.type >= SDSHELL_MSG_MAX) {
        dprintf(CRITICAL, "invalid message type\n");
        return;
    }

    if ((ctl->msg.type != SDSHELL_MSG_TARGET_SET)
            && (ctl->core_id >= SDRV_CORE_MAX))
        return;

    switch (ctl->msg.type) {
        case SDSHELL_MSG_TARGET_SET:
            if (ctl->msg.size > 0) {
                memcpy((void *)&ctl->core_id, (char *)dmsg + sizeof(ctl->msg),
                       ctl->msg.size);

                if (ctl->core_id >= SDRV_CORE_MAX) {
                    dprintf(CRITICAL, "invalid core type = %d\n", ctl->core_id);
                    return;
                }

                if (vuart_is_available())
                    ctl->logbuf_en = 1;
                else
                    ctl->logbuf_en = 0;
            }

            break;

        case SDSHELL_MSG_READ:
            if (!ctl->logbuf_en) {
                dprintf(CRITICAL, "last log buffer is not available\n");
                return;
            }

            vuart_get_last_buf_and_size(&ctl->txbuf, &ctl->bk_len);

            if (ctl->bk_len == 0)
                return;

            if (ctl->txbuf[ctl->bk_len - 1] != '\n')
                ctl->txbuf[ctl->bk_len - 1] = '\n'; //forcing a newline

            sdshell_send_msg(ctl->txbuf, ctl->bk_len);
            break;

        case SDSHELL_MSG_RUN_CMD:
            if (ctl->logbuf_en)
                vuart_update_tail();

            if (ctl->msg.size > 0) {
                cmdbuf = (char *)dmsg + sizeof(ctl->msg);
                dprintf(INFO, "cmdbuf = %s, size = %d\n", cmdbuf, ctl->msg.size);
                sdshell_save_char(cmdbuf, ctl->msg.size);
                ctl->xcon_en = 1;
            }

            break;

        case SDSHELL_MSG_PRINTON:
            if (!ctl->logbuf_en)
                return;

            vuart_update_tail();
            ctl->xcon_en = 1;
            break;

        case SDSHELL_MSG_PRINTOFF:
            ctl->xcon_en = 0;
            break;

        case SDSHELL_MSG_CLEAR:
            if (!ctl->logbuf_en)
                return;

            if (!ctl->thread_running)
                vuart_clear_buf();

            break;

        case SDSHELL_MSG_QUIT:
            sdshell_resource_free(ctl);
            break;
    }

    ctl->msg.type = SDSHELL_MSG_MAX;
    dprintf(INFO, "ctl->msg.type = %d, ctl->msg.size = %d, len = %d\n",
            ctl->msg.type, ctl->msg.size, len);
}

static int sdshell_task(void *arg)
{
    sdshell_ctl_t *ctl = arg;
    int len;

    for (;;) {
        ctl->thread_running = 0;
        event_wait(&ctl->event);
        ctl->thread_running = 1;

        do {
            len = vuart_get_new_buf(vuart_tx_buf, LINE_TXBUF_SIZE);

            if (len <= 0)
                break;

            sdshell_send_msg((char *)vuart_tx_buf, len);
        } while (len >= LINE_TXBUF_SIZE);
    }

    return 0;
}

static void sdshell_hk(uint32_t level)
{
    sdshell_ctl_t *ctl = sdshell_ctl_get();
    ctl->dev = ipcc_device_gethandle(IPCC_RRPOC_AP1, 1000);

    if (!ctl->dev)
        return;

    ctl->chan = ipcc_channel_create(ctl->dev, SDSHELL_SERVICE_EPT,
                                    SDSHELL_SERVICE_NAME, true);

    if (ctl->chan) {
        ipcc_channel_set_mtu(ctl->chan, MAX_IPCC_BUFF_SIZE);
        ipcc_channel_start(ctl->chan, sdshell_event_handle);
    }
    else {
        dprintf(CRITICAL, "failed to create ipcc channel for sdshell\n");
        return;
    }

    cbuf_initialize(&vuart_cbuf, CMD_RXBUF_SIZE);
    event_init(&ctl->event, false, EVENT_FLAG_AUTOUNSIGNAL);
    ctl->thread = thread_create("sdshell_tx", sdshell_task, ctl,
                                DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(ctl->thread);
    /* core type init */
    ctl->core_id = SDRV_CORE_MAX;
    dprintf(INFO, "sdshell init succeeded\n");
}

/* the sdshell_hk must be executed before shell_entry */
LK_INIT_HOOK(sdshell_hk, sdshell_hk, (LK_INIT_LEVEL_APPS - 1));
