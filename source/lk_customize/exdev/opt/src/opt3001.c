/*
 * opt3001.c
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

#include "i2c_hal.h"
#include "opt3001.h"

#define OPT3001_SLAVE_ID          0x44
#define OPT3001_I2C_BUS           3

/*register address start*/
#define OPT3001_RESULT          0x00
#define OPT3001_CONFIGURATION   0x01
#define OPT3001_LOW_LIMIT       0x02
#define OPT3001_HIGH_LIMIT      0x03
#define OPT3001_DEVICE_ID       0x7f
#define OPT3001_MANUFACTURER_ID 0x7e
/*register address end*/

#define OPT3001_CONFIGURATION_MASK          (3 << 9)

static int opt3001_write_reg(struct opt3001_device *dev, u8 reg, u16 val)
{

    u8 buf[2];
    int ret;

    buf[0] = val >> 8;
    buf[1] = val;
    ret = hal_i2c_write_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                 buf, 2);

    if (ret < 0) {
        dprintf(INFO, "%s(): error: i2c=%d, addr=0x%x, write reg=%x\n", __func__,
                dev->i2c_bus, dev->device_address, reg);
    }

    return ret;
}

static int opt3001_read_reg(struct opt3001_device *dev, u8 reg, u16 *val)
{
    u8 buf[2];
    int ret;

    memset(buf, 0, sizeof(buf));
    ret = hal_i2c_read_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                buf, 2);

    if (ret < 0) {
        dprintf(INFO, "%s: error: i2c=%d, addr=0x%x, read reg=%x\n", __func__,
                dev->i2c_bus, dev->device_address, reg);
    }

    *val = buf[0] << 8 | buf[1];

    return ret;
}

u16 opt3001_get_id(struct opt3001_device *dev)
{
    u16 device_id;
    opt3001_read_reg(dev, OPT3001_DEVICE_ID, &device_id);

    return device_id;
}

void opt3001_get_configuration(struct opt3001_device *dev,
                               u16 *value)
{
    opt3001_read_reg(dev, OPT3001_CONFIGURATION, value);
}

void opt3001_config(struct opt3001_device *dev, enum opt3001_config_mode mode)
{
    u16 value;

    opt3001_get_configuration(dev, &value);
    value &= ~OPT3001_CONFIGURATION_MASK;
    value |= mode;
    opt3001_write_reg(dev, OPT3001_CONFIGURATION, value);
}

float opt3001_get_result(struct opt3001_device *dev)
{
    u16 val, val1, val2;
    float value;

    opt3001_read_reg(dev, OPT3001_RESULT, &val);

    val1 = val >> 12;
    val2 = val & 0x0FFF;
    /*lux = 0.01*(2^(E[3:0])*R[11:0])*/
    value = val2 << val1;
    value = value / 100;

    return value;
}

void opt3001_shutdown(struct opt3001_device *dev)
{
    u16 value;

    opt3001_get_configuration(dev, &value);
    value &= ~OPT3001_CONFIGURATION_MASK;
    value |= OPT3001_CONFIGURATION_SHUTDOWN;
    opt3001_write_reg(dev, OPT3001_CONFIGURATION, value);
}

/**
 * @param  : i2c_bus the number of channel,3 is connected
 * @brief  : create i2c_handle，complete dev
 * @return : struct opt3001_device
*/
struct opt3001_device *opt3001_init(void)
{
    struct opt3001_device *dev = NULL;
    void *i2c_handle = NULL;

    dprintf(INFO, "%s()\n", __func__);
    dev = (struct opt3001_device *)malloc(sizeof(*dev));

    if (!dev) {
        dprintf(CRITICAL, "memory allocation failed");
        return NULL;
    }

    /*label:G9_SAFETY_IO33_I2C2*/
    hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C3);

    if (i2c_handle == NULL) {
        dprintf(CRITICAL, "no i2c3 on this domain");
        return NULL;
    }

    dev->i2c_bus = OPT3001_I2C_BUS;
    dev->device_address = OPT3001_SLAVE_ID;
    dev->i2c_handle = i2c_handle;
    dprintf(INFO, "%s() end\n\n", __func__);

    return dev;
}

void opt3001_deinit(struct opt3001_device *dev)
{
    hal_i2c_release_handle(dev->i2c_handle);
    free(dev);
    dprintf(INFO, "%s() end\n", __func__);
}
