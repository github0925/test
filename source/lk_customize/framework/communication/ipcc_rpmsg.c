/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * Copyright (c) 2019 Semidrive Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <list.h>
#include <assert.h>
#include <kernel/mutex.h>
#include "ipcc_rpmsg.h"
#include "ipcc_internal.h"
#include "sd_ipcc.h"

DCF_PACKED_BEGIN
/*!
 * Common header for all rpmsg messages.
 * Every message sent/received on the rpmsg bus begins with this header.
 */
struct rpmsg_msg_hdr
{
    uint32_t src;      /*!< source endpoint address */
    uint32_t dst;      /*!< destination endpoint address */
    uint32_t reserved; /*!< reserved for future use */
    uint16_t len;     /*!< length of payload (in bytes) */
    uint16_t flags;   /*!< message flags */
} DCF_PACKED_END;

DCF_PACKED_BEGIN
/*!
 * Common message structure.
 * Contains the header and the payload.
 */
struct rpmsg_ipcc_msg
{
    struct rpmsg_msg_hdr hdr; /*!< RPMsg message header */
    uint8_t data[1];    /*!< bytes of message payload data */
} DCF_PACKED_END;

/* rpmsg_msg_hdr contains a reserved field,
 * this implementation of RPMSG uses this reserved
 * field to hold the idx and totlen of the buffer
 * not being returned to the vring in the receive
 * callback function. This way, the no-copy API
 * can use this field to return the buffer later.
 */
struct rpmsg_hdr_reserved
{
    uint16_t rfu; /* reserved for future usage */
    uint16_t idx;
};

#define CONFIG_ALLOC_TX_BUFFER (1)

/* Interface which is used to interact with the sd_ipcc_chan layer,
 * a different interface is used, when the local processor is the MASTER
 * and when it is the REMOTE.
 */
struct dcf_ipcc_ops
{
    int (*send_data)(sd_ipcc_chan_t *chan, void *buffer, uint32_t len, uint16_t idx);
    int (*send_data_nocopy)(sd_ipcc_chan_t *chan, void *buffer, uint32_t len, uint16_t idx);
    void *(*send_alloc)(sd_ipcc_chan_t *chan, uint32_t *len, uint16_t *idx);
    void *(*recv_data)(sd_ipcc_chan_t *chan, uint32_t *len, uint16_t *idx);
    int (*recv_free)(sd_ipcc_chan_t *chan, void *buffer, uint32_t len, uint16_t idx);
};

/* Zero-Copy extension macros */
#define RPMSG_STD_MSG_FROM_BUF(buf) (struct rpmsg_ipcc_msg *)((char *)buf - offsetof(struct rpmsg_ipcc_msg, data))

/*!
 * @brief
 * Create a new rpmsg endpoint, which can be used
 * for communication.
 *
 * @param rpmsg_dcf_dev    RPMsg Lite instance
 * @param addr              Local endpoint address
 *
 * @return       NULL if not found, node pointer containing the ept on success
 *
 */
static struct rpmsg_dcf_endpoint *
rpmsg_dcf_get_endpoint_from_addr(struct rpmsg_dcf_instance *rpmsg_dcf_dev, unsigned long addr)
{
    struct rpmsg_dcf_endpoint *rl_ept;

    list_for_every_entry(&rpmsg_dcf_dev->endpoints, rl_ept,
                         struct rpmsg_dcf_endpoint, node) {
        if (rl_ept->addr == addr)
        {
            return rl_ept;
        }
    }
    return NULL;
}

/***************************************************************
   mmm    mm   m      m      mmmmm    mm     mmm  m    m  mmmm
 m"   "   ##   #      #      #    #   ##   m"   " #  m"  #"   "
 #       #  #  #      #      #mmmm"  #  #  #      #m#    "#mmm
 #       #mm#  #      #      #    #  #mm#  #      #  #m      "#
  "mmm" #    # #mmmmm #mmmmm #mmmm" #    #  "mmm" #   "m "mmm#"
****************************************************************/

/*!
 * @brief
 * Called when remote side calls sd_ipcc_chan_kick()
 * at its transmit sd_ipcc_chan.
 * In this callback, the buffer is read-out
 * of the rvq and user callback is called.
 *
 * @param vq  Virtqueue affected by the kick
 *
 */
