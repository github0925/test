/*
 * pmu.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R Performance Monitor driver.
 *
 * Revision History:
 * -----------------
 */
#include <arch/arm.h>
#include <bits.h>
#include <debug.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum pmu_evt_cntr {
    PMU_EVT_COUNTER1  = 0,
    PMU_EVT_COUNTER2  = 1,
    PMU_EVT_COUNTER3  = 2,
} pmu_evt_cntr_e;

typedef enum pmu_evt {
    PMU_EVT_SOFTWARE_INCREMENT              = 0x0,
    PMU_EVT_ICACHE_MISS                     = 0x1,
    PMU_EVT_DCACHE_MISS                     = 0x3,
    PMU_EVT_DCACHE_ACCESS                   = 0x4,
    PMU_EVT_DATA_READ                       = 0x6,
    PMU_EVT_DATA_WRITE                      = 0x7,
    PMU_EVT_INSTRUCTION_EXECUTED            = 0x8,
    PMU_EVT_DUAL_ISSUED_INSTUCTIONS         = 0x5e,
    PMU_EVT_EXECPTION_TAKEN                 = 0x9,
    PMU_EVT_EXECPTION_RETURN                = 0xa,
    PMU_EVT_CHANGE_CONTEXT_ID               = 0xb,
    PMU_EVT_SW_CHANGE_PC                    = 0xc,
    PMU_EVT_BRANCH_IMMEDIATE                = 0xd,
    PMU_EVT_PROCEDURE_RETURN                = 0xe,
    PMU_EVT_UNALIGNED_ACCESS                = 0xf,
    PMU_EVT_BRANCH_NOT_PREDICTED            = 0x10,
    PMU_EVT_CYCLE_CNT                       = 0x11,
    PMU_EVT_BRANCH_PREDICTED                = 0x12,
    PMU_EVT_STALL_INSTRUCTION               = 0x40,
    PMU_EVT_STALL_DATA_DEPENDENCY           = 0x41,
    PMU_EVT_DCACHE_WRITE_BACK               = 0x42,
    PMU_EVT_EXT_MEMORY_REQ                  = 0x43,
    PMU_EVT_STALL_LSU_BUSY                  = 0x44,
    PMU_EVT_DRAIN_STORE_BUFFER              = 0x45,
    PMU_EVT_FIQ_DISABLE_CYCLES              = 0x46,
    PMU_EVT_IRQ_DISABLE_CYCLES              = 0x47,
    PMU_EVT_ETMEXTOUTM0                     = 0x48,
    PMU_EVT_ETMEXTOUTM1                     = 0x49,
    PMU_EVT_ICACHE_TAG_ECCERR               = 0x4a,
    PMU_EVT_ICACHE_DATA_ECCERR              = 0x4b,
    PMU_EVT_DCACHE_TAG_ECCERR               = 0x4c,
    PMU_EVT_DCACHE_DATA_ECCERR              = 0x4d,
    PMU_EVT_TCM_ECCERR_PFU                  = 0x4e,
    PMU_EVT_TCM_ECCERR_LSU                  = 0x4f,
    PMU_EVT_STORE_BUFFER_MERGE              = 0x50,
    PMU_EVT_LSU_STALL_BY_STORE_BUFFER       = 0x51,
    PMU_EVT_LSU_STALL_BY_STORE_QUEUE        = 0x52,
    PMU_EVT_INT_DIV                         = 0x53,
    PMU_EVT_STALL_CYCLE_BY_INT_DIV          = 0x54,
    PMU_EVT_PLD_LINEFILL                    = 0x55,
    PMU_EVT_PLD_NO_LINEFILL                 = 0x56,
    PMU_EVT_NON_CACHEABLE_AXI_ACCESS        = 0x57,
    PMU_EVT_ICACHE_ACCESS                   = 0x58,
    PMU_EVT_STORE_BUFFER_SLOT_ATTR_CONFLICT = 0x59,
    PMU_EVT_DUAL_ISSUE_CASE_A               = 0x5a,
    PMU_EVT_DUAL_ISSUE_CASE_B1_B2_F2_F2D    = 0x5b,
    PMU_EVT_DUAL_ISSUE_CASE_OTHER           = 0x5c,
    PMU_EVT_DOUBLE_PRECISION_FLOAT_EXEC     = 0x5d,
    PMU_EVT_DCACHE_DATA_FATAL_ECCERR        = 0x60,
    PMU_EVT_DCACHE_TAG_FATAL_ECCERR         = 0x61,
    PMU_EVT_PROCESSOR_LIVELOCK              = 0x62,
    PMU_EVT_ATCM_MB_ECCERR                  = 0x64,
    PMU_EVT_B0TCM_MB_ECCERR                 = 0x65,
    PMU_EVT_B1TCM_MB_ECCERR                 = 0x66,
    PMU_EVT_ATCM_SB_ECCERR                  = 0x67,
    PMU_EVT_B0TCM_SB_ECCERR                 = 0x68,
    PMU_EVT_B1TCM_SB_ECCERR                 = 0x69,
    PMU_EVT_TCM_CORRECTABLE_ECCERR_LSU      = 0x6a,
    PMU_EVT_TCM_CORRECTABLE_ECCERR_PFU      = 0x6b,
    PMU_EVT_TCM_FATAL_ECCERR_AXI_SLAVE      = 0x6c,
    PMU_EVT_TCM_CORRECTABLE_ECCERR_AXI_SLAVE = 0x6d,
    PMU_EVT_CORRECTABLE_EVENTS              = 0x6e,
    PMU_EVT_FATAL_EVENTS                    = 0x6f,
    PMU_EVT_CORRECTABLE_BUS_FAULTS          = 0x70,
    PMU_EVT_FATAL_BUS_FAULTS                = 0x71,
    PMU_EVT_ACP_DCACHE_ACCESS               = 0x72,
    PMU_EVT_ACP_DCACHE_INVALIDATE           = 0x73,
} pmu_evt_e;

