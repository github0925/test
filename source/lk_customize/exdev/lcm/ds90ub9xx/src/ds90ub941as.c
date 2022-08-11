/*
* ds90ub941as.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 05/21/2020 BI create this file
*/
#include <string.h>
#include <i2c_hal.h>
#include "ds90ub9xx.h"


#define DS90UB941AS_LOG     1

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

static int ds90ub941as_bl_init(void* i2c_handle)
{
    void *handle = i2c_handle;
    uint8_t value = 0;

    LOGD("%s enter\n", __func__);

    //prot0 gpio0 config
    ds90ub9xx_write_reg(handle, 0x0c, 0x1E, 0x04);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(handle, 0x0c, 0x17, value);
    mdelay(10);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x0c, 0x0D, &value);
    value &= ~0x0f;
    value |= 0x03;
    ds90ub9xx_write_reg(handle, 0x0c, 0x0D, value);

    value = 0;
    ds90ub9xx_read_reg(handle, 0x2c, 0x1D, &value);
    value &= ~0x0f;
    value |= 0x05;
    ds90ub9xx_write_reg(handle, 0x2c, 0x1D, value);
    //mdelay(20);

    //port1 D_gpio0 config
    ds90ub9xx_write_reg(handle, 0x0d, 0x1E, 0x04);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(handle, 0x0d, 0x17, value);
    mdelay(10);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x0d, 0x0D, &value);
    value &= ~0x0f;
    value |= 0x03;
    ds90ub9xx_write_reg(handle, 0x0d, 0x0D, value);

    value = 0;
    ds90ub9xx_read_reg(handle, 0x3c, 0x1D, &value);
    value &= ~0x0f;
    value |= 0x05;
    ds90ub9xx_write_reg(handle, 0x3c, 0x1D, value);

    LOGD("%s end\n", __func__);

    return 0;

}

void ds90ub941as_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    int ret = 0;
    uint8_t value = 0;

    LOGD("%s enter\n", __func__);
    ret = ds90ub9xx_read_reg(handle, addr, 0x1E, &value);
    if (ret < 0) {
        LOGE("[err] : screen offline !\n");
        return;
    }

    //ds90ub9xx_write_reg(handle, addr, 0x01,0x08); //Disable DSI
#if defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x12, 0x04); //set 18-bit per pixel
#endif
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x40, 0x05);//Select DSI Port 0 digital registers
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x21);//Select DSI_CONFIG_1 register
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x60);//Set DSI_VS_POLARITY=DSI_HS_POLARITY=1(active low)

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(handle, addr, 0x40, 0x09);//Select DSI Port 1 digital registers
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x21);//Select DSI_CONFIG_1 register
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x60);//Set DSI_VS_POLARITY=DSI_HS_POLARITY=1(active low)

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x5B, 0x05);//Force Independent 2:2 mode
    //ds90ub9xx_write_reg(handle, addr, 0x5B, 0x01);//Forced Single FPD-Link III Transmitter mode (Port 1 disabled)
    ds90ub9xx_write_reg(handle, addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 0

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x40, 0x04);//Select DSI Port 0 digital registers
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x05);//Select DPHY_SKIP_TIMING register
#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x10);//Write TSKIP_CNT value for 200 MHz DSI clock frequency (1024x600, Round(65x0.2) -5)
#elif defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x16);//Write TSKIP_CNT value for 250 MHz DSI clock frequency (1280x800, Round(65x0.25) -5)
#else
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x15);//Write TSKIP_CNT value for 300 MHz DSI clock frequency (1920x720, Round(65x0.3) -5)
#endif

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(handle, addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 1

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x40, 0x08);//Select DSI Port 1 digital registers
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x05);//Select DPHY_SKIP_TIMING register
#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x10);//Write TSKIP_CNT value for 200 MHz DSI clock frequency (1920x720, Round(65x0.2) -5)
#elif defined(MIPI_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x16);//Write TSKIP_CNT value for 250 MHz DSI clock frequency (1280x800, Round(65x0.25) -5)
#else
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x15);//Write TSKIP_CNT value for 300 MHz DSI clock frequency (1920x720, Round(65x0.3) -5)
#endif
    ds90ub9xx_write_reg(handle, addr, 0x01, 0x00);//Enable DSI

    LOGD("%s done\n", __func__);
}

void ds90ub948_init(void* i2c_handle, uint32_t slave_addr)
{
    LOGD("%s enter\n", __func__);

#if defined(MIPI_HSD123_SERDES_1920X720_LCD)
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val;

    ds90ub9xx_write_reg(handle, addr, 0x49, 0x60);//set MAPSEL high
    ds90ub9xx_read_reg(handle, addr, 0x49, &val);
    LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);
#endif

#if defined(MIPI_KD070_SERDES_1024X600_LCD)
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val;

    if (0) {
        ds90ub9xx_write_reg(handle, addr, 0x1E, 0x9);
        ds90ub9xx_read_reg(handle, addr, 0x1E, &val);
        LOGD("%s: read: reg=0x1D, val=0x%x\n", __func__, val);

        ds90ub9xx_write_reg(handle, addr, 0x20, 0x9);
        ds90ub9xx_read_reg(handle, addr, 0x20, &val);
        LOGD("%s: read: reg=0x20, val=0x%x\n", __func__, val);
    }
#endif

#if defined(MIPI_HSD101_SERDES_1280X800_LCD)
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val;

    if (0) {
        ds90ub9xx_write_reg(handle, addr, 0x49, 0x42);//set MAPSEL low
        ds90ub9xx_read_reg(handle, addr, 0x49, &val);

        LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);
    }
#endif

    LOGD("%s done\n", __func__);
}

void ds90ub941as_Enable_Dualdsi_DualPort(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    LOGD("%s enter\n", __func__);

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0

    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0

    ds90ub9xx_write_reg(handle, addr, 0x4F, 0x8C);//Set DSI_CONTINUOUS_CLOCK, 4 lanes, DSI Port 0

    ds90ub9xx_write_reg(handle, addr, 0x5B, 0x00);//auto detect FPD-Link

    ds90ub9xx_write_reg(handle, addr, 0x01, 0x00);//Enable DSI

    LOGD("%s done\n", __func__);
}

bool ub941_get_serdes_link(void *i2c_handle, uint32_t ub941_addr)
{
    int ret = 0;
    uint8_t val = 0;

    ret = ds90ub9xx_read_reg(i2c_handle, ub941_addr, 0x5A, &val);
    if (ret < 0)
        return false;

    LOGD("read 0x5A -> 0x%x\n", val);

    if (val & UB941_TX_STS_MASK)
        if (((val & UB941_FPD3_PORT_MASK) >> UB941_FPD3_PORT_SHIFT) == DUAL_LINK)
            return true;

    return false;
}

