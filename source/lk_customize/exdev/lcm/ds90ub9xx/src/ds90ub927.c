/*
* ds90ub927.c
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


#define DS90UB927_LOG     0

#define udelay(x) spin(x)
#define mdelay(x) spin(x * 1000)

void ds90ub927_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val;

    LOGD("%s enter\n", __func__);

    ds90ub9xx_write_reg(handle, addr, 0x03, 0x9A);//Enable I2C_PASSTHROUGH

    //ds90ub9xx_write_reg(handle, addr, 0x01, 0x01);
    ds90ub9xx_write_reg(handle, addr, 0x0D, 0x03);//Enable I2C_PASSTHROUGH

    ds90ub9xx_read_reg(handle, addr, 0x0D, &val);
    LOGD("%s: read: reg=0x1D, val=0x%x\n", __func__, val);

    LOGD("%s done\n", __func__);
}

void ds90ub928_match_927_init(void* i2c_handle, uint32_t slave_addr)
{
    void *handle = i2c_handle;
    uint32_t addr = slave_addr;
    uint8_t val;

    LOGD("%s enter\n", __func__);

    ds90ub9xx_write_reg(handle, addr, 0x1D, 0x05);

    ds90ub9xx_read_reg(handle, addr, 0x1D, &val);
    LOGD("%s: read: reg=0x1D, val=0x%x\n", __func__, val);

    LOGD("%s done\n", __func__);
}
