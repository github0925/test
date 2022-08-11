/*
 * wake_up.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "board_cfg.h"
#include "board_init.h"
#include "func_dio.h"
#include "func_i2c.h"
#include <pmu_hal.h>
#include <rtc_drv.h>
#include <irq_v.h>
#include <arm_gic_hal.h>

#define IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR (APB_IOMUXC_RTC_BASE)
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x214<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x18<<0))

void system_wake_up_trigger(void)
{
    com_gpio_set_system_power_mgr(POWER_ON);

    i2cx_gpio_set_system_power_mgr(POWER_ON);
}

enum handler_return wakeup_dispose_callback(void *arg)
{
    rtc_wakeup_enable(APB_RTC1_BASE, false, false, false);
    rtc_clr_wakeup_status(APB_RTC1_BASE);
    return 0;
}
/*set chip's internel wakeup*/
int set_soc_internal_wakeup(void)
{
    int ret = 0;
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

    ret = hal_pmu_set_internal_wakeup_enable(handle, 0, 1);

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
/*set chip's external wakeup*/
int set_soc_external_wakeup(void)
{
    void *handle;
    int ret = 0;

    can_bus_wakeup_init();

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

    /*
     * Wakeup delay by enable debounce.
     * 1) Wake0 low level wakeup  ===\
     *                                |- The two step clear PMU debounce cnt
     * 2) Wake0 high level wakeup ===/
     * 3) Set wakeup debounce time to 16ms
     * 4) Enable wakeup debounce
     */
    hal_pmu_set_external_wakeup_polarity(handle, 0, 0);
    ret = hal_pmu_set_external_wakeup_polarity(handle, 0, 1);

    if (ret != 0) {
        printf("pmu set wakeup polarity fail\n");
        return ret;
    }

    ret = hal_pmu_set_external_wakeup_debounce_delay(handle, 0, 15);

    if (ret != 0) {
        printf("pmu set wakeup debounce delay fail\n");
        return ret;
    }

    ret = hal_pmu_set_external_wakeup_debounce_enable(handle, 0, 1);

    if (ret != 0) {
        printf("pmu set wakeup debounce enable fail\n");
        return ret;
    }

    ret = hal_pmu_set_external_wakeup_enable(handle, 0, 1);

    if (ret != 0) {
        printf("pmu set external wakeup fail\n");
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

    printf("test ok\n");
    return 0;
}
/*chip's external wakeup pin enable*/
static void iomux_PMU_WAKEUP0_1_ENABLE( int direction) //0:normal ,1:opendrain
{
    int addr;
    int wdata;
    //PAD_NAME: *****SYS_WAKEUP0****
    //MUX_NAME: *****PMU_WAKEUP0****
    //config pad oe bit 4 and pad mux bit 0,1,2
    addr = REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP0;
    wdata = 1;
    wdata = ((wdata & 0xffffffef) | (direction << 4));
    writel(wdata, addr);
    //config select bit 0,1,2,3
    //no select define in register h file
}
/*chip's external wakeup pin enable*/
static void iomux_PMU_WAKEUP0_1_PULL_ENABLE(int pull_up_or_down,
        int pull_up_or_down_en) //pull_up_or_down:0-down,1-up
{
    int addr;
    int wdata;

    addr = REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP0;
    wdata = readl(addr);
    wdata = ((wdata & 0xfffffffc) | (pull_up_or_down << 1) | pull_up_or_down_en );
    writel(wdata, addr);
}

void can_bus_wakeup_init(void)
{
    iomux_PMU_WAKEUP0_1_ENABLE(1);
    iomux_PMU_WAKEUP0_1_PULL_ENABLE(0, 1);
}
