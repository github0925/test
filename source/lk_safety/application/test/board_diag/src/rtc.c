/*
 * rtc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <rtc_drv.h>
#include <platform/interrupts.h>
#include <irq_v.h>
#include <pmu_hal.h>
#include <arm_gic_hal.h>
#include "func_power.h"

extern enum handler_return wakeup_dispose_callback(void *arg);

#define get_rtc_wakeup_transform_s(x) (x*RTC_CLK_HZ)
/*Initialize the chip's internal RTC function*/
void rtc_clk_init(void)
{
    rtc_init(APB_RTC1_BASE);

    rtc_enable(APB_RTC1_BASE);

    rtc_lock(APB_RTC1_BASE);

    register_int_handler(IRQ_GIC1_RTC1_RTC_WAKEUP_IRQ_NUM, wakeup_dispose_callback,
                         NULL);

    rtc_clr_wakeup_status(APB_RTC1_BASE);
}
/*set chip's internal RTC, usually for Waking up regularly*/
static void set_rtc_timer(void)
{
    rtc_update_cmp_value(APB_RTC1_BASE, S2RtcTick(300));

    rtc_wakeup_enable(APB_RTC1_BASE, true, true, true);

    unmask_interrupt(IRQ_GIC1_RTC1_RTC_WAKEUP_IRQ_NUM);
}
/*set chip wakeup mode, such as internal arousal includes RTC and and external arousal includes can、ethernet、kl15*/
void set_system_wakeup(void)
{
    set_rtc_timer();
    set_soc_internal_wakeup();
    set_soc_external_wakeup();
}
