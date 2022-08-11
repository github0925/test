//*****************************************************************************
//
// mbox_hal.c - Driver for the rstgen hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <stdlib.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <platform.h>
#include <sys/types.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <kernel/event.h>
#include "res.h"
#include "chip_res.h"
#include "target_res.h"
#include "mbox_hal.h"

#define LOCAL_TRACE             (1U)
#define MB_MAX_CL_NUM           (16U)
#define MBOX_HANDLE_MAGIC       (0x31415926U)
#define MB_DEFAULT_ADDR         (0xfe)

/* mbox controller features */
#define CONFIG_STATIC_MB_HAL    (1)

#ifndef MU_MESSAGE_READY_INT
#define MU_MESSAGE_READY_INT    (88)
#endif

#ifndef __irq__
#define __irq__
#endif

/* POLL TX done in this interval in microsecond */
#define MB_TX_HRTMR_POLL        (2)
/* POLL TX done max spin time in microsecond */
#define MB_TX_MAX_ACK_TIME      (1000)

/* mailbox controller structure */
typedef struct _mbox_instance_ {
    mutex_t mbox_mutex;   /*!< mbox layer mutex*/
    bool mbox_inited;
    uint mb_irq;
    addr_t paddr;
    int hw_id;
    mutex_t mb_tx_mutex;

    u8 cl_num;
    struct _hal_mbox_client cl[MB_MAX_CL_NUM];
} hal_mb_instance_t;

/* mbox global instance */
#if CONFIG_STATIC_MB_HAL
static hal_mb_instance_t mb_hal;
#endif
static hal_mb_instance_t *g_hal_mbox;

inline static bool IS_HAL_READY(void)
{
    return g_hal_mbox && g_hal_mbox->mbox_inited;
}

inline static bool IS_VALID_HANDLE(hal_mb_client_t cl)
{
    return cl && (cl->magic == MBOX_HANDLE_MAGIC);
}

inline static __irq__ hal_mb_client_t get_client_by_addr(u8 addr)
{
    u32 idx;
    hal_mb_client_t cl;

    for (idx = 0;idx < MB_MAX_CL_NUM; idx++) {
        cl = &g_hal_mbox->cl[idx];
        if (cl->is_used && (addr == cl->this_addr))
            return cl;
    }

    dprintf(3, "mbox: channel addr %d not found\n", addr);

    return NULL;
}

inline static __irq__ hal_mb_client_t find_used_client_inisr(u32 rproc)
{
    u32 idx;
    hal_mb_client_t cl;

    for (idx = 0;idx < MB_MAX_CL_NUM; idx++) {
        cl = &g_hal_mbox->cl[idx];
        if (cl->is_used && (rproc == cl->mchan.remote_proc))
            return cl;
    }

    return NULL;
}

inline static status_t MB_LOCK_HAL(void)
{
    return mutex_acquire(&g_hal_mbox->mbox_mutex);
}

inline static status_t MB_UNLOCK_HAL(void)
{
    return mutex_release(&g_hal_mbox->mbox_mutex);
}

int hal_mb_set_user(hal_mb_client_t cl, void *data)
{
    if (!IS_VALID_HANDLE(cl))
        return -1;

    cl->user_data = data;

    return 0;
}

void *hal_mb_get_user(hal_mb_client_t cl)
{
    if (!IS_VALID_HANDLE(cl))
        return NULL;

    return cl->user_data;
}

hal_mb_client_t hal_mb_get_client_with_addr(u8 myaddr)
{
    uint i;
    hal_mb_client_t cl, ret = NULL;

    if (!IS_HAL_READY())
        return ret;

    if (!g_hal_mbox->cl_num) {
        dprintf(ALWAYS, "mbox: No client available %d\n", myaddr);
        return ret;
    }

    MB_LOCK_HAL();

    for (i = 0; i < MB_MAX_CL_NUM; i++) {
        cl = &g_hal_mbox->cl[i];
        if (cl->is_used && (myaddr == cl->this_addr)) {
            MB_UNLOCK_HAL();
            dprintf(ALWAYS, "mbox: this addr %d already exist\n", myaddr);
            return NULL;
        }
    }

    for (i = 0; i < MB_MAX_CL_NUM; i++) {
        cl = &g_hal_mbox->cl[i];
        if (!cl->is_used) {
            cl->is_used = true;
            cl->magic = MBOX_HANDLE_MAGIC;
            cl->this_addr = myaddr;
            ret = cl;
            g_hal_mbox->cl_num--;
            break;
        }
    }

    MB_UNLOCK_HAL();
    return ret;
}

