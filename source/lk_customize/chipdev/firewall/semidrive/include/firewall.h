/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _FIREWALL_H
#define _FIREWALL_H

#include <stdio.h>
#include <res.h>

#include <__regs_base.h>
#include "rpc_reg.h"

#if WITH_VIRT_PLATFORM
#define SCR_BASE_ADDR                   0x42000000
#else
#define SCR_BASE_ADDR                   APB_SCR_SEC_BASE
#endif
#define SCR_PROD_ENABLE_RO_START_BIT    56
#define SCR_MFG_DISABLE_RO_START_BIT    57
#define SCR_FA_ENABLE_RO_START_BIT      58
#define SCR_RW_OFFSET                   0x0
#define SCR_RO_OFFSET                   0x100
#define SCR_L16_OFFSET                  0x200
#define SCR_L31_OFFSET                  0x400
#define SCR_R16W16_OFFSET               0x600

#define RPC_MAC_GLB_CTL_OFFSET          0x7fe000 //(8*1024*1024 - 8*1024)

typedef enum scr_reg_type {
    SCR_RW = 0,
    SCR_RO = 1,
    SCR_L16 = 2,
    SCR_L31 = 3,
    SCR_R16W16 = 4
} scr_reg_type_t;

#pragma pack(1)
typedef struct firewall_cfg {
    addr_t reg_addr;
    uint32_t reg_val;
    bool overwrite;
} firewall_cfg_t;
#pragma pack()

//for cfg check
typedef enum fw_ip_type {
    IP_MAC = 0,
    IP_MPC = 1,
    IP_PPC = 2,
    IP_RPC = 3
} fw_ip_type_t;

typedef enum rapc_type {
    RAPC_MPC = 0,
    RAPC_PPC = 1,
    RAPC_RPC = 2,
    RAPC_UNPROTECT = 3
} rapc_type_t;

typedef enum rpc_type {
    RPC_SOC = 0,
    RPC_SEC = 1,
    RPC_SAF = 2
} rpc_type_t;

typedef enum permission_type {
    ACCESS_DENY = 0,
    READ_ONLY = 1,
    WRITEL_ONLY = 2,
    SECURE_ACCESS = 4,
    NONSECURE_ACCESS = 8,
    PRIVILEGE_ACCESS = 16,
    USER_ACCESS = 32,
    INVALID_PERMISSION = 64
} permission_type_t;

int firewall_init(uint32_t cfg_count, firewall_cfg_t* fw_cfg, bool cfg_check);
int share_resource(uint32_t res_id, uint32_t dom_id, bool shared);
void firewall_enable(bool fw_enable);
int rid_init(uint32_t cfg_count, firewall_cfg_t * rid_cfg);
int permission_set(uint32_t res_id, uint32_t dom_id, permission_type_t permission, bool enable);
int permission_enable(uint32_t res_id, bool enable);
#endif
