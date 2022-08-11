/*
* mipi_hsd101_serdes_1280x800_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 09/229/2020 BI create this file
*/

#include "sdm_panel.h"
#include "disp_panels.h"
#include <ds90ub9xx.h>
#include <i2c_hal.h>

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

static void *i2c_handle;
static const uint32_t ub941_i2c_addr = 0x0C;
static const uint32_t ub948_i2c_addr = 0x2C;

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 48,
    .hback_porch = 80,
    .hsync_len = 32,

    .vactive = 800,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 3,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 500 * 1000,
};

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    ds90ub941as_init(i2c_handle, ub941_i2c_addr);
    mdelay(10);
    ds90ub948_init(i2c_handle, ub948_i2c_addr);

    LOGD("%s down\n", __func__);

    return 0;
}

struct sdm_panel mipi_hsd101_serdes_1280x800_lcd = {
    .panel_name = "mipi_hsd101_serdes_1280x800_lcd",
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