#define PMCR_E      (1 << 0)        /* pmu enable */
#define PMCR_P      (1 << 1)        /* event counter reset */
#define PMCR_C      (1 << 2)        /* cycle counter reset */
#define PMCR_D      (1 << 3)        /* count every 64 clock cycles */

/* Enable the PMU. All counters are cleared. */
static inline void pmu_enable(void)
{
    uint32_t val = arm_read_pmcr();
    val |= PMCR_P | PMCR_C | PMCR_E;
    arm_write_pmcr(val);
}

/* Diable the PMU. All counters are cleared. */
static inline void pmu_disable(void)
{
    uint32_t val = arm_read_pmcr();
    val &= ~(PMCR_P | PMCR_C | PMCR_E);
    arm_write_pmcr(val);
}

/* Start cycle counter. Set div64 = true to count every
 * 64 clock cycles.
 */
static inline void pmu_start_cycle_cntr(bool div64)
{
    uint32_t val = arm_read_pmcr();

    if (div64)
        val |= PMCR_D;
    else
        val &= ~PMCR_D;

    arm_write_pmcr(val);
    arm_write_pmcntenset(1 << 31);
}

/* Stop cycle counter. */
static inline void pmu_stop_cycle_cntr(void)
{
    arm_write_pmcntenclr(1 << 31);
}

/* Stop and clear the cycle counter. */
static inline void pmu_stop_clear_cycle_cntr(void)
{
    pmu_stop_cycle_cntr();
    arm_write_pmccntr(0);
}

/* Get cycle counter value. */
static inline uint32_t pmu_get_cycle_cntr(void)
{
    return arm_read_pmccntr();
}

/* Bind event counter to specific event and start it. */
static inline void pmu_start_evt_cntr(pmu_evt_cntr_e cntr,
                                      pmu_evt_e evt)
{
    arm_write_pmselr(cntr);
    arm_write_pmxevtyper(evt);
    arm_write_pmcntenset(1 << cntr);
}

/* Stop event counter without clearing. */
static inline void pmu_stop_evt_cntr(pmu_evt_cntr_e cntr)
{
    arm_write_pmcntenclr(1 << cntr);
}

/* Stop and clear the event counter. */
static inline void pmu_stop_clear_evt_cntr(pmu_evt_cntr_e cntr)
{
    pmu_stop_evt_cntr(cntr);
    arm_write_pmselr(cntr);
    arm_write_pmxevcntr(0);
}

/* Get event counter value. */
static inline uint32_t pmu_get_evt_cntr(pmu_evt_cntr_e cntr)
{
    arm_write_pmselr(cntr);
    return arm_read_pmxevcntr();
}