static void rpmsg_dcf_rx_callback(sd_ipcc_chan_t *vq, u8* mssg, uint32_t len)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    struct rpmsg_dcf_endpoint *ept;
    int cb_ret;
    struct rpmsg_hdr_reserved *rsvd;
    struct rpmsg_dcf_instance *rpmsg_dcf_dev = (struct rpmsg_dcf_instance *) vq->priv;

    ASSERT(rpmsg_dcf_dev != NULL);

    /* Process the received data from remote node */
    rpmsg_msg = (struct rpmsg_ipcc_msg *) mssg;

#if LK_DEBUGLEVEL
    printf("new data rp%d:%d->%d %d bytes\n", vq->rproc,
                rpmsg_msg->hdr.src, rpmsg_msg->hdr.dst, rpmsg_msg->hdr.len);
#endif

    if (rpmsg_msg)
    {
        ept = rpmsg_dcf_get_endpoint_from_addr(rpmsg_dcf_dev, rpmsg_msg->hdr.dst);

        cb_ret = DCF_RELEASE;
        if (ept != NULL)
        {
            cb_ret = ept->rx_cb(rpmsg_msg->data, rpmsg_msg->hdr.len, rpmsg_msg->hdr.src, ept->rx_cb_data);
        } else {
            dprintf(0, "%s: %d->%d endpoint not found\n", __func__, rpmsg_msg->hdr.src, rpmsg_msg->hdr.dst);
        }

        if (cb_ret == DCF_HOLD)
        {
            rsvd = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;
            rsvd->idx = 0;
        }
        else
        {
            rpmsg_dcf_dev->chan_ops->recv_free(rpmsg_dcf_dev->tvq, rpmsg_msg, len, 0);
        }
    }
//    printf("%s: exit\n", __func__);
}

/*!
 * @brief
 * Called when remote side calls sd_ipcc_chan_kick()
 * at its receive sd_ipcc_chan.
 *
 * @param vq  Virtqueue affected by the kick
 *
 */
static void rpmsg_dcf_tx_callback(sd_ipcc_chan_t *vq)
{
    struct rpmsg_dcf_instance *rpmsg_dcf_dev = (struct rpmsg_dcf_instance *)vq->priv;

    ASSERT(rpmsg_dcf_dev != NULL);
    rpmsg_dcf_dev->link_state = 1;
}

/****************************************************************************

 m    m  mmmm         m    m   mm   mm   m mmmm   m      mmmmm  mm   m   mmm
 "m  m" m"  "m        #    #   ##   #"m  # #   "m #        #    #"m  # m"   "
  #  #  #    #        #mmmm#  #  #  # #m # #    # #        #    # #m # #   mm
  "mm"  #    #        #    #  #mm#  #  # # #    # #        #    #  # # #    #
   ##    #mm#"        #    # #    # #   ## #mmm"  #mmmmm mm#mm  #   ##  "mmm"
            #
 In case this processor has the MASTER role
*****************************************************************************/

/*!
 * @brief
 * Places buffer on the sd_ipcc_chan for consumption by the other side.
 *
 * @param vq      Virtqueue to use
 * @param buffer  Buffer pointer
 * @param len     Buffer length
 * @idx           Buffer index
 *
 * @return Status of function execution
 *
 */
static int ipcc_send_data(sd_ipcc_chan_t *tvq, void *buffer, uint32_t len, uint16_t idx)
{
    int status;
    status = sd_ipcc_chan_send_data(tvq, buffer, len);

#if CONFIG_ALLOC_TX_BUFFER
    if (idx != (uint16_t)-1)
        sd_ipcc_chan_free_buffer_id(tvq, idx);
#endif
    return status;
}

/*!
 * @brief
 * Places buffer on the sd_ipcc_chan for consumption by the other side.
 *
 * @param vq      Virtqueue to use
 * @param buffer  Buffer pointer
 * @param len     Buffer length
 * @idx           Buffer index
 *
 * @return Status of function execution
 *
 */
