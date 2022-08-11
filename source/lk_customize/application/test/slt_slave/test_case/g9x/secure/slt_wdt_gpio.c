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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include <lib/slt_module_test.h>
#include <trace.h>

#include <kernel/semaphore.h>

#include "chip_res.h"
#include <platform.h>
#include <wdg_hal.h>

/*
#undef  LOCAL_TRACE
#define LOCAL_TRACE 1
 */

static volatile uint32_t slt_wdt_int;
static semaphore_t slt_wdt_sem;

enum handler_return slt_wdt_handle(void *arg)
{
    LTRACEF("\n");

    hal_wdg_disable_interrupts(arg);
    hal_wdg_int_clear(arg);

    slt_wdt_int = current_time();
    sem_post(&slt_wdt_sem, 1);

    return INT_NO_RESCHEDULE;
}

int TEST_SEC_SS_17(uint times, uint timeout, char* result_string)
{
    wdg_app_config_t wdg_app_cfg;
    int ret = -1;
    uint32_t wdg_refresh_time;
    void *watchdog_handle;

    LTRACEF(" entry, times= %d, timeout =%d\n", times, timeout);

    if (!result_string) {
        LTRACEF("fail cause no memory");
        return -__LINE__;
    }

    slt_wdt_int = 0;
    sem_init(&slt_wdt_sem, 0);

    hal_wdg_creat_handle(&watchdog_handle, RES_WATCHDOG_WDT3);
    if (!watchdog_handle) {
        strcpy(result_string, "create handle fail");
        return -__LINE__;
    }

    wdg_app_cfg.workMode = wdg_mode1;
    wdg_app_cfg.seqDeltaValue = 100; //ms
    wdg_app_cfg.timeoutValue = 500; //ms
    wdg_app_cfg.windowLimitValue = 200;//ms
    ret = hal_wdg_init(watchdog_handle,  &wdg_app_cfg);
    if (!ret) {
        strcpy(result_string, "hal_wdg_init fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

    ret = hal_wdg_set_timeout(watchdog_handle, wdg_app_cfg.timeoutValue);
    if (!ret) {
        strcpy(result_string, "hal_wdg_set_timeout fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

    ret = hal_wdg_int_register(watchdog_handle, slt_wdt_handle, 1);
    if (!ret) {
        strcpy(result_string, "hal_wdg_int_register fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

    ret = hal_wdg_enable_interrupts(watchdog_handle);
    if (!ret) {
        strcpy(result_string, "hal_wdg_enable_interrupts fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

    ret = hal_wdg_enable(watchdog_handle);
    if (!ret) {
        strcpy(result_string, "hal_wdg_enable fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

    wdg_refresh_time = 3;
    while (wdg_refresh_time--) {
        thread_sleep(200);
        hal_wdg_refresh(watchdog_handle);
    }

    if (slt_wdt_int) {
        strcpy(result_string, "unexpected wdt interrupt");
        ret = -__LINE__;
        goto wdt_exit;
    }

    wdg_refresh_time = current_time();
    sem_timedwait(&slt_wdt_sem, 1000);
    printf("%s measure time = %d\n", __func__, slt_wdt_int - wdg_refresh_time);
    if (!slt_wdt_int || (slt_wdt_int - wdg_refresh_time > 600)) {
        strcpy(result_string, "missing wdt interrupt");
        ret = -__LINE__;
        hal_wdg_disable_interrupts(watchdog_handle);
        goto wdt_exit;
    }

    ret = hal_wdg_disable(watchdog_handle);
    if (ret) {
        ret = 0;
    } else {
        strcpy(result_string, "hal_wdg_disable fail");
        ret = -__LINE__;
        goto wdt_exit;
    }

wdt_exit:
    hal_wdg_disable(watchdog_handle);
    hal_wdg_deinit(watchdog_handle);
    hal_wdg_release_handle(watchdog_handle);
    sem_destroy(&slt_wdt_sem);

    LTRACEF("%s ret:%d:\n", __func__, ret);

    return ret;
}



#include <hal_port.h>
#include <hal_dio.h>

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

static Port_PinModeType GPIO_G4_mode = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};
static Port_PinModeType GPIO_G5_mode = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

int TEST_SEC_SS_18(uint times, uint timeout, char* result_string)
{
    void *slt_port_handle;
    void *slt_dio_handle;
    int ret;

    if (!result_string) {
        LTRACEF("fail cause no memory");
        return -__LINE__;
    }

    // Port setup
    ret = hal_port_creat_handle(&slt_port_handle, g_iomuxc_res.res_id[0]);

    ret = hal_port_set_pin_mode(slt_port_handle, PortConf_PIN_I2S_SC4_WS,
                                GPIO_G4_mode);
    ret = hal_port_set_pin_mode(slt_port_handle, PortConf_PIN_I2S_SC4_SD,
                                GPIO_G5_mode);
    ret = hal_port_set_to_gpioctrl(slt_port_handle, GPIO_CTRL_2, PortConf_PIN_I2S_SC4_WS);
    ret = hal_port_set_to_gpioctrl(slt_port_handle, GPIO_CTRL_2, PortConf_PIN_I2S_SC4_SD);

    // Dio setup
    ret = hal_dio_creat_handle(&slt_dio_handle, g_gpio_res.res_id[0]);

    hal_dio_write_channel(slt_dio_handle, PortConf_PIN_I2S_SC4_WS, 1);
    spin(1);
    if (!hal_dio_read_channel(slt_dio_handle, PortConf_PIN_I2S_SC4_SD)) {
        strcpy(result_string, "G4=1 but G5 is not. check connection between J12.2 & J13.2");
        ret = -__LINE__;
        goto gpio_exit;
    }

    hal_dio_write_channel(slt_dio_handle, PortConf_PIN_I2S_SC4_WS, 0);
    spin(1);
    if (hal_dio_read_channel(slt_dio_handle, PortConf_PIN_I2S_SC4_SD)) {
        strcpy(result_string, "G4=0 but G5 is not. check connection between J12.2 & J13.2");
        ret = -__LINE__;
        goto gpio_exit;
    }

    ret = 0;

gpio_exit:
    hal_port_set_pin_direction(slt_port_handle, PortConf_PIN_I2S_SC4_WS, PORT_PIN_IN);
    hal_port_release_handle(&slt_port_handle);
    hal_dio_release_handle(&slt_dio_handle);

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

SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_17, TEST_SEC_SS_17, SLT_MODULE_TEST_LEVEL_DDR, 0, 1, 5000, 0, 0);
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_18, TEST_SEC_SS_18, SLT_MODULE_TEST_LEVEL_SAMPLE_3, 0, 1, 2000, 0, 0);


