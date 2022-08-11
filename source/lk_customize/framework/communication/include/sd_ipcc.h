/*
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#ifndef __IPCC_MB_H__
#define __IPCC_MB_H__

#include <mbox_hal.h>
#include <rpbuf.h>

#define IPCC_MAX_NAME_SZ        (16)

/* Buffer is formed by payload and struct rpmsg_msg_hdr */
#define IPCC_MB_MTU             (HAL_MB_MTU - 16)

typedef enum {
    IPCC_ADDR_INVALID           = 0x0,
    IPCC_ADDR_MB_ECHO           = 0x04,
    IPCC_ADDR_MB_TEST           = 0x05,
    IPCC_ADDR_ECHO_TEST         = 0x08,
    IPCC_ADDR_RPMSG             = 0x10,
    IPCC_ADDR_RPMSG_TEST        = 0x20,
    IPCC_ADDR_DCF_BASE          = 0x30,
    IPCC_ADDR_SDPE_RPC_BASE     = 0x60,
    IPCC_ADDR_MBOX_RAW          = 0x80,
    IPCC_ADDR_VDEV_BASE         = 0xb0,
    IPCC_ADDR_LEGACY            = 0xfe,
    IPCC_ADDR_MAX               = 0xff,
} sd_ipcc_addr_t;

struct sd_ipcc_chan;
typedef void (*ipcc_usr_cb)(struct sd_ipcc_chan *tvq, u8 *mssg, u32 len);

typedef struct sd_ipcc_chan {
    /* 32bit aligned { */
    char ch_name[IPCC_MAX_NAME_SZ];
    uint32_t ch_flags;
    uint32_t myaddr;
    uint32_t rproc;
    hal_mb_client_t mb_cl;
    hal_mb_chan_t *mb_chan;

    ipcc_usr_cb callback;

    /*
     * channel stats and performance indiacator
     */
    u32 rx_bytes;
    u32 tx_bytes;
    u32 rx_cnt;
    u32 tx_cnt;
    u32 err_cnt;
    u32 drop_cnt;
    lk_bigtime_t tx_start, tx_end;

    /* RX buffer management
     * TODO: flow control in case sender's flooding message
     * in high throughput use case
     */
    spin_lock_t queue_lock;
    event_t mb_rx_event;
    thread_t *mb_rx_thread;
    sd_rpbuf_pool_t *buf_pool;
    sd_rpbuf_queue_t rx_queue;

    void *priv; /* private pointer, upper layer instance pointer */
} sd_ipcc_chan_t;

int sd_ipcc_chan_add_consumed_buffer(sd_ipcc_chan_t *chan, char *buff);

sd_ipcc_chan_t *sd_ipcc_chan_create(int dest, char *name,
        int myaddr, int flags,
        ipcc_usr_cb cb);

sd_ipcc_chan_t *sd_ipcc_chan_create_perf(int remote, char *name,
        int myaddr, int flags, ipcc_usr_cb usr_cb);

sd_ipcc_chan_t *sd_ipcc_chan_create_mq(int remote, char *name,
        int myaddr, int flags, ipcc_usr_cb usr_cb, int queues, int size);

void sd_ipcc_chan_free(sd_ipcc_chan_t *chan);

void *sd_ipcc_alloc_send_buffer(sd_ipcc_chan_t *chan, u32 *len, u16 *idx);

int sd_ipcc_chan_send_data(sd_ipcc_chan_t *chan, u8 *dat, u32 len);

int sd_ipcc_chan_send_data_nocopy(sd_ipcc_chan_t *chan, u8 *dat, u32 len, u16 mb_buf_idx);

void *sd_ipcc_chan_recv_data(sd_ipcc_chan_t *chan, u32 *len, u16 *idx);

inline static int sd_ipcc_chan_free_buffer(sd_ipcc_chan_t *chan, void *buffer)
{
    return sd_rpbuf_free_ptr(chan->buf_pool, buffer);
}

inline static int sd_ipcc_chan_free_buffer_id(sd_ipcc_chan_t *chan, unsigned int buf_id)
{
    return sd_rpbuf_free_id(chan->buf_pool, buf_id);
}

inline static sd_rpbuf_t *sd_ipcc_find_rpbuf(sd_ipcc_chan_t *chan, void *buffer)
{
    return sd_rpbuf_find_handle(chan->buf_pool, buffer);
}

inline static int sd_ipcc_chan_get_mtu(sd_ipcc_chan_t *chan, void* buf)
{
    /* subtract 16 bytes of rpmsg hdr */
    return IPCC_MB_MTU;
}

#endif //__IPCC_MB_H__
