/*
 * Copyright (C) Semidrive Semiconductor Ltd.
 * All rights reserved.
 */

#include <tca9539.h>
#include <dcf.h>

#include "touch_config.h"

#if SERDES_TP_X9H
#if !TOUCH_SERDES_DIVIDED
static struct ts_board_config tsc[] = {
    {
        TP_DSI1, TP_GOODIX_9XX, true, true, TI941_DUAL, false, RES_I2C_I2C16, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR, DP_CA_AP1,
        DEV_TYPE_MAIN, 0x76, TCA9539_P01, 0x0c, 0x2c, 3, 2, 1920, 720, 0, 0, 0, 0, PortConf_PIN_EMMC2_CLK,
        {//pin144
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_DSI2, TP_GOODIX_9XX, true, true, TI941_DUAL, false, RES_I2C_I2C16, GT9XX_DEFAULT_SLAVE_ID_14, TS_MBOX_ADDR+1, DP_CA_AP1,
        DEV_TYPE_AUX, 0x76, TCA9539_P02, 0x0d, 0x3c, 3, 2, 1920, 720, 0, 0, 0, 0, PortConf_PIN_EMMC2_CMD,
        {//pin145
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__UP ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_LVDS3, TP_GOODIX_9XX, false, false, TI947_SINGLE, false, RES_I2C_I2C14, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR+2, DP_CA_AP1,
        DEV_TYPE_AUX, 0x75, TCA9539_P06, 0, 0, 0, 0, 1920, 720, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD6,
        {//pin130
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
#ifdef ENABLE_CONTROLPANEL
    {
        TP_LVDS4, TP_GOODIX_9XX, true, true, TI947_SINGLE, false, RES_I2C_I2C15, GT9XX_DEFAULT_SLAVE_ID, 0, 0,
        DEV_TYPE_MAIN, 0x75, TCA9539_P07, 0x1a, 0x2c, 3, 2, 1920, 720, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD7,
        {//pin131
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
#endif
};
#else
static struct ts_board_config tsc[] = {
    {
        TP_DSI1, TP_GOODIX_9XX, true, true, TI941_SINGLE, true, RES_I2C_I2C16, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR, DP_CA_AP1,
        DEV_TYPE_MAIN, 0x76, TCA9539_P01, 0x0c, 0x2c, 3, 2, 1920, 720, 640, 0, 1280, 600, PortConf_PIN_EMMC2_CLK,
        {//pin144
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_DSI1, TP_GOODIX_9XX, true, true, TI941_SINGLE, true, 0, 0, 0, 0,
        DEV_TYPE_MAIN, 0x76, TCA9539_P02, 0x0c, 0x2c, 3, 2, 1920, 720, 0, 600, 1920, 120, PortConf_PIN_EMMC2_CMD,
        {//pin145
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__UP ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_LVDS3, TP_GOODIX_9XX, false, false, TI947_SINGLE, false, RES_I2C_I2C14, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR, DP_CA_AP1,
        DEV_TYPE_AUX, 0x75, TCA9539_P06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD6,
        {//pin130
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_LVDS4, TP_GOODIX_9XX, false, false, TI947_SINGLE, false, RES_I2C_I2C15, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR+1, DP_CA_AP1,
        DEV_TYPE_MAIN, 0x75, TCA9539_P07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD7,
        {//pin131
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
};
#endif
#endif

#if SERDES_TP_X9M
static struct ts_board_config tsc[] = {
    {
        TP_DSI1, TP_GOODIX_9XX, true, true, TI941_SINGLE, false, RES_I2C_I2C16, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR, DP_CA_AP1,
        DEV_TYPE_MAIN, 0x76, TCA9539_P01, 0x0c, 0x2c, 3, 2, 1920, 720, 0, 0, 1920, 720, PortConf_PIN_EMMC2_CLK,
        {//pin144
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_DSI2, TP_GOODIX_9XX, false, true, TI941_SINGLE, false, 0, 0, 0, 0,
        DEV_TYPE_MAIN, 0x76, TCA9539_P02, 0x0c, 0x2c, 3, 2, 1920, 720, 0, 0, 0, 0, PortConf_PIN_EMMC2_CMD,
        {//pin145
        ((uint32_t)PORT_PAD_MMC_SP__MIN | PORT_PAD_MMC_SN__MIN | PORT_PAD_MMC_RXSEL__IN | PORT_PAD_MMC_TXPREP__MIN | PORT_PAD_MMC_TXPREN__MIN | PORT_PAD_MMC_PULL__UP ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
        }
    },
    {
        TP_LVDS3, TP_GOODIX_9XX, false, false, TI941_SINGLE, false, RES_I2C_I2C14, GT9XX_DEFAULT_SLAVE_ID, TS_MBOX_ADDR, DP_CA_AP1,
        DEV_TYPE_AUX, 0x75, TCA9539_P06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD6,
        {//pin130
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
#ifdef ENABLE_CONTROLPANEL
    {
        TP_LVDS4, TP_GOODIX_9XX, true, true, TI941_SINGLE, false, RES_I2C_I2C15, GT9XX_DEFAULT_SLAVE_ID, 0, 0,
        DEV_TYPE_MAIN, 0x75, TCA9539_P07, 0x1a, 0x2c, 3, 2, 1920, 720, 0, 0, 0, 0, PortConf_PIN_I2S_MC_SD7,
        {//pin133
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_DOWN ),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_GPIO),
        }
    },
#endif
};
#endif


static int tsc_num = ARRAY_SIZE(tsc);

bool target_config_ts_enabled(int instance)
{
    if (instance >= tsc_num)
        return ERR_NOT_VALID;

    return (tsc[instance].enabled == true);
}

struct ts_board_config *target_config_ts_acquire(int instance)
{
    if (instance >= tsc_num)
        return NULL;

    return &tsc[instance];
}

