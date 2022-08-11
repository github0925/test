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
#include <__regs_base.h>

#define PERF_MON_BASE (vaddr_t)_ioaddr(APB_DDR_PERF_MON_BASE)

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

#define PFM_RECORD_MAGIC    0x5a5a5a5a
#define PFM_MON_HEAD_OFFSET    0x1000
#define PFM_RECORD_BUFFER_SIZE  PFM_POOL_SIZE

#define PFM_RECORD_BUFFER    (vaddr_t)_ioaddr((addr_t)pfm_mem_pool)
static uint8_t pfm_mem_pool[PFM_RECORD_BUFFER_SIZE];

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

static inline uint32_t pfm_enable_group(uint32_t nr)
{
    if (nr > 16)
        return -1;
    writel(BIT_MASK(nr), PFM_MON_GROUP);
    return 0;
}

static inline uint32_t pfm_group_nr(void)
{
    unsigned long ret = readl(PFM_MON_GROUP) & 0xffff;
    for (int i = 15; i >= 0; i--)
    {
        if (bitmap_test(&ret, i))
        {
            return i + 1;
        }
    }
    return 0;
}

/* Time in ms */
static inline uint32_t pfm_cfg_time_window(uint32_t time)
{
    writel(time, PFM_MON_BASE_CNT_CMP);
    return 0;
}

static inline uint32_t pfm_master_observer(uint32_t group, pfm_observer_t *observer)
{
    if (group > 16)
        return -1;
    writel((observer->master0)<<6, PFM_MON_AXI_ID0(group));
    writel((observer->msk0)<<6, PFM_MON_AXI_ID0_MSK(group));

    writel((observer->master1)<<6, PFM_MON_AXI_ID1(group));
    writel((observer->msk1)<<6, PFM_MON_AXI_ID1_MSK(group));

    return 0;
}

static inline uint32_t pfm_stop_trigger(uint32_t group, pfm_stop_condition_t *condition)
{
    if (group > 8)
        return -1;
    writel(condition->rd_thr, PFM_MON_AXI_RD_THR(group));
    writel(condition->wr_thr, PFM_MON_AXI_WR_THR(group));

    writel(condition->rd_bcnt_thr, PFM_MON_AXI_RD_BCNT_THR(group));
    writel(condition->wr_bcnt_thr, PFM_MON_AXI_WR_BCNT_THR(group));

    return 0;
}

struct pfm_record_irq {
    uint32_t rd_cnt;
    uint32_t wr_cnt;
    uint32_t rd_bcntl;
    uint32_t rd_bcnth;
    uint32_t wr_bcntl;
    uint32_t wr_bcnth;
};

static inline uint32_t pfm_transaction_record(uint32_t nr_group, struct pfm_record_irq *record)
{
    if (nr_group > 16)
        return -1;

    arch_invalidate_cache_range(PFM_MON_AXI_RD_CNT(0), nr_group * 24);
    for (uint32_t i = 0; i < nr_group; i++)
    {
        record[i].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
        record[i].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
        record[i].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
        record[i].rd_bcnth = readl(PFM_MON_AXI_RD_BCNTH(i));
        record[i].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
        record[i].wr_bcnth = readl(PFM_MON_AXI_WR_BCNTH(i));
    }
    return 0;
}

/* Enable pfm */
static inline void pfm_enable_counter(void)
{
    /* Clear base counter */
    writel(2, PFM_MON_BASE_CNT_CTL);
    /* Enable base counter */
    writel(1, PFM_MON_BASE_CNT_CTL);
}

static inline void pfm_disable_counter(void)
{
    writel(0, PFM_MON_BASE_CNT_CTL);
}

static inline int pfm_is_running(void)
{
    return !!(readl(PFM_MON_BASE_CNT_CTL) & 0x1);
}

static inline int pfm_timeout_irq(void)
{
    return !!(readl(PFM_MON_MISC_ST) & 0x10);
}

static inline void pfm_enable_irq_mode(void)
{
    /* Enable base counter timeout irq && auto restart */
    uint32_t ret = readl(PFM_MON_AXI_CNT_CTL1);
    ret |= 2;
    writel(ret, PFM_MON_AXI_CNT_CTL1);
    /* Enable timeout irq  */
    ret = readl(PFM_MON_MISC_INT_CTL);
    ret |= 1;
    writel(ret, PFM_MON_MISC_INT_CTL);
}

static inline void pfm_enable_dma_mode(void)
{
    /* Enable AXI threshold irq, auto restart */
    writel(1, PFM_MON_AXI_CNT_CTL1);
    /* Disable base counter timeout irq */
    uint32_t ret = readl(PFM_MON_MISC_INT_CTL);
    ret &= 0;
    writel(ret, PFM_MON_MISC_INT_CTL);
}

static inline void pfm_save_config(pfm_head_t *pfm_head)
{
    pfm_head->magic = PFM_RECORD_MAGIC;
    pfm_head->group_nr = pfm_group_nr();
    for (uint32_t i = 0; i < pfm_head->group_nr; i++)
    {
        pfm_head->observer_config[i].master0 = readl(PFM_MON_AXI_ID0(i)) >> 6;
        pfm_head->observer_config[i].msk0 = readl(PFM_MON_AXI_ID0_MSK(i)) >> 6;
        pfm_head->observer_config[i].master1 = readl(PFM_MON_AXI_ID1(i)) >> 6;
        pfm_head->observer_config[i].msk1 = readl(PFM_MON_AXI_ID1_MSK(i))  >> 6;
    }

    for (uint32_t i = 0; i < pfm_head->group_nr && i < 8; i++)
    {
        pfm_head->condition[i].rd_thr = readl(PFM_MON_AXI_RD_THR(i));
        pfm_head->condition[i].wr_thr = readl(PFM_MON_AXI_WR_THR(i));
        pfm_head->condition[i].rd_bcnt_thr = readl(PFM_MON_AXI_RD_BCNT_THR(i));
        pfm_head->condition[i].wr_bcnt_thr = readl(PFM_MON_AXI_WR_BCNT_THR(i));
    }

    pfm_head->mode = readl(PFM_MON_MISC_INT_CTL) & 1;
    pfm_head->reach_thr = 0;
}

#endif
