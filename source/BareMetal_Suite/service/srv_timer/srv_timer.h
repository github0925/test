/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    srv_timer.h
 * @brief   header file of timer service
 */

#ifndef __SRV_TIMER_H__
#define __SRV_TIMER_H__

#include <common_hdr.h>
#include <soc.h>

typedef U32 (*f_tmr_init)(module_e);
typedef U32 (*f_tmr_get_freq)(module_e);
typedef U64 (*f_tmr_tick)(module_e);
typedef BOOL (*f_tmr_is_enabled)(module_e);

typedef struct {
    f_tmr_init          init;
    f_tmr_get_freq      get_freq;
    f_tmr_tick          tick;
    f_tmr_is_enabled    is_enabled;
} tmr_ops, *tmr_ops_p;

typedef struct {
    module_e    id;
    tmr_ops_p   ops;
} hal_timer_t, *hal_timer_p;

void udelay(U32 us);
U64 tmr_tick(void);
BOOL tmr_is_enabled(void);
U32 tmr_enable(void);

#endif // __SRV_TIMER_H__
