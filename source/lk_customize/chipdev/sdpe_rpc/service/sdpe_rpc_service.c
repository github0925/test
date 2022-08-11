/*
 * sdpe_rpc_service.c
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

#include "sdpe_rpc_mbuf.h"
#include "sdpe_ctrl_service.h"
#include "vcan_service.h"
#include "vlin_service.h"
#include "eth_service.h"
#include "doip_service.h"

void sdpe_rpc_service_init(void)
{
    sdpe_rpc_mbuf_init();

    vcan_service_init();
    vlin_service_init();

    sdpe_ctrl_service_init();

#ifndef SUPPORT_SDPE_RPC_SERVER
    sdpe_rpc_sync();
#endif
}

