//*****************************************************************************
//
// rstgen_program.h - Driver for the rstgen Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#ifndef __RSTGEN_DRV_H__
#define __RSTGEN_DRV_H__

#include <sys/types.h>

#include "lib/reg.h"

//*****************************************************************************
//
// This section defines the register offsets of
// rstgen component
//
//*****************************************************************************

#define GLB_RST_EN      0x00000000
#define CORE_RST_EN     0x00000004
#define CORE_SW_RST     0x00000008
#define SW_SELF_RST     0x00000044
#define SW_OTH_RST      0x00000048
#define RST_STA         0x0000004C
#define GENERAL_REG     0x00000050
#define ISO_EN          0x00000090
#define MODULE_RST      0x00000100

#define RSTGEN_SAF_BASE    (vaddr_t)_ioaddr(APB_RSTGEN_SAF_BASE)
#define RSTGEN_SEC_BASE    (vaddr_t)_ioaddr(APB_RSTGEN_SEC_BASE)
#define RSTGEN_RTC_BASE    (vaddr_t)_ioaddr(APB_RSTGEN_RTC_BASE)

//*****************************************************************************
//
// This section defines all mask
//
//*****************************************************************************
//global_rst_en
#define RSTGEN_GLB_RST_SELF_RST_EN_MASK     ((uint32_t) (1 << 0))
#define RSTGEN_GLB_RST_SELF_RST_EN_SHIFT                (0U)
#define RSTGEN_GLB_RST_SELF_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_SELF_RST_EN_SHIFT)) & RSTGEN_GLB_RST_SELF_RST_EN_MASK)
#define RSTGEN_GLB_RST_SEM_RST_EN_MASK      ((uint32_t) (1 << 1))
#define RSTGEN_GLB_RST_SEM_RST_EN_SHIFT                (1U)
#define RSTGEN_GLB_RST_SEM_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_SEM_RST_EN_SHIFT)) & RSTGEN_GLB_RST_SEM_RST_EN_MASK)
#define RSTGEN_GLB_RST_DBG_RST_EN_MASK      ((uint32_t) (1 << 2))
#define RSTGEN_GLB_RST_DBG_RST_EN_SHIFT                (2U)
#define RSTGEN_GLB_RST_DBG_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_DBG_RST_EN_SHIFT)) & RSTGEN_GLB_RST_DBG_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG1_RST_EN_MASK     ((uint32_t) (1 << 3))
#define RSTGEN_GLB_RST_WDG1_RST_EN_SHIFT                (3U)
#define RSTGEN_GLB_RST_WDG1_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG1_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG1_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG2_RST_EN_MASK     ((uint32_t) (1 << 4))
#define RSTGEN_GLB_RST_WDG2_RST_EN_SHIFT                (4U)
#define RSTGEN_GLB_RST_WDG2_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG2_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG2_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG3_RST_EN_MASK     ((uint32_t) (1 << 5))
#define RSTGEN_GLB_RST_WDG3_RST_EN_SHIFT                (5U)
#define RSTGEN_GLB_RST_WDG3_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG3_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG3_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG4_RST_EN_MASK     ((uint32_t) (1 << 6))
#define RSTGEN_GLB_RST_WDG4_RST_EN_SHIFT                (6U)
#define RSTGEN_GLB_RST_WDG4_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG4_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG4_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG5_RST_EN_MASK     ((uint32_t) (1 << 7))
#define RSTGEN_GLB_RST_WDG5_RST_EN_SHIFT                (7U)
#define RSTGEN_GLB_RST_WDG5_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG5_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG5_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG6_RST_EN_MASK     ((uint32_t) (1 << 8))
#define RSTGEN_GLB_RST_WDG6_RST_EN_SHIFT                (8U)
#define RSTGEN_GLB_RST_WDG6_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG6_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG6_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG7_RST_EN_MASK     ((uint32_t) (1 << 9))
#define RSTGEN_GLB_RST_WDG7_RST_EN_SHIFT                (9U)
#define RSTGEN_GLB_RST_WDG7_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG7_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG7_RST_EN_MASK)
#define RSTGEN_GLB_RST_WDG8_RST_EN_MASK     ((uint32_t) (1 << 10))
#define RSTGEN_GLB_RST_WDG8_RST_EN_SHIFT                (10U)
#define RSTGEN_GLB_RST_WDG8_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_WDG8_RST_EN_SHIFT)) & RSTGEN_GLB_RST_WDG8_RST_EN_MASK)
#define RSTGEN_GLB_RST_LOCK_MASK        ((uint32_t) (1 << 31))
#define RSTGEN_GLB_RST_LOCK_SHIFT                (31U)
#define RSTGEN_GLB_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_GLB_RST_LOCK_SHIFT)) & RSTGEN_GLB_RST_LOCK_MASK)
//core reset en
#define RSTGEN_CORE_RST_SW_RST_EN_MASK      ((uint32_t) (1 << 0))
#define RSTGEN_CORE_RST_SW_RST_EN_SHIFT                (0U)
#define RSTGEN_CORE_RST_SW_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_RST_SW_RST_EN_SHIFT)) & RSTGEN_CORE_RST_SW_RST_EN_MASK)
#define RSTGEN_CORE_RST_DBG_RST_EN_MASK     ((uint32_t) (1 << 1))
#define RSTGEN_CORE_RST_DBG_RST_EN_SHIFT                (1U)
#define RSTGEN_CORE_RST_DBG_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_RST_DBG_RST_EN_SHIFT)) & RSTGEN_CORE_RST_DBG_RST_EN_MASK)
#define RSTGEN_CORE_RST_WDG_RST_EN_MASK     ((uint32_t) (1 << 2))
#define RSTGEN_CORE_RST_WDG_RST_EN_SHIFT                (2U)
#define RSTGEN_CORE_RST_WDG_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_RST_WDG_RST_EN_SHIFT)) & RSTGEN_CORE_RST_WDG_RST_EN_MASK)
#define RSTGEN_CORE_RST_SW_RST_EN_STA_MASK      ((uint32_t) (1 << 30))
#define RSTGEN_CORE_RST_SW_RST_EN_STA_SHIFT                (30U)
#define RSTGEN_CORE_RST_SW_RST_EN_STA(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_RST_SW_RST_EN_SHIFT)) & RSTGEN_CORE_RST_SW_RST_EN_MASK)
#define RSTGEN_CORE_RST_RST_LOCK_MASK       ((uint32_t) (1 << 31))
#define RSTGEN_CORE_RST_RST_LOCK_SHIFT                (31U)
#define RSTGEN_CORE_RST_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_RST_RST_LOCK_SHIFT)) & RSTGEN_CORE_RST_RST_LOCK_MASK)
//core software reset en
#define RSTGEN_CORE_SW_RST_STATIC_RST_MASK      ((uint32_t) (1 << 0))
#define RSTGEN_CORE_SW_RST_STATIC_RST_SHIFT                (0U)
#define RSTGEN_CORE_SW_RST_STATIC_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_SW_RST_STATIC_RST_SHIFT)) & RSTGEN_CORE_SW_RST_STATIC_RST_MASK)
#define RSTGEN_CORE_SW_RST_AUTO_CLR_MASK        ((uint32_t) (1 << 1))
#define RSTGEN_CORE_SW_RST_AUTO_CLR_SHIFT                (1U)
#define RSTGEN_CORE_SW_RST_AUTO_CLR(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_SW_RST_AUTO_CLR_SHIFT)) & RSTGEN_CORE_SW_RST_AUTO_CLR_MASK)
#define RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_MASK        ((uint32_t) (1 << 29))
#define RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_SHIFT                (29U)
#define RSTGEN_CORE_SW_RST_STATIC_RST_B_STA(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_SHIFT)) & RSTGEN_CORE_SW_RST_STATIC_RST_B_STA_MASK)
#define RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK        ((uint32_t) (1 << 30))
#define RSTGEN_CORE_SW_RST_CORE_RST_STA_SHIFT                (30U)
#define RSTGEN_CORE_SW_RST_CORE_RST_STA(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_CORE_SW_RST_CORE_RST_STA_SHIFT)) & RSTGEN_CORE_SW_RST_CORE_RST_STA_MASK)
//software self reset
#define RSTGEN_SELF_RST_SW_GLB_RST_MASK     ((uint32_t) (1 << 0))
#define RSTGEN_SELF_RST_SW_GLB_RST_SHIFT                (0U)
#define RSTGEN_SELF_RST_SW_GLB_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_SELF_RST_SW_GLB_RST_SHIFT)) & RSTGEN_SELF_RST_SW_GLB_RST_MASK)
#define RSTGEN_SELF_RST_SW_GLB_RST_LOCK_MASK        ((uint32_t) (1 << 31))
#define RSTGEN_SELF_RST_SW_GLB_RST_LOCK_SHIFT                (31U)
#define RSTGEN_SELF_RST_SW_GLB_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_SELF_RST_SW_GLB_RST_LOCK_SHIFT)) & RSTGEN_SELF_RST_SW_GLB_RST_LOCK_MASK)
//software other reset
#define RSTGEN_OTH_RST_SW_GLB_RST_MASK      ((uint32_t) (1 << 0))
#define RSTGEN_OTH_RST_SW_GLB_RST_SHIFT                (0U)
#define RSTGEN_OTH_RST_SW_GLB_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_OTH_RST_SW_GLB_RST_SHIFT)) & RSTGEN_OTH_RST_SW_GLB_RST_MASK)
#define RSTGEN_OTH_RST_SW_GLB_RST_LOCK_MASK     ((uint32_t) (1 << 31))
#define RSTGEN_OTH_RST_SW_GLB_RST_LOCK_SHIFT                (31U)
#define RSTGEN_OTH_RST_SW_GLB_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_OTH_RST_SW_GLB_RST_LOCK_SHIFT)) & RSTGEN_OTH_RST_SW_GLB_RST_LOCK_MASK)
//reset status
#define RSTGEN_RST_STA_PRE_SW_RST_MASK      ((uint32_t) (1 << 0))
#define RSTGEN_RST_STA_PRE_SW_RST_SHIFT                (0U)
#define RSTGEN_RST_STA_PRE_SW_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_PRE_SW_RST_SHIFT)) & RSTGEN_RST_STA_PRE_SW_RST_MASK)
#define RSTGEN_RST_STA_SELF_SW_RST_MASK     ((uint32_t) (1 << 1))
#define RSTGEN_RST_STA_SELF_SW_RST_SHIFT                (1U)
#define RSTGEN_RST_STA_SELF_SW_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SELF_SW_RST_SHIFT)) & RSTGEN_RST_STA_SELF_SW_RST_MASK)
#define RSTGEN_RST_STA_SEM_RST_MASK     ((uint32_t) (1 << 2))
#define RSTGEN_RST_STA_SEM_RST_SHIFT                (2U)
#define RSTGEN_RST_STA_SEM_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SEM_RST_SHIFT)) & RSTGEN_RST_STA_SEM_RST_MASK)
#define RSTGEN_RST_STA_DBG_RST_MASK     ((uint32_t) (1 << 3))
#define RSTGEN_RST_STA_DBG_RST_SHIFT                (3U)
#define RSTGEN_RST_STA_DBG_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_DBG_RST_SHIFT)) & RSTGEN_RST_STA_DBG_RST_MASK)
#define RSTGEN_RST_STA_WDG1_RST_MASK        ((uint32_t) (1 << 4))
#define RSTGEN_RST_STA_WDG1_RST_SHIFT                (4U)
#define RSTGEN_RST_STA_WDG1_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG1_RST_SHIFT)) & RSTGEN_RST_STA_WDG1_RST_MASK)
#define RSTGEN_RST_STA_WDG2_RST_MASK        ((uint32_t) (1 << 5))
#define RSTGEN_RST_STA_WDG2_RST_SHIFT                (5U)
#define RSTGEN_RST_STA_WDG2_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG2_RST_SHIFT)) & RSTGEN_RST_STA_WDG2_RST_MASK)
#define RSTGEN_RST_STA_WDG3_RST_MASK        ((uint32_t) (1 << 6))
#define RSTGEN_RST_STA_WDG3_RST_SHIFT                (6U)
#define RSTGEN_RST_STA_WDG3_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG3_RST_SHIFT)) & RSTGEN_RST_STA_WDG3_RST_MASK)
#define RSTGEN_RST_STA_WDG4_RST_MASK        ((uint32_t) (1 << 7))
#define RSTGEN_RST_STA_WDG4_RST_SHIFT                (7U)
#define RSTGEN_RST_STA_WDG4_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG4_RST_SHIFT)) & RSTGEN_RST_STA_WDG4_RST_MASK)
#define RSTGEN_RST_STA_WDG5_RST_MASK        ((uint32_t) (1 << 8))
#define RSTGEN_RST_STA_WDG5_RST_SHIFT                (8U)
#define RSTGEN_RST_STA_WDG5_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG5_RST_SHIFT)) & RSTGEN_RST_STA_WDG5_RST_MASK)
#define RSTGEN_RST_STA_WDG6_RST_MASK        ((uint32_t) (1 << 9))
#define RSTGEN_RST_STA_WDG6_RST_SHIFT                (9U)
#define RSTGEN_RST_STA_WDG6_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG6_RST_SHIFT)) & RSTGEN_RST_STA_WDG6_RST_MASK)
#define RSTGEN_RST_STA_WDG7_RST_MASK        ((uint32_t) (1 << 10))
#define RSTGEN_RST_STA_WDG7_RST_SHIFT                (10U)
#define RSTGEN_RST_STA_WDG7_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG7_RST_SHIFT)) & RSTGEN_RST_STA_WDG7_RST_MASK)
#define RSTGEN_RST_STA_WDG8_RST_MASK        ((uint32_t) (1 << 11))
#define RSTGEN_RST_STA_WDG8_RST_SHIFT                (11U)
#define RSTGEN_RST_STA_WDG8_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_WDG8_RST_SHIFT)) & RSTGEN_RST_STA_WDG8_RST_MASK)
#define RSTGEN_RST_STA_SINC_PRE_SW_RST_MASK     ((uint32_t) (1 << 16))
#define RSTGEN_RST_STA_SINC_PRE_SW_RST_SHIFT                (16U)
#define RSTGEN_RST_STA_SINC_PRE_SW_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_PRE_SW_RST_SHIFT)) & RSTGEN_RST_STA_SINC_PRE_SW_RST_MASK)
#define RSTGEN_RST_STA_SINC_SELF_SW_RST_MASK        ((uint32_t) (1 << 17))
#define RSTGEN_RST_STA_SINC_SELF_SW_RST_SHIFT                (17U)
#define RSTGEN_RST_STA_SINC_SELF_SW_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_SELF_SW_RST_SHIFT)) & RSTGEN_RST_STA_SINC_SELF_SW_RST_MASK)
#define RSTGEN_RST_STA_SINC_SEM_RST_MASK        ((uint32_t) (1 << 18))
#define RSTGEN_RST_STA_SINC_SEM_RST_SHIFT                (18U)
#define RSTGEN_RST_STA_SINC_SEM_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_SEM_RST_SHIFT)) & RSTGEN_RST_STA_SINC_SEM_RST_MASK)
#define RSTGEN_RST_STA_SINC_DBG_RST_MASK        ((uint32_t) (1 << 19))
#define RSTGEN_RST_STA_SINC_DBG_RST_SHIFT                (19U)
#define RSTGEN_RST_STA_SINC_DBG_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_DBG_RST_SHIFT)) & RSTGEN_RST_STA_SINC_DBG_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG1_RST_MASK       ((uint32_t) (1 << 20))
#define RSTGEN_RST_STA_SINC_WDG1_RST_SHIFT                (20U)
#define RSTGEN_RST_STA_SINC_WDG1_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG1_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG1_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG2_RST_MASK       ((uint32_t) (1 << 21))
#define RSTGEN_RST_STA_SINC_WDG2_RST_SHIFT                (21U)
#define RSTGEN_RST_STA_SINC_WDG2_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG2_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG2_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG3_RST_MASK       ((uint32_t) (1 << 22))
#define RSTGEN_RST_STA_SINC_WDG3_RST_SHIFT                (22U)
#define RSTGEN_RST_STA_SINC_WDG3_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG3_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG3_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG4_RST_MASK       ((uint32_t) (1 << 23))
#define RSTGEN_RST_STA_SINC_WDG4_RST_SHIFT                (23U)
#define RSTGEN_RST_STA_SINC_WDG4_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG4_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG4_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG5_RST_MASK       ((uint32_t) (1 << 24))
#define RSTGEN_RST_STA_SINC_WDG5_RST_SHIFT                (24U)
#define RSTGEN_RST_STA_SINC_WDG5_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG5_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG5_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG6_RST_MASK       ((uint32_t) (1 << 25))
#define RSTGEN_RST_STA_SINC_WDG6_RST_SHIFT                (25U)
#define RSTGEN_RST_STA_SINC_WDG6_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG6_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG6_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG7_RST_MASK       ((uint32_t) (1 << 26))
#define RSTGEN_RST_STA_SINC_WDG7_RST_SHIFT                (26U)
#define RSTGEN_RST_STA_SINC_WDG7_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG7_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG7_RST_MASK)
#define RSTGEN_RST_STA_SINC_WDG8_RST_MASK       ((uint32_t) (1 << 27))
#define RSTGEN_RST_STA_SINC_WDG8_RST_SHIFT                (27U)
#define RSTGEN_RST_STA_SINC_WDG8_RST(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_RST_STA_SINC_WDG8_RST_SHIFT)) & RSTGEN_RST_STA_SINC_WDG8_RST_MASK)
//iso en
#define RSTGEN_ISO_EN_B_MASK        ((uint32_t) (1 << 0))
#define RSTGEN_ISO_EN_B_SHIFT                (0U)
#define RSTGEN_ISO_EN_B(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_ISO_EN_B_SHIFT)) & RSTGEN_ISO_EN_B_MASK)
//module reset
#define RSTGEN_MODULE_RST_N_MASK        ((uint32_t) (1 << 0))
#define RSTGEN_MODULE_RST_N_SHIFT                (0U)
#define RSTGEN_MODULE_RST_N(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_MODULE_RST_N_SHIFT)) & RSTGEN_MODULE_RST_N_MASK)
#define RSTGEN_MODULE_RST_EN_MASK       ((uint32_t) (1 << 1))
#define RSTGEN_MODULE_RST_EN_SHIFT                (1U)
#define RSTGEN_MODULE_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_MODULE_RST_EN_SHIFT)) & RSTGEN_MODULE_RST_EN_MASK)
#define RSTGEN_MODULE_RST_STA_MASK      ((uint32_t) (1 << 30))
#define RSTGEN_MODULE_RST_STA_SHIFT                (30U)
#define RSTGEN_MODULE_RST_STA(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_MODULE_RST_STA_SHIFT)) & RSTGEN_MODULE_RST_STA_MASK)
#define RSTGEN_MODULE_RST_LOCK_MASK     ((uint32_t) (1 << 31))
#define RSTGEN_MODULE_RST_LOCK_SHIFT                (31U)
#define RSTGEN_MODULE_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << RSTGEN_MODULE_RST_LOCK_SHIFT)) & RSTGEN_MODULE_RST_LOCK_MASK)

