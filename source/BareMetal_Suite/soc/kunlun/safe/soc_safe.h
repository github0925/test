/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __SOC_SAFE_H__
#define __SOC_SAFE_H__

#define MID_SELF                MID_SAF_PLAT
#define MID_COUTERPART          MID_SEC_PLAT

#define MB_CPU_SELF         MB_CPU_CR5_SAF
#define MB_CPU_FRD          MB_CPU_CR5_SEC
#define MB_MSG_ID_TX           0
#define MB_MSG_ID_RX           1

#include "__regs_base.h"

#define TCM_SYSTEM_ADDR        SAF_TCM_SYSTEM_ADDR

#define IRAM_BASE   IRAM1_BASE
#define IRAM_SIZE   0x40000
#define IRAM_END    (IRAM_BASE + IRAM_SIZE - 1)

#define CE_SMEM_BASE    CE1_MEM_BASE

#define FUSE_ACC_DOM    FUSE_ACC_SAFE

#define SOC_GET_ROMC_BASE()    APB_ROMC1_BASE

#define SCR_BASE    APB_SCR_SAF_BASE

#define ROM_BLD "kunlun_safe"

#define FUSE_WDOG_EN()  FUSE_SAFETY_WDOG_EN()

void soc_handover_resources(void);

#define EIC_BASE    APB_EIC_SAF_BASE
#define EIC_EN_ID   33

#define SEM_BASE    APB_SEM1_BASE

#define CE_SERAM_SIZE_in_KB     8
#define CE_SESRAM_SASIZE_in_KB    4

#define CKGEN_RBASE APB_CKGEN_SAF_BASE
#define OSPI_CLK_SLICE_ID   7

#include <scr/scr.h>
#if defined(TC_z1)      /* z1 may has no the latest ECO */
#define SOC_IS_PEER_ACCESSIBLE()    true
#else
#define SOC_IS_PEER_ACCESSIBLE() \
    (1 == scr_bit_get(APB_SCR_SAF_BASE, RO,\
                SCR_SAF_RSTGEN_SEC_MAIN_RST_B_AND_MAIN_GATING_EN_RO_START_BIT))
#endif

#define UART_ROOT_CLK_FREQ  80000000u

#define MB_ID_THIS_CPU   MB_CPU_CR5_SAF
#endif  /* __SOC_SAFE_H__ */
