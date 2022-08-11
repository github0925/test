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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ipcc_rpmsg.h"
#include "ipcc_rpmsg_ns.h"
#include "ipcc_internal.h"

int rpmsg_dcf_format_message(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                              unsigned long src,
                              unsigned long dst,
                              char *data,
                              unsigned long size,
                              int flags,
                              unsigned long timeout);

#define DCF_NS_NAME_SIZE (32)

/*!
 * struct rpmsg_dcf_ns_msg - dynamic name service announcement message
 * @name: name of remote service that is published
 * @addr: address of remote service that is published
 * @flags: indicates whether service is created or destroyed
 *
 * This message is sent across to publish a new service, or announce
 * about its removal. When we receive these messages, an appropriate
 * rpmsg channel (i.e device) is created/destroyed. In turn, the ->probe()
 * or ->remove() handler of the appropriate rpmsg driver will be invoked
 * (if/as-soon-as one is registered).
 */
DCF_PACKED_BEGIN
struct rpmsg_dcf_ns_msg
{
    char name[DCF_NS_NAME_SIZE];
    uint32_t addr;
    uint32_t flags;
} DCF_PACKED_END;

/*!
 * @brief
 * Nameservice callback, called in interrupt context
 *
 * @param payload     Pointer to the buffer containing received data
 * @param payload_len Size of data received, in bytes
 * @param src         Pointer to address of the endpoint from which data is received
 * @param priv        Private data provided during endpoint creation
 *
 * @return  DCF_RELEASE, message is always freed
 *
 */
static int rpmsg_dcf_ns_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    struct rpmsg_dcf_ns_msg *ns_msg_ptr = payload;
    struct rpmsg_dcf_ns_callback_data *cb_ctxt = priv;
    ASSERT(priv);
    ASSERT(cb_ctxt->cb);

    /* Drop likely bad messages received at nameservice address */
    if (payload_len == sizeof(struct rpmsg_dcf_ns_msg))
    {
        cb_ctxt->cb(ns_msg_ptr->addr, ns_msg_ptr->name, ns_msg_ptr->flags, cb_ctxt->user_data);
    }

    return DCF_RELEASE;
}

rpmsg_dcf_ns_handle rpmsg_dcf_ns_bind(struct rpmsg_dcf_instance *rpmsg_dcf_dev, rpmsg_dcf_ns_new_ept_cb app_cb, void *user_data)
{
    struct rpmsg_dcf_ns_context *ns_ctxt;

    if (app_cb == NULL)
    {
        return NULL;
    }

    {
        struct rpmsg_dcf_ns_callback_data *cb_ctxt;

        cb_ctxt = malloc(sizeof(struct rpmsg_dcf_ns_callback_data));
        if (cb_ctxt == NULL)
        {
            return NULL;
        }
        ns_ctxt = malloc(sizeof(struct rpmsg_dcf_ns_context));
        if (ns_ctxt == NULL)
        {
            free(cb_ctxt);
            return NULL;
        }

        /* Set-up the nameservice callback context */
        cb_ctxt->user_data = user_data;
        cb_ctxt->cb = app_cb;

        ns_ctxt->cb_ctxt = cb_ctxt;

        ns_ctxt->ept = rpmsg_dcf_create_ept(rpmsg_dcf_dev, DCF_NS_EPT_ADDR, rpmsg_dcf_ns_rx_cb, (void *)ns_ctxt->cb_ctxt);
    }

    return (rpmsg_dcf_ns_handle)ns_ctxt;
}

int rpmsg_dcf_ns_unbind(struct rpmsg_dcf_instance *rpmsg_dcf_dev, rpmsg_dcf_ns_handle handle)
{
    struct rpmsg_dcf_ns_context *ns_ctxt = (struct rpmsg_dcf_ns_context *)handle;

    {
        int retval;

        retval = rpmsg_dcf_destroy_ept(rpmsg_dcf_dev, ns_ctxt->ept);
        free(ns_ctxt->cb_ctxt);
        free(ns_ctxt);
        return retval;
    }
}

int rpmsg_dcf_ns_announce(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                      struct rpmsg_dcf_endpoint *new_ept,
                      const char *ept_name,
                      unsigned long flags)
{
    struct rpmsg_dcf_ns_msg ns_msg;

    if (ept_name == NULL)
    {
        return DCF_ERR_PARAM;
    }

    if (new_ept == NULL)
    {
        return DCF_ERR_PARAM;
    }

    strncpy(ns_msg.name, ept_name, DCF_NS_NAME_SIZE);
    ns_msg.flags = flags;
    ns_msg.addr = new_ept->addr;

    return rpmsg_dcf_format_message(rpmsg_dcf_dev, new_ept->addr, DCF_NS_EPT_ADDR, (char *)&ns_msg,
                                     sizeof(struct rpmsg_dcf_ns_msg), DCF_NO_FLAGS, DCF_BLOCK);
}
