/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _RPC_REG_H
#define _RPC_REG_H

#include "__regs_ap_rpc.h"

//--------------------------------------------------------------------------
// RPC base address
// Description     :   every RPC has its base address
//--------------------------------------------------------------------------
#if WITH_VIRT_PLATFORM
#define RPC_BASE_ADDR_CKGEN_SOC         0x41800000
#define RPC_BASE_ADDR_SEC               0x42800000
#define RPC_BASE_ADDR_SAF               0x46800000
#else
#if ARM_CPU_CORTEX_R5
#define RPC_BASE_ADDR_CKGEN_SOC         0xf6800000
#define RPC_BASE_ADDR_SEC               0xf8800000
#define RPC_BASE_ADDR_SAF               0xfc800000
#else
#define RPC_BASE_ADDR_CKGEN_SOC         0x36800000
#define RPC_BASE_ADDR_SEC               0x38800000
#define RPC_BASE_ADDR_SAF               0x3c800000
#endif
#endif

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REG_DOM
// Register Offset : 0x0
// Description     :
// group 0/1 for CKGEN, group 2/3 for SCR, group 4 for RSTGEN, group 5/6/7 for IOMUXC
//--------------------------------------------------------------------------
#define ADDR_REMAP_TIMES                 0x400   //4B->4k
#define RPC_REG_DOM_JUMP                 0x4 * ADDR_REMAP_TIMES
#define RPC_REGG0_DOM_(i)                REG(REG_AP_APB_RPC_REGG0_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG1_DOM_(i)                REG(REG_AP_APB_RPC_REGG1_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG2_DOM_(i)                REG(REG_AP_APB_RPC_REGG2_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG3_DOM_(i)                REG(REG_AP_APB_RPC_REGG3_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG4_DOM_(i)                REG(REG_AP_APB_RPC_REGG4_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG5_DOM_(i)                REG(REG_AP_APB_RPC_REGG5_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG6_DOM_(i)                REG(REG_AP_APB_RPC_REGG6_DOM + (i) * RPC_REG_DOM_JUMP)
#define RPC_REGG7_DOM_(i)                REG(REG_AP_APB_RPC_REGG7_DOM + (i) * RPC_REG_DOM_JUMP)

#define RPC_REG_DOM_PER_SEL_LOCK_SHIFT   REGG0_DOM_PER_SEL_LOCK_FIELD_OFFSET
#define RPC_REG_DOM_PER_SEL_LOCK_MASK    1UL << RPC_REG_DOM_PER_SEL_LOCK_SHIFT
#define RPC_REG_DOM_PER_SEL_SHIFT    REGG0_DOM_PER_SEL_FIELD_OFFSET
#define RPC_REG_DOM_PER_SEL_MASK     3UL << RPC_REG_DOM_PER_SEL_SHIFT
#define RPC_REG_DOM_DID_LOCK_SHIFT   REGG0_DOM_DID_LOCK_FIELD_OFFSET
#define RPC_REG_DOM_DID_LOCK_MASK    1UL << RPC_REG_DOM_DID_LOCK_SHIFT
#define RPC_REG_DOM_DID_SHIFT            REGG0_DOM_DID_FIELD_OFFSET
#define RPC_REG_DOM_DID_MASK             0xF << RPC_REG_DOM_DID_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GLB_CTL
// Register Offset : 0xe000
// Description     :
//--------------------------------------------------------------------------
#define RPC_GLB_CTL                     REG(REG_AP_APB_RPC_GLB_CTL)

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_PER0
// Register Offset : 0xe400
// Description     :
//--------------------------------------------------------------------------
#define PRC_DOM_JUMP                    0x34
#define RPC_DOM_PER0_(i)                REG(REG_AP_APB_RPC_DOM_PER0) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_PER1
// Register Offset : 0xe404
// Description     :
//--------------------------------------------------------------------------
#define RPC_DOM_PER1_(i)                REG(REG_AP_APB_RPC_DOM_PER1)  + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER0
// Register Offset : 0xe408
// Description     :
//--------------------------------------------------------------------------
#define RPC_SEC_PER0_(i)                REG(REG_AP_APB_RPC_SEC_PER0) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER1
// Register Offset : 0xe40c
// Description     :
//--------------------------------------------------------------------------
#define RPC_SEC_PER1_(i)                REG(REG_AP_APB_RPC_SEC_PER1) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER2
// Register Offset : 0xe410
// Description     :
//--------------------------------------------------------------------------
#define RPC_SEC_PER2_(i)                REG(REG_AP_APB_RPC_SEC_PER2) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER3
// Register Offset : 0xe414
// Description     :
//--------------------------------------------------------------------------
#define RPC_SEC_PER3_(i)                REG(REG_AP_APB_RPC_SEC_PER3) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER0
// Register Offset : 0xe418
// Description     :
//--------------------------------------------------------------------------
#define RPC_PRI_PER0_(i)                REG(REG_AP_APB_RPC_PRI_PER0) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER1
// Register Offset : 0xe41c
// Description     :
//--------------------------------------------------------------------------
#define RPC_PRI_PER1_(i)                REG(REG_AP_APB_RPC_PRI_PER1) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER2
// Register Offset : 0xe420
// Description     :
//--------------------------------------------------------------------------
#define RPC_PRI_PER2_(i)                REG(REG_AP_APB_RPC_PRI_PER2) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER3
// Register Offset : 0xe424
// Description     :
//--------------------------------------------------------------------------
#define RPC_PRI_PER3_(i)                REG(REG_AP_APB_RPC_PRI_PER3) + (i) * PRC_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM
// Register Offset : 0xf000
// Description     : temp offset 0x100
//--------------------------------------------------------------------------
#define RPC_RGN_DOM_JUMP                0x38
#define RPC_RGN_DOM_(i)                 REG(REG_AP_APB_RPC_RGN_DOM) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM_PER0
// Register Offset : 0xf004
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_DOM_PER0_(i)            REG(REG_AP_APB_RPC_RGN_DOM_PER0) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM_PER1
// Register Offset : 0xf008
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_DOM_PER1_(i)            REG(REG_AP_APB_RPC_RGN_DOM_PER1) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER0
// Register Offset : 0xf00c
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_SEC_PER0_(i)            REG(REG_AP_APB_RPC_RGN_SEC_PER0) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER1
// Register Offset : 0xf010
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_SEC_PER1_(i)            REG(REG_AP_APB_RPC_RGN_SEC_PER1) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER2
// Register Offset : 0xf014
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_SEC_PER2_(i)            REG(REG_AP_APB_RPC_RGN_SEC_PER2) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER3
// Register Offset : 0xf018
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_SEC_PER3_(i)            REG(REG_AP_APB_RPC_RGN_SEC_PER3) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER0
// Register Offset : 0xf01c
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_PRI_PER0_(i)            REG(REG_AP_APB_RPC_RGN_PRI_PER0) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER1
// Register Offset : 0xf020
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_PRI_PER1_(i)            REG(REG_AP_APB_RPC_RGN_PRI_PER1) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER2
// Register Offset : 0xf024
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_PRI_PER2_(i)            REG(REG_AP_APB_RPC_RGN_PRI_PER2) + (i) * RPC_RGN_DOM_JUMP

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER3
// Register Offset : 0xf028
// Description     :
//--------------------------------------------------------------------------
#define RPC_RGN_PRI_PER3_(i)            REG(REG_AP_APB_RPC_RGN_PRI_PER3) + (i) * RPC_RGN_DOM_JUMP

#endif
