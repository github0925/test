/*
 * board_cfg.h.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __CFG_H_
#define __CFG_H_
#include "hal_port.h"
#include "hal_dio.h"
#include "func_i2c.h"

#define debug_show_dg       0x0
#define debug_show_null     0x1

#define AP_DOMAIN           0x5
#define SAFETY_DOMAIN       0x1

#define COM_GPIO            0X0
#define IO_EXTEND           0x1

/*ADC paraments configure*/
#define ADC_CHN_TOTAL       0x8
#define READ_ADC_CHN_MAX    0x3

#define POWER_OFF           0x0
#define POWER_ON            0x1

#define OFF_LINE            0x0
#define ON_LINE             0x1

typedef enum {
    U3001 = 0x0,
    U3103 = 0x1,
    DEFAULT,
} I2C_DEV_E;

#define U3001_IO_EXTEND_PIN_NR_1      1
#define U3001_IO_EXTEND_PIN_NR_2      2
#define U3001_IO_EXTEND_PIN_NR_3      3
#define U3001_IO_EXTEND_PIN_NR_4      4
#define U3001_IO_EXTEND_PIN_NR_5      5
#define U3001_IO_EXTEND_PIN_NR_6      6
#define U3001_IO_EXTEND_PIN_NR_7      7
#define U3001_IO_EXTEND_PIN_NR_8      8
#define U3001_IO_EXTEND_PIN_NR_13     13
#define U3001_IO_EXTEND_PIN_NR_14     14
#define U3001_IO_EXTEND_PIN_NR_15     15
#define U3001_IO_EXTEND_PIN_NR_16     16

#define U3103_IO_EXTEND_PIN_NR_1      1
#define U3103_IO_EXTEND_PIN_NR_2      2
#define U3103_IO_EXTEND_PIN_NR_3      3
#define U3103_IO_EXTEND_PIN_NR_4      4
#define U3103_IO_EXTEND_PIN_NR_5      5
#define U3103_IO_EXTEND_PIN_NR_6      6
#define U3103_IO_EXTEND_PIN_NR_7      7
#define U3103_IO_EXTEND_PIN_NR_8      8
#define U3103_IO_EXTEND_PIN_NR_9      9
#define U3103_IO_EXTEND_PIN_NR_10     10
#define U3103_IO_EXTEND_PIN_NR_11     11
#define U3103_IO_EXTEND_PIN_NR_12     12
#define U3103_IO_EXTEND_PIN_NR_13     13
#define U3103_IO_EXTEND_PIN_NR_14     14
#define U3103_IO_EXTEND_PIN_NR_15     15
#define U3103_IO_EXTEND_PIN_NR_16     16

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

#define set_resp_err_state(x, y) (x=y)
#define set_para_value(x, y)     (x=y)

#define CONTEXT_LOCK(context) \
    spin_lock_saved_state_t __state; \
    spin_lock_irqsave(&context.lock, __state)

#define CONTEXT_UNLOCK(context) \
    spin_unlock_irqrestore(&context.lock, __state)

