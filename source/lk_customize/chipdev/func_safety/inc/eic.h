/*
 * eic.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __EIC_H__
#define __EIC_H__

#include <assert.h>
#include <stdbool.h>

/*
 * EIC modules. These EIC modules are same IPs with different
 * enable and injection signals.
 *
 * NOTE - do not modify the order of this enum.
 */
enum eic {
    EIC_SAF = 0,
    EIC_SEC,
    EIC_HPI,
    EIC_VSN,
    EIC_MAX,
};

/*
 * EIC signal id format:
 *
 *  [31:24] eic_id (see enum eic)
 *  [23:16] injection point (0 ~ 95)
 *  [15:0]  injection bit (0 ~ 511)
 */
enum eic_signal;

#include "eic_hw.h"

/*
 * Get EIC module of EIC signal.
 */
inline enum eic eic_get_module(enum eic_signal signal)
{
    return (enum eic)((uint32_t)signal >> 24);
}

/*
 * Get injection point of EIC signal.
 */
inline uint32_t eic_get_point(enum eic_signal signal)
{
    return ((uint32_t)signal >> 16) & 0xff;
}

void eic_enable_injectpoint(enum eic eic, uint32_t point,
                            bool enable);
void eic_inject(enum eic_signal signal, bool inject);

#endif
