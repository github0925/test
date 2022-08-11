//*****************************************************************************
//
// This section defines clkgen_drv.h
// WDG component
//
//*****************************************************************************
#ifndef __CLKGEN_DRV_H__
#define __CLKGEN_DRV_H__
/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <lib/reg.h>

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define DEFAULT_CLKGEN_LOG_LEVEL    3

//*****************************************************************************
//
// This section defines the register offsets of
// clkgen component
//
//*****************************************************************************
#define CKGEN_SAF_BASE    (vaddr_t)_ioaddr(APB_CKGEN_SAF_BASE)
#define CKGEN_SEC_BASE    (vaddr_t)_ioaddr(APB_CKGEN_SEC_BASE)
#define CKGEN_SOC_BASE    (vaddr_t)_ioaddr(APB_CKGEN_SOC_BASE)
#define CKGEN_DISP_BASE    (vaddr_t)_ioaddr(APB_CKGEN_DISP_BASE)
#define SCR_SAF_BASE    (vaddr_t)_ioaddr(APB_SCR_SAF_BASE)
#define SCR_SEC_BASE    (vaddr_t)_ioaddr(APB_SCR_SEC_BASE)

//*****************************************************************************
//
// This section defines all mask
//
//*****************************************************************************
//clkgen ip slice ctl 0x0
#define CLKGEN_IP_SLICE_CTL_CG_EN_MASK      ((uint32_t) (1 << 0))
#define CLKGEN_IP_SLICE_CTL_CG_EN_SHIFT                (0U)
#define CLKGEN_IP_SLICE_CTL_CG_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_CG_EN_SHIFT)) & CLKGEN_IP_SLICE_CTL_CG_EN_MASK)
#define CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL_MASK        ((uint32_t) (0x7 << 1))
#define CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL_SHIFT                (1U)
#define CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL_SHIFT)) & CLKGEN_IP_SLICE_CTL_CLK_SRC_SEL_MASK)
#define CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM_MASK        ((uint32_t) (0x7 << 4))
#define CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM_SHIFT                (4U)
#define CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM_SHIFT)) & CLKGEN_IP_SLICE_CTL_PRE_DIV_NUM_MASK)
#define CLKGEN_IP_SLICE_CTL_RESERVED_MASK       ((uint32_t) (0x7 << 7))
#define CLKGEN_IP_SLICE_CTL_RESERVED_SHIFT                (7U)
#define CLKGEN_IP_SLICE_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_RESERVED_SHIFT)) & CLKGEN_IP_SLICE_CTL_RESERVED_MASK)
#define CLKGEN_IP_SLICE_CTL_POST_DIV_NUM_MASK       ((uint32_t) (0x3f << 10))
#define CLKGEN_IP_SLICE_CTL_POST_DIV_NUM_SHIFT                (10U)
#define CLKGEN_IP_SLICE_CTL_POST_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_POST_DIV_NUM_SHIFT)) & CLKGEN_IP_SLICE_CTL_POST_DIV_NUM_MASK)
#define CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_MASK       ((uint32_t) (1 << 28))
#define CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT                (28U)
#define CLKGEN_IP_SLICE_CTL_CG_EN_STATUS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_SHIFT)) & CLKGEN_IP_SLICE_CTL_CG_EN_STATUS_MASK)
#define CLKGEN_IP_SLICE_CTL_PRE_BUSY_MASK       ((uint32_t) (1 << 30))
#define CLKGEN_IP_SLICE_CTL_PRE_BUSY_SHIFT                (30U)
#define CLKGEN_IP_SLICE_CTL_PRE_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_PRE_BUSY_SHIFT)) & CLKGEN_IP_SLICE_CTL_PRE_BUSY_MASK)
#define CLKGEN_IP_SLICE_CTL_POST_BUSY_MASK      ((uint32_t) (1 << 31))
#define CLKGEN_IP_SLICE_CTL_POST_BUSY_SHIFT                (31U)
#define CLKGEN_IP_SLICE_CTL_POST_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_CTL_POST_BUSY_SHIFT)) & CLKGEN_IP_SLICE_CTL_POST_BUSY_MASK)
//clkgen bus slice ctl 0x200
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK       ((uint32_t) (1 << 0))
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A_SHIFT                (0U)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CG_EN_A_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CG_EN_A_MASK)
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A_MASK     ((uint32_t) (0x7 << 1))
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A_SHIFT                (1U)
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_A_MASK)
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A_MASK     ((uint32_t) (0x7 << 4))
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A_SHIFT                (4U)
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A_SHIFT)) & CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_A_MASK)
#define CLKGEN_BUS_SLICE_CTL_RESERVED_MASK      ((uint32_t) (0x3 << 7))
#define CLKGEN_BUS_SLICE_CTL_RESERVED_SHIFT                (7U)
#define CLKGEN_BUS_SLICE_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_RESERVED_SHIFT)) & CLKGEN_BUS_SLICE_CTL_RESERVED_MASK)
#define CLKGEN_BUS_SLICE_CTL_A_B_SEL_MASK       ((uint32_t) (1 << 9))
#define CLKGEN_BUS_SLICE_CTL_A_B_SEL_SHIFT                (9U)
#define CLKGEN_BUS_SLICE_CTL_A_B_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_A_B_SEL_SHIFT)) & CLKGEN_BUS_SLICE_CTL_A_B_SEL_MASK)

