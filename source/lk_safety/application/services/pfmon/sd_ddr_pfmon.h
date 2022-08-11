/* __regs_sd_ddr_pfmon.h
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: implement semidrive pfm driver
 *
 **/

#ifndef __REGS_SD_DDR_PFMON_H
#define __REGS_SD_DDR_PFMON_H

#include <bits.h>
#include <reg.h>
#include <lib/reg.h>
#include <stdio.h>

#define APB_DDR_PERF_MON_BASE_ 0xf3020000

#define PERF_MON_BASE (vaddr_t)_ioaddr(APB_DDR_PERF_MON_BASE_)

#define PFM_MON_BASE_CNT_CTL       PERF_MON_BASE
#define PFM_MON_BASE_CNT_CMP       PERF_MON_BASE + 4

#define PFM_MON_MISC_ST            PERF_MON_BASE + 0xA0
#define PFM_MON_MISC_INT_CTL       PERF_MON_BASE + 0xA4
#define PFM_MON_AXI_CNT_CTL0       PERF_MON_BASE + 0x100
#define PFM_MON_AXI_CNT_CTL1       PERF_MON_BASE + 0x104

#define PFM_MON_AXI_RD_THR(i)      PERF_MON_BASE + 0x110 + i*16
#define PFM_MON_AXI_WR_THR(i)      PERF_MON_BASE + 0x114 + i*16
#define PFM_MON_AXI_RD_BCNT_THR(i) PERF_MON_BASE + 0x118 + i*16
#define PFM_MON_AXI_WR_BCNT_THR(i) PERF_MON_BASE + 0x11C + i*16

#define PFM_MON_AXI_ID0(i)         PERF_MON_BASE + 0x200 + i*16
#define PFM_MON_AXI_ID0_MSK(i)     PERF_MON_BASE + 0x204 + i*16
#define PFM_MON_AXI_ID1(i)         PERF_MON_BASE + 0x208 + i*16
#define PFM_MON_AXI_ID1_MSK(i)     PERF_MON_BASE + 0x20C + i*16

#define PFM_MON_AXI_RND_CNT        PERF_MON_BASE + 0x1000
#define PFM_MON_AXI_CNT_ST         PERF_MON_BASE + 0x1004

#define PFM_MON_AXI_RD_CNT(i)      PERF_MON_BASE + 0x1010 + i*24
#define PFM_MON_AXI_WR_CNT(i)      PERF_MON_BASE + 0x1014 + i*24
#define PFM_MON_AXI_RD_BCNTL(i)    PERF_MON_BASE + 0x1018 + i*24
#define PFM_MON_AXI_RD_BCNTH(i)    PERF_MON_BASE + 0x101C + i*24
#define PFM_MON_AXI_WR_BCNTL(i)    PERF_MON_BASE + 0x1020 + i*24
#define PFM_MON_AXI_WR_BCNTH(i)    PERF_MON_BASE + 0x1024 + i*24

#define PFM_MON_GROUP                    PFM_MON_AXI_CNT_CTL0


#define PFM_MON_ENABLE_SHIFT             (0U)
#define PFM_MON_ENABLE_MASK              (uint32_t)(1 << PFM_MON_ENABLE_SHIFT)

#define PFM_MON_CLEAR_REG_SHIFT          (1U)
#define PFM_MON_CLEAR_REG_MASK           (uint32_t)(1 << PFM_MON_CLEAR_REG_SHIFT)

#define PFM_MON_BASE_CNT_TIMEOUT_SHIFT   (4U)
#define PFM_MON_BASE_CNT_TIMEOUT_MASK    (uint32_t)(1 << PFM_MON_BASE_CNT_TIMEOUT_SHIFT)

#define PFM_MON_GROUP_SHIFT              (16U)
#define PFM_MON_GROUP_MSK                (uint32_t)(1 << PFM_MON_GROUP_SHIFT - 1)

#define PFM_MON_REACH_THR_SHIFT          (8U)
#define PFM_MON_REACH_THR_MSK            (uint32_t)(1 << PFM_MON_REACH_THR_SHIFT - 1)

#define PFM_MON_BCNTH_SHIFT              (8u)
#define PFM_MON_BCNTH_MSK                (uint32_t)(1 << PFM_MON_BCNTH_SHIFT - 1)

#if 0
#ifdef DDR_2133
#define time_window_per_ms 533250
#elif DDR_4266
#define time_window_per_ms 1066500
#endif
#endif

#define PFM_RECORD_ON_DDR 0

