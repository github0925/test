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

static uint32_t ub941_i2c_addr = 0x0C;
static uint32_t ub948_i2c_addr = 0x2C;

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 40,
    .hback_porch = 40,
    .hsync_len = 40,
    .vactive = 720,
    .vfront_porch = 50,
    .vback_porch = 50,
    .vsync_len = 10,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 600 * 1000,
};

static int panel_check_link(void)
{
    int ret = 0;
    uint8_t value;
    void *i2c_handle = NULL;
    uint32_t i2c_res = RES_I2C_I2C16;

    ret = hal_i2c_creat_handle(&i2c_handle, i2c_res);
    if (!ret) {
        LOGE("%s creat handle err! ret:%d\n", __func__, ret);
        return ret;
    }

    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x03, 0x08); //I2C Pass-through Port0/Port1
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x17, 0x9e); //config I2C Pass All

    ret = ds90ub9xx_read_reg(i2c_handle, ub941_i2c_addr, 0x06, &value);    //get slave addr
    if (!ret && value)
        ub948_i2c_addr = value >> 1;

    ret = ds90ub94x_read_gpio_reg(i2c_handle, ub948_i2c_addr, GPIO6_REG);
    if (ret == 1)
        mipi_hsd123_serdes_1920x720_lcd.fps = 60;    //dual link
    else
        mipi_hsd123_serdes_1920x720_lcd.fps = 51;

    LOGD("read gpio%d -> 0x%x\n", GPIO6_REG, ret);

    ds90ub941as_Enable_Dualdsi_DualPort(i2c_handle, ub941_i2c_addr);

    hal_i2c_release_handle(i2c_handle);

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
    int ret = 0;
    void *i2c_handle = NULL;

    LOGD("%s enter\n", __func__);

    ret = hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);
    if (!ret) {
        LOGE("%s creat handle err! ret:%d\n", __func__, ret);
        return ret;
    }

#ifndef BACKLIGHT_GPIO
    //ds90ub941as_bl_init(handle);
    ret = ds90ub94x_948_gpio_config(i2c_handle, ub941_i2c_addr, ub948_i2c_addr, GPIO0, OUTPUT, DES_PORT0);
    if (ret < 0)
        LOGE("%s backlight GPIO0 config error !\n", __func__);
#endif

    if (!(if_board_type(BOARD_TYPE_MS, PART_BOARD_TYPE) &&
        if_board_type(BOARDID_MAJOR_TI_A02, PART_BOARD_ID_MAJ)))
        ds90ub941as_init(i2c_handle, ub941_i2c_addr);

    mdelay(10);

    ds90ub948_init(i2c_handle, ub948_i2c_addr);

    LOGD("%s down\n", __func__);

    hal_i2c_release_handle(i2c_handle);
    return ret;
}
#if defined(S3SIN1)
static struct sis_info info_sis[2] = {
    {
        .pos_x = 0,
        .pos_y = 0,
        .width = 640,
        .height = 600,
    },
    {
        .pos_x = 640,
        .pos_y = 0,
        .width = 1280,
        .height = 600,
    }
};

static struct sis_info screen = {
    .pos_x = 0,
    .pos_y = 0,
    .width = 1920,
    .height = 720,
};
#endif

struct sdm_panel mipi_hsd123_serdes_1920x720_lcd = {
    .panel_name = "mipi_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE, // or IF_TYPE_I2C
    .width_mm = 240,
    .height_mm = 160,
    .fps = 51,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
#if defined(S3SIN1)
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

