/*
* timer.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: platform timer high level api
*
* Revision History:
* -----------------
* varsion
* 012,      10/28/2019 wang yongjun
*/
#include <reg.h>
#include <err.h>
#include <stdio.h>
#include <trace.h>
#include <lib/cbuf.h>
#include <platform.h>
#include <kernel/thread.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <platform/timer.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <irq_v.h>
#include <__regs_base.h>
#include <target_res.h>
#include <timer_hal.h>
#include <lib/console.h>

#define TIMER_DEBUG_LEVEL INFO
#define TIMER_FREQ (24000000)

static void *kernel_tick;


#if 0


typedef struct {
    bool inited;
    void *arg;
    platform_timer_callback callback;
} platform_timer_context_t;

static void *kernel_tick;
platform_timer_context_t timer_context = {0};

static enum handler_return platform_tick(void);

/******************************************************************************
 ** \brief Init the timer hardware.
 **        Called in platform_early_init()
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
void timer_init_early(void)
{
    timer_hal_glb_cfg_t glb_cfg;
    timer_hal_ovf_cfg_t ovf_cfg;
    timer_instance_t *instance;

    timer_hal_creat_handle(&timer_handle, RES_KERNEL_TIMER);

    if (timer_handle != NULL) {
        instance = (timer_instance_t *)timer_handle;
        glb_cfg.cascade = true;
        glb_cfg.clk_sel = TIMER_HAL_SEL_LF_CLK;
        glb_cfg.clk_frq = TIMER_FREQ;
        glb_cfg.clk_div = 2;
        timer_hal_global_init(timer_handle, &glb_cfg);
        ovf_cfg.cnt_val = 0;
        ovf_cfg.ovf_val = 0xFFFFFFFF;
        ovf_cfg.ovf_cbk = NULL;
        ovf_cfg.periodic = true;
        timer_hal_ovf_init(timer_handle, TIMER_HAL_G0, &ovf_cfg);
        timer_hal_ovf_init(timer_handle, TIMER_HAL_G1, &ovf_cfg);
        timer_context.inited = true;
        dprintf(TIMER_DEBUG_LEVEL,
                "Platform timer init finished, cnt per ms:%d, cnt per us:%d\n",
                instance->cnt_per_ms,
                instance->cnt_per_us);
    }
}



static status_t platform_timer_set(void *handle,
                                   platform_timer_callback callback, void *arg, lk_time_t interval,
                                   bool isperiodic)
{
    timer_hal_ovf_cfg_t ovf_cfg;
    timer_instance_t *instance = (timer_instance_t *)timer_handle;

    if ((!timer_handle) || (!timer_context.inited)) {
        return ERR_NOT_VALID;
    }

    timer_context.arg = arg;
    timer_context.callback = callback;

    ovf_cfg.cnt_val = 0;
    ovf_cfg.ovf_val = interval * instance->cnt_per_ms;
    ovf_cfg.ovf_cbk = platform_tick;
    ovf_cfg.periodic = isperiodic;
    timer_hal_ovf_init(timer_handle, TIMER_HAL_LOCAL_A, &ovf_cfg);
    timer_hal_func_irq_register(timer_handle);

    return NO_ERROR;
}

/******************************************************************************
 ** \brief Init the timer hardware.
 **        Called in kernel_init()->timer_init()
 **
 ** \param [in]
 ** \param [out]
 *****************************************************************************/
#if PLATFORM_HAS_DYNAMIC_TIMER
status_t platform_set_oneshot_timer(platform_timer_callback callback,
                                    void *arg, lk_time_t interval)
{
    return platform_timer_set(timer_handle, callback, arg, interval, false);
}

void platform_stop_timer(void)
{
    timer_hal_int_disable(timer_handle, TIMER_HAL_CNT_LA_OVF_BIT_OFFSET);
    timer_hal_int_clear(timer_handle, TIMER_HAL_CNT_LA_OVF_BIT_OFFSET);
}
#else
status_t platform_set_periodic_timer(platform_timer_callback callback,
                                     void *arg, lk_time_t interval)
{
    return platform_timer_set(timer_handle, callback, arg, interval, true);
}
#endif


#endif

/******************************* kernel timestamp implementation *******************************/

static uint64_t timer_current_stamp(void)
{
    return hal_timer_glb_cntr_get(kernel_tick);
}

lk_bigtime_t current_time_hires(void)
{
    timer_instance_t *instance = (timer_instance_t *)kernel_tick;
    lk_bigtime_t us = timer_current_stamp() / instance->cnt_per_us;
    // dprintf(TIMER_DEBUG_LEVEL, "current_time_hires:%d", (uint32_t)us);
    return (lk_bigtime_t)us;
}

lk_time_t current_time(void)
{
    timer_instance_t *instance = (timer_instance_t *)kernel_tick;
    lk_time_t ms = timer_current_stamp() / instance->cnt_per_ms;
    // dprintf(TIMER_DEBUG_LEVEL, "current_time:%d", (uint32_t)ms);
    return (lk_time_t)ms;
}

void timer_init_early(void)
{
    hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg;
    timer_instance_t *instance;

    hal_timer_creat_handle(&kernel_tick, RES_KERNEL_TIMER);

    if (kernel_tick != NULL) {
        instance = (timer_instance_t *)kernel_tick;
        glb_cfg.cascade = true;
        glb_cfg.clk_sel = HAL_TIMER_SEL_LF_CLK;
        glb_cfg.clk_frq = TIMER_FREQ;
        glb_cfg.clk_div = 1;
        hal_timer_global_init(kernel_tick, &glb_cfg);
        ovf_cfg.cnt_val = 0;
        ovf_cfg.ovf_val = 0xFFFFFFFF;
        ovf_cfg.periodic = true;
        hal_timer_ovf_init(kernel_tick, HAL_TIMER_G0, &ovf_cfg);
        hal_timer_ovf_init(kernel_tick, HAL_TIMER_G1, &ovf_cfg);

        dprintf(TIMER_DEBUG_LEVEL,
                "Platform timer init finished, cnt per ms:%d, cnt per us:%d\n",
                instance->cnt_per_ms,
                instance->cnt_per_us);
    }
}
/******************************* vPortTickTimerEnable implementation *******************************/

void vPortTickTimerEnable( void (*Tick_Handler)( void ) )
{
    //hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg;
    //timer_instance_t *instance;

    ovf_cfg.cnt_val = 0;
    ovf_cfg.ovf_val = configTICK_OVF_VAL;
    ovf_cfg.periodic = true;
    hal_timer_ovf_init(kernel_tick, HAL_TIMER_LOCAL_A, &ovf_cfg);

    hal_timer_int_cbk_register(kernel_tick, HAL_TIMER_CNT_LA_OVF_INT_SRC, (hal_timer_int_cbk)Tick_Handler);
    hal_timer_int_src_enable(kernel_tick, HAL_TIMER_CNT_LA_OVF_INT_SRC);
}
