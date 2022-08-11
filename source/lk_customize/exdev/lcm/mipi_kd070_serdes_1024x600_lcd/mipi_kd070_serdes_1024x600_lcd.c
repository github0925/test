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

#if 0
extern const domain_res_t g_iomuxc_res;
extern const domain_res_t g_gpio_res;

static const Port_PinModeType MODE_GPIO_G10_DSI_PWDN_N = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};


static void dsi_pwn_n_port_config(void)
{
    static void *port_handle;
    static void *dio_handle;
    bool ioret;
    // Port setup
    ioret = hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);

    if (!ioret) {
        return;
    }

    ioret = hal_port_set_pin_mode(port_handle, PortConf_PIN_I2S_SC6_WS,
                                  MODE_GPIO_G10_DSI_PWDN_N);
    hal_port_release_handle(&port_handle);

    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    hal_dio_write_channel(dio_handle, PortConf_PIN_I2S_SC6_WS, 1);
    hal_dio_release_handle(&dio_handle);
}
#endif

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

static void *i2c_handle;
static const uint32_t ub941_i2c_addr = 0x0C;
static const uint32_t ub948_i2c_addr = 0x2C;

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1024,
    .hfront_porch = 120,
    .hback_porch = 120,
    .hsync_len = 80,
    .vactive = 720,
    .vfront_porch = 10,
    .vback_porch = 10,
    .vsync_len = 15,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 400 * 1000,
};

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    LOGD("%s enter\n", __func__);

    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C12);

    ds90ub941as_init(i2c_handle, ub941_i2c_addr);
    mdelay(10);
    ds90ub948_init(i2c_handle, ub948_i2c_addr);

    LOGD("%s down\n", __func__);

    return 0;
}


struct sdm_panel mipi_kd070_serdes_1024x600_lcd = {
    .panel_name = "mipi_kd070_serdes_1024x600_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE, // or IF_TYPE_I2C
    .width_mm = 240,
    .height_mm = 160,
    .fps = 60,
    .reset_timing = reset_timing,
    .display_timing = &timing,
    .mipi = &mipi_data,
    .sis_num = 0,
    .sis = NULL,
    .rtos_screen = NULL,
    .panel_post_begin = panel_post_begin,
    .panel_post_end = panel_post_end,
};

