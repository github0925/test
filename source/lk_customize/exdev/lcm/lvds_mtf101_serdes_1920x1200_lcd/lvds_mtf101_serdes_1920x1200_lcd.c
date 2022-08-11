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
#include <hal_port.h>
#include <hal_dio.h>


#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 128,
    .hback_porch = 12,
    .hsync_len = 20,
    .vactive = 1200,
    .vfront_porch = 19,
    .vback_porch = 20,
    .vsync_len = 4,

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

extern const domain_res_t g_gpio_res;

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
    static void *dio_handle;
    bool ioret;

    LOGD("cuishang >>> d9 lvds screen \n");
    //backlight
    //GPIO_A5_LCD_PWDN_N PULL heigh GPIO.IO29
    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (!ioret) {
        return -1;
    }
    hal_dio_write_channel(dio_handle, PortConf_PIN_GPIO_A5, 1);
    dprintf(0, "%s(): config PortConf_PIN_GPIO_A5 pull high for backlight EN\n",
        __func__);
    hal_dio_release_handle(&dio_handle);

    //CPU_GPIO_D8_PWM5_CH0 pwm
#else
    static uint8_t config_flg = false;
    if (config_flg > 0)
        return 0;

    static void *i2c_handle;
    uint32_t ub947_i2c_addr = 0x1A;
    uint32_t ub948_i2c_addr = 0x3C;
    LOGD(" config x9u b lvds 12 request i2c14 \n");
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);
    //config vido
    //947 lvds input config
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x4F, 0x80, 0xC0);
    //947 Single FPD-Link III mode.
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x5B, 3, 0x03);
    //config 948 i2c
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e);
    //948 Single FPD Link III based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x08, 0x18);
    //948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);

    LOGD("config x9u b lvds 3 request i2c15 \n");
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);
    //947 lvds input config
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x4F, 0xc0, 0xC0);
    //947 Single FPD-Link III mode.
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub947_i2c_addr, 0x5B, 1, 0x03);
    //config 948 i2c
    ds90ub9xx_write_reg(i2c_handle, ub947_i2c_addr, 0x17, 0x9e);
    //948 Single FPD Link III based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x10, 0x18);
    //948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);

    config_flg = true;
#endif
    LOGD("%s down\n", __func__);

    return 0;
}

struct sdm_panel lvds_mtf101_serdes_1920x1200_lcd = {
    .panel_name = "lvds_mtf101_serdes_1920x1200_lcd",
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
