/*
* cmd_adc_test.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive lk fake_tbu drv
*
* Revision History:
* -----------------
* 001, 08/13/2019 arrayhu implement this
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <trace.h>
#include <platform.h>
#include <lib/console.h>
#include <err.h>
#include <__regs_base.h>
#include <platform/interrupts.h>
#include <dma_hal.h>
#include <dw_adc.h>
#include <lib/slt_module_test.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#define ADC_TEST_BASE   ((vaddr_t)paddr_to_kvaddr(APB_ADC_BASE))
#else
#define ADC_TEST_BASE APB_ADC_BASE
#endif

#define LOCAL_TRACE 0 //close local trace 1->0

#define ADC_INT_ARRAY_NUM   128
#define ADC_CONVERT_MODE_TYPE_MAX_NUM 4
#define ADC_CONVERT_SINGLE_CHANNEL_MAX_NUM 1
#define ADC_CONVERT_SINGLE_TEST_RESULT_NUM 4 //must > ADC_CONVERT_MODE_TYPE_MAX_NUM*ADC_CONVERT_SINGLE_CHANNEL_MAX_NUM

#define adc_vlot2cvt(mV)    (mV * 4096 / 1800)
#if PLATFORM_G9X
#define ADC_TEST_CHANNEL    0
#define ADC_RESULT_MAX      adc_vlot2cvt(430)
#define ADC_RESULT_MIN      adc_vlot2cvt(350)
#else
#define ADC_TEST_CHANNEL    1
#define ADC_RESULT_MAX      adc_vlot2cvt(1600)
#define ADC_RESULT_MIN      adc_vlot2cvt(1500)
#endif

//entries disable
static adc_entry_cfg_t entries_test_1[32] = {
    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
};

//ignore enable
static adc_entry_cfg_t entries_test_2[32] = {
    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
};
//all entries ch rct test,time setup no way to test
static adc_entry_cfg_t entries_cfg[ADC_MAX_ENTRIES_NUM] = {
    {.rct_v = 0, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 1, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 2, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 3, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 4, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 5, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 6, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 7, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 8, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 9, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 10, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 11, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 12, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 13, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 14, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 15, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 16, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 17, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 18, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 19, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 20, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 21, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 22, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 23, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 24, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 25, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 26, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 27, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 28, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 29, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 30, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 31, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 0, .ch = 32, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 48, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 64, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 80, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 96, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 112, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 34, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 0, .ch = 50, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 66, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 82, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 98, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 114, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 38, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 54, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 70, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 20, .ch = 86, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 118, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 42, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 58, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 74, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 90, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 106, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 200, .ch = 122, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 46, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 62, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 78, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 100, .ch = 94, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 110, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 126, .setup_time_sel = 1, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 127, .setup_time_sel = 2, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 255, .ch = 127, .setup_time_sel = 3, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
};

//test skip
static adc_entry_cfg_t entries_test_3[32] = {
    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 1, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 1, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 1, .entry_en = 0, .sample_cycle = 0 },
};

//test cycle
static adc_entry_cfg_t entries_test_4[16] = {
    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 0 },
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 1 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 2 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 8 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 15 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 31 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 48 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 65 },

    {.rct_v = 10, .ch = 0, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle =  80},
    {.rct_v = 10, .ch = 17, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 110 },
    {.rct_v = 10, .ch = 34, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 127 },
    {.rct_v = 10, .ch = 51, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 164 },

    {.rct_v = 10, .ch = 68, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 189 },
    {.rct_v = 10, .ch = 85, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 212 },
    {.rct_v = 10, .ch = 102, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 244 },
    {.rct_v = 10, .ch = 119, .setup_time_sel = 0, .ignore_result_en = 0, .skip_en = 0, .entry_en = 1, .sample_cycle = 255 },
};

static adc_monitor_cfg_t mons_cfg[ADC_MAX_MONITORS_NUM] = {
    { .monitor_en = 0, .monitor_ch = 0, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 1, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 2, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 3, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 4, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 5, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 6, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
    { .monitor_en = 0, .monitor_ch = 7, .low_flag = 0, .high_flag = 0, .in_range_flag = 0, .out_of_range_flag = 0, .discard_v_result = 0, .v_detected = 0, .v_low_threshold = 0, .v_high_threshold = 0 },
};
const char adc_test_help[] = {
    "refer line 276, " __FILE__ "\n"
    /*
    "Usage:adc_test [test_content]\n" \
    "            [test_content] can be these:\n" \
    "                resolution"
    "                dmacfg\n" \
    "                dmaread\n" \
    "                interrupt\n" \
    "                entries\n" \
    "                all\n" \
    "                help\n" \
    "            Sample:adc_test interrupt\n"
    */
};

