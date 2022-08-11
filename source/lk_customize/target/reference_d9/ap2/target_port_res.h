#ifndef __TARGET_PORT_RES_H__
#define __TARGET_PORT_RES_H__
#include "hal_port.h"

#define MSHC_PORT_PIN_NUM_MAX 16

#define MSHC1_PORT_CFG_INDEX (0)
#define MSHC2_PORT_CFG_INDEX (1)
#define MSHC3_PORT_CFG_INDEX (2)
#define MSHC4_PORT_CFG_INDEX (3)

struct mshc_pin_cfg {
    int pin_num;
    Port_PinType pin_index[MSHC_PORT_PIN_NUM_MAX];
};

#define MSHC_PORT_CFG_DATA                                                     \
    {                                                                          \
        [MSHC1_PORT_CFG_INDEX] =                                               \
            {                                                                  \
                .pin_num = 12,                                                 \
                .pin_index =                                                   \
                    {                                                          \
                        PortConf_PIN_EMMC1_CLK,                                \
                        PortConf_PIN_EMMC1_CMD,                                \
                        PortConf_PIN_EMMC1_DATA0,                              \
                        PortConf_PIN_EMMC1_DATA1,                              \
                        PortConf_PIN_EMMC1_DATA2,                              \
                        PortConf_PIN_EMMC1_DATA3,                              \
                        PortConf_PIN_EMMC1_DATA4,                              \
                        PortConf_PIN_EMMC1_DATA5,                              \
                        PortConf_PIN_EMMC1_DATA6,                              \
                        PortConf_PIN_EMMC1_DATA7,                              \
                        PortConf_PIN_EMMC1_STROBE,                             \
                        PortConf_PIN_EMMC1_RESET_N,                            \
                    },                                                         \
            },                                                                 \
        [MSHC2_PORT_CFG_INDEX] =                                               \
            {                                                                  \
                .pin_num = 0,                                                  \
                .pin_index = {0},                                              \
            },                                                                 \
        [MSHC3_PORT_CFG_INDEX] =                                               \
            {                                                                  \
                .pin_num = 6,                                                  \
                .pin_index =                                                   \
                    {                                                          \
                        PortConf_PIN_EMMC2_DATA4,                              \
                        PortConf_PIN_EMMC2_DATA5,                              \
                        PortConf_PIN_EMMC2_DATA6,                              \
                        PortConf_PIN_EMMC2_DATA7,                              \
                        PortConf_PIN_EMMC2_STROBE,                             \
                        PortConf_PIN_EMMC2_RESET_N,                            \
                    },                                                         \
            },                                                                 \
        [MSHC4_PORT_CFG_INDEX] =                                               \
            {                                                                  \
                .pin_num = 0,                                                  \
                .pin_index = {0},                                              \
            },                                                                 \
    };

#endif /* ___TARGET_PORT_RES_H__ */
