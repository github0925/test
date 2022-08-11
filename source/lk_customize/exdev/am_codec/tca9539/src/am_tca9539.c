/**
 *@file am_tca9539.c
 *@author yi shao (yi.shao@semidrive.com)
 *@brief
 *@version 0.1
 *@date 2021-05-10
 *
 *@copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */
#include <debug.h>
#include <platform.h>

#include "am_tca9539.h"

#include "am.h"
#include "tca9539.h"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
/* GPIO not need read just set CACHE_FREE to init val
  This regs is virtual reg for GPIO one register for one GPIO Pin.
*/
static sdrv_am_reg_t tca9539_regs[] = {
    {TCA9539_00_PIN_REG0, 0x0, CACHE_FREE},
    {TCA9539_01_PIN_REG1, 0x0, CACHE_FREE},
    {TCA9539_02_PIN_REG2, 0x0, CACHE_FREE},
    {TCA9539_03_PIN_REG3, 0x0, CACHE_FREE},
    {TCA9539_04_PIN_REG4, 0x0, CACHE_FREE},
    {TCA9539_05_PIN_REG5, 0x0, CACHE_FREE},
    {TCA9539_06_PIN_REG6, 0x0, CACHE_FREE},
    {TCA9539_07_PIN_REG7, 0x0, CACHE_FREE},
    {TCA9539_08_PIN_REG8, 0x0, CACHE_FREE},
    {TCA9539_09_PIN_REG9, 0x0, CACHE_FREE},
    {TCA9539_0A_PIN_REG10, 0x0, CACHE_FREE},
    {TCA9539_0B_PIN_REG11, 0x0, CACHE_FREE},
    {TCA9539_0C_PIN_REG12, 0x0, CACHE_FREE},
    {TCA9539_0D_PIN_REG13, 0x0, CACHE_FREE},
    {TCA9539_0E_PIN_REG14, 0x0, CACHE_FREE},
    {TCA9539_0F_PIN_REG15, 0x0, CACHE_FREE}
};
/**
 * @brief tca9539 GPIO control.
 *
 */
