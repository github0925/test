/*
 * sdpe_rpc_cfg.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_RPC_CFG_H_
#define _SDPE_RPC_CFG_H_

#include "dcf_common.h"

#define SDPE_RPC_DEBUG              INFO

enum sdpe_rpc_service_e
{
    SDPE_RPC_SDPE_SERVICE = 0,
    SDPE_RPC_VCAN_SERVICE,
    SDPE_RPC_VLIN_SERVICE,
    SDPE_RPC_VETH1_SERVICE,
    SDPE_RPC_VETH2_SERVICE,
    SDPE_RPC_VDOIP_SERVICE,
    SDPE_RPC_MAX_SERVICE
};

#define SDPE_RPC_MP_SAF_ADDR_BASE   IPCC_ADDR_SDPE_RPC_BASE

#ifdef SUPPORT_SDPE_RPC_DBUF
#define SDPE_RPC_MBUF_ADDR          (SAF_SDPE_RPC_MEMBASE)
#define SDPE_RPC_MAX_MBUF_NUM       10
#endif

typedef struct sdpe_rpc_config {
    uint8_t     server_id;
    uint8_t     client_id;
    uint32_t    local_addr;
} sdpe_rpc_config_t;

static const struct sdpe_rpc_config g_sdpe_rpc_cfg_list[] = {
    [SDPE_RPC_SDPE_SERVICE] = {DP_CR5_MPC, DP_CR5_SAF, SDPE_RPC_MP_SAF_ADDR_BASE},
#ifdef SUPPORT_SDPE_RPC_MULTI_TASK
    [SDPE_RPC_VCAN_SERVICE] = {DP_CR5_MPC, DP_CR5_SAF, SDPE_RPC_MP_SAF_ADDR_BASE + 1},
    [SDPE_RPC_VLIN_SERVICE] = {DP_CR5_MPC, DP_CR5_SAF, SDPE_RPC_MP_SAF_ADDR_BASE + 2},
#endif

#if defined(SUPPORT_SDPE_RPC_AP1)
    [SDPE_RPC_VETH1_SERVICE] = {DP_CR5_MPC, DP_CA_AP1, 1024},
    [SDPE_RPC_VETH2_SERVICE] = {DP_CR5_MPC, DP_CA_AP1, 1025},
    [SDPE_RPC_VDOIP_SERVICE] = {DP_CR5_MPC, DP_CA_AP1, 1026},
#else
    [SDPE_RPC_VETH1_SERVICE] = {DP_CR5_MPC, DP_CA_AP2, 1024},
    [SDPE_RPC_VETH2_SERVICE] = {DP_CR5_MPC, DP_CA_AP2, 1025},
    [SDPE_RPC_VDOIP_SERVICE] = {DP_CR5_MPC, DP_CA_AP2, 1026},
#endif
};

#ifdef SUPPORT_SDPE_RPC_MBUF
typedef struct sdpe_rpc_mbuf_cfg {
    uint16_t size;
    uint16_t num;
} sdpe_rpc_mbuf_cfg_t;

typedef struct sdpe_rpc_mbuf_cfgs {
    uint32_t base;
    uint32_t buf_num;
    struct sdpe_rpc_mbuf_cfg cfg[SDPE_RPC_MAX_MBUF_NUM];
} sdpe_rpc_mbuf_cfgs_t;

static const struct sdpe_rpc_mbuf_cfgs g_sdpe_rpc_mbuf_cfg = {
    .base           = SDPE_RPC_MBUF_ADDR,
    .buf_num        = 1,
    .cfg[0].size    = 256,
    .cfg[0].num     = 100,
};
#endif

static const struct sdpe_rpc_config *sdpe_rpc_find_cfg(uint8_t service_id)
{
    if (service_id >= SDPE_RPC_MAX_SERVICE)
        return NULL;

    return &g_sdpe_rpc_cfg_list[service_id];
}

#endif