/*mbox HAL interface*/
hal_mb_client_t hal_mb_get_client()
{
    /* 0xfe is a reserved addr for back-compatibility */
    return hal_mb_get_client_with_addr(MB_DEFAULT_ADDR);
}

void hal_mb_put_client(hal_mb_client_t cl)
{
    if (!IS_HAL_READY())
        return;

    if (!IS_VALID_HANDLE(cl))
        return;

    MB_LOCK_HAL();

    g_hal_mbox->cl_num++;
    hal_mb_free_channel(&cl->mchan);
    cl->is_used = false;
    cl->magic = 0;
    cl->this_addr = 0;
    /*TODO: to check and free msg waiting in tx queue */

    MB_UNLOCK_HAL();

    return;
}

static void __irq__ __received_data(u32 rproc, u32 receiver, void *data, u16 len)
{
    hal_mb_client_t cl;
    sd_msghdr_t *msg;

    if (!IS_HAL_READY())
        return;

    /* vdsp use rmh1 & rmh2 only */
    if (rproc == IPCC_RRPOC_VDSP) {
        cl = find_used_client_inisr(rproc);
        msg = NULL;
    }
    else {
        cl = get_client_by_addr((u8)receiver);
        msg = (sd_msghdr_t *)data;
    }

    if (!IS_VALID_HANDLE(cl)) {
#if CONFIG_CHECK_ADDR_LEGACY
        /* if no addr match, fall back and try the same proc client */
        cl = find_used_client_inisr(rproc);
        if (!IS_VALID_HANDLE(cl)) {
            dprintf(2, "mbox: msg from rproc%d client:%d not found\n",
                    rproc, receiver);
            return;
        }
#else
        return;
#endif
    }

    if (len < sizeof(sd_msghdr_t)) {
        dprintf(0, "mbox: bad msg from rproc%d client:%d\n", rproc, receiver);
        return;
    }

    if (cl->rx_new_data) {
        if (rproc == IPCC_RRPOC_VDSP) {
            u32 msg[2];
            msg[0] = receiver;
            msg[1] = (u32)(long)data;
            //printf("from vdsp, got 0x%lx, 0x%lx\n", (long unsigned int)msg[0], (long unsigned int)msg[1]);
            cl->rx_new_data(cl, msg, 8);
        } else {
            cl->rx_new_data(cl, msg->data, msg->dat_len - sizeof(sd_msghdr_t));
        }
    }
}

hal_mb_chan_t* hal_mb_request_channel_with_addr(hal_mb_client_t cl,
    bool low_latency, hal_mb_rx_cb cb, hal_mb_proc_t remote, int address)
{
    void * msgq_base = 0;
    int i;

    if (!IS_VALID_HANDLE(cl))
        return NULL;

    /* TODO: move the tx buffer to middleware, keep heap-free here
     * the message queue is as tx buffer
     */
    msgq_base = malloc(MBOX_TX_BUF_SZ);
    if (!msgq_base) {
        dprintf(0, "mbox: no memory available\n");
        return NULL;
    }

    cl->rx_new_data = cb;
    cl->tx_complete = NULL;
    cl->tx_block = true;
    cl->low_latency = low_latency;

    memset(&cl->mchan, 0, sizeof(cl->mchan));
    spin_lock_init(&cl->mchan.lock);

    cl->mchan.msg_free = 0;
    cl->mchan.msg_count = 0;
    for (i = 0; i < MBOX_TX_QUEUE_LEN;i++) {
        cl->mchan.msg_data[i] = (u8 *)msgq_base + i * MBOX_TX_MTU;
    }

    cl->mchan.remote_proc = remote;
    cl->mchan.dest_addr = address;
    if (cl->mchan.remote_proc > IPCC_RRPOC_MAX) {
        dprintf(CRITICAL, "mbox: rproc%d out of range\n", cl->mchan.remote_proc);
        goto request_fail_out;
    }

    cl->mchan.hwchan = sd_mbox_request_channel(remote, cl->this_addr);
    if (!cl->mchan.hwchan) {
        dprintf(0, "mbox: no more hwchan\n");
        goto request_fail_out;
    }

    dprintf(2, "mbox: requested channel to %d:%d \n", remote, address);

    return &cl->mchan;

request_fail_out:
    if (msgq_base)
        free(msgq_base);

    return NULL;
}