static int ipcc_send_data_nocopy(sd_ipcc_chan_t *tvq, void *buffer, uint32_t len, uint16_t idx)
{
    int status;
    status = sd_ipcc_chan_send_data_nocopy(tvq, buffer, len, idx);

    /* As long as the length of the sd_ipcc_chan ring buffer is not shorter
     * than the number of buffers in the pool, this function should not fail.
     * This condition is always met, so we don't need to return anything here */
    return status;
}

/*!
 * @brief
 * Provides buffer to transmit messages.
 *
 * @param vq      Virtqueue to use
 * @param len     Length of returned buffer
 * @param idx     Buffer index
 *
 * return Pointer to buffer.
 */
static void *send_alloc_buffer(sd_ipcc_chan_t *chan, uint32_t *len, uint16_t *idx)
{
    return sd_ipcc_alloc_send_buffer(chan, len, idx);
}

/*!
 * @brief
 * Retrieves the received buffer from the sd_ipcc_chan.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 * @return  Pointer to received buffer
 *
 */
static void *ipcc_rx_data(sd_ipcc_chan_t *chan, uint32_t *len, uint16_t *idx)
{
    return sd_ipcc_chan_recv_data(chan, len, idx);
}

/*!
 * @brief
 * Places the used buffer back on the sd_ipcc_chan.
 *
 * @param vq   Virtqueue to use
 * @param len  Size of received buffer
 * @param idx  Index of buffer
 *
 */
static int recv_free_buffer(sd_ipcc_chan_t *chan, void *buffer, uint32_t len, uint16_t idx)
{
    return sd_ipcc_chan_free_buffer(chan, buffer);
}

/* Interface used in case this processor is MASTER */
static const struct dcf_ipcc_ops master_chan_ops = {
    ipcc_send_data, ipcc_send_data_nocopy, send_alloc_buffer, ipcc_rx_data, recv_free_buffer,
};

/*************************************************

 mmmmmm mmmmm mmmmmmm        mm   m mmmmmmm     m
 #      #   "#   #           #"m  # #     #  #  #
 #mmmmm #mmm#"   #           # #m # #mmmmm" #"# #
 #      #        #           #  # # #      ## ##"
 #mmmmm #        #           #   ## #mmmmm #   #

**************************************************/
struct rpmsg_dcf_endpoint *rpmsg_dcf_create_ept(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                                                  unsigned long addr,
                                                  rl_ept_rx_cb_t rx_cb,
                                                  void *rx_cb_data)
{
    struct rpmsg_dcf_endpoint *rl_ept;
    unsigned int i;

    if (!rpmsg_dcf_dev)
    {
        return NULL;
    }

    mutex_acquire(&rpmsg_dcf_dev->lock);
    {
        if (addr == DCF_ADDR_ANY)
        {
            /* find lowest free address */
            for (i = 1; i < 0xFFFFFFFF; i++)
            {
                if (rpmsg_dcf_get_endpoint_from_addr(rpmsg_dcf_dev, i) == NULL)
                {
                    addr = i;
                    break;
                }
            }
            if (addr == DCF_ADDR_ANY)
            {
                /* no address is free, cannot happen normally */
                mutex_release(&rpmsg_dcf_dev->lock);
                return NULL;
            }
        }
        else
        {
            if (rpmsg_dcf_get_endpoint_from_addr(rpmsg_dcf_dev, addr) != NULL)
            {
                /* Already exists! */
                mutex_release(&rpmsg_dcf_dev->lock);
                return NULL;
            }
        }

        rl_ept = malloc(sizeof(struct rpmsg_dcf_endpoint));
        if (!rl_ept)
        {
            mutex_release(&rpmsg_dcf_dev->lock);
            return NULL;
        }

        memset(rl_ept, 0x00, sizeof(struct rpmsg_dcf_endpoint));

        rl_ept->addr = addr;
        rl_ept->rx_cb = rx_cb;
        rl_ept->rx_cb_data = rx_cb_data;

        list_add_tail(&rpmsg_dcf_dev->endpoints, &rl_ept->node);
    }
    mutex_release(&rpmsg_dcf_dev->lock);

    return rl_ept;
}
/*************************************************

 mmmmmm mmmmm mmmmmmm        mmmm   mmmmmm m
 #      #   "#   #           #   "m #      #
 #mmmmm #mmm#"   #           #    # #mmmmm #
 #      #        #           #    # #      #
 #mmmmm #        #           #mmm"  #mmmmm #mmmmm

**************************************************/

