/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    srv_timer.c
 * @brief   timer service
 */

#include <common_hdr.h>
#include "srv_timer.h"

//#define TMR_DBG(fmt, args...)     DBG(fmt, ##args)
#define TMR_DBG(fmt, args...)

/* declare in hal timer driver */
extern hal_timer_t glb_timer;

BOOL tmr_is_enabled(void)
{
    hal_timer_p tmr = (hal_timer_p) &glb_timer;

    if (NULL != tmr->ops->is_enabled) {
        return tmr->ops->is_enabled(tmr->id);
    } else {
        return FALSE;
    }
}

U32 tmr_enable(void)
{
    hal_timer_p tmr = (hal_timer_p) &glb_timer;

    if (NULL == tmr->ops->init) {
        return -1;
    } else {
        return tmr->ops->init(tmr->id);
    }
}

void udelay(U32 us)
{
    hal_timer_p tmr = (hal_timer_p) &glb_timer;
    U64 tick_to = 0;

    if ((NULL == tmr->ops->is_enabled) || (NULL == tmr->ops->tick)) {
        return;
    }

    if (!tmr->ops->is_enabled(tmr->id)) {
        DBG("Opps, glb timer not been enabled yet\n");
        return;
    }

    tick_to = tmr->ops->tick(tmr->id) + SOC_us_TO_TICK(us);

    TMR_DBG("%s: us=%d, tick_to = 0x%x_0x%x\n", __FUNCTION__,
            us, (U32)(tick_to >> 32), (U32)(tick_to));

    while (tmr->ops->tick(tmr->id) < tick_to);
}

U64 tmr_tick(void)
{
    hal_timer_p tmr = (hal_timer_p) &glb_timer;

    if ((NULL == tmr->ops->is_enabled) || (NULL == tmr->ops->tick)
        || (!tmr->ops->is_enabled(tmr->id))) {
        DBG("Opps, glb timer not been enabled yet\n");
        return 0ULL;
    }

    return tmr->ops->tick(tmr->id);
}