hal_mb_chan_t *hal_mb_request_channel(hal_mb_client_t cl, bool low_latency,
                                      hal_mb_rx_cb cb, hal_mb_proc_t remote)
{
    return hal_mb_request_channel_with_addr(cl, low_latency,
                cb, remote, MB_DEFAULT_ADDR);
}

void hal_mb_free_channel(hal_mb_chan_t *chan)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);

    if (!IS_VALID_HANDLE(cl))
        return;

    if (spin_lock_held(&chan->lock))
        spin_unlock(&chan->lock);

    if (chan->hwchan) {
        sd_mbox_free_channel(chan->hwchan);
    }

    free(chan->msg_data[0]);

    memset(&cl->mchan, 0, sizeof(cl->mchan));

    return;
}

int hal_mb_cancel_lastsend(hal_mb_chan_t *chan);

static int __alloc_avail_buf(hal_mb_chan_t *chan)
{
    int idx;
    spin_lock_saved_state_t flags;

    spin_lock_irqsave(&chan->lock, flags);

    /* See if there is any space left */
    if (chan->msg_count == MBOX_TX_QUEUE_LEN) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -1;
    }

    idx = chan->msg_free;
    chan->msg_count++;

    if (idx == MBOX_TX_QUEUE_LEN - 1)
        chan->msg_free = 0;
    else
        chan->msg_free++;

    spin_unlock_irqrestore(&chan->lock, flags);

    return idx;
}

static status_t __free_used_buf(hal_mb_chan_t *chan)
{
    spin_lock_saved_state_t flags;

    spin_lock_irqsave(&chan->lock, flags);

    if (chan->msg_count)
        chan->msg_count--;

    spin_unlock_irqrestore(&chan->lock, flags);

    return 0;
}

static status_t __send_data_sync(hal_mb_chan_t *chan, u8 *data, lk_time_t timeout)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    u32 spin_usec = MB_TX_MAX_ACK_TIME;
    int ret = 0;
    lk_time_t end_time;

    if (!cl->mchan.hwchan) {
        dprintf(0, "mbox: hwchan of %s not exist\n", chan->name);
        return ERR_NO_RESOURCES;
    }

    if (!timeout) {
        dprintf(0, "mbox: %s nb not supported\n", __func__);
        return ERR_NOT_SUPPORTED;
    }

    end_time = current_time() + timeout;
    do {
        ret = sd_mbox_send_data(chan->hwchan, data);
        if (ret < 0) {
            thread_yield();
        }

    } while ((ret == ERR_NO_MSG) && TIME_LT(current_time(), end_time));

    if (ret < 0) {
        if (ret != ERR_NOT_READY)
           dprintf(0, "mbox: send fail %d %d:%d\n", ret,
                   chan->remote_proc, chan->dest_addr);
        goto send_fail_out;
    }

    while (!sd_mbox_last_tx_done(chan->hwchan)) {
        /* spin polling ack signal in MB_TX_MAX_ACK_TIME interval */
        if (spin_usec >= MB_TX_HRTMR_POLL) {
            spin(MB_TX_HRTMR_POLL);
            spin_usec -= MB_TX_HRTMR_POLL;
            continue;
        }

        /* fallback: polling in 1 ms interval */
        if (TIME_LT(current_time(), end_time)) {
            thread_sleep(1);
        } else {
            ret = ERR_TIMED_OUT;
            hal_mb_cancel_lastsend(chan);
            break;
        }
    }

