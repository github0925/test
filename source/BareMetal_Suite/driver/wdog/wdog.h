/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#ifndef __WDOG_H__
#define __WDOG_H__

#include <common_hdr.h>
#include <soc.h>

typedef struct {
    BOOL (*is_enabled) (uintptr_t base);
    void (*cfg) (uintptr_t base, U32 tmo_us);
    void (*enable) (uintptr_t base);
    void (*disable) (uintptr_t base);
    void (*refresh) (uintptr_t base);
    U32 (*get_cnt) (uintptr_t base);
    U32 (*get_tmo_us)(uintptr_t base);

    void (*rf_refresh_if_en)(uintptr_t base);
} hw_wdog_ops_t;

typedef struct {
    char *name;
    module_e m;
    hw_wdog_ops_t *ops;
} hw_wdog_t;

BOOL wdog_is_enabled(void);
void wdog_cfg(U32 tmo_us);
void wdog_enable(void);
void wdog_disable(void);
void wdog_refresh(void);
U32 wdog_get_cnt(void);
U32 wdog_get_tmo_us(void);
hw_wdog_t *wdog_get_self(void);

#endif  /* __WDOG_H__ */
