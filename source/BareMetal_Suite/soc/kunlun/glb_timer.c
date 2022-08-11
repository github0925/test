/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    glb_timer.c
 * @brief   the only-one ATB global free-running timer vairable
 */

#include <common_hdr.h>
#include <srv_timer/srv_timer.h>
#include <timer/sdrv_timer/sdrv_timer.h>

const tmr_ops sdrv_tmr_ops = {
    sdrv_tmr_init,
    sdrv_tmr_get_freq,
    sdrv_tmr_tick,
    sdrv_tmr_is_enabled
};

const hal_timer_t glb_timer =  {
#if defined(TGT_safe) || defined(TGT_fpga)
    TIMER1,
#elif defined(TGT_sec)
    TIMER3,
#elif defined(TGT_ap)
    TIMER4,
#endif
    (tmr_ops_p) &sdrv_tmr_ops
};
