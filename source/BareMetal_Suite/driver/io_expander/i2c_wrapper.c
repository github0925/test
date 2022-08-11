/*
 * i2c_wrapper.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: i2c transcation layer based on
 * Semidrive I2C HAL to support manipulation of
 * tca9539.
 *
 * Revision History:
 * -----------------
 * 0.1, 2/9/2020 init version
 */

#include <common_hdr.h>
#include <soc.h>
#include "TCA9539.h"
#include "i2c/dw_i2c/include/dw_i2c.h"
#include "error.h"

#define MAX_I2C_BUS_NUM 4
dw_i2c_context  i2c_ctx[MAX_I2C_BUS_NUM];

inline static dw_i2c_context *get_i2c_ctx(module_e m)
{
    uint32_t i = 0;

    for (; i < MAX_I2C_BUS_NUM; i++) {
        if (i2c_ctx[i].info.io_base == soc_get_module_base(m)
            && i2c_ctx[i].is_configured) {
            break;
        }
    }

    if (i != MAX_I2C_BUS_NUM) {
        return &i2c_ctx[i];
    } else {
        return NULL;
    }
}

bool TCA9539_i2c_init(module_e m, uint8_t address)
{
    addr_t b = soc_get_module_base(m);
    dw_i2c_config_t tca9539_i2c_cfg = {
        .io_base = b,
        .speed = I2C_SPEED_STANDARD,
        .addr_bits = ADDR_7BITS,
        .mode = MASTER_MODE,
        .slave_addr = address,
    };
    uint32_t i = 0;

    for (; i < MAX_I2C_BUS_NUM; i++) {
        if (!i2c_ctx[i].is_configured) {
            break;
        }
    }

    if (i < MAX_I2C_BUS_NUM) {
        return dw_i2c_set_busconfig(&i2c_ctx[i], &tca9539_i2c_cfg);
    } else {
        return false;
    }
}

bool TCA9539_i2c_write_reg(module_e m, uint8_t reg, uint8_t data)
{
    dw_i2c_context *ctx = get_i2c_ctx(m);

    if (NULL == ctx) {
        return false;
    } else {
        return (NO_ERROR == dw_i2c_write_reg_bytes(ctx, ctx->info.slave_addr, reg, &data, 1));
    }
}

bool TCA9539_i2c_read_reg(module_e m, uint8_t reg, uint8_t *data)
{
    dw_i2c_context *ctx = get_i2c_ctx(m);

    if (NULL == ctx) {
        return false;
    } else {
        return (NO_ERROR == dw_i2c_read_reg_bytes(ctx, ctx->info.slave_addr, reg, data, 1));
    }
}

bool TCA9539_i2c_deinit(module_e m)
{
    dw_i2c_context *ctx = get_i2c_ctx(m);

    if (NULL == ctx) {
        return false;
    } else {
        memset((void *)ctx, 0, sizeof(dw_i2c_context));
        return true;
    }
}
