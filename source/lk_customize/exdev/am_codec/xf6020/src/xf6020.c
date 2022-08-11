/**
 *@file xf6020.c
 *@author liang lou (liang.lou@semidrive.com)
 *@brief
 *@version 0.1
 *@date 2021-05-17
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#include <debug.h>
#include <heap.h>
#include <platform.h>
#include <res_loader.h>

#include "xf6020.h"

#include "am.h"
#include "i2c_hal.h"

#define CODEC_XF6020_DBG_PRT 2
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define XF6020_I2C_IF
static sdrv_am_reg_t xf6020_regs[] = {{XF6020_00_VOICE_MODE, 0x0, CACHE_FREE},
                                      {XF6020_01_PARAM, 0x0, CACHE_FREE},
                                      {XF6020_02_BYPASS, 0x0, CACHE_FREE},
                                      {XF6020_0C_VERSION, 0x0, CACHE_FREE}};

static sdrv_am_ctl_t xf6020_ctls[] = {
    {XF6020_00_FF_VOICEMODE, XF6020_00_VOICE_MODE, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_01_FF_PARAM, XF6020_01_PARAM, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_02_FF_BYPASS, XF6020_02_BYPASS, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_0C_FF_VERSION, XF6020_0C_VERSION, 0xff, 0x0, CTL_NORMAL_TYPE},
};

static int xf6020_read_reg(void *handle, unsigned int addr, unsigned int reg,
                           void *data)
{
    unsigned char tx[3] = {0}, rx[4] = {0};
    int wlen, rlen;
    int ret;
    char *p = (char *)data;
    if (reg != XF6020_0C_VERSION) {
        wlen = 2;
        rlen = 1;
        tx[0] = (unsigned char)(0xFF & (reg >> 8));
        tx[1] = (unsigned char)(0xFF & reg);
        ret = hal_i2c_read_reg_data(handle, addr, tx, wlen, rx, rlen);
        dprintf(CODEC_XF6020_DBG_PRT,
                "%s: result: read i2c ext addr=0x%x,reg=0x%x 0x%x\n", __func__,
                addr, reg, rx[0]);
        if (ret < 0) {
            dprintf(0, "%s: error: read xf6020 reg=%x,ret=%x\n", __func__, reg,
                    ret);
            return ret;
        }
        p[0] = rx[0] & 0xff;
        return ret;
    }
    /* xf6020 reg version will return 4 bytes data */
    wlen = 2;
    rlen = 4;
    tx[0] = (unsigned char)(0xFF & (reg >> 8));
    tx[1] = (unsigned char)(0xFF & reg);
    ret = hal_i2c_read_reg_data(handle, addr, tx, wlen, rx, rlen);
    if (ret < 0) {
        dprintf(0, "%s: error: read xf6020 reg=%x,ret=%x\n", __func__, reg,
                ret);
        return ret;
    }
    p[0] = rx[0] & 0xff;
    p[1] = rx[1] & 0xff;
    p[2] = rx[2] & 0xff;
    p[3] = rx[3] & 0xff;
    return ret;
}
static int xf6020_write_reg(void *handle, unsigned int addr, unsigned int reg,
                            unsigned int val)
{
    uint8_t tx[4];
    int wlen;
    int regcnt;
    int ret = 0;
    dprintf(CODEC_XF6020_DBG_PRT, "[xf6020] %s (%XH, %XH,%XH)\n", __FUNCTION__,
            reg, val, addr);
    wlen = 1;
    regcnt = 2;
    if (reg != XF6020_0C_VERSION) {
        tx[0] = (unsigned char)(0xFF & (reg >> 8));
        tx[1] = (unsigned char)(0xFF & reg);
        tx[2] = (unsigned char)(0xFF & val);
        ret = hal_i2c_write_reg_data(handle, addr, tx, regcnt, &tx[2], wlen);
        if (ret < 0) {
            dprintf(0, "%s: error: %p reg=0x%x, val=0x%x\n", __func__, handle,
                    reg, val);
        }
    }
    return ret;
}

/**
 * @brief init xf6020 function
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_xf6020_initialize(am_codec_dev_t *dev)
{
    if (dev->codec_type != CODEC_TYPE_XF6020) {
        _ERR_FUNC_LINE_
        return false;
    }
    // test
    return true;
}
static bool sdrv_xf6020_read_ctl(am_codec_dev_t *dev, unsigned int ctl,
                          unsigned int *val)
{
    _NO_IMPL_PRT_
    return false;
}
static bool sdrv_xf6020_write_ctl(am_codec_dev_t *dev, unsigned int ctl,
                           unsigned int val)
{
    _NO_IMPL_PRT_
    return false;
}
/**
 * @brief read a register value from xf6020
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_xf6020_read_reg(am_codec_dev_t *dev, unsigned int reg,
                          unsigned int *val)
{
    int ret;
    if (dev->codec_type != CODEC_TYPE_XF6020) {
        _ERR_FUNC_LINE_
        return false;
    }
    ret = xf6020_read_reg(dev->dev_handle, dev->addr, reg, val);
    if (ret < 0) {
        return false;
    }
    return true;
}
/**
 * @brief write a val to an xf6020 register.
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_xf6020_write_reg(am_codec_dev_t *dev, unsigned int reg,
                           unsigned int val)
{
    int ret;
    if (dev->codec_type != CODEC_TYPE_XF6020) {
        _ERR_FUNC_LINE_
        return false;
    }
    ret = xf6020_write_reg(dev->dev_handle, dev->addr, reg, val);
    if (ret < 0) {
        return false;
    }
    return true;
}
/**
 * @brief reset xf6020 codec
 *
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_xf6020_reset(am_codec_dev_t *dev)
{
    /*     TODO: need consider multi instance.
        clean reg cache*/
    for (uint i = 0; i < ARRAY_SIZE(xf6020_regs); i++) {
        xf6020_regs[i].status = CACHE_FREE;
    }
    return true;
}

static struct am_ctl_interface xf6020_drv = {
    sdrv_xf6020_initialize, ///< xf6020 initialize
    sdrv_xf6020_read_ctl,   ///< xf6020 read ctl:TBD
    sdrv_xf6020_write_ctl,  ///< xf6020 write_ctl:TBD
    sdrv_xf6020_read_reg,   ///< xf6020 read reg
    sdrv_xf6020_write_reg,  ///< xf6020 write_reg
    sdrv_xf6020_reset,      ///< xf6020 reset
    NULL,                   ///< xf6020 burn dsp fw
    NULL,                    ///< xf6020 user func
    NULL,                   ///< writeable_reg NULL will return all true.
    NULL,                   ///< read_reg NULL will return all true.
    xf6020_regs,            ///< xf6020 register data structure
    xf6020_ctls,            ///< xf6020 controls data structure
    ARRAY_SIZE(xf6020_regs),
};
/**
 * @brief get xf6020 ctl interface
 *
 * @return struct am_ctl_interface*
 */
struct am_ctl_interface *sdrv_xf6020_get_ctl_interface(void)
{
    return &xf6020_drv;
}
