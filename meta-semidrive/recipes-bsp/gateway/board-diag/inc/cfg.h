/*
 * cfg.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __CFG_H_
#define __CFG_H_
#include <sys/types.h>

typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
typedef u_int16_t Port_PinType;
/*
   Port0
*/

#define PortConf_PIN_OSPI1_SCLK  (Port_PinType )(0)
#define PortConf_PIN_OSPI1_SS0  (Port_PinType )(1)
#define PortConf_PIN_OSPI1_DATA0  (Port_PinType )(2)
#define PortConf_PIN_OSPI1_DATA1  (Port_PinType )(3)
#define PortConf_PIN_OSPI1_DATA2  (Port_PinType )(4)
#define PortConf_PIN_OSPI1_DATA3  (Port_PinType )(5)
#define PortConf_PIN_OSPI1_DATA4  (Port_PinType )(6)
#define PortConf_PIN_OSPI1_DATA5  (Port_PinType )(7)
#define PortConf_PIN_OSPI1_DATA6  (Port_PinType )(8)
#define PortConf_PIN_OSPI1_DATA7  (Port_PinType )(9)
#define PortConf_PIN_OSPI1_DQS  (Port_PinType )(10)
#define PortConf_PIN_OSPI1_SS1  (Port_PinType )(11)
#define PortConf_PIN_RGMII1_TXC  (Port_PinType )(12)
#define PortConf_PIN_RGMII1_TXD0  (Port_PinType )(13)
#define PortConf_PIN_RGMII1_TXD1  (Port_PinType )(14)
#define PortConf_PIN_RGMII1_TXD2  (Port_PinType )(15)
#define PortConf_PIN_RGMII1_TXD3  (Port_PinType )(16)
#define PortConf_PIN_RGMII1_TX_CTL  (Port_PinType )(17)
#define PortConf_PIN_RGMII1_RXC  (Port_PinType )(18)
#define PortConf_PIN_RGMII1_RXD0  (Port_PinType )(19)
#define PortConf_PIN_RGMII1_RXD1  (Port_PinType )(20)
#define PortConf_PIN_RGMII1_RXD2  (Port_PinType )(21)
#define PortConf_PIN_RGMII1_RXD3  (Port_PinType )(22)
#define PortConf_PIN_RGMII1_RX_CTL  (Port_PinType )(23)
#define PortConf_PIN_GPIO_A0  (Port_PinType )(24)
#define PortConf_PIN_GPIO_A1  (Port_PinType )(25)
#define PortConf_PIN_GPIO_A2  (Port_PinType )(26)
#define PortConf_PIN_GPIO_A3  (Port_PinType )(27)
#define PortConf_PIN_GPIO_A4  (Port_PinType )(28)
#define PortConf_PIN_GPIO_A5  (Port_PinType )(29)
#define PortConf_PIN_GPIO_A6  (Port_PinType )(30)
#define PortConf_PIN_GPIO_A7  (Port_PinType )(31)

/*
   Port1
*/

#define PortConf_PIN_GPIO_A8  (Port_PinType )(32)
#define PortConf_PIN_GPIO_A9  (Port_PinType )(33)
#define PortConf_PIN_GPIO_A10  (Port_PinType )(34)
#define PortConf_PIN_GPIO_A11  (Port_PinType )(35)
#define PortConf_PIN_GPIO_B0  (Port_PinType )(36)
#define PortConf_PIN_GPIO_B1  (Port_PinType )(37)
#define PortConf_PIN_GPIO_B2  (Port_PinType )(38)
#define PortConf_PIN_GPIO_B3  (Port_PinType )(39)
#define PortConf_PIN_GPIO_B4  (Port_PinType )(40)
#define PortConf_PIN_GPIO_B5  (Port_PinType )(41)
#define PortConf_PIN_GPIO_B6  (Port_PinType )(42)
#define PortConf_PIN_GPIO_B7  (Port_PinType )(43)
#define PortConf_PIN_GPIO_B8  (Port_PinType )(44)
#define PortConf_PIN_GPIO_B9  (Port_PinType )(45)
#define PortConf_PIN_GPIO_B10  (Port_PinType )(46)
#define PortConf_PIN_GPIO_B11  (Port_PinType )(47)
#define PortConf_PIN_GPIO_C0  (Port_PinType )(48)
#define PortConf_PIN_GPIO_C1  (Port_PinType )(49)
#define PortConf_PIN_GPIO_C2  (Port_PinType )(50)
#define PortConf_PIN_GPIO_C3  (Port_PinType )(51)
#define PortConf_PIN_GPIO_C4  (Port_PinType )(52)
#define PortConf_PIN_GPIO_C5  (Port_PinType )(53)
#define PortConf_PIN_GPIO_C6  (Port_PinType )(54)
#define PortConf_PIN_GPIO_C7  (Port_PinType )(55)
#define PortConf_PIN_GPIO_C8  (Port_PinType )(56)
#define PortConf_PIN_GPIO_C9  (Port_PinType )(57)
#define PortConf_PIN_GPIO_C10  (Port_PinType )(58)
#define PortConf_PIN_GPIO_C11  (Port_PinType )(59)
#define PortConf_PIN_GPIO_C12  (Port_PinType )(60)
#define PortConf_PIN_GPIO_C13  (Port_PinType )(61)
#define PortConf_PIN_GPIO_C14  (Port_PinType )(62)
#define PortConf_PIN_GPIO_C15  (Port_PinType )(63)


