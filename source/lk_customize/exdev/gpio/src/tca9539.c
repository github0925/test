/*
 * tca9539.c
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

#include "tca9539.h"

#define TCA9539_LOG                 SPEW

#define TCA9539_REG_INPUT_PORT0     0x0
#define TCA9539_REG_INPUT_PORT1     0x1
#define TCA9539_REG_OUTPUT_PORT0    0x2
#define TCA9539_REG_OUTPUT_PORT1    0x3
#define TCA9539_REG_INVERS_PORT0    0x4
#define TCA9539_REG_INVERS_PORT1    0x5
#define TCA9539_REG_CONFIG_PORT0    0x6
#define TCA9539_REG_CONFIG_PORT1    0x7

#define TCA9539_INTERRUPT           0x0

#define MAX_DEVICE_NUM 16
static struct tca9539_device *pdev[MAX_DEVICE_NUM];

static int tca9539_write_reg(struct tca9539_device *dev, u8 reg, u8 val)
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

static int tca9539_read_reg(struct tca9539_device *dev, u8 reg, u8 *val)
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


static enum handler_return tca9539_int_handler(void *data)
{
    dprintf(TCA9539_LOG, "%s\n", __func__);
    return INT_RESCHEDULE;
}

void tca9539_dump_registers(struct tca9539_device *dev)
{
    u8 reg, val;

    dprintf(TCA9539_LOG, "%s: dev->i2c_bus=%d, dev->device_address=0x%x\n",
            __func__, dev->i2c_bus, dev->device_address);

    reg = TCA9539_REG_INPUT_PORT0;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "input0: 0x%x\n", val);

    reg = TCA9539_REG_INPUT_PORT1;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "input0: 0x%x\n\n", val);

    reg = TCA9539_REG_OUTPUT_PORT0;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "output0: 0x%x\n", val);
    reg = TCA9539_REG_OUTPUT_PORT1;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "output1: 0x%x\n\n", val);

    reg = TCA9539_REG_INVERS_PORT0;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "inver0: 0x%x\n", val);
    reg = TCA9539_REG_INVERS_PORT1;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "inver1: 0x%x\n\n", val);

    reg = TCA9539_REG_CONFIG_PORT0;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "config0: 0x%x\n", val);
    reg = TCA9539_REG_CONFIG_PORT1;
    tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "config1: 0x%x\n", val);
}

static int tca9539_output_enable(struct tca9539_device *dev,
                                 int port_index)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port_index >= 1 && port_index <= 8) {
        reg = TCA9539_REG_CONFIG_PORT0;
    }
    else if (port_index >= 9 && port_index <= 16) {
        port_index -= 8;
        reg = TCA9539_REG_CONFIG_PORT1;
    }
    else {
        printf("error port index: %d\n", port_index);
        return ret;
    }

    ret = tca9539_read_reg(dev, reg, &val);
    val &= ~(1 << (port_index - 1));
    ret = tca9539_write_reg(dev, reg, val);

    return ret;
}

static int tca9539_output_val(struct tca9539_device *dev, int port_index,
                              int v)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port_index >= 1 && port_index <= 8) {
        reg = TCA9539_REG_OUTPUT_PORT0;
    }
    else if (port_index >= 9 && port_index <= 16) {
        reg = TCA9539_REG_OUTPUT_PORT1;
        port_index -= 8;
    }
    else {
        printf("error port index: %d\n", port_index);
        return ret;
    }

    ret = tca9539_read_reg(dev, reg, &val);

    if (v == 1)
        val |= 1 << (port_index - 1);
    else
        val &= ~(1 << (port_index - 1));

    ret = tca9539_write_reg(dev, reg, val);

    return ret;
}

static int tca9539_input_val(struct tca9539_device *dev, int port_index)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port_index >= 1 && port_index <= 8) {
        reg = TCA9539_REG_INPUT_PORT0;
    }
    else if (port_index >= 9 && port_index <= 16) {
        reg = TCA9539_REG_INPUT_PORT1;
        port_index -= 8;
    }
    else {
        printf("error port index: %d\n", port_index);
        return ret;
    }

    ret = tca9539_read_reg(dev, reg, &val);
    dprintf(TCA9539_LOG, "\t	reg=0x%x, val=0x%x, \n", reg, val);
    ret = (val & (1 << (port_index - 1))) >> (port_index - 1);
    dprintf(TCA9539_LOG, "\t	i2c=%d, addr=0x%x, port=%d, val=0x%x, \n",
            dev->i2c_bus, dev->device_address, port_index, ret);
    return ret;
}

static int tca9539_input_enable(struct tca9539_device *dev, int port_index)
{
    int ret = 0;
    u8 reg, val = 0;

    if (port_index >= 1 && port_index <= 8) {
        reg = TCA9539_REG_CONFIG_PORT0;
    }
    else if (port_index >= 9 && port_index <= 16) {
        reg = TCA9539_REG_CONFIG_PORT1;
        port_index -= 8;
    }
    else {
        printf("error port index: %d\n", port_index);
        return ret;
    }

    ret = tca9539_read_reg(dev, reg, &val);

    val |= 1 << (port_index - 1);
    dprintf(TCA9539_LOG, "%s: reg:0x%x, val:0x%x\n", __func__, reg, val);
    ret = tca9539_write_reg(dev, reg, val);

    //tca9539_dump_registers(dev);

    return ret;
}

static struct tca9539_dev_ops dev_ops = {
    .output_enable = tca9539_output_enable,
    .output_val = tca9539_output_val,
    .input_val = tca9539_input_val,
    .input_enable = tca9539_input_enable,
};

struct tca9539_device *tca9539_init(int i2c_bus, u8 addr)
{
    struct tca9539_device *dev = NULL;
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

    dprintf(TCA9539_LOG, "%s %d %d\n", __func__, i2c_bus, i);

    switch (i2c_bus) {
        case 1:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C1);
            break;

        case 2:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C2);
            break;

        case 3:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C3);
            break;

        case 4:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C4);
            break;

        case 5:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C5);
            break;

        case 6:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C6);
            break;

        case 7:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C7);
            break;

        case 8:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C8);
            break;

        case 9:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C9);
            break;

        case 10:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C10);
            break;

        case 11:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C11);
            break;

        case 12:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C12);
            break;

        case 13:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C13);
            break;

        case 14:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C14);
            break;

        case 15:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C15);
            break;

        case 16:
            hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);
            break;

        default:
            printf("wrong i2c bus\n");
            return 0;
    }

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
            pdev[i]->count++;
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

    dev->ops = dev_ops;

    dprintf(TCA9539_LOG, "%s() end\n\n", __func__);
    return dev;
}

int tca9539_enable_i2cpoll(struct tca9539_device *dev)
{
    void *i2c_handle = NULL;
    i2c_app_config_t i2c_conf;

    i2c_handle = dev->i2c_handle;

    if (!i2c_handle)
        return -1;

    i2c_conf = hal_i2c_get_busconfig(i2c_handle);
    i2c_conf.poll = 1;
    hal_i2c_set_busconfig(i2c_handle, &i2c_conf);
    return 0;
}

void tca9539_deinit(struct tca9539_device *dev)
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
