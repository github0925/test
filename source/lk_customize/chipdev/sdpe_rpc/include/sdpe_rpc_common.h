/*
 * sdpe_rpc_common.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_COMMON_H_
#define _SDPE_RPC_COMMON_H_

#include <macros.h>

#ifndef WARN
#define WARN 1
#endif

#define MEM_BARRIER() __asm__ volatile("dsb" : : : "memory")

#ifndef SDPE_RPC_PACKED_BEGIN
#define SDPE_RPC_PACKED_BEGIN
#endif

#ifndef SDPE_RPC_PACKED_END
#define SDPE_RPC_PACKED_END __PACKED
#endif

#define SDPE_RPC_CREATE_MSGID(service, msg)     (service << 8) + msg
#define SDPE_RPC_GET_SERVICE_ID(id)             (uint8_t)(id >> 8)
#define SDPE_RPC_GET_MSG_ID(id)                 (uint8_t)(id & 0xff)

#ifdef SUPPORT_SDPE_RPC_DBUF
#define SDPE_RPC_GET_SHARE_BUF(structure)                   \
    sdpe_rpc_alloc_mbuf(sizeof(structure))
#else
#define SDPE_RPC_GET_SHARE_BUF(structure)                   \
({                                                          \
    static structure static_msg __SECTION(".sdpe_rpc_buf"); \
    (&static_msg);                                          \
})
#endif

#if defined (SUPPORT_SDPE_RPC_SINGLE_USER) || defined (SUPPORT_SDPE_RPC_DBUF)
#define SDPE_RPC_GET_MULTI_SHARE_BUF(structure, num, id)            \
    SDPE_RPC_INIT_MSG(structure)
#else
#define SDPE_RPC_GET_MULTI_SHARE_BUF(structure, num, id)            \
({                                                                  \
    static structure static_msg[num] __SECTION(".sdpe_rpc_buf");    \
    (&static_msg[id]);                                              \
})
#endif

#define SDPE_RPC_OFFSET_OF(structure, member)               \
    ((uintptr_t) &(((structure *) 0)->member))

#define SDPE_RPC_COPY_MSG_DATA(member, size, addr)          \
{                                                           \
    new_config->member = (void*)addr;                       \
    memcpy(new_config->member, config->member, size);       \
    addr += ALIGN(size, 4);                                 \
}

typedef int (*sdpe_rpc_usr_hdl)(void *arg, uint8_t *data, uint32_t len);

#endif
