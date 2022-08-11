/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 *******************************************************/

#include <wdog/wdog.h>

#if defined (CFG_WDOG_SDRV_WDT)
extern hw_wdog_ops_t sdrv_wdt_ops;

const hw_wdog_t g_hw_wdog_inst = {
#if defined(TGT_safe)
    "wdog timer 1",
    WDOG1,
#else
    "wdog timer 3",
    WDOG3,
#endif
    &sdrv_wdt_ops,
};
#endif
