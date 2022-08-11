/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
*sfety domain timer1 timer2
*/
#include <app.h>
#include <lib/console.h>
#include <kernel/event.h>
#include <lk_wrapper.h>

#include "Can.h"
#include "event_groups.h"
#include <string.h>

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
#include <lib/slt_module_test.h>

typedef lk_bigtime_t (*timer_func_t)(void);
#define TIMER_FREQ (24000000)
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

extern lk_bigtime_t current_time_hires(void);
static void *timer1_tick;

typedef enum {
    TIMER_1 = 0,
    TIMER_2 = 1
} timer_e;

typedef struct {
    timer_e nr;
    timer_func_t timer_func;
} slt_timer_t;

static uint64_t slt_timer_current_stamp(void)
{
    return hal_timer_glb_cntr_get(timer1_tick);
}

static lk_bigtime_t slt_current_time_hires(void)
{
    timer_instance_t *instance = (timer_instance_t *)timer1_tick;
    lk_bigtime_t us = slt_timer_current_stamp() / instance->cnt_per_us;
    return (lk_bigtime_t)us;
}

static lk_time_t slt_current_time(void)
{
    timer_instance_t *instance = (timer_instance_t *)timer1_tick;
    lk_time_t ms = slt_timer_current_stamp() / instance->cnt_per_ms;
    return (lk_time_t)ms;
}

static void slt_timer_init_early(void)
{
    hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg;

    hal_timer_creat_handle(&timer1_tick, RES_TIMER_TIMER2);

    if (timer1_tick != NULL) {
        glb_cfg.cascade = true;
        glb_cfg.clk_sel = HAL_TIMER_SEL_LF_CLK;
        glb_cfg.clk_frq = TIMER_FREQ;
        glb_cfg.clk_div = 1;
        hal_timer_global_init(timer1_tick, &glb_cfg);
        ovf_cfg.cnt_val = 0;
        ovf_cfg.ovf_val = 0xFFFFFFFF;
        ovf_cfg.periodic = true;
        hal_timer_ovf_init(timer1_tick, HAL_TIMER_G0, &ovf_cfg);
        hal_timer_ovf_init(timer1_tick, HAL_TIMER_G1, &ovf_cfg);
    }
}

static lk_bigtime_t timer1_get_current_time(void)
{
    return slt_current_time_hires();
}

static lk_bigtime_t timer2_get_current_time(void)
{
    return current_time_hires();
}

static int slt_timer_internal_ip_diagnose(slt_timer_t *slt_timer)
{
    int ret = -1;
    lk_bigtime_t end_time1;
    lk_bigtime_t end_time2;

    for (uint8_t i = TIMER_1; i <= TIMER_2; i++) {
        end_time1 = slt_timer[i].timer_func();
        thread_sleep(10);
        end_time2 = slt_timer[i].timer_func();

        if (end_time1 != end_time2) {
            printf("timer%d pass!\n", i);
        }
        else {
            printf("timer%d fail!\n", i);
            goto out;
        }
    }

    ret = 0;

out:
    return ret;
}

int TEST_SAFE_SS_08(uint times, uint timeout, char *result_string)
{
    int ret = -1;

    slt_timer_t slt_timer[] = {
        {
            .nr = TIMER_1,
            .timer_func = timer1_get_current_time
        },
        {
            .nr = TIMER_2,
            .timer_func = timer2_get_current_time
        }
    };

    slt_timer_init_early();

    ret = slt_timer_internal_ip_diagnose(slt_timer);

    return ret;
}

// test case name: module_test_sample1
// test case entry: slt_module_test_sample_hook_1
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size
SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_08, TEST_SAFE_SS_08,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);

