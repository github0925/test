/*
 * rtc_demo.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <rtc_drv.h>
#include <platform/interrupts.h>
#include <irq_v.h>
#include <pmu_hal.h>
#include <arm_gic_hal.h>

static char rtcdemo_usage[]= {
    "rtcdemo RTC TIMEOUT NEEDPOWEROFF\n"
    "\t\t\t  RTC:1,2.  TIMEOUT unit second.  NEEDPOWEROFF:0,1"
};

static enum handler_return wakeup_irq_handler(void *arg)
{
    addr_t base = (addr_t)arg;
    rtc_wakeup_enable(base, false, false, false);
    rtc_clr_wakeup_status(base);
    printf("wakeup_irq_handler: RTC timeouted.\n");
    return 0;
}

static int32_t pmu_config_internal_wakeup_source(bool en, addr_t base)
{
    int32_t ret = 0;
    void *handle = NULL;

    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
    if (ret != 0) {
        printf("get handle fail\n");
        return ret;
    }

    ret = hal_pmu_init(handle);
    if (ret != 0) {
        printf("pmu init fail\n");
        return ret;
    }

    if (base == APB_RTC1_BASE) {
        ret = hal_pmu_set_internal_wakeup_enable(handle, 0, en);
    }
    else if (base == APB_RTC2_BASE) {
        ret = hal_pmu_set_internal_wakeup_enable(handle, 1, en);
    }

    if (ret != 0) {
        printf("pmu set internal wakeup enable fail\n");
        return ret;
    }

    ret = hal_pmu_exit(handle);
    if (ret != 0) {
        printf("pmu exit fail\n");
        return ret;
    }

    ret = hal_pmu_release_handle(handle);
    if (ret != 0) {
        printf("release handle fail\n");
        return ret;
    }

    return ret;
}

static int32_t pmu_shut_down(void)
{
    int32_t ret = 0;
    void *handle = NULL;

    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);
    if (ret != 0) {
        printf("get handle fail\n");
        return ret;
    }

    ret = hal_pmu_init(handle);
    if (ret != 0) {
        printf("pmu init fail\n");
        return ret;
    }

    ret = hal_pmu_powerdown(handle);
    if (ret != 0) {
        printf("pmu set pd fail\n");
        return ret;
    }

    ret = hal_pmu_exit(handle);
    if (ret != 0) {
        printf("pmu exit fail\n");
        return ret;
    }

    ret = hal_pmu_release_handle(handle);
    if (ret != 0) {
        printf("release handle fail\n");
        return ret;
    }

    return ret;
}

int rtcdemo_main(int argc, const cmd_args *argv)
{
    addr_t rtc_base;
    uint32_t irq_num;
    uint32_t timeout;
    bool enter_poweroff;

    rtc_base = (argv[1].u == 1) ? APB_RTC1_BASE : APB_RTC2_BASE;
    irq_num = (argv[1].u == 1) ? IRQ_GIC1_RTC1_RTC_WAKEUP_IRQ_NUM : IRQ_GIC1_RTC2_RTC_WAKEUP_IRQ_NUM;
    timeout = argv[2].u;
    enter_poweroff = (argv[3].u == 1) ? true : false;

    if (rtc_base != APB_RTC1_BASE) {
        printf("RTC2 is used by ap, please use RTC1.\n");
        return true;
    }

    rtc_init(rtc_base);
    rtc_enable(rtc_base);
    rtc_lock(rtc_base);
    rtc_clr_wakeup_status(rtc_base);
    rtc_update_cmp_value(rtc_base, S2RtcTick(timeout));

    if (enter_poweroff) {
        rtc_wakeup_enable(rtc_base, true, false, true);
        printf("Going to poweroff, and wakeup after %d second.\n", timeout);
        pmu_config_internal_wakeup_source(true, rtc_base);
        pmu_shut_down();
    }
    else {
        register_int_handler(irq_num, wakeup_irq_handler, (void *)rtc_base);
        unmask_interrupt(irq_num);
        rtc_wakeup_enable(rtc_base, true, true, false);
        printf("Wait %d seconds for RTC wakeup interrupt.\n", timeout);
    }

    return true;
}

int rtcdemo_set(int argc, const cmd_args *argv)
{
    uint32_t set_time = argv[1].u;
    rtc_set_tick(APB_RTC1_BASE, S2RtcTick(set_time));
    return true;
}

int rtcdemo_read(int argc, const cmd_args *argv)
{
    uint64_t tick;
    tick = rtc_get_tick(APB_RTC1_BASE);
    printf("RTC time : %lld second.\n", RtcTick2S(tick));
    return true;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("rtcdemo", rtcdemo_usage, (console_cmd)&rtcdemo_main)
STATIC_COMMAND("rtcset", "set second for rtc", (console_cmd)&rtcdemo_set)
STATIC_COMMAND("rtcread", "get rtc second", (console_cmd)&rtcdemo_read)
STATIC_COMMAND_END(rtcdemo);
#endif
