/*
* lt9611_i2c.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 03/19/2021 BI create this file
*/
#include <string.h>
#include <i2c_hal.h>

#define lt9611_I2C_RW_LOG 1

static void *lt9611_i2c_handle = NULL;
static uint32_t lt9611_i2c_addr = 0;

struct i2c_client {
    uint16_t flags;
    uint16_t addr;
    uint16_t adapter;
};

int lt9611_i2c_init(uint32_t i2c_res_glb_idx, uint8_t i2c_addr)
{
    hal_i2c_creat_handle(&lt9611_i2c_handle, i2c_res_glb_idx);
    if (lt9611_i2c_handle == NULL){
        dprintf(0, "[err]%s(), lt9611_i2c_handle is NULL !\n", __func__);
        return -1;
    }
    dprintf(0, "%s(), lt9611_i2c_handle creat succesed !\n", __func__);
    lt9611_i2c_addr = i2c_addr;

    return 0;
}


static int lt9611_read_reg(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(i2c_handle, slave_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        dprintf(0, "%s: error: read reg=%x\n", __func__, reg);
        return ret;
    }

    *val = buf[0];
    dprintf(lt9611_I2C_RW_LOG, "%s(), read reg=0x%x,value= 0x%x\n", __func__, reg, buf[0]);
    return 0;
}

static int lt9611_write_reg(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(i2c_handle, slave_addr, buf, 1, buf + 1, 1);
    dprintf(lt9611_I2C_RW_LOG, "%s(), ret=%d, buf[0]=0x%x-%x\n", __func__, ret,
            buf[0], buf[1]);

    if (ret < 0)
        dprintf(0, "%s: error: reg=%x, val=%x\n", __func__, reg, val);

#if !(lt9611_I2C_RW_LOG)
    uint8_t value;
    lt9611_read_reg(i2c_handle, slave_addr, reg, &value);
#endif

    return ret;
}

static int lt9611_write_reg_witch_mask(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t val, uint8_t mask)
{
    void *handle = i2c_handle;
    uint8_t value = 0;
    lt9611_read_reg(handle, slave_addr, reg, &value);
    value &= ~mask;
    value |= val;
    lt9611_write_reg(handle, slave_addr, reg, value);
    return 0;
}


uint8_t HDMI_ReadI2C_Byte(uint8_t RegAddr)
{

    uint8_t val = 0;
    int ret = 0;

    ret = lt9611_read_reg(lt9611_i2c_handle, lt9611_i2c_addr, RegAddr, &val);
    if (ret < 0)
        return 0;

    return val;
}

bool HDMI_WriteI2C_Byte(uint8_t RegAddr, uint8_t d)
{
    int ret = 0;

    ret = lt9611_write_reg(lt9611_i2c_handle, lt9611_i2c_addr, RegAddr, d);
    if (ret < 0)
        return false;

    return true;
}


