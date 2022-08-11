/*
 * tca6408.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <kernel/mutex.h>
#include <platform.h>

#include "i2c_hal.h"

#include "tca6408.h"

#define TCA6408_LOG                 SPEW

#define TCA6408_REG_INPUT_PORT     0x0
#define TCA6408_REG_OUTPUT_PORT    0x1
#define TCA6408_REG_INVERS_PORT    0x2
#define TCA6408_REG_CONFIG_PORT    0x3

#define TCA6408_INTERRUPT           0x0

#define MAX_DEVICE_NUM 16
static struct tca6408_device *pdev[MAX_DEVICE_NUM];

static int tca6408_write_reg(struct tca6408_device *dev, u8 reg, u8 val)
{
    u8 buf[2];
    int ret;

    buf[0] = reg;
    buf[1] = val;

    ret = hal_i2c_write_reg_data(dev->i2c_handle, dev->device_address, buf, 1,
                                 buf + 1, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: i2c=%d, addr=0x%x, reg=%x, val=%x\n",
                __func__, dev->i2c_bus, dev->device_address, reg, val);
        return ret;
    }

    return 0;
}

static int tca6408_read_reg(struct tca6408_device *dev, u8 reg, u8 *val)
{

    u8 buf[1];
    int ret;

    memset(buf, 0, sizeof(buf));

    ret = hal_i2c_read_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                buf, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: i2c=%d, addr=0x%x, read reg=%x\n", __func__,
                dev->i2c_bus, dev->device_address, reg);
        return ret;
    }

    *val = buf[0];
    return 0;
}


static enum handler_return tca6408_int_handler(void *data)
{
    dprintf(TCA6408_LOG, "%s\n", __func__);
    return INT_RESCHEDULE;
}

void tca6408_dump_registers(struct tca6408_device *dev)
{
    u8 reg, val;

    dprintf(TCA6408_LOG, "%s: dev->i2c_bus=%d, dev->device_address=0x%x\n",
            __func__, dev->i2c_bus, dev->device_address);

    reg = TCA6408_REG_INPUT_PORT;
    tca6408_read_reg(dev, reg, &val);
    dprintf(TCA6408_LOG, "input: 0x%x\n", val);

    reg = TCA6408_REG_OUTPUT_PORT;
    tca6408_read_reg(dev, reg, &val);
    dprintf(TCA6408_LOG, "output: 0x%x\n", val);

    reg = TCA6408_REG_INVERS_PORT;
    tca6408_read_reg(dev, reg, &val);
    dprintf(TCA6408_LOG, "inver: 0x%x\n", val);

    reg = TCA6408_REG_CONFIG_PORT;
    tca6408_read_reg(dev, reg, &val);
    dprintf(TCA6408_LOG, "config: 0x%x\n", val);

}

int tca6408_output_enable(struct tca6408_device *dev,
                          enum tca6408_port port)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port >= TCA6408_P0 && port < TCA6408_PORT_NUM) {
        reg = TCA6408_REG_CONFIG_PORT;
    }
    else {
        printf("error port: %d\n", port);
        return ret;
    }

    ret = tca6408_read_reg(dev, reg, &val);
    val &= ~(1 << port);
    ret = tca6408_write_reg(dev, reg, val);

    return ret;
}

int tca6408_output_val(struct tca6408_device *dev, enum tca6408_port port,
                       int v)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port >= TCA6408_P0 && port < TCA6408_PORT_NUM) {
        reg = TCA6408_REG_OUTPUT_PORT;
    }
    else {
        printf("error port: %d\n", port);
        return ret;
    }

    ret = tca6408_read_reg(dev, reg, &val);

    if (v == 1)
        val |= 1 << port;
    else
        val &= ~(1 << port);

    ret = tca6408_write_reg(dev, reg, val);

    return ret;
}

int tca6408_input_val(struct tca6408_device *dev, enum tca6408_port port)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port >= TCA6408_P0 && port < TCA6408_PORT_NUM) {
        reg = TCA6408_REG_INPUT_PORT;
    }
    else {
        printf("error port index: %d\n", port);
        return ret;
    }

    ret = tca6408_read_reg(dev, reg, &val);
    dprintf(TCA6408_LOG, "\t	reg=0x%x, val=0x%x, \n", reg, val);
    ret = (val & (1 << port)) >> port;
    dprintf(TCA6408_LOG, "\t	i2c=%d, addr=0x%x, port=%d, val=0x%x, \n",
            dev->i2c_bus, dev->device_address, port, ret);
    return ret;
}

int tca6408_input_enable(struct tca6408_device *dev,
                         enum tca6408_port port)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port >= TCA6408_P0 && port < TCA6408_PORT_NUM) {
        reg = TCA6408_REG_CONFIG_PORT;
    }
    else {
        printf("error port: %d\n", port);
        return ret;
    }

    ret = tca6408_read_reg(dev, reg, &val);

    val |= 1 << port;
    dprintf(TCA6408_LOG, "%s: reg:0x%x, val:0x%x\n", __func__, reg, val);
    ret = tca6408_write_reg(dev, reg, val);

    //tca6408_dump_registers(dev);

    return ret;
}

#if 0
static struct tca6408_dev_ops dev_ops = {
    .output_enable = tca6408_output_enable,
    .output_val = tca6408_output_val,
    .input_val = tca6408_input_val,
    .input_enable = tca6408_input_enable,
};
#endif

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


struct tca6408_device *tca6408_init(int i2c_bus, u8 addr)
{
    struct tca6408_device *dev = NULL;
    void *i2c_handle = NULL;
    int i;

    for (i = 0; i < MAX_DEVICE_NUM; i++) {
        if (pdev[i] != NULL) {
            if (pdev[i]->i2c_bus == i2c_bus && pdev[i]->device_address == addr) {
                pdev[i]->count++;
                return pdev[i];
            }
        }
    }

    dprintf(TCA6408_LOG, "%s %d %d\n", __func__, i2c_bus, i);


    if (i2c_bus < 1 || i2c_bus > 16) {
        printf("wrong i2c bus\n");
        return NULL;
    }
    else
        hal_i2c_creat_handle(&i2c_handle, i2c_res[i2c_bus - 1]);

    dev = malloc(sizeof(*dev));

    if (!dev)
        return NULL;

    memset(dev, 0, sizeof(*dev));
    dev->i2c_bus = i2c_bus;
    dev->device_address = addr;
    dev->i2c_handle = i2c_handle;

    for (i = 0; i < MAX_DEVICE_NUM; i++) {
        if (pdev[i] == NULL) {
            pdev[i] = dev;
            break;
        }
    }

    if (i == MAX_DEVICE_NUM)
        printf("have no space!\n");

#if 0
    i2c_app_config_t config;
    config = hal_i2c_get_busconfig(i2c_handle);
    printf("get config: speed=%d.\n", config.speed);
    config.speed = HAL_I2C_SPEED_STANDARD;
    config.addr_bits = HAL_I2C_ADDR_7BITS;
    config.mode = HAL_I2C_MASTER_MODE;
    printf("set config: speed=%d.\n", config.speed);
    hal_i2c_set_busconfig(i2c_handle, &config);
#endif

    //dev->ops = dev_ops;

    dprintf(TCA6408_LOG, "%s() end\n\n", __func__);
    return dev;
}

void tca6408_deinit(struct tca6408_device *dev)
{
    for (int i = 0; i < MAX_DEVICE_NUM; i++) {
        if (pdev[i] == dev) {
            dev->count--;

            if (dev->count == 0) {
                hal_i2c_release_handle(dev->i2c_handle);
                free(dev);
                pdev[i] = NULL;
            }
        }
    }
}
