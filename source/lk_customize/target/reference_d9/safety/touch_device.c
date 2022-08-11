/*
 * Copyright (C) Semidrive Semiconductor Ltd.
 * All rights reserved.
 */

#include <disp_hal.h>
#include "tca9539.h"
#include <touch_device.h>
#include "boardinfo_hwid_usr.h"

static struct ts_board_config tsc[] = {
#if SERDES_TP_X9H
#if !TOUCH_SERDES_DIVIDED
    {
        /*---DSI1---*/
        TS_ENABLE, "goodix", RES_I2C_I2C16, 0x5d,
        TS_SUPPORT_ANRDOID_MAIN, INFOTAINMENT,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x76, TCA9539_P01},
        {true, TI941_DUAL, 0x0c, 0x2c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_EMMC2_CLK, //irq-pin:144
            {0, 0}
        }
    },
    {
        /*---DSI2---*/
        TS_ENABLE, "goodix", RES_I2C_I2C16, 0x14,
        TS_SUPPORT_ANRDOID_AUX1, ENTERTAINMENT,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x76, TCA9539_P02},
        {true, TI941_DUAL, 0x0d, 0x3c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_EMMC2_CMD, //irq-pin:145
            {0, 0}
        }
    },
#ifdef ENABLE_CONTROLPANEL
    {
        /*---LVDS4---*/
        TS_ENABLE, "goodix", RES_I2C_I2C15, 0x5d,
        TS_SUPPORT_CTRLPANEL_MAIN, CONTROLPANEL,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x75, TCA9539_P07},
        {true, TI947_SINGLE, 0x1a, 0x2c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_I2S_MC_SD7, //irq-pin:131
            {0, 0}
        }
    },
    {
        /*---LVDS3---*/
        TS_DISABLE, "goodix", RES_I2C_I2C14, 0x5d,
        TS_SUPPORT_CTRLPANEL_AUX1, ENTERTAINMENT,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x75, TCA9539_P06},
        {false, TI947_SINGLE, 0x0, 0x0, 0, 0},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_I2S_MC_SD6, //irq-pin:130
            {0, 0}
        }
    },
#endif
#else
    {
        /*---DSI1---*/
        TS_ENABLE, "goodix", RES_I2C_I2C16, 0x5d,
        TS_SUPPORT_ANRDOID_MAIN | TS_SUPPORT_CTRLPANEL_MAIN, INFOTAINMENT,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x76, TCA9539_P01},
        {true, TI941_SINGLE, 0x0c, 0x2c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_EMMC2_CLK, //irq-pin:144
            {0, 0}
        }
    },
#endif
#elif SERDES_TP_X9M
    {
        /*---DSI1---*/
        TS_ENABLE, "goodix", RES_I2C_I2C16, 0x5d,
        TS_SUPPORT_ANRDOID_MAIN, INFOTAINMENT,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x76, TCA9539_P01},
        {true, TI941_SINGLE, 0x0c, 0x2c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_EMMC2_CLK, //irq-pin:144
            {0, 0}
        }
    },
#ifdef ENABLE_CONTROLPANEL
    {
        /*---LVDS4---*/
        TS_ENABLE, "goodix", RES_I2C_I2C15, 0x5d,
        TS_SUPPORT_CTRLPANEL_MAIN, CONTROLPANEL,
        {1920, 720, 10, 0, 0, 0},
        {false, 12, 0x75, TCA9539_P07},
        {true, TI947_SINGLE, 0x1a, 0x2c, 3, 2},
        {0}, //reset-pin:0 not used yet
        {
            PortConf_PIN_I2S_MC_SD7, //irq-pin:131
            {0, 0}
        }
    },
#endif
#endif
};

void get_touch_device(struct ts_board_config **_tsc, int *_tsc_num)
{
    *_tsc = tsc;
    *_tsc_num = sizeof(tsc) / sizeof(tsc[0]);
}

