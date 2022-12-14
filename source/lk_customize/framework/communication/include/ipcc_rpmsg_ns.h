/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#ifndef _RPMSG_MB_NS_H
#define _RPMSG_MB_NS_H

#include "ipcc_rpmsg.h"

//! @addtogroup rpmsg_ns
//! @{

#define DCF_NS_EPT_ADDR (0x35)

/* Up to 32 flags available */
enum rpmsg_dcf_ns_flags
{
    DCF_NS_CREATE = 0,
    DCF_NS_DESTROY = 1,
};

/*! \typedef rpmsg_dcf_ns_new_ept_cb
    \brief New endpoint NS callback function type.
*/
typedef void (*rpmsg_dcf_ns_new_ept_cb)(unsigned int new_ept,
                                    const char *new_ept_name,
                                    unsigned long flags,
                                    void *user_data);

struct rpmsg_dcf_ns_callback_data
{
    rpmsg_dcf_ns_new_ept_cb cb;
    void *user_data;
};

struct rpmsg_dcf_ns_context
{
    struct rpmsg_dcf_endpoint *ept;
    struct rpmsg_dcf_ns_callback_data *cb_ctxt;
};

typedef struct rpmsg_dcf_ns_context *rpmsg_dcf_ns_handle;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/* Exported API functions */

/*!
 * @brief Registers application nameservice callback
 *
 * @param rpmsg_dcf_dev    RPMsg-Lite instance
 * @param app_cb            Application nameservice callback
 * @param user_data         Application nameservice callback data
 *
 * @return NameService handle, to be kept for unbinding.
 *
 */
rpmsg_dcf_ns_handle rpmsg_dcf_ns_bind(struct rpmsg_dcf_instance *rpmsg_dcf_dev, rpmsg_dcf_ns_new_ept_cb app_cb, void *user_data);

/*!
 * @brief Unregisters application nameservice callback and cleans up
 *
 * @param rpmsg_dcf_dev    RPMsg-Lite instance
 * @param handle            NameService handle
 *
 * @return Status of function execution, DCF_SUCCESS on success.
 *
 */
int rpmsg_dcf_ns_unbind(struct rpmsg_dcf_instance *rpmsg_dcf_dev, rpmsg_dcf_ns_handle handle);

/*!
 * @brief Sends name service announcement to remote device
 *
 * @param rpmsg_dcf_dev    RPMsg-Lite instance
 * @param new_ept           New endpoint to announce
 * @param ept_name          Name for the announced endpoint
 * @param flags             Channel creation/deletion flags
 *
 * @return Status of function execution, DCF_SUCCESS on success
 *
 */
int rpmsg_dcf_ns_announce(struct rpmsg_dcf_instance *rpmsg_dcf_dev,
                      struct rpmsg_dcf_endpoint *new_ept,
                      const char *ept_name,
                      unsigned long flags);

//! @}

#if defined(__cplusplus)
}
#endif

#endif /* _RPMSG_MB_NS_H */
