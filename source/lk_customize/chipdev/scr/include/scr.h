/*
 * scr.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR driver header.
 *
 * Revision History:
 * -----------------
 */
#ifndef _SCR_H
#define _SCR_H

#include <reg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "lib/reg.h"
#include "__regs_base.h"
#include "scr_hw.h"

typedef uint64_t scr_signal_t;

enum scr_id {
    SCR_SAFETY = 0,
    SCR_SEC,
    SCR_HPI,
};

enum scr_type {
    /* Bits [31:0] are RW. */
    RW = 0,

    /* Bits [31:0] are RO. */
    RO,

    /* Two types of L16 registers:
     *
     * 1) When L16 register bits are [15:0], bits [15:0] are RW, while
     *    [31:16] are lock bits. Write 1 to [31:16] wil lock the
     *    corresponding RW bits.
     *
     * 2) When L16 register bits are [31:16], these are sticky bits.
     *    Write 1 to [31:16] to change bit value, meanwhlie lock the
     *    same bit.
     *
     * In both cases, lock bits cannot be modified without chipset
     * reboot.
     */
    L16,

    /* Bits [30:0] are RW. Write 1 to [31] to lock all RW bits. The
     * lock bit cannot be modified without chipset reboot.
     */
    L31,

    /* Bits are [15:0] RW. Bits [31:16] are RO. */
    R16W16,
};

/*
 * 48 bits SCR signal format.
 *
 * [47:40]  enum scr_id
 * [39:32]  enum scr_type
 * [31:16]  SCR register offset
 * [15:8]   Register field start bit (0~31)
 * [7:0]    Register field width (1~32)
 */

/*-----------------------------------------------------
  SCR signal parser helpers.
  -----------------------------------------------------*/

static inline enum scr_id _scr_id(scr_signal_t scr_signal)
{
    uint32_t val = (uint32_t)(scr_signal >> 40) & 0xff;
    return (enum scr_id)val;
}

static inline enum scr_type _scr_type(scr_signal_t scr_signal)
{
    uint32_t val = (uint32_t)(scr_signal >> 32) & 0xff;
    return (enum scr_type)val;
}

static inline uint32_t _scr_start_bit(scr_signal_t scr_signal)
{
    return (uint32_t)(scr_signal >> 8) & 0xff;
}

static inline uint32_t _scr_width(scr_signal_t scr_signal)
{
    return (uint32_t)scr_signal & 0xff;
}

static inline uint32_t _scr_reg(scr_signal_t scr_signal)
{
    const int scr_shift[] = {
        [SCR_SAFETY] = 10,
        [SCR_SEC]    = 10,
        [SCR_HPI]    = 0,
    };

    uint32_t reg = (uint32_t)(scr_signal >> 16) & 0xffff;
    return reg << scr_shift[_scr_id(scr_signal)];
}

/*-----------------------------------------------------
  SCR register read/write helpers.
  -----------------------------------------------------*/

/* Get physical address of SCR register. */
static inline uint32_t _scr_reg_paddr(enum scr_id scr_id, uint32_t reg)
{
    const addr_t scr_base[] = {
        [SCR_SAFETY] = APB_SCR_SAF_BASE,
        [SCR_SEC]    = APB_SCR_SEC_BASE,
        [SCR_HPI]    = APB_SCR_HPI_BASE,
    };

    return scr_base[scr_id] + reg;
};

static inline uint32_t _scr_read_reg(enum scr_id scr_id, uint32_t reg)
{
    return readl(_ioaddr(_scr_reg_paddr(scr_id, reg)));
}

static inline void
_scr_write_reg(enum scr_id scr_id, uint32_t reg, uint32_t val)
{
    writel(val, _ioaddr(_scr_reg_paddr(scr_id, reg)));
}

uint32_t scr_get(scr_signal_t scr_signal);
bool scr_set(scr_signal_t scr_signal, uint32_t value);
bool scr_lock(scr_signal_t scr_signal);
bool scr_is_locked(scr_signal_t scr_signal);

#endif /* _SCR_H */
