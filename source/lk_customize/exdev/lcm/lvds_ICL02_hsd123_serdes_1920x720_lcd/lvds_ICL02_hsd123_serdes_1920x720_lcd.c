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

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 52,
    .hback_porch = 32,
    .hsync_len = 24,
    .vactive = 720,
    .vfront_porch = 8,
    .vback_porch = 21,
    .vsync_len = 3,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

static uint8_t CheckSum(uint8_t *addr, int count)
{
    uint32_t sum = 0;
    while(count > 0)
    {
        sum = sum + *addr;
        addr += 1;
        count -= 1;
    }
    while(sum >> 8)
        sum = (sum & 0xff) + (sum >> 8);

    return (uint8_t)(~sum);
}

static int dump_948_rgeister(void *handle, int8_t addr_948)
{
    uint8_t value = 0;
    LOGD("%s start!!!\n",__func__);
    ds90ub9xx_read_reg(handle, addr_948, 0x34, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x49, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x1f, &value);
    //ds90ub9xx_read_reg(handle, addr_948, 0x06, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x1C, &value);
    LOGD("%s end!!!\n",__func__);
    return 0;
}

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    static void *i2c_handle;
    const uint32_t ub947_i2c_addr = 0x0C;
    const uint32_t ub948_i2c_addr = 0x2C;

    LOGD("%s enter\n", __func__);

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);
    //config vido
    //947 dsi input config
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x4F, 0x80, 0xC0);
    //947 Auto-Detect FPD-Link III mode.
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x5B, 0, 0x01);
    //config 948 i2c
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e);
    //948 Auto-detect based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x8, 0x18);
    //948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);

    //config backlight
    //enable 948 bl pin
    ds90ub9xx_write_reg(i2c_handle, ub948_i2c_addr, 0x1f, 0x09);
    LOGD("%s config gpio3 backlight en pin hight !\n", __func__);
    //read rtc time select day or night
    uint8_t value = 0;
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x03, 1<<3, 0x08);
    ds90ub9xx_read_reg(i2c_handle, ub947_i2c_addr, 0x17, &value);
    //config default bl level
    const uint32_t slave_i2c_addr = 0x18;
    int ret = 0;
    uint8_t bl_buf[7] = {0x01, 0x00, 15, 0x00, 0x00, 0x00, 0x00};
    bl_buf[6] = CheckSum(bl_buf, 7);
    ret = hal_i2c_write_reg_data(i2c_handle, slave_i2c_addr, bl_buf, 0, bl_buf, 7);
    if (ret < 0)
        LOGE("[err]:%s bl level set failed !\n", __func__);

    LOGD("%s down\n", __func__);

    return 0;
}

struct sdm_panel lvds_ICL02_hsd123_serdes_1920x720_lcd = {
    .panel_name = "lvds_ICL02_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 292,
    .height_mm = 109,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};
