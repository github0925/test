/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _RPC_H
#define _RPC_H

#include <sys/types.h>
#include <fw_reg.h>
#include <assert.h>

#define RPC_COUNT                   3
#define REG_MAX_COUNT               0x800
#define IOMUXC_SAF_REG_COUNT        156
#define CKGEN_SAF_REG_COUNT         6
#define RSTGEN_SAF_REG_COUNT        6
#define SCR_SAF_REG_COUNT           6
#define CKGEN_SEC_REG_COUNT         6
#define RSTGEN_SEC_REG_COUNT        4
#define IOMUXC_SEC_REG_COUNT        6
#define SCR_SEC_REG_COUNT           2
#define CKGEN_SOC_REG_COUNT         6
#define DOM_CFG_OPTION_MAX          4
#define REG_REMAP_SIZE              0x1000  //4B->4k

#define CKGEN_OFFSET                RPC_REGG0_DOM_(0)
#define SCR_OFFSET                  RPC_REGG2_DOM_(0)
#define RSTGEN_OFFSET               RPC_REGG4_DOM_(0)
#define IOMUXC_OFFSET               RPC_REGG5_DOM_(0)

typedef struct rpc_reg_dom_t {
    addr_t    base_addr;
    uint32_t  sel_lock;
    uint32_t  per_sel;
    uint32_t  did_lock;
    uint32_t  did;
} rpc_reg_dom_t;

typedef struct rpc_reg_group_t {
    addr_t    addr_offset;
    uint32_t  reg_num;
} rpc_reg_group_t;

typedef struct rpc_dom_per_info_t {
    uint32_t did;
    uint32_t cfg_num;
} rpc_dom_per_info_t;

#endif