#define CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_MASK      ((uint32_t) (0x3f << 10))
#define CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_SHIFT                (10U)
#define CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_SHIFT)) & CLKGEN_BUS_SLICE_CTL_POST_DIV_NUM_MASK)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B_MASK       ((uint32_t) (1 << 16))
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B_SHIFT                (16U)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CG_EN_B_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CG_EN_B_MASK)
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B_MASK     ((uint32_t) (0x7 << 17))
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B_SHIFT                (17U)
#define CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CLK_SRC_SEL_B_MASK)
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B_MASK     ((uint32_t) (0x7 << 20))
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B_SHIFT                (20U)
#define CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B_SHIFT)) & CLKGEN_BUS_SLICE_CTL_PRE_DIV_NUM_B_MASK)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_MASK        ((uint32_t) (1 << 27))
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT                (27U)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CG_EN_B_STATUS_MASK)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_MASK        ((uint32_t) (1 << 28))
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT                (28U)
#define CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_SHIFT)) & CLKGEN_BUS_SLICE_CTL_CG_EN_A_STATUS_MASK)
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_MASK        ((uint32_t) (1 << 29))
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_SHIFT                (29U)
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_SHIFT)) & CLKGEN_BUS_SLICE_CTL_PRE_BUSY_B_MASK)
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_MASK        ((uint32_t) (1 << 30))
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_SHIFT                (30U)
#define CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_SHIFT)) & CLKGEN_BUS_SLICE_CTL_PRE_BUSY_A_MASK)
#define CLKGEN_BUS_SLICE_CTL_POST_BUSY_MASK     ((uint32_t) (1 << 31))
#define CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT                (31U)
#define CLKGEN_BUS_SLICE_CTL_POST_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_CTL_POST_BUSY_SHIFT)) & CLKGEN_BUS_SLICE_CTL_POST_BUSY_MASK)
//clkgen bus slice gasket 0x204
#define CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_MASK      ((uint32_t) (0x7 << 0))
#define CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_SHIFT                (0U)
#define CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_Q_DIV_NUM_MASK)
#define CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_MASK      ((uint32_t) (0x7 << 3))
#define CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_SHIFT                (3U)
#define CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_P_DIV_NUM_MASK)
#define CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_MASK      ((uint32_t) (0x7 << 6))
#define CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_SHIFT                (6U)
#define CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_N_DIV_NUM_MASK)
#define CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_MASK      ((uint32_t) (0x7 << 9))
#define CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_SHIFT                (9U)
#define CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_M_DIV_NUM_MASK)
#define CLKGEN_BUS_SLICE_GASKET_RESERVED_MASK       ((uint32_t) (0xffff << 12))
#define CLKGEN_BUS_SLICE_GASKET_RESERVED_SHIFT                (12U)
#define CLKGEN_BUS_SLICE_GASKET_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_RESERVED_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_RESERVED_MASK)
#define CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_MASK     ((uint32_t) (1 << 28))
#define CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT                (28U)
#define CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY_MASK)
#define CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY_MASK     ((uint32_t) (1 << 29))
#define CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY_SHIFT                (29U)
#define CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_DIV_P_BUSY_MASK)
#define CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY_MASK     ((uint32_t) (1 << 30))
#define CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY_SHIFT                (30U)
#define CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_DIV_N_BUSY_MASK)
#define CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_MASK     ((uint32_t) (1 << 31))
#define CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_SHIFT                (31U)
#define CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_SHIFT)) & CLKGEN_BUS_SLICE_GASKET_DIV_M_BUSY_MASK)
//clkgen core slice ctl 0x300
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK      ((uint32_t) (1 << 0))
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A_SHIFT                (0U)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CG_EN_A_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CG_EN_A_MASK)
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A_MASK        ((uint32_t) (0X7 << 1))
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A_SHIFT                (1U)
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_A_MASK)
#define CLKGEN_CORE_SLICE_CTL_RESERVED_MASK     ((uint32_t) (0x1f<< 4))
#define CLKGEN_CORE_SLICE_CTL_RESERVED_SHIFT                (4U)
#define CLKGEN_CORE_SLICE_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_RESERVED_SHIFT)) & CLKGEN_CORE_SLICE_CTL_RESERVED_MASK)
#define CLKGEN_CORE_SLICE_CTL_A_B_SEL_MASK      ((uint32_t) (1<< 9))
#define CLKGEN_CORE_SLICE_CTL_A_B_SEL_SHIFT                (9U)
#define CLKGEN_CORE_SLICE_CTL_A_B_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_A_B_SEL_SHIFT)) & CLKGEN_CORE_SLICE_CTL_A_B_SEL_MASK)
#define CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_MASK     ((uint32_t) (0x3f<< 10))
#define CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_SHIFT                (10U)
#define CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_SHIFT)) & CLKGEN_CORE_SLICE_CTL_POST_DIV_NUM_MASK)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B_MASK      ((uint32_t) (1<< 16))
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B_SHIFT                (16U)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CG_EN_B_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CG_EN_B_MASK)
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B_MASK        ((uint32_t) (0x7<< 17))
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B_SHIFT                (17U)
#define CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CLK_SRC_SEL_B_MASK)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_MASK       ((uint32_t) (1<< 27))
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT                (27U)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CG_EN_B_STATUS_MASK)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_MASK       ((uint32_t) (1<< 28))
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT                (28U)
#define CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_SHIFT)) & CLKGEN_CORE_SLICE_CTL_CG_EN_A_STATUS_MASK)
#define CLKGEN_CORE_SLICE_CTL_POST_BUSY_MASK        ((uint32_t) (1<< 31))
#define CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT                (31U)
#define CLKGEN_CORE_SLICE_CTL_POST_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_CTL_POST_BUSY_SHIFT)) & CLKGEN_CORE_SLICE_CTL_POST_BUSY_MASK)
//clkgen lp gating en 0x400
#define CLKGEN_LP_GATING_EN_SW_GATING_EN_MASK       ((uint32_t) (1<< 0))
#define CLKGEN_LP_GATING_EN_SW_GATING_EN_SHIFT                (0U)
#define CLKGEN_LP_GATING_EN_SW_GATING_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_LP_GATING_EN_SW_GATING_EN_SHIFT)) & CLKGEN_LP_GATING_EN_SW_GATING_EN_MASK)
#define CLKGEN_LP_GATING_EN_SW_GATING_DIS_MASK      ((uint32_t) (1<< 1))
#define CLKGEN_LP_GATING_EN_SW_GATING_DIS_SHIFT                (1U)
#define CLKGEN_LP_GATING_EN_SW_GATING_DIS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_LP_GATING_EN_SW_GATING_DIS_SHIFT)) & CLKGEN_LP_GATING_EN_SW_GATING_DIS_MASK)