/*gpio read*/
#define GPIO_C2                  PortConf_PIN_GPIO_C2
#define GPIO_D11                 PortConf_PIN_GPIO_D11
#define GPIO_D6                  PortConf_PIN_GPIO_D6
#define GPIO_E0                  PortConf_PIN_RGMII1_TXC
#define OSPI1_SS1                PortConf_PIN_OSPI1_DATA4
#define GPIO_E9                  PortConf_PIN_RGMII1_RXD2
#define GPIO_E8                  PortConf_PIN_RGMII1_RXD1
#define PGOOD_0_8V               PortConf_PIN_GPIO_B5
#define PGOOD_3_3V               PortConf_PIN_OSPI1_SS1
#define PGOOD_PMIC               PortConf_PIN_GPIO_B5
#define PGOOD_1_8V               PortConf_PIN_OSPI1_DATA5
#define IOEX_CANFD17_INH_AP      U3001_IO_EXTEND_PIN_NR_14   //U3001   P15
#define IOEX_CANFD17_ERR_AP      U3001_IO_EXTEND_PIN_NR_13   //U3001   P14
#define CANFD18_INH_AP           PortConf_PIN_RGMII2_TXD3
#define CANFD18_ERR_AP           PortConf_PIN_RGMII2_RXD0
#define IOEX_CANFD19_INH_AP      U3001_IO_EXTEND_PIN_NR_1    //U3001
#define IOEX_CANFD19_ERR_AP      U3001_IO_EXTEND_PIN_NR_2    //U3001
#define IOEX_CANFD20_INH_AP      U3103_IO_EXTEND_PIN_NR_13   //U3103
#define IOEX_CANFD20_ERR_AP      U3103_IO_EXTEND_PIN_NR_14   //U3103
#define IOEX_CANFD3_INH_SAFETY   U3001_IO_EXTEND_PIN_NR_15   //U3001
#define IOEX_CANFD3_ERR_SAFETY   U3001_IO_EXTEND_PIN_NR_16   //U3001
#define IOEX_CANFD10_INH_AP      U3001_IO_EXTEND_PIN_NR_8    //U3001
#define IOEX_CANFD10_ERR_AP      U3001_IO_EXTEND_PIN_NR_7    //U3001
#define IOEX_CANFD11_INH_AP      U3001_IO_EXTEND_PIN_NR_5    //U3001
#define IOEX_CANFD11_ERR_AP      U3001_IO_EXTEND_PIN_NR_6    //U3001
#define IOEX_CANFD12_INH_AP      U3001_IO_EXTEND_PIN_NR_3    //U3001
#define IOEX_CANFD12_ERR_AP      U3001_IO_EXTEND_PIN_NR_4    //U3001
#define IOEX_CANFD13_INH_AP      U3103_IO_EXTEND_PIN_NR_1    //U3103
#define IOEX_CANFD13_ERR_AP      U3103_IO_EXTEND_PIN_NR_2    //U3103
#define IOEX_CANFD14_INH_AP      U3103_IO_EXTEND_PIN_NR_5    //U3103
#define IOEX_CANFD14_ERR_AP      U3103_IO_EXTEND_PIN_NR_6    //U3103
#define IOEX_CANFD15_INH_AP      U3103_IO_EXTEND_PIN_NR_9    //U3103
#define IOEX_CANFD15_ERR_AP      U3103_IO_EXTEND_PIN_NR_10   //U3103
#define CANFD16_INH_AP           PortConf_PIN_OSPI2_DATA2
#define CANFD16_ERR_AP           PortConf_PIN_EMMC2_STROBE
#define GPIO_C11                 PortConf_PIN_GPIO_C11       //59
#define GPIO_C9                  PortConf_PIN_GPIO_C9        //57
#define GPIO_C14                 PortConf_PIN_GPIO_C14       //62
#define GPIO_C15                 PortConf_PIN_GPIO_C15       //63

#define CANFD1_INH_SAFETY        PortConf_PIN_RGMII2_TXD2
#define CANFD1_ERR_SAFETY        PortConf_PIN_I2S_SC4_SCK
#define CANFD2_INH_SAFETY        PortConf_PIN_I2S_SC3_SD
#define CANFD2_ERR_SAFETY        PortConf_PIN_RGMII2_TXC
#define CANFD3_INH_SAFETY        U3001_IO_EXTEND_PIN_NR_15
#define CANFD3_ERR_SAFETY        U3001_IO_EXTEND_PIN_NR_16
#define CANFD4_INH_SAFETY        PortConf_PIN_OSPI2_DATA6
#define CANFD4_ERR_SAFETY        PortConf_PIN_OSPI2_SCLK
#define CANFD9_INH_AP            PortConf_PIN_RGMII2_RXD1
#define CANFD9_ERR_AP            PortConf_PIN_RGMII2_RX_CTL
#define CANFD10_INH_AP           U3001_IO_EXTEND_PIN_NR_8
#define CANFD10_ERR_AP           U3001_IO_EXTEND_PIN_NR_7
#define CANFD11_INH_AP           U3001_IO_EXTEND_PIN_NR_5
#define CANFD11_ERR_AP           U3001_IO_EXTEND_PIN_NR_6
#define CANFD12_INH_AP           U3001_IO_EXTEND_PIN_NR_3
#define CANFD12_ERR_AP           U3001_IO_EXTEND_PIN_NR_4
#define CANFD17_INH_AP           U3001_IO_EXTEND_PIN_NR_14
#define CANFD17_ERR_AP           U3001_IO_EXTEND_PIN_NR_13

