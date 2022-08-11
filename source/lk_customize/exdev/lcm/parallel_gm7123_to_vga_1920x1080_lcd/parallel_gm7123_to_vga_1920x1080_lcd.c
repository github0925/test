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

#define PAALLEL_TO_VGA_TIMING 720

#if (PAALLEL_TO_VGA_TIMING == 1080) //1080p
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 1080,
    .vfront_porch = 4,
    .vback_porch = 36,
    .vsync_len = 5,
};

#elif (PAALLEL_TO_VGA_TIMING == 1920720)//1920x720
static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 88,
    .hback_porch = 148,
    .hsync_len = 44,
    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,
};

#elif (PAALLEL_TO_VGA_TIMING == 720) //720p
static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 110,
    .hback_porch = 220,
    .hsync_len = 40,
    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,
};
#endif



struct sdm_panel parallel_gm7123_to_vga_1920x1080_lcd = {
    .panel_name = "parallel_gm7123_to_vga_1920x1080_lcd",
    .if_type = IF_TYPE_PARALLEL,
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


