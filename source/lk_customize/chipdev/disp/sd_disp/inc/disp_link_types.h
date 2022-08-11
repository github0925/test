/*
* disp_link_types.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/31/2019 BI create this file
*/
#ifndef __DISP_LINK_TYPES__
#define __DISP_LINK_TYPES__
enum {
    DP_RES_NC = -1,
    DP_RES_DP1,
    DP_RES_DP2,
    DP_RES_DP3,
    DP_RES_DP1_DP2
};

enum {
    DC_RES_NC = -1,
    DC_RES_DC1,
    DC_RES_DC2,
    DC_RES_DC3,
    DC_RES_DC4,
    DC_RES_DC5,
    DC_RES_DC1_DC2,
    DC_RES_DC3_DC4
};

enum {
    IF_RES_NC = -1,
    IF_RES_PARALLEL,
    IF_RES_MIPI_DSI1,
    IF_RES_MIPI_DSI2,
    IF_RES_LVDS1,
    IF_RES_LVDS2,
    IF_RES_LVDS3,
    IF_RES_LVDS4,
    IF_RES_LVDS1_LVDS2,
    IF_RES_LVDS3_LVDS4,
    IF_RES_DSI1_DSI2,
    IF_RES_LVDS1_4
};

enum {
    LVDS_CMB_NC = -1,
    LVDS_CMB_FALSE,
    LVDS_CMB_TRUE
};

enum {
    LVDS_MODE_NC = -1,
    LVDS_MODE_SEPARATE,
    LVDS_MODE_DUPLICATE,
    LVDS_MODE_DUAL
};

enum {
    PIXEL_BPP_NC = -1,
    PIXEL_BPP_16,
    PIXEL_BPP_18,
    PIXEL_BPP_24,
    PIXEL_BPP_30
};

/**
 *struct display_resource represent a display hardware resource infomation.
 */
struct display_resource {
    char name[128];
    int type;
    int dp_res;
    int dc_res;
    int if_res;
    int lvds_cmb;
    int lvds_mode;
    int pixel_bpp;
    int clk;
    int gui_enable;
};

#endif //__DISP_LINK_TYPES__