/*
   Port2
*/

#define PortConf_PIN_GPIO_D0  (Port_PinType )(64)
#define PortConf_PIN_GPIO_D1  (Port_PinType )(65)
#define PortConf_PIN_GPIO_D2  (Port_PinType )(66)
#define PortConf_PIN_GPIO_D3  (Port_PinType )(67)
#define PortConf_PIN_GPIO_D4  (Port_PinType )(68)
#define PortConf_PIN_GPIO_D5  (Port_PinType )(69)
#define PortConf_PIN_GPIO_D6  (Port_PinType )(70)
#define PortConf_PIN_GPIO_D7  (Port_PinType )(71)
#define PortConf_PIN_GPIO_D8  (Port_PinType )(72)
#define PortConf_PIN_GPIO_D9  (Port_PinType )(73)
#define PortConf_PIN_GPIO_D10  (Port_PinType )(74)
#define PortConf_PIN_GPIO_D11  (Port_PinType )(75)
#define PortConf_PIN_GPIO_D12  (Port_PinType )(76)
#define PortConf_PIN_GPIO_D13  (Port_PinType )(77)
#define PortConf_PIN_GPIO_D14  (Port_PinType )(78)
#define PortConf_PIN_GPIO_D15  (Port_PinType )(79)
#define PortConf_PIN_OSPI2_SCLK  (Port_PinType )(80)
#define PortConf_PIN_OSPI2_SS0  (Port_PinType )(81)
#define PortConf_PIN_OSPI2_DATA0  (Port_PinType )(82)
#define PortConf_PIN_OSPI2_DATA1  (Port_PinType )(83)
#define PortConf_PIN_OSPI2_DATA2  (Port_PinType )(84)
#define PortConf_PIN_OSPI2_DATA3  (Port_PinType )(85)
#define PortConf_PIN_OSPI2_DATA4  (Port_PinType )(86)
#define PortConf_PIN_OSPI2_DATA5  (Port_PinType )(87)
#define PortConf_PIN_OSPI2_DATA6  (Port_PinType )(88)
#define PortConf_PIN_OSPI2_DATA7  (Port_PinType )(89)
#define PortConf_PIN_OSPI2_DQS  (Port_PinType )(90)
#define PortConf_PIN_OSPI2_SS1  (Port_PinType )(91)
#define PortConf_PIN_RGMII2_TXC  (Port_PinType )(92)
#define PortConf_PIN_RGMII2_TXD0  (Port_PinType )(93)
#define PortConf_PIN_RGMII2_TXD1  (Port_PinType )(94)
#define PortConf_PIN_RGMII2_TXD2  (Port_PinType )(95)


/*
   Port3
*/

