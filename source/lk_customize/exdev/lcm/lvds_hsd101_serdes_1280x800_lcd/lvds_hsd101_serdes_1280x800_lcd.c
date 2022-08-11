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
#if (TARGET_REFERENCE_X9U == 1)
static const uint32_t ub947_i2c_addr = 0x0C;
#else
static const uint32_t ub947_i2c_addr = 0x1A;
#endif
static const uint32_t ub948_i2c_addr = 0x2C;

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 48,
    .hback_porch = 80,
    .hsync_len = 32,

    .vactive = 800,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 3,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_JEIDA,
};

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);
    LOGD("request i2c15 \n");

    ds90ub947_init(i2c_handle, ub947_i2c_addr);
    mdelay(10);
    ds90ub948_match_947_init(i2c_handle, ub948_i2c_addr);

    LOGD("%s down\n", __func__);
    return 0;
}

struct sdm_panel lvds_hsd101_serdes_1280x800_lcd = {
    .panel_name = "lvds_hsd101_serdes_1280x800_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};