/*gpio write*/
#define GPIOB_4                  PortConf_PIN_GPIO_B4
#define GPIOB_5                  PortConf_PIN_GPIO_B5
#define GPIOE_1                  PortConf_PIN_RGMII1_TXD0
#define GPIOE_3                  PortConf_PIN_RGMII1_TXD2
#define CANFD17_EN_AP            PortConf_PIN_EMMC2_DATA1
#define CANFD17_STDBY_AP         PortConf_PIN_EMMC2_DATA2
#define CANFD18_EN_AP            PortConf_PIN_RGMII2_RXC
#define CANFD18_STDBY_AP         PortConf_PIN_RGMII2_TX_CTL
#define CANFD19_EN_AP            PortConf_PIN_EMMC2_CMD
#define CANFD19_STDBY_AP         PortConf_PIN_EMMC2_CLK
#define IOEX_CANFD20_EN_AP       U3103_IO_EXTEND_PIN_NR_15   //U3103
#define IOEX_CANFD20_STDBY_AP    U3103_IO_EXTEND_PIN_NR_16   //U3103
#define CANFD9_EN_AP             PortConf_PIN_RGMII2_RXD3
#define CANFD9_STDBY_AP          PortConf_PIN_RGMII2_RXD2
#define CANFD10_EN_AP            PortConf_PIN_EMMC2_DATA0
#define CANFD10_STDBY_AP         PortConf_PIN_EMMC2_DATA3
#define CANFD11_EN_AP            PortConf_PIN_EMMC2_DATA4
#define CANFD11_STDBY_AP         PortConf_PIN_EMMC2_DATA7
#define CANFD12_EN_AP            PortConf_PIN_EMMC2_RESET_N
#define CANFD12_STDBY_AP         PortConf_PIN_EMMC2_DATA5
#define IOEX_CANFD13_EN_AP       U3103_IO_EXTEND_PIN_NR_3    //U3103
#define IOEX_CANFD13_STDBY_AP    U3103_IO_EXTEND_PIN_NR_4    //U3103
#define IOEX_CANFD14_EN_AP       U3103_IO_EXTEND_PIN_NR_7    //U3103
#define IOEX_CANFD14_STDBY_AP    U3103_IO_EXTEND_PIN_NR_8    //U3103
#define IOEX_CANFD15_EN_AP       U3103_IO_EXTEND_PIN_NR_11   //U3103
#define IOEX_CANFD15_STDBY_AP    U3103_IO_EXTEND_PIN_NR_12   //U3103
#define CANFD16_EN_AP            PortConf_PIN_OSPI2_DATA4
#define CANFD16_STDBY_AP         PortConf_PIN_EMMC2_DATA6
#define GPIO_D14                 PortConf_PIN_GPIO_D14
#define GPIO_D13                 PortConf_PIN_GPIO_D13
#define GPIO_C7                  PortConf_PIN_GPIO_C7
#define GPIO_C5                  PortConf_PIN_GPIO_C5
#define GPIO_D8                  PortConf_PIN_GPIO_D8
#define GPIO_D7                  PortConf_PIN_GPIO_D7
#define GPIO_C4                  PortConf_PIN_GPIO_C4
#define GPIO_D15                 PortConf_PIN_GPIO_D15
#define G9_5072_INH              PortConf_PIN_OSPI1_DATA6
#define G9_5050_INH              PortConf_PIN_OSPI1_DATA7
#define IOEX_2122WAKE_1          U3103_IO_EXTEND_PIN_NR_9   //U3001
#define IOEX_2122WAKE_2          U3103_IO_EXTEND_PIN_NR_10  //U3001
#define IOEX_2122WAKE_3          U3103_IO_EXTEND_PIN_NR_11  //U3001
#define IOEX_2122WAKE_4          U3103_IO_EXTEND_PIN_NR_12  //U3001