#if PFM_RECORD_ON_DDR

#define PFM_RECORD_MAGIC    0x5a5a5a5a
#define PFM_RECORD_MEM_ADDR    0x33200000
#define PFM_MON_HEAD_OFFSET    0x1000
#define PFM_RECORD_BUFFER    (vaddr_t)_ioaddr(PFM_RECORD_MEM_ADDR)
#define PFM_RECORD_BUFFER_SIZE  3*1024*1024

#endif

/* DDR_4266 */
#define time_window_per_ms 1066500

typedef struct pfm_observer{
    uint32_t master0;
    uint32_t msk0;
    uint32_t master1;
    uint32_t msk1;
} pfm_observer_t;

typedef struct pfm_stop_condition {
    uint32_t rd_thr;
    uint32_t wr_thr;
    uint32_t rd_bcnt_thr;
    uint32_t wr_bcnt_thr;
} pfm_stop_condition_t;

typedef struct pfm_head {
    uint32_t magic;
    uint32_t start_time;
    uint32_t time_window;
    uint32_t group_nr;
    pfm_observer_t observer_config[16];
    pfm_stop_condition_t condition[8];
    uint32_t mode;
    uint32_t rounds;
    uint32_t pool_size;
    uint32_t record_offset;
    uint32_t reach_thr;
} pfm_head_t;

struct pfm_record_irq {
    uint32_t rd_cnt;
    uint32_t wr_cnt;
    uint32_t rd_bcntl;
    uint32_t rd_bcnth;
    uint32_t wr_bcntl;
    uint32_t wr_bcnth;
};

enum pfm_master_id {
    SAF_PLATFORM,
    SEC_PLATFORM,
    MP_PLATFORM,
    AP1,
    AP2,
    VDSP,
    ADSP,
    RESERVED1,
    DMA1,
    DMA2,
    DMA3,
    DMA4,
    DMA5,
    DMA6,
    DMA7,
    DMA8,
    CSI1,
    CSI2,
    CSI3,
    DC1,
    DC2,
    DC3,
    DC4,
    DP1,
    DP2,
    DP3,
    DC5,
    G2D1,
    G2D2,
    VPU1,
    VPU2,
    MJPEG,
    MSHC1,
    MSHC2,
    MSHC3,
    MSHC4,
    ENET_QOS1,
    ENET_QOS2,
    USB1,
    USB2,
    AI,
    RESERVED2,
    RESERVED3,
    RESERVED4,
    RESERVED5,
    RESERVED6,
    RESERVED7,
    CE1,
    CE2_VCE1,
    CE2_VCE2,
    CE2_VCE3,
    CE2_VCE4,
    CE2_VCE5,
    CE2_VCE6,
    CE2_VCE7,
    CE2_VCE8,
    GPU1_OS1,
    GPU1_OS2,
    GPU1_OS3,
    GPU1_OS4,
    GPU1_OS5,
    GPU1_OS6,
    GPU1_OS7,
    GPU1_OS8,
    GPU2_OS1,
    GPU2_OS2,
    GPU2_OS3,
    GPU2_OS4,
    GPU2_OS5,
    GPU2_OS6,
    GPU2_OS7,
    GPU2_OS8,
    PTB,
    CSSYS,
    RESERVED8,
    RESERVED9,
    RESERVED10,
    RESERVED11,
    RESERVED12,
    RESERVED13,
    PCIE1_0,
    PCIE1_1,
    PCIE1_2,
    PCIE1_3,
    PCIE1_4,
    PCIE1_5,
    PCIE1_6,
    PCIE1_7,
    PCIE1_8,
    PCIE1_9,
    PCIE1_10,
    PCIE1_11,
    PCIE1_12,
    PCIE1_13,
    PCIE1_14,
    PCIE1_15,
    PCIE2_0,
    PCIE2_1,
    PCIE2_2,
    PCIE2_3,
    PCIE2_4,
    PCIE2_5,
    PCIE2_6,
    PCIE2_7,
    RESERVED14,
    RESERVED15,
    RESERVED16,
    RESERVED17,
    RESERVED18,
    RESERVED19,
    RESERVED20,
    RESERVED21,
    RESERVED22,
    RESERVED23,
    RESERVED24,
    RESERVED25,
    RESERVED26,
    RESERVED27,
    RESERVED28,
    RESERVED29,
    RESERVED30,
    RESERVED31,
    RESERVED32,
    RESERVED33,
    RESERVED34,
    RESERVED35,
    RESERVED36,
    RESERVED37
};

#endif
