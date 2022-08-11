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
#include <wdg_hal.h>
#include <sdunittest.h>

#include <trace.h>

#define LOCAL_TRACE 1 //close local trace 1->0

//minute num
#define HTOL_TEST_TIME_BETWEEN_MIN 6

enum handler_return wdg_irq_handle(void* arg)
{
    LTRACEF("wdg_irq_handle instance\n");
    //add reset or mendump func
    hal_wdg_disable_interrupts(arg);
    hal_wdg_int_clear(arg);
    platform_halt(HALT_ACTION_REBOOT, HALT_REASON_HW_WATCHDOG);
    /* We should never get here; watchdog handlers should always be fatal. */
    //DEBUG_ASSERT(false);
    return INT_NO_RESCHEDULE;
}

static int htol_watchdog_thread(void* arg)
{
    wdg_app_config_t wdg_app_cfg;
    bool ret = false;
    uint32_t wdg_number = 0;
    void* watchdog_handle;
    uint32_t res_glb_idx = RES_WATCHDOG_WDT3;
    int i = 0;

    LTRACEF("htol_watchdog_thread enter\n");

    hal_wdg_creat_handle(&watchdog_handle, res_glb_idx);

    /*get handle ok */
    if (watchdog_handle) {
        wdg_app_cfg.workMode = wdg_mode1;
        wdg_app_cfg.seqDeltaValue = 100; //ms
        wdg_app_cfg.timeoutValue = 500; //ms
        wdg_app_cfg.windowLimitValue = 1000;//ms
        ret = hal_wdg_init(watchdog_handle, &wdg_app_cfg);

        if (ret == true) {
            ret = hal_wdg_int_register(watchdog_handle, wdg_irq_handle, true);
        }

        if (ret == true) {
            ret = hal_wdg_set_timeout(watchdog_handle, 1500); //timeout 1500ms
        }
        else {
            hal_wdg_deinit(watchdog_handle);
        }

        if (ret == true) {
            ret = hal_wdg_enable_interrupts(watchdog_handle);
        }
        else {
            hal_wdg_deinit(watchdog_handle);
        }

        if (ret == true) {
            ret = hal_wdg_enable(watchdog_handle);
        }
        else {
            hal_wdg_deinit(watchdog_handle);
        }

        if (ret != true) {
            ret = hal_wdg_release_handle(watchdog_handle);
        }

        while (1) {
            spin(200 * 1000);
            hal_wdg_refresh(watchdog_handle);
            i++;

            if (i == 100) {
                LTRACEF("hal_wdg_refresh \n");
                i = 0;
            }
        }
    }
    else {
        LTRACEF("watchdog gei handle failed:watchdog number =%d \n", res_glb_idx);
    }

    return 0;
}

static int htol_test_thread(void* arg)
{
    int run_time = 0;
    int min = 0;
    int secend = 0;

    LTRACEF("htol_test_thread entry, test after 60s\n");

    while (1) {
        //wait min
        min = HTOL_TEST_TIME_BETWEEN_MIN;

        while (min) {
            secend = 60;

            while (secend) {
                spin(1000 * 1000); //sleep 1 second
                secend--;
            }

            min--;
        }

        LTRACEF("htol_test_thread run_one_testsuite_%d start\n", run_time);
        run_one_testsuite("x9_htol");
        LTRACEF("htol_test_thread run_one_testsuite_%d end \n", run_time);
        run_time++;
    }

    return 0;
}

static void htol_entry(const struct app_descriptor* app, void* args)
{
    thread_t* t1 = thread_create(
                       "htol_watchdog", &htol_watchdog_thread, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_t* t2 = thread_create(
                       "htol_test", &htol_test_thread, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_resume(t1);
    thread_resume(t2);

    while (1) {

    }
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("htol_entry", "htol test entry", (console_cmd)&htol_entry)
STATIC_COMMAND_END(htol_entry);

#endif

APP_START(htol_entry)
.flags = 0,
.entry = htol_entry,
APP_END
