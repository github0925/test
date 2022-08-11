/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <platform.h>
#include <pow2.h>
#include <assert.h>

#include "sd_ipcc.h"
#include "rpmsg_common.h"
#include "ipcc_internal.h"
#include "rpbuf.h"

/* call by mbox callback as fake interrupt */
static void ipcc_mb_interrupt(sd_ipcc_chan_t *chan, u8 *mssg, u32 len)
{
#if LK_DEBUGLEVEL > 1
    dprintf(2, "ipcc: chan %s mssg: %p len: %d\n", chan->ch_name, mssg, len);
    hexdump(mssg, len);
#endif

    if (chan && chan->callback) {
        chan->callback(chan, mssg, len);
    }
}

void *sd_ipcc_alloc_send_buffer(sd_ipcc_chan_t *chan, u32 *len, u16 *idx)
{
    sd_rpbuf_t *rpbuf;

    if (!len || !idx || *len > HAL_MB_MTU) {
        return NULL;
    }

    rpbuf = sd_rpbuf_alloc(chan->buf_pool, SD_RPBUF_F_TX);
    if (!rpbuf) {
        return NULL;
    }

    /* reseve msg header for mbox */
//    sd_rpbuf_reserve(rpbuf, 4);
    *len = MIN(*len, rpbuf->buf_size);

    *idx = rpbuf->buf_id & 0xffff;

    return sd_rpbuf_pop_data(rpbuf, NULL, 0);
}

static void ipcc_mb_receive_callback(hal_mb_client_t cl, void *mssg, u16 len)
{
    sd_ipcc_chan_t *chan = NULL;
    sd_rpbuf_t *ibuf;
    spin_lock_saved_state_t flags;

    chan = hal_mb_get_user(cl);
    if (!chan) {
        dprintf(ALWAYS, "ipcc: no chan (0x%x) found\n", (int)cl->this_addr);
        return;
    }

    dprintf(2, "ipcc: chan %s mssg: %p len: %d isr\n", chan->ch_name, mssg, len);

    if (chan->mb_rx_thread) {
        spin_lock_irqsave(&chan->queue_lock, flags);

        ibuf = sd_rpbuf_alloc(chan->buf_pool, SD_RPBUF_F_RX);
        if (!ibuf) {
            chan->drop_cnt++;
            spin_unlock_irqrestore(&chan->queue_lock, flags);
            dprintf(ALWAYS, "ipcc: %s no enough rxbuf, need flow control\n", chan->ch_name);
            return;
        }

        if (mssg && len)
            sd_rpbuf_push_data(ibuf, mssg, len);

        sd_rpbuf_enqueue(&chan->rx_queue, ibuf);
        spin_unlock_irqrestore(&chan->queue_lock, flags);
        event_signal(&chan->mb_rx_event, false);
        return;
    }

    chan->rx_cnt++;
    chan->rx_bytes += len;

    /* this is a IPI channel with msg */
    ipcc_mb_interrupt(chan, mssg, len);

    return;
}

static int ipcc_mb_rx_thread(void *arg)
{
    sd_ipcc_chan_t *chan = (sd_ipcc_chan_t *)arg;
    sd_rpbuf_t *ibuf = NULL;
    spin_lock_saved_state_t flags;

    dprintf(INFO, "ipcc: rx thread is started\n");
    for (;;) {
wait_event:
        event_wait(&chan->mb_rx_event);

        spin_lock_irqsave(&chan->queue_lock, flags);
        while(!sd_rpbuf_queue_empty(&chan->rx_queue)) {
            ibuf = sd_rpbuf_dequeue(&chan->rx_queue);
            if (!ibuf) {
                spin_unlock_irqrestore(&chan->queue_lock, flags);
                goto wait_event;
            }

            chan->rx_cnt++;
            chan->rx_bytes += ibuf->data_len;
            spin_unlock_irqrestore(&chan->queue_lock, flags);

            /* this user callback may call mbox API again */
            ipcc_mb_interrupt(chan, ibuf->buffer, ibuf->data_len);

            spin_lock_irqsave(&chan->queue_lock, flags);
        }

        spin_unlock_irqrestore(&chan->queue_lock, flags);
    }

    return 0;
}