#define PortConf_PIN_RGMII2_TXD3  (Port_PinType )(96)
#define PortConf_PIN_RGMII2_TX_CTL  (Port_PinType )(97)
#define PortConf_PIN_RGMII2_RXC  (Port_PinType )(98)
#define PortConf_PIN_RGMII2_RXD0  (Port_PinType )(99)
#define PortConf_PIN_RGMII2_RXD1  (Port_PinType )(100)
#define PortConf_PIN_RGMII2_RXD2  (Port_PinType )(101)
#define PortConf_PIN_RGMII2_RXD3  (Port_PinType )(102)
#define PortConf_PIN_RGMII2_RX_CTL  (Port_PinType )(103)
#define PortConf_PIN_I2S_SC3_SCK  (Port_PinType )(104)
#define PortConf_PIN_I2S_SC3_WS  (Port_PinType )(105)
#define PortConf_PIN_I2S_SC3_SD  (Port_PinType )(106)
#define PortConf_PIN_I2S_SC4_SCK  (Port_PinType )(107)
#define PortConf_PIN_I2S_SC4_WS  (Port_PinType )(108)
#define PortConf_PIN_I2S_SC4_SD  (Port_PinType )(109)
#define PortConf_PIN_I2S_SC5_SCK  (Port_PinType )(110)
#define PortConf_PIN_I2S_SC5_WS  (Port_PinType )(111)
#define PortConf_PIN_I2S_SC5_SD  (Port_PinType )(112)
#define PortConf_PIN_I2S_SC6_SCK  (Port_PinType )(113)
#define PortConf_PIN_I2S_SC6_WS  (Port_PinType )(114)
#define PortConf_PIN_I2S_SC6_SD  (Port_PinType )(115)
#define PortConf_PIN_I2S_SC7_SCK  (Port_PinType )(116)
#define PortConf_PIN_I2S_SC7_WS  (Port_PinType )(117)
#define PortConf_PIN_I2S_SC7_SD  (Port_PinType )(118)
#define PortConf_PIN_I2S_SC8_SCK  (Port_PinType )(119)
#define PortConf_PIN_I2S_SC8_WS  (Port_PinType )(120)
#define PortConf_PIN_I2S_SC8_SD  (Port_PinType )(121)
#define PortConf_PIN_I2S_MC_SCK  (Port_PinType )(122)
#define PortConf_PIN_I2S_MC_WS  (Port_PinType )(123)
#define PortConf_PIN_I2S_MC_SD0  (Port_PinType )(124)
#define PortConf_PIN_I2S_MC_SD1  (Port_PinType )(125)
#define PortConf_PIN_I2S_MC_SD2  (Port_PinType )(126)
#define PortConf_PIN_I2S_MC_SD3  (Port_PinType )(127)


/*
   Port4
*/

#define PortConf_PIN_I2S_MC_SD4  (Port_PinType )(128)
#define PortConf_PIN_I2S_MC_SD5  (Port_PinType )(129)
#define PortConf_PIN_I2S_MC_SD6  (Port_PinType )(130)
#define PortConf_PIN_I2S_MC_SD7  (Port_PinType )(131)
#define PortConf_PIN_EMMC1_CLK  (Port_PinType )(132)
#define PortConf_PIN_EMMC1_CMD  (Port_PinType )(133)
#define PortConf_PIN_EMMC1_DATA0  (Port_PinType )(134)
#define PortConf_PIN_EMMC1_DATA1  (Port_PinType )(135)
#define PortConf_PIN_EMMC1_DATA2  (Port_PinType )(136)
#define PortConf_PIN_EMMC1_DATA3  (Port_PinType )(137)
#define PortConf_PIN_EMMC1_DATA4  (Port_PinType )(138)
#define PortConf_PIN_EMMC1_DATA5  (Port_PinType )(139)
#define PortConf_PIN_EMMC1_DATA6  (Port_PinType )(140)
#define PortConf_PIN_EMMC1_DATA7  (Port_PinType )(141)
#define PortConf_PIN_EMMC1_STROBE  (Port_PinType )(142)
#define PortConf_PIN_EMMC1_RESET_N  (Port_PinType )(143)
#define PortConf_PIN_EMMC2_CLK  (Port_PinType )(144)
#define PortConf_PIN_EMMC2_CMD  (Port_PinType )(145)
#define PortConf_PIN_EMMC2_DATA0  (Port_PinType )(146)
#define PortConf_PIN_EMMC2_DATA1  (Port_PinType )(147)
#define PortConf_PIN_EMMC2_DATA2  (Port_PinType )(148)
#define PortConf_PIN_EMMC2_DATA3  (Port_PinType )(149)
#define PortConf_PIN_EMMC2_DATA4  (Port_PinType )(150)
#define PortConf_PIN_EMMC2_DATA5  (Port_PinType )(151)
#define PortConf_PIN_EMMC2_DATA6  (Port_PinType )(152)
#define PortConf_PIN_EMMC2_DATA7  (Port_PinType )(153)
#define PortConf_PIN_EMMC2_STROBE  (Port_PinType )(154)
#define PortConf_PIN_EMMC2_RESET_N  (Port_PinType )(155)
#define DIO_STANDBY      0XFF
/*gpio read*/
#define GPIO_C2             PortConf_PIN_GPIO_C2
#define GPIO_D11            PortConf_PIN_GPIO_D11
#define GPIO_D6             PortConf_PIN_GPIO_D6
#define GPIO_E0             PortConf_PIN_RGMII1_TXC
#define OSPI1_SS1           PortConf_PIN_OSPI1_SS1
#define GPIO_E9             PortConf_PIN_RGMII1_RXD2
#define GPIO_E8             PortConf_PIN_RGMII1_RXD1
#define PGOOD_3_3V          PortConf_PIN_OSPI1_DATA3
#define PGOOD_1_8V          PortConf_PIN_OSPI1_DATA5
#define CANFD17_INH_AP      PortConf_PIN_EMMC2_DATA1
#define CANFD17_ERR_AP      PortConf_PIN_EMMC2_DATA2
#define CANFD18_INH_AP      PortConf_PIN_RGMII2_TXD3
#define CANFD18_ERR_AP      PortConf_PIN_RGMII2_RXD0
#define CANFD19_INH_AP      PortConf_PIN_RGMII2_RXD1
#define CANFD19_ERR_AP      PortConf_PIN_RGMII2_RX_CTL
#define CANFD20_INH_AP      PortConf_PIN_OSPI2_DATA2
#define CANFD20_ERR_AP      PortConf_PIN_EMMC2_STROBE
#define CANFD13_INH_AP      PortConf_PIN_RGMII2_TXD2
#define CANFD13_ERR_AP      PortConf_PIN_I2S_SC4_SCK
#define CANFD14_INH_AP      PortConf_PIN_I2S_SC3_SD
#define CANFD14_ERR_AP      PortConf_PIN_RGMII2_TXC
#define CANFD15_INH_AP      PortConf_PIN_OSPI2_SS1      //91
#define CANFD15_ERR_AP      PortConf_PIN_OSPI2_DATA5    //87
#define CANFD16_INH_AP      PortConf_PIN_OSPI2_DATA6    //88
#define CANFD16_ERR_AP      PortConf_PIN_OSPI2_SCLK     //80
#define GPIO_C11            PortConf_PIN_GPIO_C11       //59
#define GPIO_C9             PortConf_PIN_GPIO_C9        //57
#define GPIO_C14            PortConf_PIN_GPIO_C14       //62
#define GPIO_C15            PortConf_PIN_GPIO_C15       //63
#define SYS_WAKEUP0         DIO_STANDBY

