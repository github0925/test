/*
 * tmp411.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
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
#include <sys/types.h>
#include "i2c_hal.h"
#include "tmp411.h"

#define TMP411_SLAVE_ID 0x4c
#define TMP411_I2C_BUS  3

/*
 * The TMP401 registers, note some registers have different addresses for
 * reading and writing
*/

/*read address start*/
#define TMP411_LOCAL_TEMP_HIGH_READ           0x00
#define TMP411_STATUS                     0x02
#define TMP411_CONVERSION_RATE_READ           0x04
#define TMP411_CONFIG_READ                0x03
#define TMP411_LOCAL_HIGH_LIMIT_READ          0x05
#define TMP411_LOCAL_LOW_LIMIT_READ           0x06
#define TMP411_LOCAL_TEMP_LOW_READ            0x15
#define TMP411_LOCAL_THERM_LIMIT_READ         0x20
#define TMP411_MANUFACTURER_ID_REG            0xFE
/*read address end*/

/*write address start*/
#define TMP411_CONFIG_WRITE               0x09
/*write address end*/

#define TEMPERATURE_RANGE_MASK                (1 << 2)
#define ALERT_PIN_MASK                        (1 << 7)

#define TMP411_LOCAL_RESOLUTION               (0.5)
#define TMP411_LOCAL_LOW_BYTE_VALUE           (1 << 7)

static int tmp411_read_reg(struct tmp411_device *dev, u8 reg, u8 *val)
{
    int ret;

    ret = hal_i2c_read_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                val, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: i2c=%d, read reg=0x%x\n", __func__,
                dev->i2c_bus, reg);
    }

    return ret;
}

static int tmp411_write_reg(struct tmp411_device *dev, u8 reg, u8 val)
{
    int ret;

    ret = hal_i2c_write_reg_data(dev->i2c_handle, dev->device_address, &reg, 1,
                                 &val, 1);

    if (ret < 0) {
        dprintf(INFO, "%s: error: i2c=%d, write reg=0x%x\n", __func__,
                dev->i2c_bus, reg);
    }

    return ret;
}

/**
 * @brief : offset of 64(40h) is subtracted to the extended binary value
*/
bool check_extended_binary(struct tmp411_device *dev)
{
    u8 val;
    tmp411_read_reg(dev, TMP411_CONFIG_READ, &val);
    dprintf(INFO, "config =0x%x\n", val);
    return ((val & TEMPERATURE_RANGE_MASK) ? 1 : 0);
}

void extended_range_config(struct tmp411_device *dev)
{
    u8 val;
    tmp411_read_reg(dev, TMP411_CONFIG_WRITE, &val);
    val |= TEMPERATURE_RANGE_MASK;
    tmp411_write_reg(dev, TMP411_CONFIG_WRITE, val);
}

void alert_pin_config(struct tmp411_device *dev)
{
    u8 val;
    tmp411_read_reg(dev, TMP411_CONFIG_WRITE, &val);
    val |= ALERT_PIN_MASK;
    tmp411_write_reg(dev, TMP411_CONFIG_WRITE, val);
}

u8 tmp411_get_manufacture_id(struct tmp411_device *dev)
{
    u8 device_id;
    tmp411_read_reg(dev, TMP411_MANUFACTURER_ID_REG, &device_id);
    return device_id;
}

u8 tmp411_get_local_temp_low_limit(struct tmp411_device *dev)
{
    u8 result;
    tmp411_read_reg(dev, TMP411_LOCAL_LOW_LIMIT_READ, &result);
    dprintf(INFO, "low limit =0x%x\n", result);
    return result;
}

u8 tmp411_get_local_temp_high_limit(struct tmp411_device *dev)
{
    u8 result;
    tmp411_read_reg(dev, TMP411_LOCAL_HIGH_LIMIT_READ, &result);
    dprintf(INFO, "high limit =0x%x\n", result);
    return result;
}

u8 tmp411_get_local_temp_therm_limit(struct tmp411_device *dev)
{
    u8 result;
    tmp411_read_reg(dev, TMP411_LOCAL_THERM_LIMIT_READ, &result);
    dprintf(INFO, "therm limit =0x%x\n", result);
    return result;
}

float tmp411_get_result(struct tmp411_device *dev)
{
    u8 high, low, range_flag, offset;
    float result;

    range_flag = check_extended_binary(dev);

    if (range_flag) {
        dprintf(CRITICAL, "tmp411 is configured for extended range\n");
    }

    offset = range_flag ? 64 : 0;

    tmp411_read_reg(dev, TMP411_LOCAL_TEMP_HIGH_READ, &high);
    dprintf(INFO, "local temp high limit =0x%x\n", high);

    tmp411_read_reg(dev, TMP411_LOCAL_TEMP_LOW_READ, &low);
    dprintf(INFO, "local temp low limit =0x%x\n", low);

    result = high + (low & TMP411_LOCAL_LOW_BYTE_VALUE) * TMP411_LOCAL_RESOLUTION -
             offset;

    return result;
}

/**
 * @param  : i2c_bus the number of channel,3 is connected
 * @brief  : create i2c_handle,complete dev
 * @return : struct tmp411_device
*/
struct tmp411_device *tmp411_init(void)
{
    struct tmp411_device *dev = NULL;
    void *i2c_handle = NULL;

    dprintf(INFO, "%s()\n", __func__);
    dev = (struct tmp411_device *)malloc(sizeof(*dev));

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

    dev->i2c_bus = TMP411_I2C_BUS;
    dev->device_address = TMP411_SLAVE_ID;
    dev->i2c_handle = i2c_handle;
    dprintf(INFO, "%s() end\n\n", __func__);

    return dev;
}

void tmp411_deinit(struct tmp411_device *dev)
{
    hal_i2c_release_handle(dev->i2c_handle);
    free(dev);
    dprintf(INFO, "%s() end\n", __func__);
}