static sd_ipcc_chan_t *sd_ipcc_chan_create_detailed(int remote, char *name,
        int myaddr, int flags, bool threaded, ipcc_usr_cb usr_cb, uint32_t bufnum, uint32_t bufsz)
{
    sd_ipcc_chan_t *chan;
    rpmsg_cfg_extend_t cfg;
    char thread_name[32];

    cfg.init_flags = flags;
    chan = malloc(sizeof(sd_ipcc_chan_t));

    if (!chan) {
        dprintf(0, "%s: NO memory\n", __func__);
        return NULL;
    }
    memset(chan, 0, sizeof(sd_ipcc_chan_t));

    chan->mb_cl = hal_mb_get_client_with_addr((u8)cfg.mbox_src);
    if (!chan->mb_cl) {
        dprintf(0, "%s: get mb handle failed\n", __func__);
        goto fail_out;
    }

    dprintf(SPEW, "ipcc: get mb handle src %d dst %d\n", cfg.mbox_src, cfg.mbox_dst);

    chan->mb_chan = hal_mb_request_channel_with_addr(chan->mb_cl, false,
                                           ipcc_mb_receive_callback,
                                           remote, cfg.mbox_dst);
    if (!chan->mb_chan) {
        dprintf(0, "%s: request mb channel failed\n", __func__);
        goto fail_out;
    }

    snprintf(chan->ch_name, IPCC_MAX_NAME_SZ, "%s/%d", name, remote);
    chan->myaddr = cfg.mbox_src;
    chan->ch_flags = flags;
    chan->callback = usr_cb;
    chan->rproc = remote;

    if (threaded) {
        event_init(&chan->mb_rx_event, false, EVENT_FLAG_AUTOUNSIGNAL);
        spin_lock_init(&chan->queue_lock);
        sd_rpbuf_init_queue(&chan->rx_queue);

        chan->buf_pool = sd_rpbuf_create_pool(bufsz, bufnum);
        if (!chan->buf_pool) {
            dprintf(CRITICAL, "failed to malloc rx buffers\n");
            goto fail_out;
        }

        snprintf(thread_name, 32, "rx-%s", chan->ch_name);
        chan->mb_rx_thread = thread_create(thread_name, ipcc_mb_rx_thread,
                      (void *)chan, THREAD_PRI_IPCC_RX, CONFIG_CHAN_STACK_SIZE);
        if (!chan->mb_rx_thread) {
            dprintf(CRITICAL, "failed to create thread create\n");
            goto fail_out;
        }
        thread_detach_and_resume(chan->mb_rx_thread);
        dprintf(1, "rx thread prio=%d stack=%d\n", THREAD_PRI_IPCC_RX, CONFIG_CHAN_STACK_SIZE);
    }

    hal_mb_set_user(chan->mb_cl, (void *)chan);

    return chan;

fail_out:
    if (chan->buf_pool)
        sd_rpbuf_remove_pool(chan->buf_pool);

    if (chan->mb_chan)
        hal_mb_free_channel(chan->mb_chan);

    if (chan->mb_cl)
        hal_mb_put_client(chan->mb_cl);

    free(chan);

    return NULL;
}

sd_ipcc_chan_t *sd_ipcc_chan_create_mq(int remote, char *name,
        int myaddr, int flags, ipcc_usr_cb usr_cb, int queues, int size)
{
    return sd_ipcc_chan_create_detailed(remote, name,
                myaddr, flags, true, usr_cb, queues, size);
}

sd_ipcc_chan_t *sd_ipcc_chan_create(int remote, char *name,
        int myaddr, int flags, ipcc_usr_cb usr_cb)
{
    return sd_ipcc_chan_create_detailed(remote, name,
                myaddr, flags, false, usr_cb, 0, 0);
}

void sd_ipcc_chan_free(sd_ipcc_chan_t *chan)
{
    if (chan->buf_pool)
        sd_rpbuf_remove_pool(chan->buf_pool);

    hal_mb_free_channel(chan->mb_chan);
    hal_mb_put_client(chan->mb_cl);

    memset(chan, 0xcd, sizeof(sd_ipcc_chan_t));
    free(chan);
}

