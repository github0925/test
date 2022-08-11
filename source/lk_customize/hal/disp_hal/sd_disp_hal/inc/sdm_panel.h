//*****************************************************************************
//
// disp_hal.h - Prototypes for the disp hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
// SDM  (semidrive display module)
//
//
//*****************************************************************************

#ifndef __SDM_PANEL_H__
#define __SDM_PANEL_H__

#include <stdint.h>
#include <disp_common.h>
#include <disp_link_types.h>

#define MAX_DATA 100

enum {
    IF_TYPE_INVALID = -1,
    IF_TYPE_NONE,
    IF_TYPE_DSI,
    IF_TYPE_LVDS,
    IF_TYPE_INTERNAL = IF_TYPE_LVDS,
    IF_TYPE_I2C,
    IF_TYPE_SPI,
    IF_TYPE_HDMI,
    IF_TYPE_PARALLEL
};

enum {
    LVDS_MAP_FORMAT_NC = -1,
    LVDS_MAP_FORMAT_JEIDA,
    LVDS_MAP_FORMAT_SWPG
};

typedef enum
{
    LCM_POLARITY_RISING  = 0,
    LCM_POLARITY_FALLING = 1
} LCM_POLARITY;

struct display_timing {
    uint32_t tcon_clk;
    uint32_t hactive;
    uint32_t hfront_porch;
    uint32_t hback_porch;
    uint32_t hsync_len;
    uint32_t vactive;
    uint32_t vfront_porch;
    uint32_t vback_porch;
    uint32_t vsync_len;

    /* polarity parameters */
    LCM_POLARITY dsp_clk_pol;
    LCM_POLARITY de_pol;
    LCM_POLARITY vsync_pol;
    LCM_POLARITY hsync_pol;

    uint32_t map_format;
};

struct sis_info {
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
};

struct dsi_cmd_desc {
    uint8_t data_type;
    uint8_t wait;
    uint8_t wc_h;
    uint8_t wc_l;
    uint8_t playload[MAX_DATA];
};

struct info_mipi {
    uint16_t burst_mode;
    uint16_t video_bus_width;
    uint32_t lane_number;
    uint32_t phy_freq;
    bool use_dcs;
    struct dsi_cmd_desc *init_data;
    struct dsi_cmd_desc *sleep_data;
};

struct sdm_panel {
    const char *panel_name;
    int if_type;
    int cmd_intf;
    uint32_t width_mm;
    uint32_t height_mm;
    int fps;
    // for panel voltage tuning
    uint8_t *cmd_data;
    uint8_t *reset_timing;
    struct display_timing *display_timing;
    struct info_mipi *mipi;
    int sis_num;
    struct sis_info *sis;
    struct sis_info *rtos_screen;
    int (*panel_post_begin)(void);
    int (*panel_post_end)(void);
};

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

struct sdm_buffer {
    int layer;
    int layer_dirty;
    int layer_en;
    int fmt;
    rect_t src;
    unsigned long addr[4];//YUVA
    int src_stride[4];
    rect_t start;
    rect_t dst;
    int ckey_en;
    int ckey;
    int alpha_en;
    char alpha;
    int z_order;
    int security;
};

struct sdm_post_config {
    size_t n_bufs;
    struct sdm_buffer *bufs;
    size_t custom_data_size;
    void *custom_data;
};

struct sdm_pq_params {
    float contrast; //k
    float luminance;//l
    float saturation;//s
    float chroma;//h
};

#endif //__SDM_PANEL_H__