#define CLKGEN_LP_GATING_EN_GATING_LOCK_MASK        ((uint32_t) (1<< 31))
#define CLKGEN_LP_GATING_EN_GATING_LOCK_SHIFT                (31U)
#define CLKGEN_LP_GATING_EN_GATING_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_LP_GATING_EN_GATING_LOCK_SHIFT)) & CLKGEN_LP_GATING_EN_GATING_LOCK_MASK)
//clkgen uuu slice 0x600
#define CLKGEN_UUU_SLICE_Q_DIV_NUM_MASK     ((uint32_t) (0xf<< 0))
#define CLKGEN_UUU_SLICE_Q_DIV_NUM_SHIFT                (0U)
#define CLKGEN_UUU_SLICE_Q_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_Q_DIV_NUM_SHIFT)) & CLKGEN_UUU_SLICE_Q_DIV_NUM_MASK)

#define CLKGEN_UUU_SLICE_P_DIV_NUM_MASK     ((uint32_t) (0xf<< 4))
#define CLKGEN_UUU_SLICE_P_DIV_NUM_SHIFT                (4U)
#define CLKGEN_UUU_SLICE_P_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_P_DIV_NUM_SHIFT)) & CLKGEN_UUU_SLICE_P_DIV_NUM_MASK)

#define CLKGEN_UUU_SLICE_N_DIV_NUM_MASK     ((uint32_t) (0xf<< 8))
#define CLKGEN_UUU_SLICE_N_DIV_NUM_SHIFT                (8U)
#define CLKGEN_UUU_SLICE_N_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_N_DIV_NUM_SHIFT)) & CLKGEN_UUU_SLICE_N_DIV_NUM_MASK)

#define CLKGEN_UUU_SLICE_M_DIV_NUM_MASK     ((uint32_t) (0xf<< 12))
#define CLKGEN_UUU_SLICE_M_DIV_NUM_SHIFT                (12U)
#define CLKGEN_UUU_SLICE_M_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_M_DIV_NUM_SHIFT)) & CLKGEN_UUU_SLICE_M_DIV_NUM_MASK)

#define CLKGEN_UUU_SLICE_UUU_SEL_MASK       ((uint32_t) (0xf<< 16))
#define CLKGEN_UUU_SLICE_UUU_SEL_SHIFT                (16U)
#define CLKGEN_UUU_SLICE_UUU_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_UUU_SEL_SHIFT)) & CLKGEN_UUU_SLICE_UUU_SEL_MASK)

#define CLKGEN_UUU_SLICE_UUU_GATING_ACK_MASK        ((uint32_t) (1<< 20))
#define CLKGEN_UUU_SLICE_UUU_GATING_ACK_SHIFT                (20U)
#define CLKGEN_UUU_SLICE_UUU_GATING_ACK(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_UUU_GATING_ACK_SHIFT)) & CLKGEN_UUU_SLICE_UUU_GATING_ACK_MASK)

#define CLKGEN_UUU_SLICE_DBG_DIV_NUM_MASK       ((uint32_t) (0xf<< 27))
#define CLKGEN_UUU_SLICE_DBG_DIV_NUM_SHIFT                (27U)
#define CLKGEN_UUU_SLICE_DBG_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_DBG_DIV_NUM_SHIFT)) & CLKGEN_UUU_SLICE_DBG_DIV_NUM_MASK)

#define CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN_MASK     ((uint32_t) (1<< 31))
#define CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN_SHIFT                (31U)
#define CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN_SHIFT)) & CLKGEN_UUU_SLICE_UUU_DBG_GATING_EN_MASK)
//clkgen ip slice dbg ctl 0x700
#define CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL_SHIFT                (0U)
#define CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL_SHIFT)) & CLKGEN_IP_SLICE_DBG_CTL_IP_SLICE_DBG_SEL_MASK)

