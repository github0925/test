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

#ifndef _RPMSG_DCF_H
#define _RPMSG_DCF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <kernel/mutex.h>
#include <rpbuf.h>

#include "ipcc_config.h"

//! @addtogroup rpmsg_lite
//! @{

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DCF_VERSION "2.2.0" /*!< Current RPMsg DCF version */

/* Shared memory "allocator" parameters */
#define DCF_WORD_SIZE (sizeof(unsigned long))
#define DCF_WORD_ALIGN_UP(a)                                                                                  \
    (((((unsigned long)a) & (DCF_WORD_SIZE - 1)) != 0) ? ((((unsigned long)a) & (~(DCF_WORD_SIZE - 1))) + 4) : \
                                                        ((unsigned long)a))
#define DCF_WORD_ALIGN_DOWN(a)                                                                          \
    (((((unsigned long)a) & (DCF_WORD_SIZE - 1)) != 0) ? (((unsigned long)a) & (~(DCF_WORD_SIZE - 1))) : \
                                                        ((unsigned long)a))

/* Definitions for device types , null pointer, etc.*/
#define DCF_SUCCESS (0)
#define DCF_NULL ((void *)0)
#define DCF_REMOTE (0)
#define DCF_MASTER (1)
#define DCF_TRUE (1)
#define DCF_FALSE (0)
#define DCF_ADDR_ANY (0xFFFFFFFF)
#define DCF_RELEASE (0)
#define DCF_HOLD (1)
#define DCF_DONT_BLOCK (0)
#define DCF_BLOCK (0xFFFFFFFF)

/* Error macros. */
#define DCF_ERRORS_BASE (-5000)
#define DCF_ERR_NO_MEM (DCF_ERRORS_BASE - 1)
#define DCF_ERR_BUFF_SIZE (DCF_ERRORS_BASE - 2)
#define DCF_ERR_PARAM (DCF_ERRORS_BASE - 3)
#define DCF_ERR_DEV_ID (DCF_ERRORS_BASE - 4)
#define DCF_ERR_MAX_VQ (DCF_ERRORS_BASE - 5)
#define DCF_ERR_NO_BUFF (DCF_ERRORS_BASE - 6)
#define DCF_NOT_READY (DCF_ERRORS_BASE - 7)
#define DCF_ALREADY_DONE (DCF_ERRORS_BASE - 8)
#define DCF_ERR_BAD_SRVID (DCF_ERRORS_BASE - 9)
#define DCF_ERR_BAD_DATA (DCF_ERRORS_BASE - 10)


/* Init flags */
#define DCF_NO_FLAGS (0)

/*! \typedef rl_ept_rx_cb_t
    \brief Receive callback function type.
*/
typedef int (*rl_ept_rx_cb_t)(void *payload, int payload_len, unsigned long src, void *priv);

/*!
 * RPMsg Lite Endpoint structure
 */
struct rpmsg_dcf_endpoint
{
    unsigned long addr;   /*!< endpoint address */
    rl_ept_rx_cb_t rx_cb; /*!< ISR callback function */
    void *rx_cb_data;     /*!< ISR callback data */
    void *rfu;            /*!< reserved for future usage */
    struct list_node node;
    /* 16 bytes aligned on 32bit architecture */
};

/*!
 * Structure describing the local instance
 * of RPMSG communication stack and
 * holds all runtime variables needed internally
 * by the stack.
 */
struct rpmsg_dcf_instance
{
    struct sd_ipcc_chan *tvq;           /*!< transmit virtqueue */
    struct list_node endpoints;         /*!< linked list of endpoints */
    mutex_t lock;                         /*!< local mutex lock */
    unsigned int link_state;            /*!< state of the link, up/down*/

    struct dcf_ipcc_ops const *chan_ops; /*!< ops functions table pointer */
};

/*******************************************************************************
 * API
 ******************************************************************************/

/* Exported API functions */

/*!
 * @brief Initializes the RPMsg-DCF communication stack.
 * Must be called prior to any other RPMSG lite API.
 * To be called by the master side.
 *
 * @param shmem_addr       Shared memory base used for this instance of RPMsg-DCF
 * @param shmem_length     Length of memory area given by previous parameter
 * @param link_id          Link ID used to define the rpmsg-lite instance, see rpmsg_platform.h
 * @param init_flags       Initialization flags
 * @param env_cfg          Initialization data for the environement RPMsg-DCF layer
 * @param static_context   RPMsg-DCF preallocated context pointer, used in case of static api (DCF_USE_STATIC_API)
 *
 * @return  New RPMsg-DCF instance pointer or NULL.
 *
 */
struct rpmsg_dcf_instance *rpmsg_dcf_device_init(int link_id,
                                                   uint32_t init_flags);

/**
 * @brief Initializes the RPMsg-DCF communication stack.
 * Must be called prior to any other RPMsg-DCF API.
 * To be called by the remote side.
 *
 * @param shmem_addr       Shared memory base used for this instance of RPMsg-DCF
 * @param link_id          Link ID used to define the rpmsg-lite instance, see rpmsg_platform.h
 * @param init_flags       Initialization flags
 * @param env_cfg          Initialization data for the environement RPMsg-DCF layer
 * @param static_context   RPMsg-DCF preallocated context pointer, used in case of static api (DCF_USE_STATIC_API)
 *
 * @return  New RPMsg-DCF instance pointer or NULL.
 *
 */
struct rpmsg_dcf_instance *rpmsg_dcf_remote_init(void *shmem_addr, int link_id, uint32_t init_flags);