//offset
#define RSTGEN_CORE_RST_EN_OFF(core_idx)    (CORE_RST_EN + 0x8*(core_idx))
#define RSTGEN_CORE_SW_RST_OFF(core_idx)    (CORE_SW_RST + 0x8*(core_idx))
#define RSTGEN_SW_SELF_RST_OFF              (SW_SELF_RST)
#define RSTGEN_SW_OTH_RST_OFF               (SW_OTH_RST)
#define RSTGEN_RST_STA_OFF                  (RST_STA)
#define RSTGEN_GENERAL_RST_OFF              (GENERAL_REG)
#define RSTGEN_ISO_EN_OFF(iso_idx)          (ISO_EN + 0x04 * iso_idx)
#define RSTGEN_MODULE_RST_OFF(module_idx)   (0x100U + 0x4 * (module_idx))
#define SOC_RSTGEN_REG_MAP(reg_off)         ((reg_off) << 10)                                                                     \

void rstgen_get_default_config(uint32_t *global_rst_maks);
bool rstgen_init(vaddr_t base, const uint32_t global_rst_maks);
bool rstgen_global_rst_enable(vaddr_t base, uint32_t mask);
bool rstgen_global_rst_disable(vaddr_t base, uint32_t mask);
bool rstgen_sw_self_rst(vaddr_t base, bool rst_release);
bool rstgen_sw_oth_rst(vaddr_t base, bool rst_release);
uint32_t rstgen_get_rst_sta(vaddr_t base);
bool rstgen_iso_enable(vaddr_t base, uint32_t iso_idx);
bool rstgen_iso_disable(vaddr_t base, uint32_t iso_idx);
uint32_t rstgen_iso_status(vaddr_t base, uint32_t iso_idx);
bool rstgen_core_rst_enable(vaddr_t base, uint32_t core_idx, uint32_t mask);
bool rstgen_core_rst_disable(vaddr_t base, uint32_t core_idx, uint32_t mask);
bool rstgen_core_reset(vaddr_t base, uint32_t core_idx);
bool rstgen_core_ctl(vaddr_t base, uint32_t core_idx, bool release);
bool rstgen_module_ctl(vaddr_t base, uint32_t module_idx, bool rst_release);
uint32_t rstgen_module_status(vaddr_t base, uint32_t module_idx);
uint32_t rstgen_core_status(vaddr_t base, uint32_t core_idx);
#endif