//static struct {
//  u32 count;
//  u32 flag_array[ADC_INT_ARRAY_NUM];
//}test_int_v  _test_v = {0,{0}};
//static uint8_t dst_data[128*4] = {0xff};
static void adc_dma_init(void)
{
    /*
    struct dma_dev_cfg cfg_rx;
    struct dma_chan *rx_chan = dma_chan_req();

    cfg_rx.direction = DMA_DEV2MEM;
    cfg_rx.dst_addr  = 0x30860100;//fifo addr;
    cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    cfg_rx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_rx.dst_maxburst = DMA_BURST_TR_4ITEMS;
    res = dma_dev_config(rx_chan, &cfg_rx);

    struct dma_desc *desc_rx;
    desc_rx = dw_prep_dma_dev(rx_chan, (addr_t)dst_data, 128, 0);
    desc_rx->pending_timeout = 10;

    dma_submit(desc_rx);
    enum dma_status ret_rx = dma_sync_wait(desc_rx, -1); //10 ms time out -> -1
    if (DMA_COMP != ret_rx) {
        dma_terminate(desc_rx);
    }*/
}

static int do_dma_cfg_adc_test(void)
{
    return NO_ERROR;
}

static volatile uint8_t dst_data[128 * 4] = {  0xff };
static void rx_irq_evt_handle(enum dma_status status, u32 param, void* context)
{
    if (status == DMA_PENDING) {
        printf("rx status (%d) addr(0x%x) context(%p) \n", status, param, context);
    }
    else {
        printf("rx status (%d) err(0x%x) context(%p) \n", status, param, context);
    }
}

static int do_dma_read_adc_test(void)
{

    adc_loop_repeat_cfg_t loop_repeat = {.loop_end = ADC_MAX_ENTRIES_NUM - 1, .repeat = 0};//no repeat
    adc_result_reg_t buffer[ADC_FIFO_MAX_DEPTH] = {0};

    printf("dma read test:\n");

    for (int i = 0; i < 128 * 4; i++) {
        dst_data[i] = 0xff;
        //printf("initdata:%x\n",dst_data[i]);
    }


    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_fifo_threshold_dma(ADC_TEST_BASE, 32);
    printf("adc_dma_cfg val:%08x", *(volatile unsigned int*)(ADC_TEST_BASE + 0x1c));
    adc_set_dma_en_bit(ADC_TEST_BASE, true);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);
    printf("adc_dma_cfg val:%08x", *(volatile unsigned int*)(ADC_TEST_BASE + 0x1c));
    //adc_set_fifo_threshold_dma(ADC_TEST_BASE, 32);
//-----------------------------------------------------------------------------------------
//dma cfg
    /*
        struct dma_dev_cfg cfg_rx;
        struct dma_chan *rx_chan = hal_dma_chan_req(DMA_PERI_ADC);
        if (!rx_chan)
        {
            printf("request dma chan fail\n");
            return ERR_GENERIC;
        }
        //int res;

        cfg_rx.direction = DMA_DEV2MEM;
        cfg_rx.src_addr  = ADC_TEST_BASE + 0x100; //fifo addr;
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_rx.src_maxburst = DMA_BURST_TR_1ITEM;
        cfg_rx.dst_maxburst = DMA_BURST_TR_1ITEM;
        dma_dev_config(rx_chan, &cfg_rx);

        struct dma_desc *desc_rx;
        desc_rx = dw_prep_dma_dev(rx_chan, (addr_t)dst_data, 128 * 4, DMA_INTERRUPT | DMA_PENDING_TIMEOUT); //128*4 bytes
        //desc_rx->pending_timeout = 10;
        desc_rx->dmac_irq_evt_handle = rx_irq_evt_handle;
        //dma_submit(desc_rx);
        */
//dma cfg end
//-------------------------------------------------------------------------------------------
//start adc
    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);
