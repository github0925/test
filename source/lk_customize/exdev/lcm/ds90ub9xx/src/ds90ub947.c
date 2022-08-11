/*
* ds90ub947.c
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


#define DS90UB947_LOG     0

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)


void ds90ub947_bl_init(void)
{
    static void *i2c_handle;
    uint8_t value = 0;
    LOGD("%s enter\n", __func__);

    //cluster gpio 0
    LOGD("%s config lvds12 gpio 0\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);
    if (i2c_handle == NULL)
        goto CONFIG_LVDS34_GPIO0;
    void *handle = i2c_handle;
    ds90ub9xx_write_reg(handle, 0x1A, 0x1E, 0x01);
    ds90ub9xx_write_reg(handle, 0x1A, 0x03, 0x08);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(handle, 0x1A, 0x17, value);
    mdelay(10);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x1A, 0x0D, &value);
    value &= ~0x0f;
    value |= 0x03;
    ds90ub9xx_write_reg(handle, 0x1A, 0x0D, value);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x2c, 0x1D, &value);
    value &= ~0x0f;
    value |= 0x05;
    ds90ub9xx_write_reg(handle, 0x2c, 0x1D, value);

CONFIG_LVDS34_GPIO0:
    //control panel gpio 0
    LOGD("%s config lvds34 gpio 0\n", __func__);
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);
    if (i2c_handle == NULL)
        goto DS90UB947_BL_CONFIG_OUT;
    handle = i2c_handle;
    ds90ub9xx_write_reg(handle, 0x1A, 0x1E, 0x01);
    ds90ub9xx_write_reg(handle, 0x1A, 0x03, 0x08);
    value = 0x1e | 0x80;
    ds90ub9xx_write_reg(handle, 0x1A, 0x17, value);
    mdelay(10);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x1A, 0x0D, &value);
    value &= ~0x0f;
    value |= 0x03;
    ds90ub9xx_write_reg(handle, 0x1A, 0x0D, value);
    value = 0;
    ds90ub9xx_read_reg(handle, 0x2c, 0x1D, &value);
    value &= ~0x0f;
    value |= 0x05;
    ds90ub9xx_write_reg(handle, 0x2c, 0x1D, value);
DS90UB947_BL_CONFIG_OUT:

    LOGD("%s end\n", __func__);

}

void ds90ub947_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;

    LOGD("%s enter\n", __func__);

#if defined(LVDS_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x12, 0x04);


    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    //OLDI_DUAL:Single-pixel mode,MAPSEL: OpenLDI Bit Mapping
    ds90ub9xx_write_reg(handle, addr, 0x4f, 0x40);

    //REPEATER:Disable repeater mode
    ds90ub9xx_write_reg(handle, addr, 0xc2, 0x98);

    //COAX:Enable FPD-Link III for coaxial cabling
    ds90ub9xx_write_reg(handle, addr, 0x5b, 0xa0);


#else
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x04);//Use I2D ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x02);//Select FPD-Link III Port 1
    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH, FPD-Link III Port 1

    ds90ub9xx_write_reg(handle, addr, 0x1E, 0x01);//Select FPD-Link III Port 0
    ds90ub9xx_write_reg(handle, addr, 0x5B, 0x02);//Force dual mode

    ds90ub9xx_write_reg(handle, addr, 0x40, 0x10);// select OLDI register
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x49);// force PLL controller in PPM reset state
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x16);
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x47);// force PLL LOCK Low
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x20);
    ds90ub9xx_write_reg(handle, addr, 0x42, 0xA0);// reset PLL divider
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x20);
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x00);// release PLL LOCK control
    ds90ub9xx_write_reg(handle, addr, 0x41, 0x49);// release PLL state control
    ds90ub9xx_write_reg(handle, addr, 0x42, 0x00);
#endif

    LOGD("%s done\n", __func__);
}

void ds90ub948_match_947_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val = 0;

    LOGD("%s enter\n", __func__);

#if defined(LVDS_HSD101_SERDES_1280X800_LCD)
    ds90ub9xx_write_reg(handle, addr, 0x49, 0x42);//set MAPSEL low
#else
    ds90ub9xx_write_reg(handle, addr, 0x49, 0x60);//set MAPSEL high
#endif
    ds90ub9xx_read_reg(handle, addr, 0x49, &val);
    LOGD("%s: read: reg=0x49, val=0x%x\n", __func__, val);

    LOGD("%s done\n", __func__);
}
