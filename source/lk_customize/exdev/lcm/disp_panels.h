/*
* disp_panels.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/
#ifndef __DISP_LCM_H__
#define __DISP_LCM_H__
#include "disp_hal.h"
#include "sdm_panel.h"
#ifdef SUPPORT_BOARDINFO
#include <boardinfo_hwid_usr.h>
#endif

extern struct sdm_panel mipi_nt35597_boe_1280x720;
extern struct sdm_panel hdmi_adv7511_1280_720_lcd;
extern struct sdm_panel mipi_nt35596_boe_1920x1080;
extern struct sdm_panel lvds_atk10_1_1280x800_lcd;
extern struct sdm_panel mipi_hsd123_1920x720_lcd;
extern struct sdm_panel mipi_hsd123_1920x720_2_lcd;
extern struct sdm_panel lvds_youda_1920x720_lcd;
extern struct sdm_panel mipi_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_hsd123_serdes_1920x720_2_lcd;
extern struct sdm_panel mipi_hsd123_serdes_1920x720_v9_lcd;
extern struct sdm_panel mipi_hsd123_serdes_1920x720_v9ts_lcd;
extern struct sdm_panel lvds_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel lvds_atk10_1_serdes_1280x800_lcd;
extern struct sdm_panel lvds_dv489fbm_3840x720_lcd;
extern struct sdm_panel mipi_kd070_serdes_1024x600_lcd;
extern struct sdm_panel mipi_hsd101_serdes_1280x800_lcd;
extern struct sdm_panel mipi_hsd101_serdes_1280x800_2_lcd;
extern struct sdm_panel lvds_hsd101_serdes_1280x800_lcd;
extern struct sdm_panel mipi_ICL02_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_ICL02_hsd162_serdes_2608x720_lcd;
extern struct sdm_panel lvds_ICL02_hsd123_serdes_1920x720_lcd;
extern struct sdm_panel mipi_mtf101_serdes_1920x1200_lcd;
extern struct sdm_panel mipi_mtf101_serdes_1920x1200_2_lcd;
extern struct sdm_panel lvds_mtf101_serdes_1920x1200_lcd;
extern struct sdm_panel lvds_kd070_serdes_1024x600_lcd;
extern struct sdm_panel lvds_hsd156_serdes_1920x1080_lcd;
extern struct sdm_panel mipi_lt9611_to_hdmi_1920x1080_lcd;
extern struct sdm_panel mipi_lt9611_to_hdmi_1920x1080_2_lcd;
extern struct sdm_panel parallel_gm7123_to_vga_1920x1080_lcd;

extern struct sdm_panel **registered_panels[];

int sdm_panel_probe(display_handle *handle, int sub_id, struct sdm_panel *panels[], int n_panels);
int sdm_connect_panel(display_handle *handle, int sub_id, struct sdm_panel *panel);
int sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel);

static inline int if_board_type(int type, int board_type)
{
#ifdef SUPPORT_BOARDINFO
    return (type == get_part_id(board_type));
#else
    return 0;
#endif
}

#endif
