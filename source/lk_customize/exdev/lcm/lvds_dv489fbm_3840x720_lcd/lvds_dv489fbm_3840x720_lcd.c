/*
* lvds_dv489fbm_3840x720_lcd.c
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
    .hactive = 3840,
    .hfront_porch = 200,
    .hback_porch = 200,
    .hsync_len = 160,

    .vactive = 720,
    .vfront_porch = 35,
    .vback_porch = 35,
    .vsync_len = 20,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,

    .map_format = LVDS_MAP_FORMAT_SWPG,
};

#ifndef PANEL_DOWN_OFFSET
#define PANEL_DOWN_OFFSET 26
#endif
static struct sis_info info_sis[2] = {
    {
        .pos_x = 0,
        .pos_y = 0 + PANEL_DOWN_OFFSET,
        .width = 1280,
        .height = 480,
    },
    {
        .pos_x = 2560,
        .pos_y = 0 + PANEL_DOWN_OFFSET,
        .width = 1280,
        .height = 480,
    }
};

static struct sis_info screen = {
    .pos_x = 1280,
    .pos_y = 0 + PANEL_DOWN_OFFSET,
    .width = 1280,
    .height = 480,
};

struct sdm_panel lvds_dv489fbm_3840x720_lcd = {
    .panel_name = "lvds_dv489fbm_3840x720_lcd",
    .if_type = IF_TYPE_LVDS,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 217,
    .height_mm = 136,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 2,
    .sis = info_sis,
    .rtos_screen = &screen,
};


