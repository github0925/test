/*
 * sdpe_rpc_doip.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_DOIP_H_
#define _SDPE_RPC_DOIP_H_

#include "sdpe_rpc_common.h"

#define SDPE_RPC_DOIP_PACKET_LEN     1480

#define SDPE_RPC_DOIP_GET_MSGID(id) \
    SDPE_RPC_CREATE_MSGID(SDPE_RPC_VDOIP_SERVICE, id)

enum vdoip_cb_types_e
{
    VDOIP_MSG_START = 0,
    VDOIP_DATA_REQ,
    VDOIP_DATA_CNF,
    VDOIP_DATA_SOM_IND,
    VDOIP_DATA_IND,
    VDOIP_MSG_END
};

typedef struct vdoip_instance {
    struct sdpe_rpc_dev priv;
} vdoip_instance_t;

SDPE_RPC_PACKED_BEGIN
struct vdoip_data_req_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_len;
    uint8_t t_data[0];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vdoip_data_cnf_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vdoip_data_som_ind_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_len;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vdoip_data_ind_s {
    struct sdpe_doip_addr t_addr;
    uint32_t t_result;
    uint32_t t_len;
    uint8_t t_data[0];
} SDPE_RPC_PACKED_END;

#endif
