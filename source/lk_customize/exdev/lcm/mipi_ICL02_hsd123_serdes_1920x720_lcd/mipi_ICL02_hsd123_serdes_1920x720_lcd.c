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
#include <app.h>
#include <lib/console.h>


#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

#define TEST_941_948_COLORBAR false
#define DUMP_941_948_REGISTER false

static uint8_t reset_timing[] = {
    5, 20, 5,
};

static struct display_timing timing = {
    .hactive = 1920,
    .hfront_porch = 52,
    .hback_porch = 32,
    .hsync_len = 24,
    .vactive = 720,
    .vfront_porch = 8,
    .vback_porch = 21,
    .vsync_len = 3,
};

static struct info_mipi mipi_data = {
    .burst_mode = 2,
    .video_bus_width = 24,
    .lane_number = 4,
    .phy_freq = 660000,//700000,//711144,//2028 * 752 * 60 * 7 /1000,
};

static uint8_t CheckSum(uint8_t *addr, int count)
{
    uint32_t sum = 0;
    while(count > 0)
    {
        sum = sum + *addr;
        addr += 1;
        count -= 1;
    }
    while(sum >> 8)
        sum = (sum & 0xff) + (sum >> 8);

    return (uint8_t)(~sum);
}

static int dump_948_all_rgeister(void *handle, int8_t addr_948)
{
    uint8_t value;
    for(int i = 0; i < 0xff; i++)
    {
        ds90ub9xx_read_reg(handle, addr_948, i, &value);
    }
    return 0;
}

static int dump_941_all_rgeister(void *handle, int8_t addr_941)
{
    uint8_t value;
    for(int i = 0; i < 0xff; i++)
    {
        ds90ub9xx_read_reg(handle, addr_941, i, &value);
    }
    return 0;
}

static int dump_948_rgeister(void *handle, int8_t addr_948)
{
    uint8_t value = 0;
    LOGD("%s start!!!\n",__func__);
    ds90ub9xx_read_reg(handle, addr_948, 0x34, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x49, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x1f, &value);
    //ds90ub9xx_read_reg(handle, addr_948, 0x06, &value);
    ds90ub9xx_read_reg(handle, addr_948, 0x1C, &value);
    LOGD("%s end!!!\n",__func__);
    return 0;
}

static int dump_941_rgeister(void *handle, int8_t addr_941)
{
    LOGD("%s !!!start\n",__func__);

    uint8_t value = 0;
    ds90ub9xx_read_reg(handle, addr_941, 0x06, &value);
    ds90ub9xx_read_reg(handle, addr_941, 0x4F, &value);
    ds90ub9xx_read_reg(handle, addr_941, 0x5B, &value);
    //ds90ub9xx_read_reg(handle, addr_941, 0x06, &value);
    //ds90ub9xx_read_reg(handle, addr_941, 0x06, &value);
    LOGD("%s !!!end\n",__func__);
    return 0;
}

static int panel_post_begin(void)
{
    return 0;
}

static int panel_post_end(void)
{
    static void *i2c_handle;
    const uint32_t ub941_i2c_addr = 0x0C;
    const uint32_t ub948_i2c_addr = 0x2C;

    LOGD("%s enter\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);
    i2c_app_config_t i2c_conf = hal_i2c_get_busconfig(i2c_handle);
    LOGD("lcm i2c conf poll is %d\n", i2c_conf.poll);
    if (!i2c_conf.poll) {
        i2c_conf.poll = 1;
        hal_i2c_set_busconfig(i2c_handle, &i2c_conf);
    }
    LOGD("%s config 941 \n", __func__);
    //config vido
    //941 dsi input config(Single-DSI mode,Select DSI Input port 0,4 Lanes)
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub941_i2c_addr, 0x4F, 0x0C, 0x6C);
    //941 Dual FPD-Link III mode (Single, Dual, or Replicate)
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub941_i2c_addr, 0x5B, 0x3, 0x07);
    uint8_t value = 0;
    ds90ub9xx_read_reg(i2c_handle, ub941_i2c_addr, 0x06, &value);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x17, 0x9e);

    LOGD("%s config 948 \n", __func__);
    //config video
    //948 Dual link based on received data
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x34, 0x8, 0x18);
    //948 Dual FPD/OLDI output
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub948_i2c_addr, 0x49, 0, 0x03);

    //config backlight
    //enable 948 bl pin
    ds90ub9xx_write_reg(i2c_handle, ub948_i2c_addr, 0x1f, 0x09);

#if TEST_941_948_COLORBAR
    //color bar test !
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x64, 0x07);

    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x03);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x02);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x04);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0xf8);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x05);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0xe7);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x06);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x33);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x07);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x80);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x08);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x07);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x09);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x2d);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x0A);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x28);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x0B);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x0a);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x0C);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x28);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x66, 0x0D);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x67, 0x32);

    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x65, 0x04);
    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x64, 0x35);

    ds90ub9xx_write_reg(i2c_handle, ub941_i2c_addr, 0x17, 0x9e);
    ds90ub9xx_write_reg(i2c_handle, ub948_i2c_addr, 0x1f, 0x09);
#endif

#if DUMP_941_948_REGISTER
    dump_941_rgeister(i2c_handle, ub941_i2c_addr);
    dump_948_rgeister(i2c_handle, ub948_i2c_addr);
    //dump_941_all_rgeister(i2c_handle, ub941_i2c_addr);
    //dump_948_all_rgeister(i2c_handle, ub948_i2c_addr);

#endif
    //config backlight
    //config serdes i2c pass through
    ds90ub9xx_write_reg_witch_mask(i2c_handle, ub941_i2c_addr, 0x03, 1<<3, 0x08);
    const uint32_t slave_i2c_addr = 0x18;
    int ret = 0;
    uint8_t bl_buf[7] = {0x01, 0x00, 15, 0x00, 0x00, 0x00, 0x00};
    //read rtc time select day or night
    //config default bl level
    bl_buf[6] = CheckSum(bl_buf, 7);
    ret = hal_i2c_write_reg_data(i2c_handle, slave_i2c_addr, bl_buf, 0, bl_buf, 7);
    if (ret < 0)
        LOGD("[err]:%s bl level set failed !\n", __func__);

    LOGD("%s down\n", __func__);
    return 0;
}


struct sdm_panel mipi_ICL02_hsd123_serdes_1920x720_lcd = {
    .panel_name = "mipi_ICL02_hsd123_serdes_1920x720_lcd",
    .if_type = IF_TYPE_DSI,
    .cmd_intf = IF_TYPE_NONE, // or IF_TYPE_I2C
    .width_mm = 292,
    .height_mm = 109,
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

#if DUMP_941_948_REGISTER
static int dump_serdes_register(int argc, const cmd_args *argv)
{
    static void *i2c_handle;
    const uint32_t ub941_i2c_addr = 0x0C;
    const uint32_t ub948_i2c_addr = 0x2C;
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);
    LOGD("request i2c11 \n");
    dump_941_all_rgeister(i2c_handle, ub941_i2c_addr);
    dump_948_all_rgeister(i2c_handle, ub948_i2c_addr);
    return 0;
}


#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("dump_123serdes_register", "dump 123 serdes register",  (console_cmd)&dump_serdes_register)
STATIC_COMMAND_END(dump123_serdes);
#endif

#endif

