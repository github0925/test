/*
 * scr_hal.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR HAL.
 *
 * Platform and driver codes call scr chip driver directly. Applications
 * need to use HAL SCR interfaces, which provide resource control.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>

#include "scr_hal.h"
#include "res.h"

/*
 * Design of SCR driver.
 *
 * From resource management point of view, there're 2 types of
 * SCR controllers:
 *
 * 1) SCR_SEC and SCR_SAF
 *  Permission is controlled by Register Protection Control (RPC)
 *  module, in register level.
 *
 * 2) SCR_HPI
 *  Permission is controlled by APBMUX.
 *
 * The SCR modules are assigned to SEC domain by default, and the
 * SEC core configures most of SCR signals during system power up,
 * using chipdev driver directly.
 *
 * The SCR signals requiring dynamic configuration after power up
 * are assigned to specified domain, thus must be configured using
 * HAL driver.
 */

/* Create handle for specified signal. */
scr_handle_t hal_scr_create_handle(scr_signal_t scr_signal)
{
    /* TODO - check SCR resource permissoin. */
    #if 0
    enum scr_id scr_id = _scr_id(scr_signal);
    uint32_t scr_reg = _scr_reg(scr_signal);
    const domain_res_t *res;

    extern const domain_res_t g_scr_res;
    extern const domain_res_t g_scr_hpi_res;

    switch (scr_id)
    {
        case SCR_SAFETY:
        case SCR_SEC:
            res = &g_scr_res;
            break;

        case SCR_HPI:
            res = &g_scr_hpi_res;
            break;

        default:
            ASSERT(false);
            return (scr_handle_t)0;
    }

    /* Check resource permission. */
    for (int i = 0; i < res->res_num; i++) {
        if (res->res_info[i].phy_addr <= _scr_reg_paddr(scr_id, scr_reg)
            && _scr_reg_paddr(scr_id, scr_reg) <
               res->res_info[i].phy_addr + res->res_info[i].addr_range) {
            return (scr_handle_t)scr_signal;
        }
    }

    return (scr_handle_t)0;
    #else
    return (scr_handle_t)scr_signal;
    #endif
}

void hal_scr_delete_handle(scr_handle_t handle)
{
    ASSERT(handle);
}

uint32_t hal_scr_get(scr_handle_t handle)
{
    ASSERT(handle);
    return scr_get((scr_signal_t)handle);
}

bool hal_scr_set(scr_handle_t handle, uint32_t value)
{
    ASSERT(handle);
    return scr_set((scr_signal_t)handle, value);
}

bool hal_scr_lock(scr_handle_t handle)
{
    ASSERT(handle);
    return scr_lock((scr_signal_t)handle);
}

bool hal_scr_is_locked(scr_handle_t handle)
{
    ASSERT(handle);
    return scr_is_locked((scr_signal_t)handle);
}