#define CLKGEN_IP_SLICE_DBG_CTL_RESERVED_MASK       ((uint32_t) (0xffff<< 16))
#define CLKGEN_IP_SLICE_DBG_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_IP_SLICE_DBG_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_DBG_CTL_RESERVED_SHIFT)) & CLKGEN_IP_SLICE_DBG_CTL_RESERVED_MASK)
//clkgen bus slice dbg ctl 0x704
#define CLKGEN_BUS_SLICE_DBG_CTL_BUS_SLICE_DBG_SEL_MASK     ((uint32_t) (0xffff<< 0))
#define CLKGEN_BUS_SLICE_DBG_CTL_BUS_SLICE_DBG_SEL_SHIFT                (0U)
#define CLKGEN_BUS_SLICE_DBG_CTL_BUS_SLICE_DBG_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_DBG_CTL_BUS_SLICE_DBG_SEL_SHIFT)) & CLKGEN_BUS_SLICE_DBG_CTL_BUS_SLICE_DBG_SEL_MASK)

#define CLKGEN_BUS_SLICE_DBG_CTL_RESERVED_MASK      ((uint32_t) (0xffff<< 16))
#define CLKGEN_BUS_SLICE_DBG_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_BUS_SLICE_DBG_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_DBG_CTL_RESERVED_SHIFT)) & CLKGEN_BUS_SLICE_DBG_CTL_RESERVED_MASK)
//clkgen core slice dbg ctl 0x708
#define CLKGEN_CORE_SLICE_DBG_CTL_CORE_SLICE_DBG_SEL_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_CORE_SLICE_DBG_CTL_CORE_SLICE_DBG_SEL_SHIFT                (0U)
#define CLKGEN_CORE_SLICE_DBG_CTL_CORE_SLICE_DBG_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_DBG_CTL_CORE_SLICE_DBG_SEL_SHIFT)) & CLKGEN_CORE_SLICE_DBG_CTL_CORE_SLICE_DBG_SEL_MASK)