int rpmsg_dcf_destroy_ept(struct rpmsg_dcf_instance *rpmsg_dcf_dev, struct rpmsg_dcf_endpoint *rl_ept)
{
    if (!rpmsg_dcf_dev)
    {
        return DCF_ERR_PARAM;
    }

    if (!rl_ept)
    {
        return DCF_ERR_PARAM;
    }

    mutex_acquire(&rpmsg_dcf_dev->lock);

    list_delete(&rl_ept->node);

    mutex_release(&rpmsg_dcf_dev->lock);

    free(rl_ept);
    return NO_ERROR;
}

/******************************************

mmmmmmm m    m          mm   mmmmm  mmmmm
   #     #  #           ##   #   "#   #
   #      ##           #  #  #mmm#"   #
   #     m""m          #mm#  #        #
   #    m"  "m        #    # #      mm#mm

*******************************************/

int rpmsg_dcf_is_link_up(struct rpmsg_dcf_instance *rpmsg_dcf_dev)
{
    if (!rpmsg_dcf_dev)
    {
        return 0;
    }

    return rpmsg_dcf_dev->link_state;
}

/*!
 * @brief
 * Internal function to format a RPMsg compatible
 * message and sends it
 *
 * @param rpmsg_dcf_dev    RPMsg Lite instance
 * @param src               Local endpoint address
 * @param dst               Remote endpoint address
 * @param data              Payload buffer
 * @param size              Size of payload, in bytes
 * @param flags             Value of flags field
 * @param timeout           Timeout in ms, 0 if nonblocking
 *
 * @return  Status of function execution, NO_ERROR on success
 *
 */
int rpmsg_dcf_format_message(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                              unsigned long src,
                              unsigned long dst,
                              char *data,
                              unsigned long size,
                              int flags,
                              unsigned long timeout)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    int status = 0;
#if CONFIG_ALLOC_TX_BUFFER
    void *buffer;
    uint16_t idx;
    unsigned long tick_count = 0;
    uint32_t buff_len = size + sizeof(struct rpmsg_msg_hdr);
#else
    uint8_t buffer[DCF_BUFFER_PAYLOAD_SIZE];
    uint16_t idx = (uint16_t)-1;
    uint32_t buff_len = size + sizeof(struct rpmsg_msg_hdr);
#endif

    if (!rpmsg_dcf_dev)
    {
        return DCF_ERR_PARAM;
    }

    if (!data)
    {
        return DCF_ERR_PARAM;
    }

    if (!rpmsg_dcf_dev->link_state)
    {
        return DCF_NOT_READY;
    }

#if CONFIG_ALLOC_TX_BUFFER
    /* Lock the device to enable exclusive access to sd_ipcc_chans */
    mutex_acquire(&rpmsg_dcf_dev->lock);
    /* Get rpmsg buffer for sending message. */
    buffer = rpmsg_dcf_dev->chan_ops->send_alloc(rpmsg_dcf_dev->tvq, &buff_len, &idx);
    mutex_release(&rpmsg_dcf_dev->lock);

    if (!buffer && !timeout)
    {
        return DCF_ERR_NO_MEM;
    }

    while (!buffer)
    {
        thread_sleep(DCF_MS_PER_INTERVAL);
        mutex_acquire(&rpmsg_dcf_dev->lock);
        buffer = rpmsg_dcf_dev->chan_ops->send_alloc(rpmsg_dcf_dev->tvq, &buff_len, &idx);
        mutex_release(&rpmsg_dcf_dev->lock);
        tick_count += DCF_MS_PER_INTERVAL;
        if ((tick_count >= timeout) && (!buffer))
        {
            return DCF_ERR_NO_MEM;
        }
    }
#else
    if (size > DCF_BUFFER_PAYLOAD_SIZE) {
        size = DCF_BUFFER_PAYLOAD_SIZE;
        buff_len = DCF_BUFFER_PAYLOAD_SIZE + sizeof(struct rpmsg_msg_hdr);
    }
