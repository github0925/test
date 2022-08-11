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
#include "tca9539.h"
#include <dsi85.h>
#include <i2c_hal.h>

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

void *dsi85_i2c_handle;
const uint32_t dsi85_i2c_slave_address = 0x2C;

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
    .phy_freq = 700 * 1000,
};

static int panel_post_begin(void)
{
#if 0
    struct tca9539_device *pd;
#endif

    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&dsi85_i2c_handle, RES_I2C_I2C11);
#if 0
    pd = tca9539_init(12);

    /*init sequence*/
    /*1.Power on*/
    /*2.After power is applied and stable, the DSI CLK lanes MUST
      be in HS state and the DSI data lanes MUST be driven
      to LP11 state */
    /*3.Set EN pin to Low*/
    pd->ops.output_enable(2);
    pd->ops.output_val(2, 0);
    mdelay(10);

    /*Wait 10ms*/
    /*4.Tie EN pin to high*/
    /*Wait 10ms*/
    pd->ops.output_val(2, 1);
    mdelay(10);
#endif
    /*5.Initialize all CSR registers to their appropriate values based
      on the implementation (The SN65DSI8x is not
      functional until the CSR registers are initialized)*/
    sn65dsi85_regs_init(dsi85_i2c_handle, dsi85_i2c_slave_address);

    /*6.Set the PLL_EN bit(CSR 0x0D.0)*/
    /*Wait 10ms*/
    /*7.Set the SOFT_RESET bit(CSR 0x09.0)*/
    /*Wait 10ms*/
    sn65dsi85_enable(dsi85_i2c_handle, dsi85_i2c_slave_address);

    LOGD("%s down\n", __func__);

    return 0;
}

static int panel_post_end(void)
{
    static uint8_t inited = 0;
    void *handle = dsi85_i2c_handle;
    uint32_t addr = dsi85_i2c_slave_address;

    if (!inited) {
        LOGD("%s enter\n", __func__);
        /*8.Change DSI data lanes to HS state and start DSI video stream*/
        /*Wait 5ms*/
        mdelay(5);
        /*9.Read back all resisters and confirm they were correctly written*/
        sn65dsi85_dump(handle, addr);

        /*10.Write 0xFF to CSR 0xE5 to clear the error registers*/
        /*Wait 1ms*/
        /*Read CSR 0xE5. If CSR 0xE5!= 0x00, then go back to step #2 and re-initialize*/
        sn65dsi85_lock_check(handle, addr);

        LOGD("%s down\n", __func__);
    }

    return 0;
}


struct sdm_panel mipi_hsd123_1920x720_2_lcd = {
    .panel_name = "mipi_hsd123_1920x720_2_lcd",
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

