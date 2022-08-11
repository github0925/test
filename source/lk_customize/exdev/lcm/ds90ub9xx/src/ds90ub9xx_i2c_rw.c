/*
* ds90ub9xx_i2c_rw.c
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
#include <ds90ub9xx.h>
#include <kernel/thread.h>

#define DS90UB9XX_I2C_RW_LOG 1


struct i2c_client {
    uint16_t flags;
    uint16_t addr;
    uint16_t adapter;
};

int ds90ub9xx_read_reg(void* i2c_handle, uint32_t slave_addr,
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
    LOGD("%s(),addr[0x%x] read reg=0x%x,value= 0x%x\n",
        __func__, slave_addr, reg, buf[0]);
    return 0;
}

int ds90ub9xx_write_reg(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(i2c_handle, slave_addr, buf, 1, buf + 1, 1);
    LOGD("%s(), addr[0x%x] ret=%d, buf[0]=0x%x-%x\n", __func__, slave_addr, ret,
            buf[0], buf[1]);

    if (ret < 0) {
        LOGE("%s: error: reg=%x, val=%x\n", __func__, reg, val);
        return ret;
    }

#if !(DS90UB9XX_I2C_RW_LOG)
    uint8_t value;
    ds90ub9xx_read_reg(i2c_handle, slave_addr, reg, &value);
#endif

    return ret;
}

int ds90ub9xx_write_reg_witch_mask(void* i2c_handle, uint32_t slave_addr,
    uint8_t reg, uint8_t val, uint8_t mask)
{
    void *handle = i2c_handle;
    uint8_t value = 0;
    int ret = 0;
    ret = ds90ub9xx_read_reg(handle, slave_addr, reg, &value);
    if (ret < 0) {
        return ret;
    }
    value &= ~mask;
    value |= val;
    ret = ds90ub9xx_write_reg(handle, slave_addr, reg, value);
    return ret;
}

static struct ds_gpio_config gpc[] = {
    {GPIO0, 0x0D, 0x1D, 0x0F, 0, 0},
    {GPIO1, 0x0E, 0x1E, 0x0F, 0, 0},
    {GPIO2, 0x0E, 0x1E, 0xF0, 0, 0},
    {GPIO3, 0x0F, 0x1F, 0x0F, 0, 0},
};

int ds90ub948_gpio_output(void* i2c_handle, uint32_t des_addr, enum ds90ub9xx_gpio gpio,
    enum ds_gpio_level level)
{
    void *handle=  i2c_handle;
    int i = 0;

    if (handle == NULL) {
        LOGE("[err] %s : i2c handle is NULL ! \n", __func__);
        return -1;
    }

    if (gpio < D_GPIO0)
        ds90ub9xx_write_reg_witch_mask(handle, des_addr, 0x34, 0x01, 0x03); //select des port 0
        //LOGD("%s : gpio < D_GPIO0 ! \n", __func__);
    else {
        gpio %= 4;
        ds90ub9xx_write_reg_witch_mask(handle, des_addr, 0x34, 0x02, 0x03); //select des port 1
    }

    for (i = 0; i < DS_GPIO_NUM / 2; i++) {
        if(gpio == gpc[i].gpio) {
            if (level == HIGHT) {
                gpc[i].des_val = 0x9;
            } else {
                gpc[i].des_val = 0x1;
            }
            ds90ub9xx_write_reg_witch_mask(handle, des_addr, gpc[i].des_reg, gpc[i].des_val, gpc[i].value_mask);
            LOGD("%s:config ds90ub948 gpio[%d] level[%d] succesed !\n", __func__, gpio, level);
            return 0;
        }
    }
    LOGE("[err] %s:config ds90ub948 gpio[%d] level[%d] failed !\n", __func__, gpio, level);
    return -1;
}

int ds90ub94x_948_gpio_config(void* i2c_handle, uint32_t ser_addr, uint32_t des_addr,
    enum ds90ub9xx_gpio gpio, enum ds_gpio_direct direct, enum des_port_slect des_port)
{
    void *handle = i2c_handle;
    int i = 0;

    if (handle == NULL) {
        LOGE("[err] %s : i2c handle is NULL ! \n", __func__);
        return -1;
    }

    ds90ub9xx_write_reg(handle, ser_addr, 0x1E, 0x04); //Use I2C ID+1 for FPD-Link III Port 1 register access
    ds90ub9xx_write_reg(handle, ser_addr, 0x17, 0x9e); //config I2C Pass All

    if (des_port == DES_PORT0)
        ds90ub9xx_write_reg_witch_mask(handle, des_addr, 0x34, 0x01, 0x03); //select des port 0
    else if (des_port == DES_PORT1)
        ds90ub9xx_write_reg_witch_mask(handle, des_addr, 0x34, 0x02, 0x03); //select des port 1
    else {
        LOGE("[err] %s :config ds90ub941/948 DES_PORT1[%d] value is out of value ! \n",
            __func__, des_port);
        return -1;
    }

    if (gpio < D_GPIO0) {

    } else if (gpio < DS_GPIO_NUM){
        gpio %= 4;
        ser_addr += 1; // ser ID+1 for Port 1
        ds90ub9xx_write_reg(handle, ser_addr, 0x1E, 0x04); //Use I2C ID+1 for FPD-Link III Port 1 register access
        ds90ub9xx_write_reg(handle, ser_addr, 0x17, 0x9e);
    } else {
        LOGE("[err] %s :config ds90ub941/948 gpio[%d] value is out of value ! \n", __func__, gpio);
        return -1;
    }

    for (i = 0; i < DS_GPIO_NUM / 2; i++) {
        if(gpio == gpc[i].gpio) {
            if (direct != INPUT) {
                gpc[i].ser_val = 0x3;
                gpc[i].des_val = 0x5;
            } else if (direct == OUTPUT){
                gpc[i].ser_val = 0x5;
                gpc[i].des_val = 0x3;
            } else {
                LOGE("[err] %s :config ds90ub941/948 direct[%d] value is out of value ! \n",
                    __func__, direct);
                return -1;
            }
            ds90ub9xx_write_reg_witch_mask(handle, ser_addr, gpc[i].ser_reg, gpc[i].ser_val, gpc[i].value_mask);
            ds90ub9xx_write_reg_witch_mask(handle, des_addr, gpc[i].des_reg, gpc[i].des_val, gpc[i].value_mask);
            //LOGD("%x, %x, %d, %x\n %x, %x, %d, %x \n",ser_addr,gpc[i].ser_reg,gpc[i].ser_val,gpc[i].value_mask,
                //des_addr, gpc[i].des_reg, gpc[i].des_val, gpc[i].value_mask);
            LOGD("%s:config ds90ub941/948 gpio[%d] direct[%d] succesed !\n", __func__, gpio, direct);
            return 0;
        }
    }
    LOGE("[err] %s:config ds90ub941/948 gpio[%d] direct[%d] failed !\n", __func__, gpio, direct);
    return -1;
}

int ds90ub94x_948_slaveI2CID_config(void* i2c_handle, uint32_t ser_addr, uint8_t slave_ID,
    enum ser_type_slect ser_type, enum ser_port_slect ser_port, uint8_t index)
{
    void *handle = i2c_handle;
    uint8_t reg1 = 0,reg2 = 0;

    if (handle == NULL) {
        LOGE("[err] %s : i2c handle is NULL ! \n", __func__);
        return -1;
    }
    if(ser_port == SER_PORT1)
        ser_addr++;
    ds90ub9xx_write_reg(handle, ser_addr, 0x1E, 0x04); //Use I2C ID+1 for FPD-Link III Port 1 register access
    if(ser_type == SER_947)
        ds90ub9xx_write_reg(handle, ser_addr, 0x03, 0x08); //I2C Pass-through Port0/Port1
    ds90ub9xx_write_reg(handle, ser_addr, 0x17, 0x9e); //config I2C Pass All
    if(index == 0) {
        reg1 = 0x07;
        reg2 = 0x08;
    } else if(index < 8){
        index -= 1;
        reg1 = 0x70 + index;
        reg2 = 0x77 + index;
    }
    ds90ub9xx_write_reg(handle, ser_addr, reg1, slave_ID << 1);
    ds90ub9xx_write_reg(handle, ser_addr, reg2, slave_ID << 1);

    thread_sleep(10);
    LOGD("%s : end ! \n", __func__);
    return 0;
}

int ds90ub94x_read_gpio_reg(void* i2c_handle, uint32_t ser_addr, enum ds90ub9xx_gpio gpio)
{
    int ret = 0;
    uint8_t reg = 0, read_reg = 0, status_mask = 0;
    uint8_t shift = 0;
    uint8_t val = 0;

    switch (gpio) {
        case GPIO5_REG:
            reg = 0x20;
            read_reg = 0x6E;
            shift = UB948_GPIO5_REG_SHIFT;
            status_mask = UB948_GPIO5_STATUS_MASK;
            break;
        case GPIO6_REG:
            reg = 0x20;
            read_reg = 0x6E;
            shift = UB948_GPIO6_REG_SHIFT;
            status_mask = UB948_GPIO6_STATUS_MASK;
            break;
        case GPIO7_REG:
            reg = 0x21;
            read_reg = 0x6E;
            shift = UB948_GPIO7_REG_SHIFT;
            status_mask = UB948_GPIO7_STATUS_MASK;
            break;
        case GPIO8_REG:
            reg = 0x21;
            read_reg = 0x6F;
            shift = UB948_GPIO8_REG_SHIFT;
            status_mask = UB948_GPIO8_STATUS_MASK;
            break;
        case GPIO9_REG:
            reg = 0x1A;
            read_reg = 0x6F;
            shift = UB948_GPIO9_REG_SHIFT;
            status_mask = UB948_GPIO9_STATUS_MASK;
            break;
        default:
            LOGE("input gpio:%d invalid\n", gpio);
            return -1;
            break;
    }

    ret = ds90ub9xx_write_reg(i2c_handle, ser_addr, reg, (0x03 << shift));
    if (ret < 0) {
        LOGE("config input gpio:%d err\n", gpio);
        goto read_err;
    }

    ret = ds90ub9xx_read_reg(i2c_handle, ser_addr, read_reg, &val);
    if (ret < 0) {
        LOGE("read gpio:%d err\n", gpio);
        goto read_err;
    }

    return (val & status_mask) >> UB948_GPIO6_STATUS_SHIFT;

read_err:
    return ret;
}

