/*
 * sdpe_rpc_eth.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_ETH_H_
#define _SDPE_RPC_ETH_H_

#include "sdpe_rpc_common.h"

#define SDPE_RPC_ETH_PACKET_LEN     1480

enum veth_cb_types_e
{
    VETH_MSG_START = 0,
    VETH_SEND_FRAME,
    VETH_MSG_END
};

typedef struct veth_instance {
    uint8_t service_id;
    struct sdpe_rpc_dev priv;
    veth_usr_cb usr_cb;
    void *arg;
} veth_instance_t;

#endif