int sd_ipcc_chan_send_buf(sd_ipcc_chan_t *chan, sd_rpbuf_t *rpbuf)
{
    status_t ret = NO_ERROR;
    u16 len_short = (u16) rpbuf->data_len;
    u32 timeout = 100;
//    spin_lock_saved_state_t flags;

#if LK_DEBUGLEVEL > 0
    /* debug only
     * spend much to print, low speed uart may be timeout
     */
    timeout = 500;
#endif

    chan->tx_start = current_time_hires();
    chan->tx_cnt++;
    chan->tx_bytes += rpbuf->data_len;
    ret = hal_mb_send_data(chan->mb_chan, rpbuf->buffer + rpbuf->wroff, len_short, timeout);

    if (ret != NO_ERROR) {
        chan->err_cnt++;
        if (ret == ERR_NOT_READY || ret == ERR_TIMED_OUT)
            dprintf(INFO, "Fail to send, rproc%d may offline\n", chan->rproc);
        else
            dprintf(0, "Fail to send to %d ret: %d\n", chan->rproc, ret);
    }
    chan->tx_end = current_time_hires();

    dprintf(1, "%s: tx %d bytes, %lld us\n", chan->ch_name, rpbuf->data_len, chan->tx_end - chan->tx_start);

    return ret;
}

int sd_ipcc_chan_send_data(sd_ipcc_chan_t *chan, u8 *dat, u32 len)
{
    status_t ret = NO_ERROR;
    u16 len_short = (u16) len;
    u32 timeout = 100;
//    spin_lock_saved_state_t flags;

#if LK_DEBUGLEVEL > 0
    /* debug only
     * spend much to print, low speed uart may be timeout
     */
    timeout = 500;
#endif

    chan->tx_start = current_time_hires();

    chan->tx_cnt++;
    chan->tx_bytes += len;

//    spin_lock_irqsave(&chan->tx_lock, flags);
    ret = hal_mb_send_data(chan->mb_chan, dat, len_short, timeout);
//    spin_unlock_irqrestore(&chan->tx_lock, flags);

    if (ret != NO_ERROR) {
        chan->err_cnt++;
        if (ret == ERR_NOT_READY || ret == ERR_TIMED_OUT)
            dprintf(INFO, "Fail to send, rproc%d may offline\n", chan->rproc);
        else
            dprintf(0, "Fail to send to %d ret: %d\n", chan->rproc, ret);
    }
    chan->tx_end = current_time_hires();

#if LK_DEBUGLEVEL > 0
    timeout = (int)(chan->tx_end - chan->tx_start);
    if (chan->tx_end > 500)
        dprintf(0, "%s: tx %d bytes, %d us\n", chan->ch_name, len, timeout);
#endif

    return ret;
}

int sd_ipcc_chan_send_data_nocopy(sd_ipcc_chan_t *chan, u8 *dat, u32 len, u16 mb_buf_idx)
{
    status_t ret = NO_ERROR;
    u32 timeout = 100;
//    spin_lock_saved_state_t flags;

#if LK_DEBUGLEVEL > 0
    /* debug only
     * spend much to print, low speed uart may be timeout
     */
    timeout = 500;
#endif

    chan->tx_start = current_time_hires();

    chan->tx_cnt++;
    chan->tx_bytes += len;

//    spin_lock_irqsave(&chan->tx_lock, flags);
    ret = hal_mb_send_data_nocopy(chan->mb_chan, mb_buf_idx, timeout);
//    spin_unlock_irqrestore(&chan->tx_lock, flags);

    if (ret != NO_ERROR) {
        chan->err_cnt++;
        if (ret == ERR_NOT_READY || ret == ERR_TIMED_OUT)
            dprintf(INFO, "Fail to send, rproc%d may offline\n", chan->rproc);
        else
            dprintf(0, "Fail to send to %d ret: %d\n", chan->rproc, ret);
    }
    chan->tx_end = current_time_hires();

    dprintf(1, "%s: tx %d bytes, %lld us\n", chan->ch_name, len, chan->tx_end - chan->tx_start);

    return ret;
}

void *sd_ipcc_chan_recv_data(sd_ipcc_chan_t *chan, u32 *len, u16 *idx)
{
    return NULL;
}

