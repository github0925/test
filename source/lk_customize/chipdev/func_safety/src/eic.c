/*
 * eic.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <compiler.h>
#include <reg.h>

#include "lib/reg.h"
#include "__regs_base.h"
#include "eic.h"

/* EIC registers. */

/* EN bits cannot be modified when LOCK is 1. Reset value is 0. */
#define LOCK                (0)

/* Each bit of EN register corresponds to one "injection point". */
#define EN(enable_bit)      (0x100 + (enable_bit) / 32 * 4)

/* Each bit of BIT register corresponds to one "injection signal"
   within an "injection point" */
#define BIT(inject_bit)     (0x200 + (inject_bit) / 32 * 4)

/* d_ip_bipc:sem_err_inject:r0p0 */
static uint32_t eic_addr[EIC_MAX] = {
    [EIC_SAF]   = APB_EIC_SAF_BASE,
    [EIC_SEC]   = APB_EIC_SEC_BASE,
    [EIC_HPI]   = APB_EIC_HPI_BASE,
    [EIC_VSN]   = EIC_VSN_BASE,
};

static inline void eic_write_reg(enum eic eic, uint32_t reg,
                                 uint32_t val)
{
    writel(val, _ioaddr(eic_addr[eic] + reg));
}

static inline uint32_t eic_read_reg(enum eic eic, uint32_t reg)
{
    return readl(_ioaddr(eic_addr[eic] + reg));
}

static inline void eic_modify_reg(enum eic eic, uint32_t reg,
                                  uint32_t mask, uint32_t val)
{
    uint32_t _val = eic_read_reg(eic, reg);
    _val &= ~mask;
    _val |= val;
    eic_write_reg(eic, reg, _val);
}

static inline uint32_t eic_get_bit(enum eic_signal signal)
{
    return signal & 0xfffful;
}

/*
 * Enable/disable inject point. Inject point must be enabled, before
 * a signal can be toggled by eic_inject().
 *
 * Injection signal bits are shared by all the injection points
 * connected to one a EIC module, so the user must make sure
 * only one injection point is enabled.
 */
void eic_enable_injectpoint(enum eic eic, uint32_t point, bool enable)
{
    eic_write_reg(eic, LOCK, 0);
    eic_modify_reg(eic, EN(point),
                   1ul << (point % 32),
                   (uint32_t)enable << (point % 32));
}

/*
 * Toggle error on specified EIC signal.
 */
void eic_inject(enum eic_signal signal, bool toggle)
{
    uint32_t inject_bit = eic_get_bit(signal);
    eic_modify_reg(eic_get_module(signal), BIT(inject_bit),
                   1ul << (inject_bit % 32),
                   (uint32_t)toggle << (inject_bit % 32));
}
