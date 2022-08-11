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

static const uint32_t bl_ub947_i2c_addr = 0x1A;
static const uint32_t bl_ub948_i2c_addr = 0x2C;

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 40,
    .hback_porch = 40,
    .hsync_len = 40,

    .vactive = 720,
    .vfront_porch = 50,
    .vback_porch = 50,
    .vsync_len = 10,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

static int panel_check_link(void)
{
    int ret = 0;
    void *bl_i2c_handle = NULL;
    static uint8_t cont = 0;
    uint32_t i2c_res = RES_I2C_I2C14;

    LOGD("%s enter\n", __func__);

    if(cont > 0)
        i2c_res = RES_I2C_I2C15;    //second lvds panel

    ret = hal_i2c_creat_handle(&bl_i2c_handle, i2c_res);
    if (!ret) {
        LOGE("%s creat handle err! ret:%d\n", __func__, ret);
        return ret;
    }

    ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x03, 0x08); //I2C Pass-through Port0/Port1
    ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x17, 0x9e); //config I2C Pass All

    mdelay(10);

    ret = ds90ub94x_read_gpio_reg(bl_i2c_handle, bl_ub948_i2c_addr, GPIO6_REG);
    if (ret == 1)
        lvds_hsd123_serdes_1920x720_lcd.fps = 60;
    else
        lvds_hsd123_serdes_1920x720_lcd.fps = 51;

    LOGD("read gpio%d -> 0x%x\n", GPIO6_REG, ret);

    hal_i2c_release_handle(bl_i2c_handle);

    cont ++;

    LOGD("%s down\n", __func__);

    return ret;
}


static int panel_post_begin(void)
{
    if (if_board_type(BOARD_TYPE_MS, PART_BOARD_TYPE) &&
        if_board_type(BOARDID_MAJOR_TI_A02, PART_BOARD_ID_MAJ))
        panel_check_link();

    return 0;
}

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);

    int ret = 0;
    //ds90ub947_bl_init();
    static uint8_t bl_config_flg = false;
    if (bl_config_flg > 0)
        goto BACKLIGHT_GPIO_OUT;

    void *bl_i2c_handle = NULL;

    ret = hal_i2c_creat_handle(&bl_i2c_handle, RES_I2C_I2C14);
    if (!ret) {
        LOGE("%s creat handle err! ret:%d\n", __func__, ret);
        goto SEC_SERDES_INIT;
    }

    ret = ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x4f, 0x80);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);

    ret = ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x5b, 0x20);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);

#ifndef BACKLIGHT_GPIO
    ret = ds90ub94x_948_gpio_config(bl_i2c_handle, bl_ub947_i2c_addr, bl_ub948_i2c_addr, GPIO0, OUTPUT, DES_PORT0);
    if (ret < 0)
        LOGE("%s backlight GPIO0 config error !\n", __func__);
#endif

    hal_i2c_release_handle(bl_i2c_handle);

SEC_SERDES_INIT:
    bl_config_flg = true;

    ret = 0;
    bl_i2c_handle = NULL;

    ret = hal_i2c_creat_handle(&bl_i2c_handle, RES_I2C_I2C15);
    if (!ret) {
        LOGE("%s 2 creat handle err! ret:%d\n", __func__, ret);
        return ret;
    }

    ret = ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x4f, 0x80);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);

    ret = ds90ub9xx_write_reg(bl_i2c_handle, bl_ub947_i2c_addr, 0x5b, 0x20);
    if (ret < 0)
        LOGE("%s set dual mode is err !\n", __func__);

#ifndef BACKLIGHT_GPIO
    ret = ds90ub94x_948_gpio_config(bl_i2c_handle, bl_ub947_i2c_addr, bl_ub948_i2c_addr, GPIO0, OUTPUT, DES_PORT0);
    if (ret < 0)
        LOGE("%s backlight GPIO0 config error !\n", __func__);
#endif

    hal_i2c_release_handle(bl_i2c_handle);

BACKLIGHT_GPIO_OUT:

#if 0
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);

    ds90ub947_init(i2c_handle, ub947_i2c_addr);
    mdelay(10);
    ds90ub948_match_947_init(i2c_handle, ub948_i2c_addr);
#endif

#if defined(TARGET_REFERENCE_X9U)
{
    static uint8_t config_flag = false;
    if (config_flag > 0)
        goto X9U_SERDES_CONFIG_OUT;
    void *i2c_handle;
    const uint32_t ub947_i2c_addr = 0x1A;
    const uint32_t ub948_i2c_addr = 0x2C;
    LOGD("config x9u lvds 12 \n");
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);
    //config vido
    //947 lvds input config
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x4F, 0x80, 0xC0);
    //947 Single FPD-Link III mode.
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x5B, 1, 0x03);
    //config 948 i2c
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e);
    //948 Single FPD Link III based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x10, 0x18);
    //948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);
    hal_i2c_release_handle(i2c_handle);
    config_flag = true;
}
X9U_SERDES_CONFIG_OUT:
#endif

    LOGD("%s down\n", __func__);

    return ret;
}

struct sdm_panel lvds_hsd123_serdes_1920x720_lcd = {
    .panel_name = "lvds_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 51,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};