/*gpio write*/
#define GPIOB4              PortConf_PIN_GPIO_B4
#define GPIOB5              PortConf_PIN_GPIO_B5
#define GPIOE1              PortConf_PIN_RGMII1_TXD0
#define GPIOE3              PortConf_PIN_RGMII1_TXD2
#define CANFD18_EN_AP       PortConf_PIN_RGMII2_RXC
#define CANFD18_STDBY_AP    PortConf_PIN_RGMII2_TX_CTL
#define CANFD19_EN_AP       PortConf_PIN_RGMII2_RXD3
#define CANFD19_STDBY_AP    PortConf_PIN_RGMII2_RXD2
#define CANFD20_EN_AP       PortConf_PIN_OSPI2_DATA4
#define CANFD20_STDBY_AP    PortConf_PIN_EMMC2_DATA6
#define CANFD9_EN_AP        PortConf_PIN_EMMC2_CMD
#define CANFD9_STDBY_AP     PortConf_PIN_EMMC2_CLK
#define CANFD10_EN_AP       PortConf_PIN_EMMC2_DATA0
#define CANFD10_STDBY_AP    PortConf_PIN_EMMC2_DATA3
#define CANFD11_EN_AP       PortConf_PIN_EMMC2_DATA4
#define CANFD11_STDBY_AP    PortConf_PIN_EMMC2_DATA7
#define CANFD12_EN_AP       PortConf_PIN_EMMC2_RESET_N
#define CANFD12_STDBY_AP    PortConf_PIN_EMMC2_DATA5
#define CANFD13_EN_AP       PortConf_PIN_I2S_SC3_SCK
#define CANFD13_STDBY_AP    PortConf_PIN_OSPI2_DATA1
#define CANFD14_EN_AP       PortConf_PIN_OSPI2_DQS
#define CANFD14_STDBY_AP    PortConf_PIN_OSPI2_SS0
#define CANFD15_EN_AP       PortConf_PIN_I2S_SC3_WS
#define CANFD15_STDBY_AP    PortConf_PIN_OSPI2_DATA3
#define CANFD16_EN_AP       PortConf_PIN_OSPI2_DATA0
#define CANFD16_STDBY_AP    PortConf_PIN_OSPI2_DATA7
#define GPIO_D14            PortConf_PIN_GPIO_D14
#define GPIO_D13            PortConf_PIN_GPIO_D13
#define GPIO_C7             PortConf_PIN_GPIO_C7
#define GPIO_C5             PortConf_PIN_GPIO_C5
#define GPIO_D8             PortConf_PIN_GPIO_D8
#define GPIO_D7             PortConf_PIN_GPIO_D7
#define GPIO_C4             PortConf_PIN_GPIO_C4
#define GPIO_D15            PortConf_PIN_GPIO_D15