/*!
 *
 * @brief Deinitialized the RPMsg-DCF communication stack
 * This function always succeeds.
 * rpmsg_dcf_init() can be called again after this
 * function has been called.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 *
 * @return Status of function execution, DCF_SUCCESS on success.
 */
int rpmsg_dcf_deinit(struct rpmsg_dcf_instance *rpmsg_dcf_dev);

/*!
 * @brief Create a new rpmsg endpoint, which can be used
 * for communication.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 * @param addr              Desired address, DCF_ADDR_ANY for automatic selection
 * @param rx_cb             Callback function called on receive
 * @param rx_cb_data        Callback data pointer, passed to rx_cb
 * @param ept_context       Endpoint preallocated context pointer, used in case of static api (DCF_USE_STATIC_API)
 *
 * @return DCF_NULL on error, new endpoint pointer on success.
 *
 */
struct rpmsg_dcf_endpoint *rpmsg_dcf_create_ept(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                                                  unsigned long addr,
                                                  rl_ept_rx_cb_t rx_cb,
                                                  void *rx_cb_data);

/*!
 * @brief This function deletes rpmsg endpoint and performs cleanup.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 * @param rl_ept            Pointer to endpoint to destroy
 *
 */
int rpmsg_dcf_destroy_ept(struct rpmsg_dcf_instance *rpmsg_dcf_dev, struct rpmsg_dcf_endpoint *rl_ept);

/*!
 *
 * @brief Sends a message contained in data field of length size
 * to the remote endpoint with address dst.
 * ept->addr is used as source address in the rpmsg header
 * of the message being sent.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 * @param ept               Sender endpoint
 * @param dst               Remote endpoint address
 * @param data              Payload buffer
 * @param size              Size of payload, in bytes
 * @param timeout           Timeout in ms, 0 if nonblocking
 *
 * @return Status of function execution, DCF_SUCCESS on success.
 *
 */
int rpmsg_dcf_send(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                    struct rpmsg_dcf_endpoint *ept,
                    unsigned long dst,
                    char *data,
                    unsigned long size,
                    unsigned long timeout);

/*!
 * @brief Function to get the link state
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 *
 * @return True when link up, false when down.
 *
 */
int rpmsg_dcf_is_link_up(struct rpmsg_dcf_instance *rpmsg_dcf_dev);
void *rpmsg_dcf_import_buffer(struct rpmsg_dcf_instance *rpmsg_dev, sd_rpbuf_t *rpbuf, unsigned long *src, int *len);
int rpmsg_dcf_release_rpbuf(struct rpmsg_dcf_instance *rpmsg_dcf_dev, sd_rpbuf_t *rpbuf);
int rpmsg_dcf_copy_payload(struct rpmsg_dcf_instance *rpmsg_dcf_dev, sd_rpbuf_t *rpbuf, unsigned long *src, char *data, int *len);
sd_rpbuf_t *rpmsg_dcf_get_rpbuf(struct rpmsg_dcf_instance *rpmsg_dcf_dev, void *buffer);

/*!
 * @brief Releases the rx buffer for future reuse in vring.
 * This API can be called at process context when the
 * message in rx buffer is processed.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 * @param rxbuf             Rx buffer with message payload
 *
 * @return Status of function execution, DCF_SUCCESS on success.
 */
int rpmsg_dcf_release_rx_buffer(struct rpmsg_dcf_instance *rpmsg_dcf_dev, void *rxbuf);

/*!
 * @brief Allocates the tx buffer for message payload.
 *
 * This API can only be called at process context to get the tx buffer in vring. By this way, the
 * application can directly put its message into the vring tx buffer without copy from an application buffer.
 * It is the application responsibility to correctly fill the allocated tx buffer by data and passing correct
 * parameters to the rpmsg_dcf_send_nocopy() function to perform data no-copy-send mechanism.
 *
 * @param     rpmsg_dcf_dev    RPMsg-DCF instance
 * @param[in] size              Pointer to store maximum payload size available
 * @param[in] timeout           Integer, wait upto timeout ms or not for buffer to become available
 *
 * @return The tx buffer address on success and NULL on failure.
 *
 * @see rpmsg_dcf_send_nocopy
 */
void *rpmsg_dcf_alloc_tx_buffer(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                                 uint32_t *size,
                                 unsigned long timeout);

/*!
 * @brief Sends a message in tx buffer allocated by rpmsg_dcf_alloc_tx_buffer()
 *
 * This function sends txbuf of length len to the remote dst address,
 * and uses ept->addr as the source address.
 * The application has to take the responsibility for:
 *  1. tx buffer allocation (rpmsg_dcf_alloc_tx_buffer())
 *  2. filling the data to be sent into the pre-allocated tx buffer
 *  3. not exceeding the buffer size when filling the data
 *  4. data cache coherency
 *
 * After the rpmsg_dcf_send_nocopy() function is issued the tx buffer is no more owned
 * by the sending task and must not be touched anymore unless the rpmsg_dcf_send_nocopy()
 * function fails and returns an error.
 *
 * @param rpmsg_dcf_dev    RPMsg-DCF instance
 * @param[in] ept           Sender endpoint pointer
 * @param[in] dst           Destination address
 * @param[in] data          TX buffer with message filled
 * @param[in] size          Length of payload
 *
 * @return 0 on success and an appropriate error value on failure.
 *
 * @see rpmsg_dcf_alloc_tx_buffer
 */
int rpmsg_dcf_send_nocopy(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                           struct rpmsg_dcf_endpoint *ept,
                           unsigned long dst,
                           void *data,
                           unsigned long size);

//! @}

#if defined(__cplusplus)
}
#endif

#endif /* _RPMSG_DCF_H */