#endif

    dprintf(2, "%s send data buff_len: %d\n", __func__, buff_len);
    rpmsg_msg = (struct rpmsg_ipcc_msg *)buffer;

    /* Initialize RPMSG header. */
    rpmsg_msg->hdr.dst = dst;
    rpmsg_msg->hdr.src = src;
    rpmsg_msg->hdr.len = size;
    rpmsg_msg->hdr.flags = flags;

    /* Copy data to rpmsg buffer. */
    memcpy(rpmsg_msg->data, data, size);

    mutex_acquire(&rpmsg_dcf_dev->lock);
    /* Enqueue buffer on sd_ipcc_chan. */
    status = rpmsg_dcf_dev->chan_ops->send_data(rpmsg_dcf_dev->tvq, buffer, buff_len, idx);

    mutex_release(&rpmsg_dcf_dev->lock);

    return status;
}

int rpmsg_dcf_send(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                    struct rpmsg_dcf_endpoint *ept,
                    unsigned long dst,
                    char *data,
                    unsigned long size,
                    unsigned long timeout)
{
    if (!ept)
    {
        return DCF_ERR_PARAM;
    }

    // FIXME : may be just copy the data size equal to buffer length and Tx it.
    if (size > DCF_BUFFER_PAYLOAD_SIZE)
    {
        return DCF_ERR_BUFF_SIZE;
    }

    return rpmsg_dcf_format_message(rpmsg_dcf_dev, ept->addr, dst, data, size, DCF_NO_FLAGS, timeout);
}

void *rpmsg_dcf_alloc_tx_buffer(struct rpmsg_dcf_instance *rpmsg_dcf_dev, uint32_t *size, unsigned long timeout)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    struct rpmsg_hdr_reserved *reserved = NULL;
    void *buffer;
    uint16_t idx;
    unsigned int tick_count = 0;
    uint32_t real_size;

    if (!size)
        return NULL;

    if (!rpmsg_dcf_dev->link_state)
    {
        *size = 0;
        return NULL;
    }

    real_size = *size + sizeof(struct rpmsg_msg_hdr);

    /* Lock the device to enable exclusive access to sd_ipcc_chans */
    mutex_acquire(&rpmsg_dcf_dev->lock);
    /* Get rpmsg buffer for sending message. */
    buffer = rpmsg_dcf_dev->chan_ops->send_alloc(rpmsg_dcf_dev->tvq, &real_size, &idx);
    mutex_release(&rpmsg_dcf_dev->lock);

    if (!buffer && !timeout)
    {
        *size = 0;
        return NULL;
    }

    while (!buffer)
    {
        thread_sleep(DCF_MS_PER_INTERVAL);
        mutex_acquire(&rpmsg_dcf_dev->lock);
        buffer = rpmsg_dcf_dev->chan_ops->send_alloc(rpmsg_dcf_dev->tvq, &real_size, &idx);
        mutex_release(&rpmsg_dcf_dev->lock);
        tick_count += DCF_MS_PER_INTERVAL;
        if ((tick_count >= timeout) && (!buffer))
        {
            *size = 0;
            return NULL;
        }
    }

    rpmsg_msg = (struct rpmsg_ipcc_msg *)buffer;

    /* keep idx and totlen information for nocopy tx function */
    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;
    reserved->idx = idx;

    /* return the maximum payload size */
    *size = real_size - sizeof(struct rpmsg_msg_hdr);

    return rpmsg_msg->data;
}

int rpmsg_dcf_send_nocopy(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                           struct rpmsg_dcf_endpoint *ept,
                           unsigned long dst,
                           void *data,
                           unsigned long size)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    unsigned long src;
    struct rpmsg_hdr_reserved *reserved = NULL;

    if (!ept || !data)
    {
        return DCF_ERR_PARAM;
    }

    if (size > DCF_BUFFER_PAYLOAD_SIZE)
    {
        return DCF_ERR_BUFF_SIZE;
    }

    if (!rpmsg_dcf_dev->link_state)
    {
        return DCF_NOT_READY;
    }

    src = ept->addr;
    rpmsg_msg = RPMSG_STD_MSG_FROM_BUF(data);

    /* Initialize RPMSG header. */
    rpmsg_msg->hdr.dst = dst;
    rpmsg_msg->hdr.src = src;
    rpmsg_msg->hdr.len = size;
    rpmsg_msg->hdr.flags = DCF_NO_FLAGS;

    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;

    mutex_acquire(&rpmsg_dcf_dev->lock);
    /* Enqueue buffer on sd_ipcc_chan. */
    rpmsg_dcf_dev->chan_ops->send_data_nocopy(rpmsg_dcf_dev->tvq, (void *)rpmsg_msg,
                                  (unsigned long)(size + sizeof(struct rpmsg_msg_hdr)),
                                  reserved->idx);

    mutex_release(&rpmsg_dcf_dev->lock);

    return NO_ERROR;
}