#define CANFD17_EN_AP       PortConf_PIN_EMMC2_DATA1
#define CANFD17_STDBY_AP    PortConf_PIN_EMMC2_DATA2

#define G9_2122_P10_INH_AP  PortConf_PIN_GPIO_D11
#define G9_ACTIVE_LINE_AP   PortConf_PIN_OSPI2_DATA5
#define G9X_GPIO_RES1_AP    PortConf_PIN_OSPI2_SS1
#define G9_2122_P7_INH_SAFETY  160
#define G9_2122_P9_INH_SAFETY  PortConf_PIN_RGMII1_TX_CTL


/*ADC paraments configure*/
#define ADC_CHN_TOTAL     8
#define READ_ADC_CHN_MAX  3

#define CAN_CHN_TOTAL       16
#define LIN_CHN_TOTAL       4
#define CAPT_CHN_TOTAL      4
#define LIN_CHN_TOTAL       4
#define I2C_CHN_TOTAL       4

#define PIN_NUM_INVALID     (0xff)

#define POWER_OFF     0
#define POWER_ON      1

#define ETH_INVALID_PORT_NUM  0xff
#define ETH_INVALID_CHN_NUM  0xff

#define ETH_INVALID_TST_MODE_IDX    0xff
#define ETH_INVALID_TST_MODE_TYPE   0xff


#define ETH_PORT_NORMAL_MODE_BIT  (0x1u << NORMAL_MODE)
#define ETH_PORT_TST_MODE1_BIT    (0x1u << TEST_MODE_1)
#define ETH_PORT_TST_MODE2_BIT    (0x1u << TEST_MODE_2)
#define ETH_PORT_TST_MODE3_BIT    (0x1u << TEST_MODE_3)
#define ETH_PORT_TST_MODE4_BIT    (0x1u << TEST_MODE_4)
#define ETH_PORT_TST_MODE5_BIT    (0x1u << TEST_MODE_5)
#define ETH_PORT_TST_MODE6_BIT    (0x1u << TEST_MODE_6)
#define ETH_PORT_TST_MODE7_BIT    (0x1u << TEST_MODE_7)

#define ETH_BASE100_TST_MODE      ETH_PORT_NORMAL_MODE_BIT \
                                  | ETH_PORT_TST_MODE1_BIT \
                                  | ETH_PORT_TST_MODE2_BIT \
                                  | ETH_PORT_TST_MODE3_BIT \
                                  | ETH_PORT_TST_MODE4_BIT \
                                  | ETH_PORT_TST_MODE5_BIT \

#define ETH_BASE1000_TST_MODE     ETH_PORT_NORMAL_MODE_BIT \
                                  | ETH_PORT_TST_MODE1_BIT \
                                  | ETH_PORT_TST_MODE2_BIT \
                                  | ETH_PORT_TST_MODE3_BIT \
                                  | ETH_PORT_TST_MODE4_BIT \
                                  | ETH_PORT_TST_MODE5_BIT \
                                  | ETH_PORT_TST_MODE6_BIT \
                                  | ETH_PORT_TST_MODE7_BIT \

typedef enum {
    NORMAL_MODE = 0X0,
    TEST_MODE_1 = 0X1,
    TEST_MODE_2 = 0X2,
    TEST_MODE_3 = 0X3,
    TEST_MODE_4 = 0X4,
    TEST_MODE_5 = 0X5,
    TEST_MODE_6 = 0X6,
    TEST_MODE_7 = 0X7,
} ETH_TEST_MODE_CMD;

typedef struct {
    uint8_t chn_num;
    uint8_t port_num;
    uint16_t mode;
} eth_chn_table_t;

typedef struct {
    uint8_t mode;
    uint8_t type;
} eth_test_mode_t;

typedef struct {
    uint8_t pin_num;
    uint8_t gpio_Pin;
} gpio_value_table_t;

extern const gpio_value_table_t gpio_write_value_table[];
extern const gpio_value_table_t gpio_read_value_table[];
extern const eth_chn_table_t eth_100_base_t1[];
extern const eth_chn_table_t eth_100_base_tx[];
extern const eth_chn_table_t eth_1000_base_t1[];
extern const eth_test_mode_t eth_test_mode[];
extern const  gpio_value_table_t eth_int_table[];
#endif