static sdrv_am_ctl_t tca9539_ctls[] = {
    {TCA9539_00_01_P00, TCA9539_00_PIN_REG0, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_01_01_P01, TCA9539_01_PIN_REG1, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_02_01_P02, TCA9539_02_PIN_REG2, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_03_01_P03, TCA9539_03_PIN_REG3, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_04_01_P04, TCA9539_04_PIN_REG4, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_05_01_P05, TCA9539_05_PIN_REG5, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_06_01_P06, TCA9539_06_PIN_REG6, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_07_01_P07, TCA9539_07_PIN_REG7, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_08_01_P08, TCA9539_08_PIN_REG8, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_09_01_P09, TCA9539_09_PIN_REG9, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0A_01_P10, TCA9539_0A_PIN_REG10, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0B_01_P11, TCA9539_0B_PIN_REG11, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0C_01_P12, TCA9539_0C_PIN_REG12, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0D_01_P13, TCA9539_0D_PIN_REG13, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0E_01_P14, TCA9539_0E_PIN_REG14, 0x1, 0x0, CTL_GPIO_TYPE},
    {TCA9539_0F_01_P15, TCA9539_0F_PIN_REG15, 0x1, 0x0, CTL_GPIO_TYPE},
};
/**
 * @brief init tca9539
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_tca9539_initialize(am_codec_dev_t *dev)
{
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    return true;
}
/**
 * @brief read tca9539.
 *
 * @param dev
 * @param ctl
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tca9539_read_ctl(am_codec_dev_t *dev, unsigned int ctl,
                           unsigned int *val)
{
    struct tca9539_device *tca9539_handle;
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    if (ctl != tca9539_ctls[ctl].ctl_id) {
        _ERR_FUNC_LINE_
        return false;
    }
    if (dev->dev_handle) {
        tca9539_handle = (struct tca9539_device *)dev->dev_handle;
        tca9539_handle->ops.input_enable(tca9539_handle,
                                         tca9539_ctls[ctl].ctl_reg + 1);
        val[0] = tca9539_handle->ops.input_val(tca9539_handle,
                                               tca9539_ctls[ctl].ctl_reg + 1);
        return true;
    }
    return false;
}
/**
 * @brief write ctl to tac9539
 *
 * @param dev
 * @param ctl
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tca9539_write_ctl(am_codec_dev_t *dev, unsigned int ctl,
                            unsigned int val)
{
    struct tca9539_device *tca9539_handle;
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    if (ctl != tca9539_ctls[ctl].ctl_id) {
        _ERR_FUNC_LINE_
        return false;
    }
    if (dev->dev_handle) {
        tca9539_handle = (struct tca9539_device *)dev->dev_handle;
        /*tca9539 #define TCA9539_P00 1*/
        tca9539_handle->ops.output_enable(tca9539_handle,
                                          tca9539_ctls[ctl].ctl_reg + 1);
        tca9539_handle->ops.output_val(tca9539_handle,
                                       tca9539_ctls[ctl].ctl_reg + 1,
                                       val & tca9539_ctls[ctl].ctl_mask);
        return true;
    }
    return false;
}
/**
 * @brief read GPIO register value
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tca9539_read_reg(am_codec_dev_t *dev, unsigned int reg,
                           unsigned int *val)
{
    struct tca9539_device *tca9539_handle;
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    /* dprintf(0, "sdrv_tca9539_read_reg reg 0x%x \n",reg); */
    if (dev->dev_handle) {
        tca9539_handle = (struct tca9539_device *)dev->dev_handle;
        tca9539_handle->ops.input_enable(tca9539_handle, reg + 1);
        val[0] = tca9539_handle->ops.input_val(tca9539_handle, reg + 1);
        dprintf(0, "sdrv_tca9539_read_reg reg 0x%x (0x%x)\n", reg, val[0]);
        return true;
    }
    return false;
}
/**
 * @brief write a value to tca9539 virtual register.It will set GPIO pin
 * 		  to write mode.
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_tca9539_write_reg(am_codec_dev_t *dev, unsigned int reg,
                            unsigned int val)
{
    struct tca9539_device *tca9539_handle;
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    /* dprintf(0, "sdrv_tca9539_write_reg reg 0x%x \n",reg); */
    if (dev->dev_handle) {
        tca9539_handle = (struct tca9539_device *)dev->dev_handle;
        dprintf(0, "sdrv_tca9539_write_reg reg 0x%x (0x%x)\n", reg, val);
        /*tca9539 #define TCA9539_P00 1*/
        tca9539_handle->ops.output_enable(tca9539_handle, reg + 1);
        tca9539_handle->ops.output_val(tca9539_handle, reg + 1, val & 0x1);
        return true;
    }
    return false;
}
/**
 * @brief reset tac9539 and clean reg status to init state.
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_tca9539_reset(am_codec_dev_t *dev)
{
    if (dev->codec_type != CODEC_TYPE_TCA9539) {
        _ERR_FUNC_LINE_
        return false;
    }
    for (uint i = 0; i <  ARRAY_SIZE(tca9539_regs); i++) {
        tca9539_regs[i].status = CACHE_FREE;
    }
    return true;
}


static struct am_ctl_interface tca9539_drv = {
    sdrv_tca9539_initialize, ///< tca9539 initialize
    sdrv_tca9539_read_ctl,   ///< tca9539 read ctl
    sdrv_tca9539_write_ctl,  ///< tca9539 write_ctl
    sdrv_tca9539_read_reg,   ///< tca9539 read from register(virtual)
    sdrv_tca9539_write_reg,  ///< tca9539 write to register(virtual)
    sdrv_tca9539_reset,      ///< tca9539 reset
    NULL,                    ///< don't support this operation
    NULL,                    ///< don't support this operation
    NULL,                    ///< gpio writeable_reg set in board sequence.
    NULL,                    ///< gpio readable_reg set in board sequence.
    tca9539_regs,            ///< tca9539 reset
    tca9539_ctls,            ///< tca9539 burn fw
    ARRAY_SIZE(tca9539_regs),
};
/**
 * @brief get io expander tca9539 control interface
 *
 * @return struct am_ctl_interface*
 */
struct am_ctl_interface *sdrv_tca9539_get_ctl_interface(void)
{
    return &tca9539_drv;
}