send_fail_out:

    return ret;
}

int hal_mb_send_data_detail(hal_mb_chan_t *chan, u8 *data, u16 len, int proto,
            int priority, u8 dest, lk_time_t timeout)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    u8 *msg_buf;
    sd_msghdr_t *msg;
    int ret = 0;

    if (!IS_VALID_HANDLE(cl))
        return ERR_INVALID_ARGS;

    if (len > HAL_MB_MTU) {
        dprintf(ALWAYS, "mbox: send data length: %d exceed limitation\n", len);
        return ERR_BAD_LEN;
    }

    ret = __alloc_avail_buf(chan);
    if (ret < 0) {
        dprintf(ALWAYS, "mbox: no avail buf\n");
        return ERR_BUSY;
    }
    msg_buf = chan->msg_data[ret];

    msg = (sd_msghdr_t *)msg_buf;
    mb_msg_init_head(msg, chan->remote_proc, proto, priority, len, dest);
    memcpy(msg->data, data, len);
    /* wait for a while, 1000 ms for hardcode */
    ret = __send_data_sync(chan, (u8 *)msg, timeout);
    if (!ret) {
        dprintf(SPEW, "mbox: Sent %d bytes to %d:%d\n", len,
                    chan->remote_proc, dest);
    }
    __free_used_buf(chan);

    return ret;
}

int hal_mb_send_data(hal_mb_chan_t *chan, u8 *data, u16 len,
                          u32 timeout)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);

    return hal_mb_send_data_detail(chan, data, len, MB_MSG_PROTO_DFT,
                                   cl->low_latency, chan->dest_addr, timeout);
}

int hal_mb_send_data_rom(hal_mb_chan_t *chan, u8 *data, u16 len)
{
  return hal_mb_send_data_detail(chan, data, len, MB_MSG_PROTO_ROM,
                                  true, 0, 1000);
}

int hal_mb_send_data_dsp(hal_mb_chan_t *chan, u8 *data, u16 len)
{
    return hal_mb_send_data_detail(chan, data, len, MB_MSG_PROTO_DSP,
                                    true, 0, 1000);
}

int hal_mb_alloc_txbuf(hal_mb_chan_t *chan, u8 **data, u16 *len)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    u8 *msg_buf;
    sd_msghdr_t *msg;
    int ret = 0;

    if (!IS_VALID_HANDLE(cl))
        return -1;

    if (!data || !len) {
        return -1;
    }

    if (*len > HAL_MB_MTU) {
        dprintf(ALWAYS, "mbox: alloc data length: %d exceed limitation", *len);
        return -1;
    }

    ret = __alloc_avail_buf(chan);
    if (ret < 0) {
        dprintf(ALWAYS, "mbox: alloc avail buf fail");
        return ret;
    }
    msg_buf = chan->msg_data[ret];

    msg = (sd_msghdr_t *) msg_buf;
    mb_msg_init_head(msg, chan->remote_proc, MB_MSG_PROTO_DFT,
                     cl->low_latency, *len, chan->dest_addr);

    *data = (u8 *) msg->data;

    return ret;
}

int hal_mb_send_data_nocopy(hal_mb_chan_t *chan, hal_mb_buf_id idx, u32 timeoutMs)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    u8 *msg_buf;
    int ret = 0;

    if (!IS_VALID_HANDLE(cl))
        return ERR_INVALID_ARGS;

    if (idx & 0x8000) {
        idx &= MBOX_TX_QUEUE_LEN - 1;
    }

    msg_buf = chan->msg_data[idx];
    ret = __send_data_sync(chan, msg_buf, timeoutMs);
    if (!ret) {
        dprintf(SPEW, "mbox: %d queued data was sent ret: %d\n", idx, ret);
    }
    __free_used_buf(chan);
    return ret;
}