#define G9_2122_P10_INH_AP       PortConf_PIN_GPIO_D11
#define G9_ACTIVE_LINE_AP        PortConf_PIN_OSPI2_DATA5
#define G9X_GPIO_RES1_AP         PortConf_PIN_OSPI2_SS1
#define G9_2122_P7_INH_SAFETY    160
#define G9_2122_P9_INH_SAFETY    PortConf_PIN_RGMII1_TX_CTL

#define CANFD1_STDBY_SAFETY      PortConf_PIN_OSPI2_DATA1
#define CANFD1_EN_SAFETY         PortConf_PIN_I2S_SC3_SCK
#define CANFD2_STDBY_SAFETY      PortConf_PIN_OSPI2_SS0
#define CANFD2_EN_SAFETY         PortConf_PIN_OSPI2_DQS
#define CANFD3_STDBY_SAFETY      PortConf_PIN_OSPI2_DATA3
#define CANFD3_EN_SAFETY         PortConf_PIN_I2S_SC3_WS
#define CANFD4_STDBY_SAFETY      PortConf_PIN_OSPI2_DATA7
#define CANFD4_EN_SAFETY         PortConf_PIN_OSPI2_DATA0
#define G9_WDT1_REST_AP          PortConf_PIN_GPIO_B7
#define G9_WDT3_REST_AP          PortConf_PIN_GPIO_D10
#define G9_SAFETY_SEST_AP        PortConf_PIN_GPIO_B5

#define READ_OPS  0
#define WRITE_OPS 1
#define CHECK_OPS 2

typedef enum {
    ADC_CHN_0 = 0,
    ADC_CHN_1 = 1,
    ADC_CHN_2 = 2,
    ADC_CHN_3 = 3,
    ADC_CHN_4 = 4,
    ADC_CHN_5 = 5,
    ADC_PMIC_CHN = 6,
    ADC_CHN_7 = 7,
} BOARD_ADC_CHN;

typedef enum {
    CAN_CHN_1 = 1,
    CAN_CHN_2 = 2,
    CAN_CHN_3 = 3,
    CAN_CHN_4 = 4,
    CAN_CHN_9 = 9,
    CAN_CHN_10 = 10,
    CAN_CHN_11 = 11,
    CAN_CHN_12 = 12,
    CAN_CHN_13 = 13,
    CAN_CHN_14 = 14,
    CAN_CHN_15 = 15,
    CAN_CHN_16 = 16,
    CAN_CHN_17 = 17,
    CAN_CHN_18 = 18,
    CAN_CHN_19 = 19,
    CAN_CHN_20 = 20
} BOARD_CAN_CHN;

typedef enum {
    LIN_CHN_0 = 0,
    LIN_CHN_1 = 1,
    LIN_CHN_2 = 2,
    LIN_CHN_3 = 3,
} BOARD_LIN_CHN;

typedef enum {
    I2C_CHN_5 = 5,
    I2C_CHN_4 = 4,
    I2C_CHN_9 = 9,
    I2C_CHN_3 = 3,
} BOARD_I2C_CHN;

typedef enum {
    CPT_CHN_0 = 0,
    CPT_CHN_1 = 1,
    CPT_CHN_2 = 2,
    CPT_CHN_3 = 3,
} BOARD_CPT_CHN;

typedef enum {
    LOW  = 0,
    HIGH = 1
} BOARD_LEVEl;

typedef enum {
    BASE_100  = 0,
    BASE_1000 = 1
} ETH_RATE_TYPE;

typedef struct {
    uint8_t channel;
    uint8_t gpio_Pin;
    uint8_t domain;
    uint8_t gpio_type;
    I2C_DEV_E i2c_dev;
} gpio_value_table_t;

typedef struct {
    u_int8_t chn_num;
    u_int8_t port_num;
} eth_chn_table_t;

typedef struct {
    uint8_t pin_num;
    uint8_t channel;
} com_chn_table_t;

typedef struct {
    uint8_t  channel;
    uint32_t Source_id;
    uint32_t target_id;
} can_chn_table_t;

typedef struct {
    uint8_t  channel;
    uint32_t Source_id;
    uint32_t target_id;
} lin_chn_table_t;

#endif
