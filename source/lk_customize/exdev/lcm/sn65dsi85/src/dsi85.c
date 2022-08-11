/*
* dsi85.c
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

#include <string.h>
#include <i2c_hal.h>
#include <dsi85_registers.h>
#include <disp_common.h>


#define DSI85_LOG     1
#define TEST_PATTERN  0

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

struct i2c_client {
    uint16_t flags;
    uint16_t addr;
    uint16_t adapter;
};


static int sn65dsi85_read_reg(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(i2c_handle, slave_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        LOGE("%s: error: read reg=%x\n", __func__, reg);
        return ret;
    }

    *val = buf[0];

    return 0;
}

static int sn65dsi85_write_reg(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(i2c_handle, slave_addr, buf, 1, buf + 1, 1);

    LOGD("%s(), ret=%d, buf[0]=0x%x-%x\n", __func__, ret,
            buf[0], buf[1]);

    if (ret < 0)
        LOGE("%s: error: reg=%x, val=%x\n", __func__, reg, val);

#if 0 //debug
    sn65dsi85_read_reg(reg, &value);
    LOGD("%s: read: reg=0x%x, value=0x%x\n", __func__, reg, value);
#endif

    mdelay(10);

    return ret;
}

void sn65dsi85_regs_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    LOGD("%s enter\n", __func__);

    /* Soft reset and disable PLL */
    sn65dsi85_write_reg(handle, addr, DSI85_SOFT_RESET, 1);
    sn65dsi85_write_reg(handle, addr, DSI85_PLL_EN, 0);

    /* ------- CORE_PLL register ------- */
    sn65dsi85_write_reg(handle, addr, DSI85_CORE_PLL, 0x3);

    /* ------- PLL_DIV register ------- */
    sn65dsi85_write_reg(handle, addr, DSI85_PLL_DIV, 0x30);
    //sn65dsi85_write_reg(DSI85_PLL_DIV, 0x68);

    /* ------- DSI ------- */
    sn65dsi85_write_reg(handle, addr, DSI85_DSI_CFG, 0x26);
    sn65dsi85_write_reg(handle, addr, DSI85_DSI_EQ, 0xCC);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_DSI_CLK_RNG, 0x46); //400M
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_DSI_CLK_RNG, 0x00);

    /* ------- LVDS ------- */
    sn65dsi85_write_reg(handle, addr, DSI85_LVDS_MODE, 0x6C);
    sn65dsi85_write_reg(handle, addr, DSI85_LVDS_SIGN, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_LVDS_TERM, 0x03);
    sn65dsi85_write_reg(handle, addr, DSI85_LVDS_ADJUST, 0x00);

    /* ------- Dimensions ------- */
#if TEST_PATTERN
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_LINE_LEN_LO, 0xC0);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_LINE_LEN_HI, 0x03);
#else
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_LINE_LEN_LO, 0x80);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_LINE_LEN_HI, 0x07);
#endif
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_LINE_LEN_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_LINE_LEN_HI, 0x00);

#if TEST_PATTERN
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_LINES_LO, 0xD0);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_LINES_HI, 0x02);
#else
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_LINES_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_LINES_HI, 0x00);
#endif
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_LINES_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_LINES_HI, 0x00);

#if TEST_PATTERN
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_SYNC_DELAY_LO, 0x3e);
#else
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_SYNC_DELAY_LO, 0xf0);
#endif
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_SYNC_DELAY_HI, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_SYNC_DELAY_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_SYNC_DELAY_HI, 0x00);

    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HSYNC_WIDTH_LO, 0x14);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HSYNC_WIDTH_HI, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_HSYNC_WIDTH_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_HSYNC_WIDTH_HI, 0x00);

    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VSYNC_WIDTH_LO, 0x16);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VSYNC_WIDTH_HI, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VSYNC_WIDTH_LO, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VSYNC_WIDTH_HI, 0x00);

    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HORZ_BACKPORCH, 0x14);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_HORZ_BACKPORCH, 0x00);

#if TEST_PATTERN
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_BACKPORCH, 0x19);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_BACKPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, 0x0a);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_HORZ_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, 0x19);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_TEST_PATTERN, 0x10);
#else
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_VERT_BACKPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_BACKPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_HORZ_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_CHB_VERT_FRONTPORCH, 0x00);
    sn65dsi85_write_reg(handle, addr, DSI85_TEST_PATTERN, 0x00);
#endif
    LOGD("%s done\n", __func__);
}

