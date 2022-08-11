/*
* mipi_lt9611_to_hdmi_1920x1080_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/

#include "sdm_panel.h"
#include "disp_panels.h"
#include <lt9611.h>
#include <hal_port.h>
#include <hal_dio.h>
#include <thread.h>
#include "tca9539.h"


#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

#define DSI_TO_HDMI_TIMING 1080

static uint8_t reset_timing[] = {
    5, 20, 5,
};
#if (DSI_TO_HDMI_TIMING == 1080)
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 1080,
    .vfront_porch = 4,
    .vback_porch = 36,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 1039500,
};
#else
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 693000,
};
#endif

extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

//PWR_EN2 1.8V
static void config_1_8v_power_pin(int level)
{
    struct tca9539_device *pd;
    printf("\n%s: \n", __func__);
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, TCA9539_P02);
    pd->ops.output_val(pd, TCA9539_P02, level);
    tca9539_deinit(pd);
    printf("\n%s: end\n", __func__);
}

static void dsi_power_port_config(void)
{
    static void *port_handle;
    static void *dio_handle;
    bool ioret;

    //PWR_EN1 pull high 10ms earlier than PWR_EN2.
    // PWR_EN1 3V3 port en
    ioret = hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    if (!ioret) {
        return;
    }
    hal_dio_write_channel(dio_handle, PortConf_PIN_GPIO_C9, 1);
    hal_dio_release_handle(&dio_handle);

    //PWR_EN2 1.8V en
    mdelay(10);
    config_1_8v_power_pin(1);
}

//RST_N
static void config_lt9611_reset_pin(int level)
{
    struct tca9539_device *pd;
    printf("\n%s: \n", __func__);
    pd = tca9539_init(12, 0x76);

    if (pd == NULL) {
        printf("init tca9359 error!\n");
        return ;
    }
    tca9539_enable_i2cpoll(pd);

    pd->ops.output_enable(pd, TCA9539_P01);
    pd->ops.output_val(pd, TCA9539_P01, level);
    tca9539_deinit(pd);
    printf("\n%s: end\n", __func__);
}

static void lt9611_chip_reset(void)
{
    config_lt9611_reset_pin(0);
    mdelay(20);
    config_lt9611_reset_pin(1);
}

static int panel_post_begin(void)
{
    //screen 1 is set
    //gpio config
    //dsi_power_port_config();
    return 0;
}

//static thread_t *lt9611_thread;
static int panel_post_end(void)
{
    printf("%s enter\n", __func__);
    const uint8_t i2c_addr = 0x3B;
    lt9611_i2c_init(RES_I2C_I2C11, i2c_addr);

    //LT9611_Reset
    lt9611_chip_reset();

    LT9611_Init();
//    lt9611_thread = thread_create("lt9611", LT9611_Init, NULL, DEFAULT_PRIORITY, 4096);
//    thread_detach_and_resume(lt9611_thread);

    printf("%s down\n", __func__);

    return 0;
}

struct sdm_panel mipi_lt9611_to_hdmi_1920x1080_2_lcd = {
    .panel_name = "mipi_lt9611_to_hdmi_1920x1080_2_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE, // or IF_TYPE_I2C
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};

