/*
* lvds_atk10_1_1280x800_lcd.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/01/2020 BI create this file
*/
#include "sdm_panel.h"
#include "disp_panels.h"

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


struct sdm_panel lvds_youda_1920x720_lcd = {
    .panel_name = "lvds_youda_1920x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 217,
    .height_mm = 136,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
};


