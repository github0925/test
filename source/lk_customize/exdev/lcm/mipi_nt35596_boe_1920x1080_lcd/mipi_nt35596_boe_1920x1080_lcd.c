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


static struct dsi_cmd_desc init_data[] = {
    {0x13, 0x78, 0x00, 0x01, {0x11}},
    {0x13, 0x64, 0x00, 0x01, {0x29}}
};

static struct dsi_cmd_desc sleep_data[] = {
    {0x13, 0x0A, 0x00, 0x01, {0x28}},
    {0x13, 0x78, 0x00, 0x01, {0x10}}
};

static uint8_t cmd_i2c_data[] = {

};

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 10,
    .hback_porch = 20,
    .hsync_len = 2,
    .vactive = 1080,
    .vfront_porch = 5,
    .vback_porch = 16,
    .vsync_len = 1,
};

static struct info_mipi mipi_data = {
    .init_data = init_data,
    .sleep_data = sleep_data,
};

struct sdm_panel mipi_nt35596_boe_1920x1080 = {
    .panel_name = "mipi_nt35596_boe_1920x1080_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_I2C, // or CMD_INTF_NONE
    .cmd_data = cmd_i2c_data,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
};

