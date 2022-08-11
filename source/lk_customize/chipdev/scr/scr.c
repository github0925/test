/*
 * scr.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR driver.
 *
 * The SCR driver provides access to all the 3 SCR modules. It's
 * caller's responsibility to guarantee permission to the target
 * SCR module.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <reg.h>
#include <lk/init.h>
#include <kernel/spinlock.h>

#include "__regs_base.h"
#include "lib/reg.h"
#include "res.h"
#include "scr.h"

static spin_lock_t g_scr_lock;

/*
 * Get SCR signal value.
 */
uint32_t scr_get(scr_signal_t scr_signal)
{
    enum scr_id scr_id = _scr_id(scr_signal);
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&g_scr_lock, state);
    uint32_t val = _scr_read_reg(scr_id, _scr_reg(scr_signal));
    spin_unlock_irqrestore(&g_scr_lock, state);

    return (val >> _scr_start_bit(scr_signal)) &
            BIT_MASK(_scr_width(scr_signal));
}

/*
 * Set SCR signal value.
 */
bool scr_set(scr_signal_t scr_signal, uint32_t value)
{
    enum scr_id scr_id = _scr_id(scr_signal);
    uint32_t start_bit = _scr_start_bit(scr_signal);
    uint32_t reg = _scr_reg(scr_signal);
    uint32_t val = _scr_read_reg(scr_id, reg);
    spin_lock_saved_state_t state;
    bool ret = false;

    spin_lock_irqsave(&g_scr_lock, state);
    if (!scr_is_locked(scr_signal)) {
        val &= ~(BIT_MASK(_scr_width(scr_signal)) << start_bit);
        val |= value << start_bit;
        _scr_write_reg(scr_id, reg, val);
        ret = true;
    }
    spin_unlock_irqrestore(&g_scr_lock, state);

    if (!ret) {
        dprintf(CRITICAL, "Cannot set SCR signal %llx\n", scr_signal);
    }

    return ret;
}

/*
 * Lock the SCR signal.
 */
bool scr_lock(scr_signal_t scr_signal)
{
    enum scr_id scr_id = _scr_id(scr_signal);
    spin_lock_saved_state_t state;

    spin_lock_irqsave(&g_scr_lock, state);

    if (scr_is_locked(scr_signal)) {
        spin_unlock_irqrestore(&g_scr_lock, state);

        dprintf(CRITICAL, "SCR signal already locked: %llx\n", scr_signal);
        return true;
    }

    uint32_t reg = _scr_reg(scr_signal);
    uint32_t val = _scr_read_reg(scr_id, reg);
    uint32_t start_bit = _scr_start_bit(scr_signal);
    bool ret = false;

    switch (_scr_type(scr_signal)) {
        case L16:
            if (start_bit < 16) {
                val |= BIT_MASK(_scr_width(scr_signal)) <<
                       (start_bit + 16);
            } else {
                /* Sticky bit. */
                val |= BIT_MASK(_scr_width(scr_signal)) << start_bit;
            }
            _scr_write_reg(scr_id, reg, val);
            ret = true;
            break;

        case L31:
            val |= 1ul << 31;
            _scr_write_reg(scr_id, reg, val);
            ret = true;
            break;

        case R16W16:
        case RW:
        case RO:
        default:
            break;
    }

    spin_unlock_irqrestore(&g_scr_lock, state);

    if (!ret) {
        dprintf(CRITICAL, "Cannot lock SCR signal %llx\n", scr_signal);
    }

    return ret;
}

/*
 * Check whether the SCR signal is locked - read only or manually
 * locked by SW.
 */
bool scr_is_locked(scr_signal_t scr_signal)
{
    enum scr_id scr_id = _scr_id(scr_signal);

    uint32_t start_bit = _scr_start_bit(scr_signal);
    uint32_t val = _scr_read_reg(scr_id, _scr_reg(scr_signal));

    bool locked;

    switch (_scr_type(scr_signal)) {
        case L16:
            if (start_bit < 16)
                locked = val & (1ul << (start_bit + 16));
            else
                locked = val & (1ul << start_bit);
            break;

        case L31:
            locked = val & (1ul << 31);
            break;

        /* Read only bits are considered locked.*/
        case R16W16:
            locked = start_bit >= 16;
            break;

        case RO:
            locked = true;
            break;

        case RW:
        default:
            locked = false;
            break;
    }

    return locked;
}
