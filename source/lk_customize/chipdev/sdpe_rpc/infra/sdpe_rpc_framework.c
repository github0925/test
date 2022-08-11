/*
 * sdpe_rpc_framework.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sdpe_rpc_framework.h"
#include "sdpe_rpc_l2.h"
#include "sdpe_rpc_cbuf.h"

#define SDPE_RPC_STATUS_NULL        0
#define SDPE_RPC_STATUS_INIT        1
#define SDPE_RPC_STATUS_READY       2
#define SDPE_RPC_CHECK_STATUS(dev)  (dev->status == SDPE_RPC_STATUS_READY)

#define SDPE_RPC_MESSAGE_HEAD       sizeof(struct sdpe_rpc_message_head)
#define SDPE_RPC_WORKQ_NUM          64

typedef struct sdpe_rpc_cookie {
    event_t sem;
} sdpe_rpc_cookie_t;

static void sdpe_rpc_send_resp(struct sdpe_rpc_dev *dev,
                               struct sdpe_rpc_message_head *message)
{
    struct sdpe_rpc_message_head resp;

    resp.msg_type = SDPE_RPC_RSP;
    resp.cookie = message->cookie;
    resp.msg_id = message->msg_id;

    mutex_acquire(&dev->tx_lock);
    sdpe_rpc_transport_send(&dev->transport, (uint8_t *)&resp,
                            sizeof(struct sdpe_rpc_message_head),
                            SDPE_RPC_MAX_WAIT);
    mutex_release(&dev->tx_lock);
}

static int sdpe_rpc_dev_workq_hdl(struct sdpe_rpc_dev *dev, uint8_t *data, uint32_t len)
{
   struct sdpe_rpc_message_head *message;
   int ret = 0;

   ASSERT(dev != NULL && data != NULL);
   ASSERT(len >= SDPE_RPC_MESSAGE_HEAD);

   message = (struct sdpe_rpc_message_head *)data;

   if (SDPE_RPC_REQ == message->msg_type) {
       dev->usr_cb(dev->arg, message->msg_id,
                   (data + SDPE_RPC_MESSAGE_HEAD),
                   (len - SDPE_RPC_MESSAGE_HEAD));

       sdpe_rpc_send_resp(dev, message);
   }
   else {
       dev->usr_cb(dev->arg,  message->msg_id,
                   (data + SDPE_RPC_MESSAGE_HEAD),
                   (len - SDPE_RPC_MESSAGE_HEAD));
   }

   return ret;
}

static void sdpe_rpc_dev_workq_send(struct sdpe_rpc_dev *dev, uint8_t *data, uint32_t len)
{
   struct sdpe_rpc_message_head *message;
   int ret;

   message = (struct sdpe_rpc_message_head *)data;

   ret = sdpe_rpc_cbuf_write(&dev->workq, data, len);
   if (ret < 0 && message->msg_type == SDPE_RPC_REQ) {
       sdpe_rpc_send_resp(dev, message);
   }
}

static int sdpe_rpc_dev_workq_thread(void *arg)
{
   struct sdpe_rpc_dev *dev = (struct sdpe_rpc_dev *)arg;
   uint8_t *recv_buf;
   uint32_t data_len;

   recv_buf = malloc(dev->rx_buf_size);
   if (recv_buf == NULL) {
       dprintf(ALWAYS, "rpc workq buffer malloc fail\n");
       return -1;
   }

   while (1) {
       data_len = sdpe_rpc_cbuf_read(&dev->workq, recv_buf);
       if (data_len > 0) {
           sdpe_rpc_dev_workq_hdl(dev, recv_buf, data_len);
       }
   }

   free(recv_buf);
   return 0;
}

static int sdpe_rpc_dev_data_hdl(struct sdpe_rpc_dev *dev, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_message_head *message;
    int ret = 0;

    ASSERT(dev != NULL && data != NULL);
    ASSERT(len >= SDPE_RPC_MESSAGE_HEAD);

    message = (struct sdpe_rpc_message_head *)data;

    if (dev->support_workq && \
        message->msg_type != SDPE_RPC_RSP) {
        sdpe_rpc_dev_workq_send(dev, data, len);
        return ret;
    }

    if (SDPE_RPC_REQ == message->msg_type) {
        dev->usr_cb(dev->arg, message->msg_id,
                    (data + SDPE_RPC_MESSAGE_HEAD),
                    (len - SDPE_RPC_MESSAGE_HEAD));

        sdpe_rpc_send_resp(dev, message);
    }
    else if (SDPE_RPC_RSP == message->msg_type) {
        struct sdpe_rpc_cookie *cookie = (struct sdpe_rpc_cookie *)message->cookie;
        event_signal(&cookie->sem, false);
    }
    else {
        dev->usr_cb(dev->arg,  message->msg_id,
                    (data + SDPE_RPC_MESSAGE_HEAD),
                    (len - SDPE_RPC_MESSAGE_HEAD));
    }

    return ret;
}

static int sdpe_rpc_dev_rx_thread(void *arg)
{
    struct sdpe_rpc_dev *dev = (struct sdpe_rpc_dev *)arg;
    uint32_t data_len;
    int ret;

    ASSERT(dev != NULL);

    while (1) {
        data_len = dev->rx_buf_size;

        if (dev->support_l2) {
            ret = sdpe_rpc_l2_recv(&dev->rpc_l2, dev->rx_buf,
                                   &data_len);
        }
        else {
            ret = sdpe_rpc_transport_recv(&dev->transport, dev->rx_buf,
                                          &data_len, true);
        }

        if (ret < 0) {
            dprintf(ALWAYS, "sdpe rpc dev recv message fail\n");
            continue;
        }

        sdpe_rpc_dev_data_hdl(dev, dev->rx_buf, data_len);
    }

    return 0;
}

static int sdpe_rpc_dev_send_normal_message(struct sdpe_rpc_dev *dev, uint16_t msg_id,
                                     uint8_t msg_type, uint32_t wait_time,
                                     uint8_t *data, uint32_t data_len)
{
    struct sdpe_rpc_message_head *msg_head;
    struct sdpe_rpc_cookie cookie;
    uint32_t message_len;
    int ret = -1;

    message_len = SDPE_RPC_MESSAGE_HEAD + data_len;

    mutex_acquire(&dev->tx_lock);

    msg_head = sdpe_rpc_transport_tx_alloc(&dev->transport, &message_len, wait_time);
    if (!msg_head) {
        dprintf(ALWAYS, "transport tx alloc fail\n");
        goto err_out_with_lock;
    }

    msg_head->msg_id = msg_id;
    msg_head->msg_type = msg_type;

    if (data != NULL)
        memcpy(msg_head->data, data, data_len);

    if (msg_type == SDPE_RPC_REQ) {
        event_init(&cookie.sem, false, EVENT_FLAG_AUTOUNSIGNAL);
        msg_head->cookie = (uint32_t)(&cookie);
    }

    ret = sdpe_rpc_transport_send_nocopy(&dev->transport, (void *)msg_head, message_len);

    mutex_release(&dev->tx_lock);

    if (ret < 0) {
        dprintf(ALWAYS, "rpc message send fail:%d\n", ret);
        goto err_out_with_sem;
    }

    if (msg_type == SDPE_RPC_REQ) {
        while (event_wait(&cookie.sem) != 0);
    }

err_out_with_sem:
    if (msg_type == SDPE_RPC_REQ) {
        event_destroy(&cookie.sem);
    }
    return ret;

err_out_with_lock:
    mutex_release(&dev->tx_lock);
    return ret;
}

static int sdpe_rpc_dev_send_l2_message(struct sdpe_rpc_dev *dev, uint16_t msg_id,
                                        uint8_t msg_type, uint32_t wait_time,
                                        uint8_t sdu_num, va_list arg)
{
    struct sdpe_rpc_message_head head;

    head.msg_id = msg_id;
    head.msg_type = SDPE_RPC_IND;

    return sdpe_rpc_l2_send(&(dev)->rpc_l2, (uint8_t *)&head,
                            sizeof(head), sdu_num, arg);
}

int sdpe_rpc_dev_send_message(struct sdpe_rpc_dev *dev, uint16_t msg_id,
                              uint8_t msg_type, uint32_t wait_time, ...)
{
    va_list arg;
    int ret = -1;

    ASSERT(dev != NULL);

    if (!SDPE_RPC_CHECK_STATUS(dev)) {
        dprintf(WARN, "rpc dev not ready\n");
        return ret;
    }

    va_start(arg, wait_time);

    if (dev->support_l2) {
        uint32_t sdu_num = va_arg(arg, uint32_t);
        mutex_acquire(&dev->tx_lock);
        ret = sdpe_rpc_dev_send_l2_message(dev, msg_id, msg_type,
                                           wait_time, sdu_num, arg);
        mutex_release(&dev->tx_lock);
    }
    else {
        uint8_t *data = va_arg(arg, uint8_t *);
        uint32_t data_len = va_arg(arg, uint32_t);
        ret = sdpe_rpc_dev_send_normal_message(dev, msg_id, msg_type,
                                               wait_time, data, data_len);
    }

    va_end(arg);
    return ret;
}

int sdpe_rpc_dev_open(struct sdpe_rpc_dev *dev)
{
    char thread_name[64];
    int ret = -1;

    ASSERT(dev != NULL);

    ret = sdpe_rpc_transport_open(&dev->transport, dev->name);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc transport open fail:%s\n", dev->name);
        goto err_out;
    }

    dev->rx_buf = malloc(dev->rx_buf_size);
    if (dev->rx_buf == NULL) {
        dprintf(ALWAYS, "rpc transport malloc fail\n");
        goto err_out;
    }

    if (dev->support_l2) {
        sdpe_rpc_l2_init(&dev->rpc_l2, &dev->transport);
    }

    if (dev->support_workq) {
        sdpe_rpc_cbuf_init(&dev->workq, SDPE_RPC_WORKQ_NUM * dev->rx_buf_size);

        snprintf(thread_name, 64, "%s_RPCWQ", dev->name);
        dev->workq_thread = thread_create(thread_name, sdpe_rpc_dev_workq_thread,
                                          dev, DEFAULT_PRIORITY, 1024);
        if (!dev->workq_thread) {
            dprintf(ALWAYS, "create workq thread fail\n");
            goto err_out_with_mem;
        }
        thread_detach_and_resume(dev->workq_thread);
    }

    dev->status = SDPE_RPC_STATUS_READY;

    snprintf(thread_name, 64, "%s_RPC", dev->name);
    dev->rx_thread = thread_create(thread_name, sdpe_rpc_dev_rx_thread,
                                   dev, HIGH_PRIORITY, 1024);
    if (!dev->rx_thread) {
        dprintf(ALWAYS, "create rx thread fail\n");
        goto err_out_with_mem;
    }
    thread_detach_and_resume(dev->rx_thread);

    return 0;

err_out_with_mem:
    if (dev->support_workq) {
        sdpe_rpc_cbuf_deinit(&dev->workq);
    }
    free(dev->rx_buf);

err_out:
    return -1;
}

int sdpe_rpc_dev_close(struct sdpe_rpc_dev *dev)
{
    ASSERT(dev != NULL);

    dev->status = SDPE_RPC_STATUS_INIT;

    sdpe_rpc_transport_close(&dev->transport);

    if (dev->support_l2) {
        sdpe_rpc_l2_uninit(&dev->rpc_l2);
    }

    if (dev->support_workq) {
        sdpe_rpc_cbuf_deinit(&dev->workq);
    }

    mutex_destroy(&dev->tx_lock);
    free(dev->rx_buf);

    return 0;
}

int sdpe_rpc_dev_init(struct sdpe_rpc_dev *dev, const char *name,
                      uint8_t remote, uint32_t addr,
                      sdpe_rpc_usr_cb usr_cb, void *arg,
                      sdpe_rpc_transport_type transport,
                      uint32_t max_msg_len, bool support_workq,
                      bool support_l2)
{
    int ret = -1;

    ASSERT(dev != NULL);

    ret = sdpe_rpc_transport_init(&dev->transport, remote, addr, transport);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc transport init fail\n");
        return ret;
    }

    mutex_init(&dev->tx_lock);
    dev->usr_cb = usr_cb;
    dev->arg = arg;
    dev->rx_buf_size = max_msg_len + SDPE_RPC_MESSAGE_HEAD;
    dev->support_l2 = support_l2;
    dev->support_workq = support_workq;

    strcpy(dev->name, name);

    dev->status = SDPE_RPC_STATUS_INIT;

    return 0;
}
