/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <wdog/wdog.h>

extern hw_wdog_t g_hw_wdog_inst;

BOOL wdog_is_enabled(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    return wdog->ops->is_enabled(b);
}

void wdog_cfg(U32 tmo_us)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    wdog->ops->cfg(b, WDOG_TMOUT_us_to_CLKs(tmo_us));
}

void wdog_enable(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    wdog->ops->enable(b);
}

void wdog_disable(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    wdog->ops->disable(b);
}

void wdog_refresh(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    wdog->ops->refresh(b);
}

U32 wdog_get_cnt(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    return wdog->ops->get_cnt(b);
}

U32 wdog_get_tmo_us(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    uintptr_t b = soc_get_module_base(wdog->m);

    return wdog->ops->get_tmo_us(b);
}

hw_wdog_t *wdog_get_self(void)
{
    hw_wdog_t *wdog = &g_hw_wdog_inst;
    return wdog;
}
