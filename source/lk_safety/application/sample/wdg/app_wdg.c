/*
* app_wdg.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: wdg samplecode.
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
#include <arch.h>
#include "timer_hal.h"
#include "wdg_hal.h"

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#define WDT_DELTATIME 6000  /* 6 secs*/
#define WDT_WINLOW 7000  /* 7 secs*/
#define WDT_TERCOUNTER  10000 /* 10 secs*/
#define RES_WDT  RES_WATCHDOG_WDT1
#define WDT_OVERFLOW_INT    true
#define RES_TIME_ID     RES_TIMER_TIMER2

void *watchdog_handle;

static void *tc_handle;

static volatile uint32_t _change_period = 1;
static volatile uint32_t _countdown_timer = 0;

static hal_timer_glb_cfg_t _glb_cfg = {
    .clk_sel = HAL_TIMER_SEL_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 24000, //23999?
    .cascade = false,
};

static hal_timer_ovf_cfg_t _ovf_cfg = {
    .periodic = true,
    .cnt_val = 0,
    .ovf_val = 250, //249?
};

static wdg_app_config_t wdt_app_cfg = {
    .workMode = wdg_mode1,
    .seqDeltaValue = WDT_DELTATIME,
    .timeoutValue = WDT_TERCOUNTER,
    .windowLimitValue = WDT_WINLOW,
    .clocksource = wdg_lp_clk,
    .divisor = 31,
};

static wdg_capability_t g_wdt_app_resource = {.version = 0x10000, \
                                              .res_category = "watchdog",
                                              .res_max_num = 8,
                                              .res_cap[0].res_glb_idx = RES_WATCHDOG_WDT1,
                                              .res_cap[0].enableIntReset = false,
                                              .res_cap[0].enableExtReset = false,
                                             };

enum handler_return _tc_handler(void)
{
    if ((_countdown_timer > (8000 / 250)) && _change_period == 0) {
        printf("WDT has been Refresh after 8s\n\r");
        _countdown_timer = 0;
        hal_wdg_refresh(watchdog_handle);
    }
    else if ((_countdown_timer > (5000 / 250)) && _change_period == 1) {
        printf("WDT has been Refresh after 5s\n\r");
        _countdown_timer = 0;
        hal_wdg_refresh(watchdog_handle);
    }
    else if ((_countdown_timer > (10000 / 250)) && _change_period == 2) {
        printf("WDT has been Refresh after 10s\n\r");
        _countdown_timer = 0;
        hal_wdg_refresh(watchdog_handle);
    }
    else if ((_countdown_timer > (10000 / 250)) && _change_period == 3) {
        printf("Watchdog Reset interrupt has been enabled, system will restart after 10s\n\r");
        _countdown_timer = 0;
    }
    else {
        _countdown_timer++;
    }

    return INT_NO_RESCHEDULE;
}

void _config_timer(void)
{
    hal_timer_creat_handle(&tc_handle, RES_TIME_ID);

    if (tc_handle == NULL) {
        printf("Failed to create timer handle!\r\n");
        return;
    }

    hal_timer_global_init(tc_handle, &_glb_cfg);
    hal_timer_ovf_init(tc_handle, HAL_TIMER_LOCAL_A, &_ovf_cfg);
    hal_timer_int_src_enable(tc_handle, HAL_TIMER_CNT_LA_OVF_INT_SRC);
    hal_timer_int_cbk_register(tc_handle, HAL_TIMER_CNT_LA_OVF_INT_SRC,
                               _tc_handler);
}
enum handler_return wdg_irq_handle(void *arg)
{
    printf("wdg_irq_handle instance\n");
    //add reset or mendump func
    hal_wdg_disable_interrupts(arg);
    hal_wdg_int_clear(arg);
    platform_halt(HALT_ACTION_REBOOT, HALT_REASON_HW_WATCHDOG);
    /* We should never get here; watchdog handlers should always be fatal. */
    //DEBUG_ASSERT(false);
    return INT_NO_RESCHEDULE;
}

void _configure_wdt(void)
{
    bool ret = false;
    hal_wdg_creat_handle(&watchdog_handle, RES_WDT);

    if (watchdog_handle) {
        ret = hal_wdg_init(watchdog_handle, &wdt_app_cfg);

        if (ret)
            ret = hal_wdg_int_register(watchdog_handle, wdg_irq_handle, true);
        else
            hal_wdg_deinit(watchdog_handle);

        if (ret) {
            ret = hal_wdg_enable_interrupts(watchdog_handle);
        }
        else
            hal_wdg_deinit(watchdog_handle);

        if (ret == false)
            ret = hal_wdg_release_handle(watchdog_handle);

    }
    else {
        printf("Failed to get wdt handle!\r\n");
    }
}
void wdt_test(int argc, const cmd_args *argv)
{
    uint8_t key;
    bool ret = false;
    _configure_wdt();
    _config_timer();
    ret = hal_wdg_enable(watchdog_handle);

    if (ret == false) {
        printf("Failed to enable wdt,quit\r\n");
        return;
    }

    while (key != 5) {
        key = getchar() - '0';

        switch (key) {
            case 1:
                _change_period = 0;
                printf("-I- WDT will be Refresh every 8s!\r\n");
                break;

            case 2:
                _change_period = 1;
                printf("-I- WDT will be Refresh every 5s!\r\n");
                break;

            case 3:
                _change_period = 2;
                printf("-I- WDT will be Refresh every 10s!\r\n");
                break;

            case 4:
                _change_period = 3;
                printf("-I- System will be restart by WDT after 10s\r\n");
                break;

            default:
                printf("-E- Invalid input!\r\n");
                break;
        }
    }

    hal_wdg_release_handle(watchdog_handle);
    printf("Quit wdt test！\r\n");

}
#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("wdt", "wdt test", (console_cmd)&wdt_test)
STATIC_COMMAND_END(wdt_cmd);
#endif