#define CLKGEN_CORE_SLICE_DBG_CTL_RESERVED_MASK     ((uint32_t) (0xffff<< 16))
#define CLKGEN_CORE_SLICE_DBG_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_CORE_SLICE_DBG_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_DBG_CTL_RESERVED_SHIFT)) & CLKGEN_CORE_SLICE_DBG_CTL_RESERVED_MASK)
//clkgen dbg ctl 0x70C
#define CLKGEN_DBG_CTL_DBG_DIV_NUM_MASK     ((uint32_t) (0xffff<< 0))
#define CLKGEN_DBG_CTL_DBG_DIV_NUM_SHIFT                (0U)
#define CLKGEN_DBG_CTL_DBG_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_DBG_CTL_DBG_DIV_NUM_SHIFT)) & CLKGEN_DBG_CTL_DBG_DIV_NUM_MASK)
#define CLKGEN_DBG_CTL_RESERVED_MASK        ((uint32_t) (0x1fff<< 16))
#define CLKGEN_DBG_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_DBG_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_DBG_CTL_RESERVED_SHIFT)) & CLKGEN_DBG_CTL_RESERVED_MASK)
#define CLKGEN_DBG_CTL_DBG_SEL_MASK     ((uint32_t) (0x3<< 29))
#define CLKGEN_DBG_CTL_DBG_SEL_SHIFT                (29U)
#define CLKGEN_DBG_CTL_DBG_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_DBG_CTL_DBG_SEL_SHIFT)) & CLKGEN_DBG_CTL_DBG_SEL_MASK)
#define CLKGEN_DBG_CTL_DBG_CLK_DIS_MASK     ((uint32_t) (1<< 31))
#define CLKGEN_DBG_CTL_DBG_CLK_DIS_SHIFT                (31U)
#define CLKGEN_DBG_CTL_DBG_CLK_DIS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_DBG_CTL_DBG_CLK_DIS_SHIFT)) & CLKGEN_DBG_CTL_DBG_CLK_DIS_MASK)
//clkgen ip slice mon ctl 0x720
#define CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL_SHIFT                (0U)
#define CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL_SHIFT)) & CLKGEN_IP_SLICE_MON_CTL_IP_SLICE_MON_SEL_MASK)
#define CLKGEN_IP_SLICE_MON_CTL_RESERVED_MASK       ((uint32_t) (0xffff<< 16))
#define CLKGEN_IP_SLICE_MON_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_IP_SLICE_MON_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_IP_SLICE_MON_CTL_RESERVED_SHIFT)) & CLKGEN_IP_SLICE_MON_CTL_RESERVED_MASK)
//clkgen bus slice mon ctl 0x724
#define CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL_MASK     ((uint32_t) (0xffff<< 0))
#define CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL_SHIFT                (0U)
#define CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL_SHIFT)) & CLKGEN_BUS_SLICE_MON_CTL_BUS_SLICE_MON_SEL_MASK)
#define CLKGEN_BUS_SLICE_MON_CTL_RESERVED_MASK      ((uint32_t) (0xffff<< 16))
#define CLKGEN_BUS_SLICE_MON_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_BUS_SLICE_MON_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_BUS_SLICE_MON_CTL_RESERVED_SHIFT)) & CLKGEN_BUS_SLICE_MON_CTL_RESERVED_MASK)
//clkgen core slice mon ctl 0x728
#define CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL_SHIFT                (0U)
#define CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL_SHIFT)) & CLKGEN_CORE_SLICE_MON_CTL_CORE_SLICE_MON_SEL_MASK)
#define CLKGEN_CORE_SLICE_MON_CTL_RESERVED_MASK     ((uint32_t) (0xffff<< 16))
#define CLKGEN_CORE_SLICE_MON_CTL_RESERVED_SHIFT                (16U)
#define CLKGEN_CORE_SLICE_MON_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_CORE_SLICE_MON_CTL_RESERVED_SHIFT)) & CLKGEN_CORE_SLICE_MON_CTL_RESERVED_MASK)
//clkgen mon ctl 0x72C
#define CLKGEN_MON_CTL_MON_DIV_NUM_MASK     ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_CTL_MON_DIV_NUM_SHIFT                (0U)
#define CLKGEN_MON_CTL_MON_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_DIV_NUM_SHIFT)) & CLKGEN_MON_CTL_MON_DIV_NUM_MASK)
#define CLKGEN_MON_CTL_MON_DIV_BUSY_MASK        ((uint32_t) (1<< 16))
#define CLKGEN_MON_CTL_MON_DIV_BUSY_SHIFT                (16U)
#define CLKGEN_MON_CTL_MON_DIV_BUSY(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_DIV_BUSY_SHIFT)) & CLKGEN_MON_CTL_MON_DIV_BUSY_MASK)
#define CLKGEN_MON_CTL_RESERVED_MASK        ((uint32_t) (0xff<< 17))
#define CLKGEN_MON_CTL_RESERVED_SHIFT                (17U)
#define CLKGEN_MON_CTL_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_RESERVED_SHIFT)) & CLKGEN_MON_CTL_RESERVED_MASK)
#define CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK        ((uint32_t) (1<< 25))
#define CLKGEN_MON_CTL_CLK_SLOW_SRC_SHIFT                (25U)
#define CLKGEN_MON_CTL_CLK_SLOW_SRC(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_CLK_SLOW_SRC_SHIFT)) & CLKGEN_MON_CTL_CLK_SLOW_SRC_MASK)
#define CLKGEN_MON_CTL_MON_CLK_DIS_STA_MASK     ((uint32_t) (1<< 26))
#define CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT                (26U)
#define CLKGEN_MON_CTL_MON_CLK_DIS_STA(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_CLK_DIS_STA_SHIFT)) & CLKGEN_MON_CTL_MON_CLK_DIS_STA_MASK)
#define CLKGEN_MON_CTL_FREQ_RDY_STA_MASK        ((uint32_t) (1<< 27))
#define CLKGEN_MON_CTL_FREQ_RDY_STA_SHIFT                (27U)
#define CLKGEN_MON_CTL_FREQ_RDY_STA(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_FREQ_RDY_STA_SHIFT)) & CLKGEN_MON_CTL_FREQ_RDY_STA_MASK)
#define CLKGEN_MON_CTL_MON_VAL_UPD_EN_MASK      ((uint32_t) (1<< 28))
#define CLKGEN_MON_CTL_MON_VAL_UPD_EN_SHIFT                (28U)
#define CLKGEN_MON_CTL_MON_VAL_UPD_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_VAL_UPD_EN_SHIFT)) & CLKGEN_MON_CTL_MON_VAL_UPD_EN_MASK)
#define CLKGEN_MON_CTL_MON_SEL_MASK     ((uint32_t) (0X3<< 29))
#define CLKGEN_MON_CTL_MON_SEL_SHIFT                (29U)
#define CLKGEN_MON_CTL_MON_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_SEL_SHIFT)) & CLKGEN_MON_CTL_MON_SEL_MASK)
#define CLKGEN_MON_CTL_MON_CLK_DIS_MASK     ((uint32_t) (1<< 31))
#define CLKGEN_MON_CTL_MON_CLK_DIS_SHIFT                (31U)
#define CLKGEN_MON_CTL_MON_CLK_DIS(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_CTL_MON_CLK_DIS_SHIFT)) & CLKGEN_MON_CTL_MON_CLK_DIS_MASK)
//clkgen mon max freq 0x730
#define CLKGEN_MON_MAX_FREQ_MON_MAX_FREQ_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_MAX_FREQ_MON_MAX_FREQ_SHIFT                (0U)
#define CLKGEN_MON_MAX_FREQ_MON_MAX_FREQ(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MAX_FREQ_MON_MAX_FREQ_SHIFT)) & CLKGEN_MON_MAX_FREQ_MON_MAX_FREQ_MASK)
#define CLKGEN_MON_MAX_FREQ_RESERVED_MASK       ((uint32_t) (0xffff<< 16))
#define CLKGEN_MON_MAX_FREQ_RESERVED_SHIFT                (16U)
#define CLKGEN_MON_MAX_FREQ_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MAX_FREQ_RESERVED_SHIFT)) & CLKGEN_MON_MAX_FREQ_RESERVED_MASK)
//clkgen mon ave freq 0x734
#define CLKGEN_MON_AVE_FREQ_MON_AVE_FREQ_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_AVE_FREQ_MON_AVE_FREQ_SHIFT                (0U)
#define CLKGEN_MON_AVE_FREQ_MON_AVE_FREQ(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_AVE_FREQ_MON_AVE_FREQ_SHIFT)) & CLKGEN_MON_AVE_FREQ_MON_AVE_FREQ_MASK)
#define CLKGEN_MON_AVE_FREQ_RESERVED_MASK       ((uint32_t) (0xffff<< 16))
#define CLKGEN_MON_AVE_FREQ_RESERVED_SHIFT                (16U)
#define CLKGEN_MON_AVE_FREQ_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_AVE_FREQ_RESERVED_SHIFT)) & CLKGEN_MON_AVE_FREQ_RESERVED_MASK)
//clkgen mon min freq 0x738
#define CLKGEN_MON_MIN_FREQ_MON_MIN_FREQ_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_MIN_FREQ_MON_MIN_FREQ_SHIFT                (0U)
#define CLKGEN_MON_MIN_FREQ_MON_MIN_FREQ(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MIN_FREQ_MON_MIN_FREQ_SHIFT)) & CLKGEN_MON_MIN_FREQ_MON_MIN_FREQ_MASK)
#define CLKGEN_MON_MIN_FREQ_RESERVED_MASK       ((uint32_t) (0xffff<< 16))
#define CLKGEN_MON_MIN_FREQ_RESERVED_SHIFT                (16U)
#define CLKGEN_MON_MIN_FREQ_RESERVED(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MIN_FREQ_RESERVED_SHIFT)) & CLKGEN_MON_MIN_FREQ_RESERVED_MASK)
//clkgen mon max duty 0x73C
#define CLKGEN_MON_MAX_DUTY_MON_MAX_HIGH_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_MAX_DUTY_MON_MAX_HIGH_SHIFT                (0U)
#define CLKGEN_MON_MAX_DUTY_MON_MAX_HIGH(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MAX_DUTY_MON_MAX_HIGH_SHIFT)) & CLKGEN_MON_MAX_DUTY_MON_MAX_HIGH_MASK)
#define CLKGEN_MON_MAX_DUTY_MON_MAX_PERIOD_MASK     ((uint32_t) (0xffff<< 16))
#define CLKGEN_MON_MAX_DUTY_MON_MAX_PERIOD_SHIFT                (16U)
#define CLKGEN_MON_MAX_DUTY_MON_MAX_PERIOD(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MAX_DUTY_MON_MAX_PERIOD_SHIFT)) & CLKGEN_MON_MAX_DUTY_MON_MAX_PERIOD_MASK)
//clkgen mon min duty 0x740
#define CLKGEN_MON_MIN_DUTY_MON_MIN_HIGH_MASK       ((uint32_t) (0xffff<< 0))
#define CLKGEN_MON_MIN_DUTY_MON_MIN_HIGH_SHIFT                (0U)
#define CLKGEN_MON_MIN_DUTY_MON_MIN_HIGH(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MIN_DUTY_MON_MIN_HIGH_SHIFT)) & CLKGEN_MON_MIN_DUTY_MON_MIN_HIGH_MASK)
#define CLKGEN_MON_MIN_DUTY_MON_MIN_PERIOD_MASK     ((uint32_t) (0xffff<< 16))
#define CLKGEN_MON_MIN_DUTY_MON_MIN_PERIOD_SHIFT                (16U)
#define CLKGEN_MON_MIN_DUTY_MON_MIN_PERIOD(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_MON_MIN_DUTY_MON_MIN_PERIOD_SHIFT)) & CLKGEN_MON_MIN_DUTY_MON_MIN_PERIOD_MASK)

