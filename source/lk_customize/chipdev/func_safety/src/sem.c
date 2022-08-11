/*
 * sem.c
 *
 * Copyright (c) 2019 semrive Semiconductor.
 * All rights reserved.
 *
 * Description: Driver for System Error Manager, which handles
 * system errors.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <compiler.h>
#include <reg.h>

#include "lib/reg.h"
#include "__regs_base.h"
#include "sem.h"

#define APB_SEM_BASE(sem) \
    (APB_SEM1_BASE + (sem) * (APB_SEM2_BASE - APB_SEM1_BASE))

#define COMMON_SET         (0x0)
#define INT_TRIG           (0x4)   /* trigger output intr 0~7 */
#define INT_ENALE_OVRD     (0x8)   /* enable output intr 0~7 */
#define INT_STATUS(sig)    (0x80 + (sig) / 32 * 4)
#define INT_ENABLE(intr, sig) \
                           (0x100 + (intr) * 0x80 + (sig) / 32 * 4)
#define TGT_VALUE(sig)     (0x500 + (sig) / 32 * 4)
#define MON_ENABLE(sig)    (0x580 + (sig) / 32 * 4)

#define INT_DC0_SEL         (0)  /* source DC output (0~3) of disp-ctrl 0. */
#define INT_DC1_SEL         (2)  /* source DC output (0~3) of disp-ctrl 1. */
#define INT_DC2_SEL         (4)  /* source DC output (0~3) of disp-ctrl 2. */
#define INT_DC3_SEL         (6)  /* source DC output (0~3) of disp-ctrl 3. */
#define INT_DC4_SEL         (8)  /* source DC output (0~3) of disp-ctrl 4. */
#define FATAL_HIGH_LOW      (10) /* fatal error (PAD1) high or low. */
#define ERROR_WAVE_SEL      (11) /* waveform A/B selection for normal/recoverable error. */
#define ERROR_DIV           (12) /* error divider ratio for waveform A. */
#define EI_BLOGAL_EN        (16) /* error injection global enable. */
#define EI_GLOBAL_EN_LOCK   (17) /* 0 to modify EI_GLOBAL_EN */
#define SIG_MON_GLB_EN      (23) /* signal monitor global enable. */
#define CMP_ENABLE          (24) /* compare output enable bits for int 0~7. */

#define INT_ENABLE_OVRD_EN  (0)
#define INT_ENABLE_OVRD_VAL (8)

static inline void sem_write_reg(enum sem sem, uint32_t reg,
                                 uint32_t val)
{
    writel(val, _ioaddr(APB_SEM_BASE(sem) + reg));
}

static inline uint32_t sem_read_reg(enum sem sem, uint32_t reg)
{
    return readl(_ioaddr(APB_SEM_BASE(sem) + reg));
}

static inline void sem_modify_reg(enum sem sem, uint32_t reg,
                                  uint32_t mask, uint32_t val)
{
    uint32_t _val = sem_read_reg(sem, reg);
    _val &= ~mask;
    _val |= val;
    sem_write_reg(sem, reg, _val);
}

void sem_enable_intr(enum sem sem, enum sem_intr intr,
                     bool enable)
{
    sem_write_reg(sem, INT_ENALE_OVRD,
                  1 << (intr + INT_ENABLE_OVRD_EN) |
                  (uint32_t)enable << (intr + INT_ENABLE_OVRD_VAL));
}

/*
 * Trigger interrupt on output.
 */
void sem_trigger_intr(enum sem sem, enum sem_intr intr)
{
    sem_write_reg(sem, INT_TRIG, 1 << intr);
}

/*
 * Check input signal status.
 */
bool sem_signal_status(enum sem sem, enum sem_signal signal)
{
    uint32_t val = sem_read_reg(sem, INT_STATUS(signal));
    return !!(val & (1 << (signal % 32)));
}

/*
 * Hundreds of SEM input signals are OR'ed to output interrupts.
 * This function is used to enable or disable the mapping from
 * input signals to SEM output interrupts.
 */
void sem_map_signal(enum sem sem, enum sem_signal signal,
                    enum sem_intr intr, bool enable)
{
    sem_modify_reg(sem, INT_ENABLE(intr, signal),
                   1 << (signal % 32), enable << (signal % 32));
}

/*
 * Enable or disable the SEM monitor.
 */
void sem_mon_enable(bool enable)
{
    sem_modify_reg(SEM1, COMMON_SET, 1 << SIG_MON_GLB_EN,
                   (uint32_t)enable << SIG_MON_GLB_EN);
}

/*
 * Enable or disable signal compare interrupts to SEM output
 * interrupt 0~7.
 *
 * Note - the SEM1 interrupt itself must also be enabled. See
 * function sem_enable_intr().
 */
void sem_mon_enable_intr(enum sem_intr intr,
                         bool enable)
{
    sem_modify_reg(SEM1, COMMON_SET, 1 << (intr + CMP_ENABLE),
                   (uint32_t)enable << (intr + CMP_ENABLE));
}

/*
 * The SEM internal monitor compares all sem_monitor_sig signals
 * with the target value (0 or 1) specified in this function. If
 * the comparision is enabled for the signal but the values
 * don't match, interrupt is triggered to output.
 *
 * compare - enable or disable comparision
 * target  - target signal value (required when "enable" is true.)
 */
void sem_mon_compare_sig(enum sem_monitor_sig signal,
                         bool compare, bool target)
{
    /* Enable or disable monitoring of specified signal. */
    sem_modify_reg(SEM1, MON_ENABLE(signal),
                   1 << (signal % 32),
                   (uint32_t)compare << (signal % 32));

    /* Set target if comparision is enabled. */
    if (compare) {
        sem_modify_reg(SEM1, TGT_VALUE(signal),
                       1 << (signal % 32),
                       (uint32_t)target << (signal % 32));
    }
}

/*
 * The Error Injection Controller (EIC) module is globally
 * enabled or disabled by SEM1.
 */
void sem_enable_eic(bool enable)
{
    sem_modify_reg(SEM1, COMMON_SET, 1 << EI_GLOBAL_EN_LOCK, 0);
    sem_modify_reg(SEM1, COMMON_SET, 1 << EI_BLOGAL_EN,
                   (uint32_t)enable << EI_BLOGAL_EN);
}
