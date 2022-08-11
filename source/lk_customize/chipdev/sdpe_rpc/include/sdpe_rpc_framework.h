/*
 * sdpe_rpc_framework.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_FRAMEWORK_H
#define _SDPE_RPC_FRAMEWORK_H

#include <kernel/mutex.h>
#include <kernel/thread.h>

#include "sdpe_rpc_transport.h"
#include "sdpe_rpc_l2.h"
#include "sdpe_rpc_cbuf.h"

#define SDPE_RPC_NAME_LEN   32

/* sdpe rpc message type */

#define SDPE_RPC_REQ        0
#define SDPE_RPC_RSP        1
#define SDPE_RPC_IND        2

typedef int (*sdpe_rpc_usr_cb)(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len);

SDPE_RPC_PACKED_BEGIN
struct sdpe_rpc_message_head {
    uint16_t msg_id;
    uint16_t msg_type;
    uint32_t cookie;
    uint8_t data[0];
} SDPE_RPC_PACKED_END;

typedef struct sdpe_rpc_dev {
    struct sdpe_rpc_transport_dev transport;
    char name[SDPE_RPC_NAME_LEN];
    sdpe_rpc_usr_cb usr_cb;
    void *arg;
    mutex_t tx_lock;
    thread_t *rx_thread;
    uint8_t *rx_buf;
    uint32_t rx_buf_size;
    uint8_t status;
    uint8_t support_l2;
    uint8_t support_workq;
    struct sdpe_rpc_l2_instance rpc_l2;
    thread_t *workq_thread;
    sdpe_rpc_cbuf_t workq;
} sdpe_rpc_dev_t;

int sdpe_rpc_dev_init(struct sdpe_rpc_dev *dev, const char *name,
                      uint8_t remote, uint32_t addr,
                      sdpe_rpc_usr_cb usr_cb, void *arg,
                      sdpe_rpc_transport_type transport,
                      uint32_t max_msg_len, bool support_workq,
                      bool support_l2);
int sdpe_rpc_dev_open(struct sdpe_rpc_dev *dev);
int sdpe_rpc_dev_close(struct sdpe_rpc_dev *dev);
int sdpe_rpc_dev_send_message(struct sdpe_rpc_dev *dev, uint16_t msg_id,
                              uint8_t msg_type, uint32_t wait_time, ...);

#endif