//clkgen fsrefclk mask
#define CLKGEN_FSREFCLK_SCR_SEL_MASK        ((uint32_t) (0x3<< 0))
#define CLKGEN_FSREFCLK_SCR_SEL_SHIFT                (0U)
#define CLKGEN_FSREFCLK_SCR_SEL(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_FSREFCLK_SCR_SEL_SHIFT)) & CLKGEN_FSREFCLK_SCR_SEL_MASK)
#define CLKGEN_FSREFCLK_XTAL_CG_EN_MASK     ((uint32_t) (1<< 2))
#define CLKGEN_FSREFCLK_XTAL_CG_EN_SHIFT                (2U)
#define CLKGEN_FSREFCLK_XTAL_CG_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_FSREFCLK_XTAL_CG_EN_SHIFT)) & CLKGEN_FSREFCLK_XTAL_CG_EN_MASK)
#define CLKGEN_FSREFCLK_FS_RC_EN_MASK       ((uint32_t) (1<< 3))
#define CLKGEN_FSREFCLK_FS_RC_EN_SHIFT                (3U)
#define CLKGEN_FSREFCLK_FS_RC_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_FSREFCLK_FS_RC_EN_SHIFT)) & CLKGEN_FSREFCLK_FS_RC_EN_MASK)
#define CLKGEN_FSREFCLK_FS_XTAL_EN_MASK     ((uint32_t) (1<< 4))
#define CLKGEN_FSREFCLK_FS_XTAL_EN_SHIFT                (4U)
#define CLKGEN_FSREFCLK_FS_XTAL_EN(x)                   (((uint32_t)(((uint32_t)(x)) << CLKGEN_FSREFCLK_FS_XTAL_EN_SHIFT)) & CLKGEN_FSREFCLK_FS_XTAL_EN_MASK)

