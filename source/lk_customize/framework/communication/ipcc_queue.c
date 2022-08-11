/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 *
 */
#include <stdlib.h>
#include <string.h>
#include <lib/reg.h>
#include <debug.h>
#include <pow2.h>
#include "rpmsg_common.h"
#include "ipcc_rpmsg.h"
#include "ipcc_queue.h"

/* Be sure sizeof(ipcc_queue_msg_t) is pow2, now 16 bytes */
#define IPCC_QUEUE_PAYLOAD_SIZE         ((int)DCF_BUFFER_PAYLOAD_SIZE)
typedef struct
{
    uint32_t src;
    uint32_t len;
    uint64_t payload;
} ipcc_queue_msg_t;

typedef struct __lk_queue__ {
    cbuf_t cbuf;
    int element_size;
    int element_queued;
    void *user_context;
} lk_queue_t, *lk_queue_handle_t;

static lk_queue_handle_t lk_queue_create(int depth, int element_size)
{
    lk_queue_handle_t queue = NULL;
    size_t size;

    queue = malloc(sizeof(lk_queue_t));
    if (!queue) {
        return NULL;
    }

    dprintf(INFO, "%s: depth:%d element_size:%d\n",
            __func__, depth, element_size);

    ASSERT(ispow2(element_size));

    queue->element_size = element_size;
    size = depth * element_size;

    cbuf_initialize(&queue->cbuf, size);
    return queue;
}

void lk_queue_delete(lk_queue_handle_t queue)
{
    if (queue) {
        cbuf_deinitialize(&queue->cbuf);
        free(queue);
    }
}

int lk_queue_pop(lk_queue_handle_t queue, ipcc_queue_msg_t *msg, lk_time_t timeout)
{
    if (cbuf_read_timeout(&queue->cbuf, msg, queue->element_size, timeout)) {
        queue->element_queued--;

#if CONFIG_LK_QUEUE_DUMP_HEX
        dprintf(ALWAYS, "%s %p dump %d bytes msg:\n", __func__, queue, msg->len);
        hexdump((const void *)msg, msg->len);
#endif
        return 1;
    }

    return 0;
}

int lk_queue_push(lk_queue_handle_t queue, ipcc_queue_msg_t *msg, int timeout)
{
    queue->element_queued++;
    cbuf_write(&queue->cbuf, msg, queue->element_size, true);

#if CONFIG_LK_QUEUE_DUMP_HEX
    dprintf(INFO, "%s %p dump msg:\n", __func__, queue);
    hexdump((const void *)msg, sizeof(ipcc_queue_msg_t));
#endif

    return 1;
}

int lk_queue_get_current_size(lk_queue_handle_t queue)
{
    return queue->element_queued;
}

int ipcc_queue_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    ipcc_queue_msg_t msg;
    lk_queue_handle_t queue = priv;

    ASSERT(queue);

    msg.src = src;
    msg.payload = (vaddr_t) payload;
    msg.len = payload_len;

#if CONFIG_LK_QUEUE_DUMP_HEX
    dprintf(ALWAYS, "%s %p From: %lu %d bytes msg:\n", __func__, queue, src, payload_len);
    hexdump((const void *)&msg, payload_len);
#endif
    /* if message is successfully added into queue then hold rpmsg buffer */
    if (lk_queue_push(queue, &msg, 0))
    {
        /* hold the rx buffer */
        return DCF_HOLD;
    }

    return DCF_RELEASE;
}

ipcc_queue_handle_t ipcc_queue_create(uint32_t depth, void *user)
{
    /* create message queue for channel default endpoint */
    lk_queue_handle_t q = lk_queue_create(depth, sizeof(ipcc_queue_msg_t));

    if (q)
        q->user_context = user;

    return q;
}

int ipcc_queue_destroy(ipcc_queue_handle_t q)
{
    ASSERT(q);

    lk_queue_delete((lk_queue_handle_t) q);
    return 0;
}

int ipcc_queue_recv_timed(ipcc_queue_handle_t q, unsigned long *src, char *pdata, uint32_t maxlen, uint32_t *len, lk_time_t timeout)
{
    ipcc_queue_msg_t msg = {0};
    lk_queue_handle_t queue = q;
    void *payload = NULL;

    int retval = DCF_SUCCESS;

    if ((!q) || (!pdata)) {
        return DCF_ERR_PARAM;
    }

    /* Get an element out of the message queue for the selected endpoint */
    if (lk_queue_pop(q, &msg, timeout)) {
        if (maxlen >= msg.len) {
            if (src != NULL) {
                *src = msg.src;
            }
            if (len != NULL) {
                *len = msg.len;
            }

            payload = (void *)((uint8_t*)0 + msg.payload);
            memcpy(pdata, payload, msg.len);
        } else {
            retval = DCF_ERR_BUFF_SIZE;
        }

        /* Return used buffers. */
        rpmsg_dcf_release_rx_buffer(queue->user_context, payload);

        return retval;
    }

    return DCF_ERR_NO_BUFF; /* failed */
}

int ipcc_queue_recv(ipcc_queue_handle_t q, unsigned long *src, char *pdata, uint32_t maxlen, uint32_t *len)
{
    return ipcc_queue_recv_timed(q, src, pdata, maxlen, len, DCF_BLOCK);
}

int ipcc_queue_get_current_size(ipcc_queue_handle_t q)
{
    if (!q) {
        return DCF_ERR_PARAM;
    }

    /* Return actual queue size. */
    return lk_queue_get_current_size((lk_queue_handle_t)q);
}