//start adc end
//-------------------------------------------------------------------------------------------
    /*
        dma_submit(desc_rx);
        enum dma_status ret_rx = dma_sync_wait(desc_rx, 50); //50 ms time out
        printf("debug ret_rx:%d\n", ret_rx);
        if (DMA_COMP != ret_rx)
        {
            dma_terminate(desc_rx);
            printf("dma wrong");
        }
        */
    adc_stop_convert(ADC_TEST_BASE);
    //adc_result_reg_t buffer[ADC_FIFO_MAX_DEPTH]={ 0};
    adc_result_reg_t* tmp = (adc_result_reg_t*)dst_data;

    printf("dma:\n");

    for (int i = 0; i < 128; i++) {
        printf("ts_ind:%d,ch:%d,result:%x\n", tmp[i].is_timetap, tmp[i].ch, tmp[i].result);
    }

    printf("\nfifo:\n");

    for (int i = 0; i < 128; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

//--------------------------------------------------------------------------------------------
    //while(adc_get_fifo_WML(ADC_TEST_BASE) >= 48);

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\ndma read test end\n");
    return NO_ERROR;
}

static int do_adc_entries_test(void)
{
    adc_loop_repeat_cfg_t loop_repeat = {.loop_end = ADC_MAX_ENTRIES_NUM - 1, .repeat = 0};//no repeat
    adc_loop_repeat_cfg_t loop_repeat1 = {.loop_end = 31, .repeat = 0};//no repeat
    adc_loop_repeat_cfg_t loop_repeat2 = {.loop_end = 31, .repeat = 0};//no repeat
    adc_loop_repeat_cfg_t loop_repeat3 = {.loop_end = 31, .repeat = 0};//no repeat
    adc_loop_repeat_cfg_t loop_repeat4 = {.loop_end = 15, .repeat = 0};//no repeat
    adc_result_reg_t buffer[ADC_FIFO_MAX_DEPTH] = {0};
    printf("entries test:\n");

    printf("general test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    while (adc_get_fifo_WML(ADC_TEST_BASE));

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\ngeneral test end\n");


    printf("entries enable/disable test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat1, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    while (adc_get_fifo_WML(ADC_TEST_BASE));

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\nentries enable/disable test end\n");

    printf("ignore test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat2, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    while (adc_get_fifo_WML(ADC_TEST_BASE));

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\nentries ignore test end\n");

    printf("skip test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat3, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    while (adc_get_fifo_WML(ADC_TEST_BASE));

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\nskip test end\n");

    printf("cycle test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat4, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    while (adc_get_fifo_WML(ADC_TEST_BASE));

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\ncycle test end\n");

    printf("runtime modify entries configuration test:\n");
    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_6_BITS_E0);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_multiple_ch(ADC_TEST_BASE, loop_repeat, entries_cfg, mons_cfg);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_MULTIPLE_CH_E3);
    adc_disable_all_int(ADC_TEST_BASE);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, true);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);

    adc_flush_fifo(ADC_TEST_BASE);
    adc_start_convert(ADC_TEST_BASE);

    //modify the configuration
    while (adc_get_fifo_WML(ADC_TEST_BASE) == 16);

    for (int i = 16; i < 24; i++) {
        adc_cfg_an_entry(ADC_TEST_BASE, entries_cfg[i - 16], i);
    }

    for (int i = 0; i < ADC_FIFO_MAX_DEPTH; i++) {
        buffer[i] = (adc_result_reg_t)get_fifo_reg_value(ADC_TEST_BASE);
        printf("ts_ind:%d,ch:%d,result:%x\n", buffer[i].is_timetap, buffer[i].ch, buffer[i].result);
    }

    adc_stop_convert(ADC_TEST_BASE);
    adc_flush_fifo(ADC_TEST_BASE);
    printf("\runtime modify entries configuration test end\n");

    printf("timestamp force reload test:");

    return NO_ERROR;

}

static volatile unsigned int _int_flags = 0;
enum handler_return adc_test_irq_handler(void* arg)
{
    adc_disable_all_int(ADC_TEST_BASE);
    _int_flags = get_int_flags_reg_val(ADC_TEST_BASE);
    printf("data:%08x flag:%08x", get_fifo_reg_value(ADC_TEST_BASE), _int_flags);
    adc_clear_all_int_flag(ADC_TEST_BASE);
    adc_set_conv_end_int_mask_bit(ADC_TEST_BASE, false);
    adc_set_fifo_int_mask_bit(ADC_TEST_BASE, false);
    adc_set_CTC_int_mask_bit(ADC_TEST_BASE, false);
    adc_set_EOL_int_mask_bit(ADC_TEST_BASE, false);
    adc_set_tmr_int_mask_bit(ADC_TEST_BASE, false);
    return INT_NO_RESCHEDULE;
}

static inline void adc_mdelay(unsigned int delay_ms)
{
    lk_time_t start = current_time();

    while (start + delay_ms > current_time()) {
        __asm__ volatile("nop");
        __asm__ volatile("nop");
    }
}

static int do_adc_int_test(void)
{
    printf("interrupt test:\n");
    printf("register adc irq num:240 handler\n");
    register_int_handler(240, &adc_test_irq_handler, (void*)0); ////240 is adc irq_num IRQ_GIC1_ADC_IRQ_NUM
    printf("EOC int test:");

    adc_init(ADC_TEST_BASE);
    adc_set_resolution(ADC_TEST_BASE, ADC_12_BITS_E3);
    adc_set_interval_bits(ADC_TEST_BASE, 0x10);
    adc_set_convert_mode(ADC_TEST_BASE, ADC_SINGLE_CH_SINGLE_E0);
    adc_set_dma_en_bit(ADC_TEST_BASE, false);
    adc_set_dma_single_en_bit(ADC_TEST_BASE, false);
    adc_set_fifo_en_bit(ADC_TEST_BASE, false);
    adc_set_timertamp_en_bit(ADC_TEST_BASE, false);
    adc_start_convert(ADC_TEST_BASE);

    adc_mdelay(5);
    adc_stop_convert(ADC_TEST_BASE);

    printf("%s", ((_int_flags & ADC_EOC_INT_FLAG_E1) ? "pass\n" : "fail\n"));
    _int_flags = 0x0;
    printf("test end");
    //printf("bypass\n");
    return NO_ERROR;
}

//slt test input channel 1, input 12v, x9 ref will change 12v/8 --> 1.5v, adc ref 1.8v, so 12bit resolution value is 1.5/(1.8/4096). real input is 1.515v, 12bit resolution value is 3447.46
static int slt_adc_resolution_test(void)
{
    u32 result[ADC_CONVERT_SINGLE_TEST_RESULT_NUM];
    int chan_index, bit_mode;
    int ret;

    adc_reset_ctrl(ADC_TEST_BASE);
    adc_set_clk_divider_bits(ADC_TEST_BASE, 8);

    //only have channel 1
    chan_index = ADC_TEST_CHANNEL;

    printf("start test channel %d \n", chan_index);

    for (bit_mode = 0; bit_mode < ADC_CONVERT_MODE_TYPE_MAX_NUM; bit_mode++) {
        printf("ADC test bits%d mode:\n", 6 + (bit_mode * 2));
        adc_set_resolution(ADC_TEST_BASE, (adc_resolution_e_t)bit_mode);
        adc_init(ADC_TEST_BASE);

        printf("adc_reg%x dump= \n", ADC_TEST_BASE);
        hexdump((char*)ADC_TEST_BASE, 52);

        {
            ret = adc_single_convert(ADC_TEST_BASE, chan_index, (u32*)result + bit_mode);

            if (ret == 1) {
                printf("test return timetamp value\n");
            }
            else if (ret == 0) {
                printf("test return convert value\n");
            }
            else {
                printf("test fail\n");
                break;
            }

            printf("ch=%d:result:%d ", chan_index, result[bit_mode]);
            printf("adc_schc dump= \n");
            hexdump((char*)(ADC_TEST_BASE + 0x28), 4);
            printf("adc_rslt dump= \n");
            hexdump((char*)(ADC_TEST_BASE + 0x100), 4);
        }

        adc_stop_convert(ADC_TEST_BASE);
        adc_flush_fifo(ADC_TEST_BASE);

    }

    adc_dinit(ADC_TEST_BASE);

    //if value <= 1.5v or >=1.6v, test fail, because input is 1.515v
    if (((result[0] & 0x3f) != 0) || ((result[1] & 0xf) != 0) || ((result[2] & 0x3) != 0) || (result[3] <= ADC_RESULT_MIN) || (result[3] >= ADC_RESULT_MAX)) {
        ret = result[3] ? result[3] : -1;
        printf("adc test single convert fail, result =%d,%d,%d,%d\n", result[0], result[1], result[2], result[3]);
    }
    else {
        printf("adc test single convert pass, result =%d,%d,%d,%d\n", result[0], result[1], result[2], result[3]);
        ret = 0;
    }

    printf("resolution test end\n");

    return ret;
}

static int do_adc_resolution_test(void)
{
    u32 result[ADC_CONVERT_SINGLE_TEST_RESULT_NUM];
    int i, bit_mode;
    int ret;
    adc_reset_ctrl(ADC_TEST_BASE);
    adc_set_clk_divider_bits(ADC_TEST_BASE, 8);

    //only have five channel,
    for (i = 0; i < ADC_CONVERT_SINGLE_CHANNEL_MAX_NUM; i++) {

        printf("start test channel %d \n", i);

        for (bit_mode = 0; bit_mode < ADC_CONVERT_MODE_TYPE_MAX_NUM; bit_mode++) {
            printf("ADC test bits%d mode:\n", 6 + (bit_mode * 2));
            adc_set_resolution(ADC_TEST_BASE, (adc_resolution_e_t)bit_mode);
            adc_init(ADC_TEST_BASE);

            printf("adc_reg%x dump= \n", ADC_TEST_BASE);
            hexdump((char*)ADC_TEST_BASE, 52);

            {
                ret = adc_single_convert(ADC_TEST_BASE, i, (u32*)result + i);

                if (ret == 1) {
                    printf("test return timetamp value\n");
                }
                else if (ret == 0) {
                    printf("test return convert value\n");
                }
                else {
                    printf("test fail\n");
                    break;
                }

                printf("ch%d:result:%d ", i, result[i]);
                printf("adc_schc dump= \n");
                hexdump((char*)(ADC_TEST_BASE + 0x28), 4);
                printf("adc_rslt dump= \n");
                hexdump((char*)(ADC_TEST_BASE + 0x100), 4);
            }

            adc_stop_convert(ADC_TEST_BASE);
            adc_flush_fifo(ADC_TEST_BASE);

        }

    }

    if (bit_mode == 4) {
        printf("resolution test end\n");
    }

    adc_dinit(ADC_TEST_BASE);
    return ERR_GENERIC;
}

int slt_module_test_adc_test(uint times, uint timeout, char* result_string)
{
    int ret = 0;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    ret = slt_adc_resolution_test();

    if (result_string != NULL) {
        if (ret  != 0) {
            strcpy(result_string, "adc test fail, error resolution value is error code");
        }
        else {
            strcpy(result_string, "adc test pass");
        }
    }
    return ret;
}

static int do_adc_test_all(void)
{
    do_adc_resolution_test();
    do_adc_entries_test();
    do_dma_read_adc_test();
    do_adc_int_test();
    return NO_ERROR;
}

int do_adc_test(int argc, const cmd_args* argv)
{
    u64 addr = 0x01;
    //u64 base_addr = INIT_WRONG_BASE;

    if (2 != argc) {
        printf("%s", adc_test_help);
        return ERR_GENERIC;
    }

    if (!strcmp(argv[1].str, "help")) {
        printf("%s", adc_test_help);
        return NO_ERROR;
    }
    else if (!strcmp(argv[1].str, "all")) {
        return (do_adc_test_all());
    }
    else if (!strcmp(argv[1].str, "dmacfg")) {
        return (do_dma_cfg_adc_test());
    }
    else if (!strcmp(argv[1].str, "dmaread")) {
        return (do_dma_read_adc_test());
    }
    else if (!strcmp(argv[1].str, "interrupt")) {
        return (do_adc_int_test());
    }
    else if (!strcmp(argv[1].str, "entries")) {
        return (do_adc_entries_test());
    }
    else if (!strcmp(argv[1].str, "resolution")) {
        return (do_adc_resolution_test());
    }
    else {
        printf("%s", adc_test_help);
        return ERR_GENERIC;
    }

    printf("error:unknown error in adc_test\n");
    return ERR_GENERIC;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("adc_test", adc_test_help, (console_cmd)&do_adc_test)
STATIC_COMMAND("slt_adc", "slt_adc_test", (console_cmd)&slt_adc_resolution_test)
STATIC_COMMAND_END(adc_test_ops);
#endif

SLT_MODULE_TEST_HOOK(adc_test, slt_module_test_adc_test);

APP_START(adc_test)
.flags = 0
         APP_END