//offset
#define CLKGEN_IP_SLICE_CTL_OFF(ip_idx)    ((0x4*(ip_idx)) << 10)
#define CLKGEN_BUS_SLICE_CTL_OFF(bus_idx)   ((0x200U + 0x8*(bus_idx))<<10)
#define CLKGEN_BUS_SLICE_GASKET_OFF(bus_idx)    ((0x204U + 0x8*(bus_idx))<<10)
#define CLKGEN_CORE_SLICE_CTL_OFF(core_idx) ((0x300U + 0x4*(core_idx))<<10)
#define CLKGEN_UUU_SLICE_WRAPPER_OFF(uuu_idx)   ((0x600U + 0x4*(uuu_idx))<<10)
#define CLKGEN_LVDS_WRAPPER_OFF(lvds_idx)   (0x10004+0x10000*lvds_idx)
#define CLKGEN_LP_GATING_EN_OFF(gating_idx) ((0x400U + 0x4*(gating_idx))<<10)
#define CLKGEN_FSREFCLK_OFF(scr_idx)    ((0x200U + 0x4*(scr_idx))<<10)
#define CLKGEN_IP_SLICE_DBG_CTL_OFF    ((0x700) << 10)
#define CLKGEN_BUS_SLICE_DBG_CTL_OFF    ((0x704) << 10)
#define CLKGEN_CORE_SLICE_DBG_CTL_OFF   ((0x708) << 10)
#define CLKGEN_DBG_CTL_OFF  ((0x70C) << 10)
#define CLKGEN_IP_SLICE_MON_CTL_OFF ((0x720) << 10)
#define CLKGEN_BUS_SLICE_MON_CTL_OFF    ((0x724) << 10)
#define CLKGEN_CORE_SLICE_MON_CTL_OFF   ((0x728) << 10)
#define CLKGEN_MON_CTL_OFF  ((0x72C) << 10)
#define CLKGEN_MON_MAX_FREQ_OFF ((0x730) << 10)
#define CLKGEN_MON_AVE_FREQ_OFF ((0x734) << 10)
#define CLKGEN_MON_MIN_FREQ_OFF ((0x738) << 10)
#define CLKGEN_MON_MAX_DUTY_OFF ((0x73C) << 10)
#define CLKGEN_MON_MIN_DUTY_OFF ((0x740) << 10)
#define CLKGEN_REG_OFF(ref_off)    ((ref_off) << 10)

// Check the arguments.
#define CLKGEN_ASSERT_PARAMETER(base)                      \
if((base != CKGEN_SAF_BASE) \
    && (base != CKGEN_SEC_BASE) \
    && (base != CKGEN_SOC_BASE) \
    && (base != CKGEN_DISP_BASE))   \
{   \
    LTRACEF("base paramenter error \n");    \
    return false;   \
}   \

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/
/*clkgen bus clock a_b_sel*/
typedef enum _clkgen_bus_clk_a_b_sel {
    bus_clk_a_b_sel_a = 0U,
    bus_clk_a_b_sel_b = 1U,
} clkgen_bus_clk_a_b_sel;

/*clkgen core clock a_b_sel*/
typedef enum _clkgen_core_clk_a_b_sel {
    core_clk_a_b_sel_a = 0U,
    core_clk_a_b_sel_b = 1U,
} clkgen_core_clk_a_b_sel;

/*clkgen slice monitor clk type*/
typedef enum _clkgen_slice_mon_type {
    slice_mon_ip_clk = 0U,
    slice_mon_bus_clk = 1U,
    slice_mon_core_clk = 2U,
} clkgen_slice_mon_type;

/*clkgen select reference clock*/
typedef enum _clkgen_slice_mon_ref_clk {
    slice_mon_ref_clk_24M = 0U,
    slice_mon_ref_clk_32K = 1U,
} clkgen_slice_mon_ref_clk_type;

/*clkgen select reference clock*/
typedef enum _clkgen_slice_mon_ret {
    mon_max_freq = 0U,
    mon_avg_freq = 1U,
    mon_min_freq = 2U,
    mon_max_duty = 3U,
    mon_min_duty = 4U,
} clkgen_slice_mon_ret_type;

/*clkgen select reference clock*/
typedef enum _clkgen_debug_module {
    debug_module_ip = 0U,
    debug_module_bus = 1U,
    debug_module_core = 2U,
    debug_module_uuu = 3U,
    debug_module_max,
} clkgen_debug_module_type;

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct _clkgen_gasket_type {
    uint8_t m_div_num;
    uint8_t n_div_num;
    uint8_t p_div_num;
    uint8_t q_div_num;
} clkgen_gasket_type_t;

typedef struct _clkgen_bus_slice_drv {
    uint32_t bus_slice_idx;
    uint8_t clk_a_b_switch;//0:select a;1:select b
    uint8_t clk_src_sel_a;
    uint8_t clk_src_sel_b;
    uint8_t pre_div_a;
    uint8_t pre_div_b;
    uint8_t post_div;
    clkgen_gasket_type_t gasket_cfg;
} clkgen_bus_slice_drv_t;

typedef struct _clkgen_core_slice_drv {
    uint32_t core_slice_idx;
    uint8_t clk_a_b_switch;//0:select a;1:select b
    uint8_t clk_src_sel_a;
    uint8_t clk_src_sel_b;
    uint8_t post_div;
} clkgen_core_slice_drv_t;

typedef struct _clkgen_default_cfg {
    uint8_t src_sel_mask;
    bool safety_mode;
} clkgen_default_cfg_t;

typedef union {
    struct  {
        u32 cg_en_a: 1;
        u32 src_sel_a: 3;
        u32 reserved1: 5;
        u32 a_b_sel: 1;
        u32 post_div_num: 6;
        u32 cg_en_b: 1;
        u32 src_sel_b: 3;
        u32 reserved2: 7;
        u32 cg_en_b_status: 1;
        u32 cg_en_a_status: 1;
        u32 reserved3: 2;
        u32 post_busy: 1;
    };

    u32 val;
} clkgen_core_ctl;

