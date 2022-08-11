/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <i2c_hal.h>
#include <hal_port.h>

/*
** this is i2c demo for how use semidrive i2c api
** the i2c slave device is TI ds90ub941, slave address is 0x0c(7bit)
*/

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int i2c_demo_func(int argc, const cmd_args *argv)
{
    void *i2c_handle;
    bool creat_stat = false;
    int ret;
    uint8_t ds941_addr = 0x0c;
    uint8_t ds941_reg = 0x0d;
    uint8_t rbuf[4] = {0x00};
    /*
    ** we have 1~16 i2c controller, the ds90ub941 conneted to i2c16 on ref board
    ** so we creat handle from "RES_I2C_I2C16"
    */
    creat_stat = hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C16);

    if (!creat_stat)
        dprintf(CRITICAL, "%s creat fail\n", __func__);

    /*
    **use wrap api hal_i2c_write_reg_data/hal_i2c_read_reg_data
    **this can be easy use and adapt to most scene read/write
    */
    uint8_t wbuf1[4] = {0x3f, 0xff, 0xff, 0xff};
    uint8_t wbuf2[4] = {0x30, 0x00, 0x00, 0x00};
    ret = hal_i2c_write_reg_data(i2c_handle, ds941_addr, &ds941_reg, 1, wbuf1,
                                 4);

    if (ret)
        dprintf(CRITICAL, "%s write fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success write data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, wbuf1[0], wbuf1[1], wbuf1[2], wbuf1[3]);

    ret = hal_i2c_read_reg_data(i2c_handle, ds941_addr, &ds941_reg, 1, rbuf,
                                4);

    if (ret)
        dprintf(CRITICAL, "%s read fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success read  data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

    ret = hal_i2c_write_reg_data(i2c_handle, ds941_addr, &ds941_reg, 1, wbuf2,
                                 4);

    if (ret)
        dprintf(CRITICAL, "%s write fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success write data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, wbuf2[0], wbuf2[1], wbuf2[2], wbuf2[3]);

    ret = hal_i2c_read_reg_data(i2c_handle, ds941_addr, &ds941_reg, 1, rbuf,
                                4);

    if (ret)
        dprintf(CRITICAL, "%s read fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success read  data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

    /*
    **use hal_i2c_common_xfer api need to filling msgs by customer
    **this can be transfer for user defined read/write format
    */
    struct i2c_msg msgs[2];
    uint8_t wbuf3[5] = {0x0d, 0x3f, 0xff, 0xff, 0xff};// 0x0d is register same as ds941_reg
    msgs[0].flags = I2C_M_WR;
    msgs[0].addr  = ds941_addr;
    msgs[0].len   = 5;
    msgs[0].buf   = wbuf3;
    ret = hal_i2c_common_xfer(i2c_handle, msgs, 1);

    if (ret)
        dprintf(CRITICAL, "%s write fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success write data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, wbuf3[1], wbuf3[2], wbuf3[3], wbuf3[4]);

    msgs[0].flags = I2C_M_WR;
    msgs[0].addr  = ds941_addr;
    msgs[0].len   = 1;
    msgs[0].buf   = wbuf3;
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = ds941_addr;
    msgs[1].len   = 4;
    msgs[1].buf   = rbuf;
    ret = hal_i2c_common_xfer(i2c_handle, msgs, 2);

    if (ret)
        dprintf(CRITICAL, "%s read fail\n", __func__);
    else
        dprintf(CRITICAL, "%s success read  data=0x%x, 0x%x, 0x%x, 0x%x\n",
                __func__, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

    hal_i2c_release_handle(i2c_handle);
    dprintf(CRITICAL, "%s i2c communication test end\n", __func__);
    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("i2c_demo", "i2c_demo",
                                    (console_cmd)&i2c_demo_func)
STATIC_COMMAND_END(i2c_demo);
#endif

