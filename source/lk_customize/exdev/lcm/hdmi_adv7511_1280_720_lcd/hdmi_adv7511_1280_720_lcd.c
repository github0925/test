#include "sdm_panel.h"
#include "disp_panels.h"

static struct display_timing timing = {
    .hactive = 1280,
    .hfront_porch = 110,
    .hback_porch = 220,
    .hsync_len = 40,

    .vactive = 720,
    .vfront_porch = 5,
    .vback_porch = 20,
    .vsync_len = 5,

    .dsp_clk_pol = LCM_POLARITY_RISING,
    .de_pol      = LCM_POLARITY_RISING,
    .vsync_pol   = LCM_POLARITY_RISING,
    .hsync_pol   = LCM_POLARITY_RISING,
};

struct sdm_panel hdmi_adv7511_1280_720_lcd = {
    .panel_name = "hdmi_adv7511_1280_720_lcd",
    .if_type = IF_TYPE_HDMI,
    .cmd_intf = IF_TYPE_NONE, // or CMD_INTF_NONE
    .cmd_data = NULL,
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .display_timing = &timing,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
};