typedef union {

    struct   {
        u32 cg_en_a: 1;
        u32 src_sel_a: 3;
        u32 pre_div_num_a: 3;
        u32 reserved1: 2;
        u32 a_b_sel: 1;
        u32 post_div_num: 6;
        u32 cg_en_b: 1;
        u32 src_sel_b: 3;
        u32 pre_div_num_b: 3;
        u32 reserved2: 4;
        u32 cg_en_b_status: 1;
        u32 cg_en_a_status: 1;
        u32 pre_busy_b: 1;
        u32 pre_busy_a: 1;
        u32 post_busy: 1;
    };
    u32 val;
} clkgen_bus_ctl;

typedef union {

    struct   {
        u32 q_div_num: 3;
        u32 p_div_num: 3;
        u32 n_div_num: 3;
        u32 m_div_num: 3;
        u32 reserved: 16;
        u32 div_q_busy: 1;
        u32 div_p_busy: 1;
        u32 div_n_busy: 1;
        u32 div_m_busy: 1;
    };
    u32 val;
} clkgen_bus_gasket;


typedef union {

    struct  {
        u32 cg_en: 1;
        u32 src_sel: 3;
        u32 pre_div_num: 3;
        u32 reserved1: 3;
        u32 post_div_num: 6;
        u32 reserved2: 12;
        u32 cg_en_status: 1;
        u32 reserved3: 1;
        u32 pre_busy: 1;
        u32 post_busy: 1;
    };
    u32 val;
} clkgen_ip_ctl;

typedef union {
    struct  {
        u32 q_div: 4;
        u32 p_div: 4;
        u32 n_div: 4;
        u32 m_div: 4;
        u32 uuu_sel0: 1;
        u32 uuu_sel1: 1;
        u32 reserved1: 2;
        u32 uuu_gating_ack: 1;
        u32 reserved2: 6;
        u32 dbg_div: 4;
        u32 dbg_gating_en: 1;
    };

    u32 val;
} clkgen_uuu_ctl;


bool clkgen_get_default_config(clkgen_default_cfg_t *def_cfg);
bool clkgen_fsrefclk_sel(vaddr_t base, uint32_t scr_idx,
                         uint32_t src_sel_mask, bool safety_mode);
bool clkgen_gating_enable(vaddr_t base, uint16_t gating_idx, bool enable);
bool clkgen_ip_slice_set(vaddr_t base, uint8_t ip_slice_idx,
                         uint8_t clk_src_sel, uint16_t pre_div, uint16_t post_div);
bool clkgen_ip_ctl_get(vaddr_t base, uint32_t slice_idx,
                       clkgen_ip_ctl *ctl);
bool clkgen_ip_ctl_set(vaddr_t base, uint32_t slice_idx,
                       const clkgen_ip_ctl *ctl);
bool clkgen_bus_slice_switch(vaddr_t base,
                             clkgen_bus_slice_drv_t *bus_clk_cfg);
bool clkgen_bus_ctl_get(vaddr_t base, uint32_t slice_idx,
                        clkgen_bus_ctl *ctl,
                        clkgen_bus_gasket *gasket);
bool clkgen_bus_ctl_set(vaddr_t base, uint32_t slice_idx,
                        const clkgen_bus_ctl *ctl,
                        const clkgen_bus_gasket *gasket);
bool clkgen_core_slice_switch(vaddr_t base,
                              clkgen_core_slice_drv_t *core_clk_cfg);
bool clkgen_core_ctl_get(vaddr_t base, uint32_t slice_idx,
                         clkgen_core_ctl *ctl);
bool clkgen_core_ctl_set(vaddr_t base, uint32_t slice_idx,
                         const clkgen_core_ctl *ctl);
bool clkgen_uuu_clock_wrapper(vaddr_t base, uint16_t uuu_clock_wrapper_idx,
                              clkgen_gasket_type_t *gasket_div, bool low_power_mode,
                              uint8_t input_clk_type);
bool clkgen_uuu_ctl_get(vaddr_t base, uint32_t slice_idx,
                        clkgen_uuu_ctl *ctl);
bool clkgen_uuu_ctl_set(vaddr_t base, uint32_t slice_idx,
                        const clkgen_uuu_ctl *ctl);

uint32_t clkgen_mon_ip_slice(vaddr_t base, uint16_t ip_slice_idx,
                             clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                             clkgen_slice_mon_ret_type ret_type);
uint32_t clkgen_mon_bus_slice(vaddr_t base, uint16_t bus_slice_idx,
                              clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                              clkgen_slice_mon_ret_type ret_type);
uint32_t clkgen_mon_core_slice(vaddr_t base, uint16_t core_slice_idx,
                               clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                               clkgen_slice_mon_ret_type ret_type);
bool clkgen_ipslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                 uint8_t dbg_div);
bool clkgen_ipslice_debug_disable(vaddr_t base);
bool clkgen_busslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div);
bool clkgen_busslice_debug_disable(vaddr_t base);
bool clkgen_coreslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                   uint8_t dbg_div);
bool clkgen_coreslice_debug_disable(vaddr_t base);
bool clkgen_uuuslice_debug_enable(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div);
bool clkgen_uuuslice_debug_disable(vaddr_t base, uint16_t slice_idx);
#endif