/******************************************

 mmmmm  m    m          mm   mmmmm  mmmmm
 #   "#  #  #           ##   #   "#   #
 #mmmm"   ##           #  #  #mmm#"   #
 #   "m  m""m          #mm#  #        #
 #    " m"  "m        #    # #      mm#mm

 *******************************************/
sd_rpbuf_t *rpmsg_dcf_get_rpbuf(struct rpmsg_dcf_instance *rpmsg_dcf_dev, void *rxbuf)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;

    if (!rpmsg_dcf_dev)
    {
        return NULL;
    }
    if (!rxbuf)
    {
        return NULL;
    }

    rpmsg_msg = RPMSG_STD_MSG_FROM_BUF(rxbuf);

    return sd_ipcc_find_rpbuf(rpmsg_dcf_dev->tvq, rpmsg_msg);
}

static void *get_payload(sd_rpbuf_t *rpbuf)
{
    return rpbuf->buffer + offsetof(struct rpmsg_ipcc_msg, data);
}

void *rpmsg_dcf_import_buffer(struct rpmsg_dcf_instance *rpmsg_dev, sd_rpbuf_t *rpbuf, unsigned long *src, int *len)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    void *payload = NULL;

    if (rpbuf) {
        rpmsg_msg = (struct rpmsg_ipcc_msg *) rpbuf->buffer;
        payload = rpbuf->buffer + offsetof(struct rpmsg_ipcc_msg, data);
        if (src != NULL) {
            *src = rpmsg_msg->hdr.src;
        }
        if (len != NULL) {
            *len = rpmsg_msg->hdr.len;
        }
    }

    return payload;
}

static void release_rx_buffer(struct rpmsg_dcf_instance *rpmsg_dcf_dev, struct rpmsg_ipcc_msg *rpmsg_msg)
{
    struct rpmsg_hdr_reserved *reserved = NULL;

    /* Get the pointer to the reserved field that contains buffer size and the index */
    reserved = (struct rpmsg_hdr_reserved *)&rpmsg_msg->hdr.reserved;

    mutex_acquire(&rpmsg_dcf_dev->lock);

    /* Return used buffer, with total length (header length + buffer size). */
    rpmsg_dcf_dev->chan_ops->recv_free(rpmsg_dcf_dev->tvq, rpmsg_msg,
                                       (unsigned long)rpmsg_msg->hdr.len,
                                       reserved->idx);

    mutex_release(&rpmsg_dcf_dev->lock);
}

int rpmsg_dcf_copy_payload(struct rpmsg_dcf_instance *rpmsg_dcf_dev, sd_rpbuf_t *rpbuf, unsigned long *src, char *data, int *len)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;
    int ret = 0;

    if (!data || !len) {
        return DCF_ERR_PARAM;
    }

    if (rpbuf) {
        rpmsg_msg = (struct rpmsg_ipcc_msg *) rpbuf->buffer;

        if (*len >= rpmsg_msg->hdr.len) {
            if (src != NULL) {
                *src = rpmsg_msg->hdr.src;
            }

            *len = rpmsg_msg->hdr.len;
            memcpy(data, get_payload(rpbuf), *len);
        } else {
            ret = DCF_ERR_BUFF_SIZE;
        }

        /* Return used buffers. */
        release_rx_buffer(rpmsg_dcf_dev, rpmsg_msg);

        return ret;
    }

    return DCF_ERR_NO_BUFF;
}

int rpmsg_dcf_release_rpbuf(struct rpmsg_dcf_instance *rpmsg_dcf_dev, sd_rpbuf_t *rpbuf)
{
    if (!rpmsg_dcf_dev)
    {
        return DCF_ERR_PARAM;
    }
    if (!rpbuf)
    {
        return DCF_ERR_PARAM;
    }

    release_rx_buffer(rpmsg_dcf_dev, (struct rpmsg_ipcc_msg *) rpbuf->buffer);

    return NO_ERROR;
}

