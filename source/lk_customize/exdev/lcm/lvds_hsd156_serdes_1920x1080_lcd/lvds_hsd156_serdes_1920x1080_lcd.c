/*
* lvds_nt35596_boe_1920x1080_lcd.c
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

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 80,
    .hback_porch = 80,
    .hsync_len = 80,

    .vactive = 1080,
    .vfront_porch = 15,
    .vback_porch = 15,
    .vsync_len = 14,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    printf("lvds_hsd156_serdes_1920x1080_lcd panel_post_end enter\n");

    //ds90ub947_bl_init();
    static void *i2c_handle;
    uint8_t value = 0;
    static uint32_t ub947_i2c_addr = 0x1A;
    static const uint32_t ub948_i2c_addr = 0x3C;
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);

    ds90ub9xx_read_reg(i2c_handle, ub947_i2c_addr, 0x06, &value);
    if(value == 0)
    {
        printf("i2c address get error, Diaplay panel not connected\n");
        return -1;
    }

    printf("ub948 i2c address:0x%x\n", value >> 1);

    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x1E, 0x04); //Use I2C ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e); //config I2C Pass All
    // 947 dsi input config
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x4F, 0x00, 0xC0);
    // 947 Auto-Detect FPD-Link III mode.
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x5B, 0, 0x01);
    // 948 Auto-detect based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x8, 0x18);
    // 948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);
    // config 948 i2c
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x03, 0x8, 0x08);

    printf("%s down\n", __func__);

    return 0;
}

#if defined(S3SIN1_1920X1080)
static struct sis_info info_sis[2] = {
    {
        .pos_x = 0,
        .pos_y = 0,
        .width = 640,
        .height = 960,
    },
    {
        .pos_x = 640,
        .pos_y = 0,
        .width = 1280,
        .height = 960,
    }
};

static struct sis_info screen = {
    .pos_x = 0,
    .pos_y = 0,
    .width = 1920,
    .height = 1080,
};
#endif

struct sdm_panel lvds_hsd156_serdes_1920x1080_lcd = {
    .panel_name = "lvds_hsd156_serdes_1920x1080_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 344,
    .height_mm = 193,
    .fps = 60,
    .display_timing = &timing,
#if defined(S3SIN1_1920X1080)
	.sis_num = 2,
	.sis = info_sis,
	.rtos_screen = &screen,
#else
	.sis_num = 0,
	.sis = NULL,
	.rtos_screen = NULL,
#endif

    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};
