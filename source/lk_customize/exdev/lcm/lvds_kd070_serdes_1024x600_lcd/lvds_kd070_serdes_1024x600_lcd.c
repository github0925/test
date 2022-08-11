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
static const uint32_t ub947_i2c_addr = 0x1A;
static const uint32_t ub948_i2c_addr = 0x3C;

static struct display_timing timing = {
    .hactive = 1024,
    .hfront_porch = 120,
    .hback_porch = 120,
    .hsync_len = 80,
    .vactive = 600,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 15,
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
    int ret = 0;
    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);
    ret = ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x12, 0x04);
    if (ret < 0) {
        LOGE("[err]: screen offline ! \n");
        return ret;
    }
#if 0
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1
#endif
    //OLDI_DUAL:Single-pixel mode,MAPSEL: OpenLDI Bit Mapping
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x4f, 0x40);

    //REPEATER:Disable repeater mode
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0xc2, 0x98);

    //COAX:Enable FPD-Link III for coaxial cabling
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x5b, 0xa0);


    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e);
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x03, 1<<3, 0x08);
    uint8_t value = 0;
    ds90ub9xx_read_reg(i2c_handle, ub947_i2c_addr, 0x06, &value);

    ds90ub9xx_write_reg(i2c_handle, ub948_i2c_addr, 0x49, 0x02);//set MAPSEL low

    LOGD("%s down\n", __func__);
    return 0;
}

struct sdm_panel lvds_kd070_serdes_1024x600_lcd = {
    .panel_name = "lvds_kd070_serdes_1024x600_lcd",
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