int rpmsg_dcf_release_rx_buffer(struct rpmsg_dcf_instance *rpmsg_dcf_dev, void *rxbuf)
{
    struct rpmsg_ipcc_msg *rpmsg_msg;

    if (!rpmsg_dcf_dev)
    {
        return DCF_ERR_PARAM;
    }
    if (!rxbuf)
    {
        return DCF_ERR_PARAM;
    }

    rpmsg_msg = RPMSG_STD_MSG_FROM_BUF(rxbuf);

    release_rx_buffer(rpmsg_dcf_dev, rpmsg_msg);

    return NO_ERROR;
}

static sd_ipcc_chan_t *alloc_notifier(int remote, char *name,
        int flags, ipcc_usr_cb usr_cb)
{
    return sd_ipcc_chan_create_mq(remote, name, IPCC_ADDR_DCF_BASE, flags,
                usr_cb, DCF_BUFFER_COUNT, DCF_BUFFER_PAYLOAD_SIZE);
}

/******************************

 mmmmm  mm   m mmmmm mmmmmmm
   #    #"m  #   #      #
   #    # #m #   #      #
   #    #  # #   #      #
 mm#mm  #   ## mm#mm    #

 *****************************/
struct rpmsg_dcf_instance *rpmsg_dcf_device_init(int link_id,
                                                   uint32_t init_flags)
{
    const char *vq_names;
    sd_ipcc_chan_t *chan;
    struct rpmsg_dcf_instance *rpmsg_dcf_dev = NULL;

    rpmsg_dcf_dev = malloc(sizeof(struct rpmsg_dcf_instance));
    if (!rpmsg_dcf_dev)
    {
        return NULL;
    }

    memset(rpmsg_dcf_dev, 0, sizeof(struct rpmsg_dcf_instance));

    /* Initialize names and callbacks*/
    vq_names = "rpmsg-ipcc";
    rpmsg_dcf_dev->chan_ops = &master_chan_ops;

    /* Create one sd_ipcc_chan for TX/RX */
    {
        chan = alloc_notifier(link_id, (char *)vq_names,
                    init_flags, rpmsg_dcf_rx_callback);
        if (chan)
        {
            /* Disable callbacks - will be enabled by the application
            * once initialization is completed.
            */
            //sd_ipcc_chan_disable_cb(chan);
        }
        else
        {
            free(rpmsg_dcf_dev);
            return NULL;
        }

        /* sd_ipcc_chan has reference to the RPMsg Lite instance */
       chan->priv = (void *)rpmsg_dcf_dev;
    }

    mutex_init(&rpmsg_dcf_dev->lock);
    list_initialize(&rpmsg_dcf_dev->endpoints);

    // FIXME - a better way to handle this , tx for master is rx for remote and vice versa.
    rpmsg_dcf_dev->tvq = chan;

    /* Install ISRs */
    rpmsg_dcf_dev->link_state = 1;

    /*
     * Let the remote device know that Master is ready for
     * communication.
     */
//    sd_ipcc_chan_kick(rpmsg_dcf_dev->tvq);

    return rpmsg_dcf_dev;
}

/*******************************************

 mmmm   mmmmmm mmmmm  mm   m mmmmm mmmmmmm
 #   "m #        #    #"m  #   #      #
 #    # #mmmmm   #    # #m #   #      #
 #    # #        #    #  # #   #      #
 #mmm"  #mmmmm mm#mm  #   ## mm#mm    #

********************************************/

int rpmsg_dcf_deinit(struct rpmsg_dcf_instance *rpmsg_dcf_dev)
{
    ASSERT(rpmsg_dcf_dev != NULL);
    ASSERT(rpmsg_dcf_dev->tvq != NULL);

    rpmsg_dcf_dev->link_state = 0;

    sd_ipcc_chan_free(rpmsg_dcf_dev->tvq);

    mutex_destroy(&rpmsg_dcf_dev->lock);

    free(rpmsg_dcf_dev);

    return NO_ERROR;
}
