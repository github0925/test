/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

#include "i2c_hal.h"

#include "max20086.h"

#define MAX20086_LOG 4

//static void *i2c_handle;

//#define MAX20086_SLAVE_ID 0x2A
#define MAX20086_SLAVE_ID 0x28

#define MAX20086_REG_MASK       0x0
#define MAX20086_REG_CONFIG     0x1
#define MAX20086_REG_ID 0x2
#define MAX20086_REG_STAT1  0x3
#define MAX20086_REG_STAT2  0x4
#define MAX20086_REG_STAT2E 0x5
#define MAX20086_REG_ADC1   0x6
#define MAX20086_REG_ADC2   0x7
#define MAX20086_REG_ADC3   0x8
#define MAX20086_REG_ADC4   0x9

#define MAX20086_INTERRUPT 0x0

static int max20086_write_reg(struct max20086_device *dev, u8 reg, u8 val)
{
    u8 buf[2];
    int ret;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(dev->i2c_handle, dev->device_address, buf, 1,
                                 buf + 1, 1);

    //dprintf(MAX20086_LOG, "%s(), ret=%d, reg=0x%x\n", __func__, ret, reg);
    if (ret < 0) {
        dprintf(INFO, "%s: error: reg=%x, val=%x\n",
                __func__, reg, val);
        return ret;
    }

    return 0;
}

static int max20086_read_reg(struct max20086_device *dev, u8 reg, u8 *val)
{

    u8 buf[1];
    int ret;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                buf, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: read reg=%x\n", __func__, reg);
        return ret;
    }

    //dprintf(MAX20086_LOG, "%s(): trans reg=0x%x, rece buf[0]=0x%x\n ", __func__, reg, buf[0]);
    *val = buf[0];
    return 0;
}

static enum handler_return max20086_int_handler(void *data)
{
    dprintf(0, "%s\n", __func__);
    return INT_RESCHEDULE;
}

static int max20086_dump_registers(struct max20086_device *dev)
{
    u8 reg, val;
    int ret;
    dprintf(MAX20086_LOG, "%s\n", __func__);

    for (int i = 0; i < 10; i++) {
        reg = i;
        ret = max20086_read_reg(dev, reg, &val);
        dprintf(MAX20086_LOG, "reg 0x%x: 0x%x\n", reg, val);
    }

    dprintf(MAX20086_LOG, "%s, end\n\n", __func__);

    return ret;
}

static int max20086_init_registers(struct max20086_device *dev)
{
    u8 reg, val;
    int ret;

    reg = 0x1;
    val = 0x10;
    dprintf(MAX20086_LOG, "write reg 0x%x: 0x%x\n", reg, val);
    ret = max20086_write_reg(dev, reg, val);
    val = 0;
    ret = max20086_read_reg(dev, reg, &val);
    dprintf(0, "[0x%x]: reg 0x%x: 0x%x\n", dev->device_address, reg, val);
    spin(5000);

    val = 0x1f;
    dprintf(MAX20086_LOG, "write reg 0x%x: 0x%x\n", reg, val);
    ret = max20086_write_reg(dev, reg, val);
    val = 0;
    ret = max20086_read_reg(dev, reg, &val);
    dprintf(0, "[0x%x]: reg 0x%x: 0x%x\n", dev->device_address, reg, val);
    spin(10000);

    return ret;
}

static struct max20086_dev_ops dev_ops = {
    .dump_register = max20086_dump_registers,
};

static const uint32_t i2c_res[16] = {
    [0] = RES_I2C_I2C1,
    [1] = RES_I2C_I2C2,
    [2] = RES_I2C_I2C3,
    [3] = RES_I2C_I2C4,
    [4] = RES_I2C_I2C5,
    [5] = RES_I2C_I2C6,
    [6] = RES_I2C_I2C7,
    [7] = RES_I2C_I2C8,
    [8] = RES_I2C_I2C9,
    [9] = RES_I2C_I2C10,
    [10] = RES_I2C_I2C11,
    [11] = RES_I2C_I2C12,
    [12] = RES_I2C_I2C13,
    [13] = RES_I2C_I2C14,
    [14] = RES_I2C_I2C15,
    [15] = RES_I2C_I2C16,
};

struct max20086_device *max20086_init(int i2c_bus, u8 addr)
{

    struct max20086_device *dev;
    void *i2c_handle;

    dprintf(MAX20086_LOG, "%s: i2c_bus =%d\n", __func__, i2c_bus);

    dev = malloc(sizeof(*dev));
    memset(dev, 0, sizeof(*dev));

    if (!dev)
        return NULL;

    if (i2c_bus < 1 || i2c_bus > 16) {
        printf("wrong i2c bus\n");
        goto err;
    }
    else
        hal_i2c_creat_handle(&i2c_handle, i2c_res[i2c_bus - 1]);


    dev->i2c_handle = i2c_handle;

    dev->device_address = addr;
    dev->ops = dev_ops;

    //register_int_handler(MAX20086_INTERRUPT, max20086_int_handler, (void *)0);
    //unmask_interrupt(TCA9539_INTERRUPT);

    max20086_dump_registers(dev);
    max20086_init_registers(dev);

    dprintf(MAX20086_LOG, "%s() end\n\n", __func__);
    return dev;
err:
    free(dev);
    dev = NULL;
    return dev;
}

int max20086_deinit(struct max20086_device *dev)
{
    if (!dev)
        return -1;

    if (dev->i2c_handle)
        hal_i2c_release_handle(dev->i2c_handle);

    free(dev);
    dev = NULL;
    return 0;
}

