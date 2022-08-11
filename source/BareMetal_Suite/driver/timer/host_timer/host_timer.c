/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    host_tmr.c
 * @brief   to provide a free-running timer for host
 */

#include <common_hdr.h>
#include <service.h>
#include <time.h>

static BOOL enabled = FALSE;

U32 host_tmr_init(module_e id)
{
#if defined(VTEST)
    writel(0xFFFFFFFFu, SYS_TB_CTRL_BASE_ADDR + 0xc);
#endif
    enabled = TRUE;

    return 0;
}

U32 host_tmr_get_freq(module_e id)
{
    if (enabled) {
        return 1000000UL;
    } else {
        return 1;
    }
}

U64 host_tmr_tick(module_e id)
{
#if !defined(VTEST)
    static U64 tick = 0;
    return tick++;
#else
    return (U64)(0xFFFFFFFFu - readl(SYS_TB_CTRL_BASE_ADDR + 0xc));
#endif
}

BOOL host_tmr_is_enabled(module_e id)
{
    return enabled;
}