int hal_mb_cancel_lastsend(hal_mb_chan_t *chan)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    status_t ret = 0;

    if (!IS_VALID_HANDLE(cl))
        return ERR_INVALID_ARGS;

    if (!cl->mchan.hwchan) {
        printf("mbox: hwchan of %s not exist\n", chan->name);
        return NO_ERROR;
    }

    while (chan->msg_count) {
        ret = sd_mbox_cancel_lastsend(chan->hwchan);
        if (!ret)
            __free_used_buf(chan);
        else
            break;
    }

    return ret;
}

int hal_mb_send_ipi(hal_mb_chan_t *chan)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);
    sd_msghdr_t msg;

    mb_msg_init_head(&msg, chan->remote_proc, MB_MSG_PROTO_DFT,
                        cl->low_latency, 0, chan->dest_addr);

    return __send_data_sync(chan, (u8*)&msg, 10);
}

int hal_mb_send_msg(hal_mb_chan_t *chan, sd_msghdr_t *msg, u32 timeoutMs)
{
    return __send_data_sync(chan, (u8 *)msg, timeoutMs);
}

int hal_mb_recv_msg(hal_mb_chan_t *chan, void *msg)
{
    hal_mb_client_t cl = containerof(chan, struct _hal_mbox_client, mchan);

    if (!IS_VALID_HANDLE(cl))
        return ERR_INVALID_ARGS;

    /* TODO: support this feature later */

    return ERR_NOT_IMPLEMENTED;
}

static enum handler_return __irq__ mb_irq_handler(void *arg)
{
    int ret;
    hal_mb_instance_t *mbox = arg;

    // disable irq c'z msg must be handled before clear TACK
    ret = sd_mbox_rx_interrupt(mbox->mb_irq, __received_data);

    return ret;
}

void hal_mb_init(void *cl, hal_mb_cfg_t *cfg)
{
    return;
}

void hal_mb_deinit(void *cl)
{
    return;
}

bool hal_mb_create_handle(void **handle, uint32_t res_glb_idx)
{
    int ret;
    hal_mb_instance_t *mbox;

    if (g_hal_mbox) {
        goto create_done;
    }

#if CONFIG_STATIC_MB_HAL
    mbox = &mb_hal;
#else
    mbox = malloc(sizeof(*mbox));
    if (!mbox) {
        dprintf(CRITICAL, "mbox: no memory for init");
        return false;
    }
#endif
    memset(mbox, 0, sizeof(*mbox));

    mutex_init(&mbox->mbox_mutex);
    mutex_init(&mbox->mb_tx_mutex);
    mbox->cl_num = MB_MAX_CL_NUM;

    mbox->mb_irq = MU_MESSAGE_READY_INT;
    ret = res_get_info_by_id(res_glb_idx, &mbox->paddr, &mbox->hw_id);
    if (ret < 0) {
        dprintf(CRITICAL, "mbox: fail to get resource info:%d", res_glb_idx);
        return false;
    }

    ret = sd_mbox_probe(mbox->paddr);
    if (ret < 0) {
        dprintf(CRITICAL, "mbox: hardware init fail ret:%d", ret);
        return false;
    }

    /* enable interrupt for self, used for async receieve */
    register_int_handler(mbox->mb_irq, mb_irq_handler, mbox);
    unmask_interrupt(mbox->mb_irq);
    mbox->mbox_inited = true;

    g_hal_mbox = mbox;

    dprintf(INFO, "mbox: inited, irq: %d \n", g_hal_mbox->mb_irq);

create_done:

    if (handle)
        *handle = (void*) g_hal_mbox;

    return true;
}

bool hal_mb_release_handle(void *handle)
{
    sd_mbox_remove();

    dprintf(INFO, "mbox: deinited\n");

    if (g_hal_mbox) {
#if !CONFIG_STATIC_MB_HAL
        free(g_hal_mbox);
#endif
        g_hal_mbox = NULL;
    }
    return true;
}

