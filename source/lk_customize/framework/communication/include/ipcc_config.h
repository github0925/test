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

#ifndef _RPMSG_DCF_CONFIG_H
#define _RPMSG_DCF_CONFIG_H


/*!
 * @addtogroup config
 * @{
 * @file
 */

//! @name Configuration options
//@{

//! @def DCF_MS_PER_INTERVAL
//!
//! Delay in milliseconds used in non-blocking API functions for polling.
//! The default value is 1.
#ifndef DCF_MS_PER_INTERVAL
#define DCF_MS_PER_INTERVAL (1)
#endif

//! @def DCF_BUFFER_PAYLOAD_SIZE
//!
//! Size of the buffer payload, it depends on MBOX MTU limitation
#ifndef DCF_BUFFER_PAYLOAD_SIZE
#define DCF_BUFFER_PAYLOAD_SIZE (IPCC_MB_MTU)
#endif

//! @def DCF_BUFFER_COUNT
//!
//! Number of the buffers, it must be power of two (2, 4, ...).
//! The default value is 2.
#ifndef DCF_BUFFER_COUNT
#define DCF_BUFFER_COUNT (4)
#endif

//! @def DCF_CLEAR_USED_BUFFERS
//!
//! Clearing used buffers before returning back to the pool of free buffers 
//! enabled/disabled.
//! The default value is 0 (disabled).
#ifndef DCF_CLEAR_USED_BUFFERS
#define DCF_CLEAR_USED_BUFFERS (0)
#endif

//! @def DCF_USE_MCMGR_IPC_ISR_HANDLER
//!
//! When enabled IPC interrupts are managed by the Multicore Manager (IPC 
//! interrupts router), when disabled RPMsg-Lite manages IPC interrupts 
//! by itself.
//! The default value is 0 (no MCMGR IPC ISR handler used).
#ifndef DCF_USE_MCMGR_IPC_ISR_HANDLER
#define DCF_USE_MCMGR_IPC_ISR_HANDLER (0)
#endif

//! @def DCF_DEBUG_CHECK_BUFFERS
//!
//! Do not use in RPMsg-Lite to Linux configuration
#ifndef DCF_DEBUG_CHECK_BUFFERS
#define DCF_DEBUG_CHECK_BUFFERS (0)
#endif

//! @def DCF_ASSERT
//!
//! Assert implementation.
#ifndef DCF_ASSERT
#define DCF_ASSERT_BOOL(b)  ASSERT(b);
#define DCF_ASSERT(x) DCF_ASSERT_BOOL((x)!=0)
#endif

/* rpmsg device feature and compile options */
#ifndef CONFIG_RPMSG_STACK_SIZE
#define CONFIG_RPMSG_STACK_SIZE     (1024)
#endif

#ifndef CONFIG_ECHO_STACK_SIZE
#define CONFIG_ECHO_STACK_SIZE      (1024 + DCF_BUFFER_PAYLOAD_SIZE)
#endif

#ifndef CONFIG_CHAN_STACK_SIZE
#define CONFIG_CHAN_STACK_SIZE      (1024)
#endif

/* A echo test service rpmsg lite */
#ifndef CONFIG_RPMSG_ECHO
#define CONFIG_RPMSG_ECHO           (1)
#endif

/* A pseudo tty device  */
#ifndef CONFIG_RPMSG_TTY
#define CONFIG_RPMSG_TTY            (0)
#endif

/* global flag to enable ipcc-rpmsg */
#ifndef CONFIG_IPCC_RPMSG
#define CONFIG_IPCC_RPMSG           (0)
#endif

/* A echo test service ipcc-rpmsg */
#ifndef CONFIG_IPCC_ECHO_EPT
#define CONFIG_IPCC_ECHO_EPT        (1)
#endif

#ifndef CONFIG_USE_IPCC_TTY
#define CONFIG_USE_IPCC_TTY         (0)
#endif

/* A inter-processor rpc based on ipcc-rpmsg */
#ifndef CONFIG_USE_IPCC_RPC
#define CONFIG_USE_IPCC_RPC         (1)
#endif

#if (CONFIG_IPCC_RPMSG == 0)
/* RPC depends on IPCC RPMSG feature */
#undef CONFIG_USE_IPCC_RPC
#define CONFIG_USE_IPCC_RPC         (0)
#endif

#define CONFIG_LK_QUEUE_DUMP_HEX    (0)
//@}

/* this rpmsg channel config is consistent with Linux dts */
typedef union _rpmsg_cfg_extend {
    struct {
        u16 mbox_src;
        u16 mbox_dst;
    };
    uint32_t init_flags;
} rpmsg_cfg_extend_t;

typedef struct rpmsg_dev_config {
    u32         remote_proc;
    paddr_t     shm_phys_base;
    u32         shm_size;
    int         is_master; /* 1: master, 0: remote or -1: disabled */
    rpmsg_cfg_extend_t ext;
    bool        pa_spacex; /* memory phyaddr has R<->A domain translation */
} rpmsg_dev_config_t;

#endif /* _RPMSG_DCF_CONFIG_H */
