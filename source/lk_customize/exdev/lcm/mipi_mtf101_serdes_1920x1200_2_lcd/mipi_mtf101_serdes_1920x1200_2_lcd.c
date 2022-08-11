/*
* mipi_nt35596_boe_1920x1080_lcd.c
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
#include <ds90ub9xx.h>
#include <i2c_hal.h>

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

static void *i2c_handle;

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 128,
    .hback_porch = 12,
    .hsync_len = 20,
    .vactive = 1200,
    .vfront_porch = 19,
    .vback_porch = 20,
    .vsync_len = 4,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 700000,
};

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    ds90ub9xx_write_reg(i2c_handle, 0x0c, 0x1E, 0x04);
    uint8_t value = 0;
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(i2c_handle, 0x0c, 0x17, value);
    ds90ub9xx_read_reg(i2c_handle, 0x0c, 0x06, &value);

    ds90ub9xx_write_reg(i2c_handle, 0x0d, 0x1E, 0x04);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(i2c_handle, 0x0d, 0x17, value);
    ds90ub9xx_read_reg(i2c_handle, 0x0d, 0x06, &value);

    LOGD("%s down\n", __func__);

    return 0;
}

struct sdm_panel mipi_mtf101_serdes_1920x1200_2_lcd = {
    .panel_name = "mipi_mtf101_serdes_1920x1200_2_lcd",
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