void sn65dsi85_dump(void* i2c_handle, uint32_t slave_addr)
{
    uint8_t val = 0;
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    /* ------- CORE_PLL register ------- */
    sn65dsi85_read_reg(handle, addr, DSI85_CORE_PLL, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CORE_PLL, val);


    /* ------- PLL_DIV register ------- */
    sn65dsi85_read_reg(handle, addr, DSI85_PLL_DIV, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_PLL_DIV, val);

    /* ------- DSI ------- */
    sn65dsi85_read_reg(handle, addr, DSI85_DSI_CFG, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_DSI_CFG, val);
    sn65dsi85_read_reg(handle, addr, DSI85_DSI_EQ, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_DSI_EQ, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_DSI_CLK_RNG, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_DSI_CLK_RNG, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_DSI_CLK_RNG, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_DSI_CLK_RNG, val);

    /* ------- LVDS ------- */
    sn65dsi85_read_reg(handle, addr, DSI85_LVDS_MODE, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_LVDS_MODE, val);
    sn65dsi85_read_reg(handle, addr, DSI85_LVDS_SIGN, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_LVDS_SIGN, val);
    sn65dsi85_read_reg(handle, addr, DSI85_LVDS_TERM, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_LVDS_TERM, val);
    sn65dsi85_read_reg(handle, addr, DSI85_LVDS_ADJUST, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_LVDS_ADJUST, val);

    /* ------- Dimensions ------- */
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_LINE_LEN_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_LINE_LEN_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_LINE_LEN_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_LINE_LEN_HI, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_LINE_LEN_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_LINE_LEN_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_LINE_LEN_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_LINE_LEN_HI, val);

    sn65dsi85_read_reg(handle, addr, DSI85_CHA_VERT_LINES_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_VERT_LINES_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_VERT_LINES_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_VERT_LINES_HI, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VERT_LINES_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VERT_LINES_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VERT_LINES_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VERT_LINES_HI, val);

    sn65dsi85_read_reg(handle, addr, DSI85_CHA_SYNC_DELAY_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_SYNC_DELAY_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_SYNC_DELAY_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_SYNC_DELAY_HI, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_SYNC_DELAY_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_SYNC_DELAY_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_SYNC_DELAY_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_SYNC_DELAY_HI, val);

    sn65dsi85_read_reg(handle, addr, DSI85_CHA_HSYNC_WIDTH_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_HSYNC_WIDTH_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_HSYNC_WIDTH_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_HSYNC_WIDTH_HI, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_HSYNC_WIDTH_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_HSYNC_WIDTH_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_HSYNC_WIDTH_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_HSYNC_WIDTH_HI, val);

    sn65dsi85_read_reg(handle, addr, DSI85_CHA_VSYNC_WIDTH_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_VSYNC_WIDTH_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_VSYNC_WIDTH_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_VSYNC_WIDTH_HI, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VSYNC_WIDTH_LO, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VSYNC_WIDTH_LO, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VSYNC_WIDTH_HI, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VSYNC_WIDTH_HI, val);

    sn65dsi85_read_reg(handle, addr, DSI85_CHA_HORZ_BACKPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_HORZ_BACKPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_HORZ_BACKPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_HORZ_BACKPORCH, val);



    sn65dsi85_read_reg(handle, addr, DSI85_CHA_VERT_BACKPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_VERT_BACKPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VERT_BACKPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VERT_BACKPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_HORZ_FRONTPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_HORZ_FRONTPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_HORZ_FRONTPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHA_HORZ_FRONTPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHA_HORZ_FRONTPORCH, val);
    sn65dsi85_read_reg(handle, addr, DSI85_CHB_VERT_FRONTPORCH, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_CHB_VERT_FRONTPORCH, val);
}

void sn65dsi85_enable(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    /*Set the PLL_EN bit(CSR 0x0D.0)*/
    sn65dsi85_write_reg(handle, addr, DSI85_PLL_EN, 0x01);
    mdelay(50);

    /*Set the SOFT_RESET bit(CSR 0x09.0)*/
    sn65dsi85_write_reg(handle, addr, DSI85_SOFT_RESET, 0x01);
    mdelay(10);
}

void sn65dsi85_lock_check(void* i2c_handle, uint32_t slave_addr)
{
    uint8_t val = 0;
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    sn65dsi85_write_reg(handle, addr, DSI85_ERR_STATUS, 0xFF);
    mdelay(50);
    sn65dsi85_read_reg(handle, addr, DSI85_ERR_STATUS, &val);
    LOGD("%s: read: reg=0x%x, val=0x%x\n", __func__, DSI85_ERR_STATUS, val);
}
