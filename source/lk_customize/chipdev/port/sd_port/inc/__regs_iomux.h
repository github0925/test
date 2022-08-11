#ifndef __REGS_IOMUX_H
#define __REGS_IOMUX_H

#if 0
#ifndef APB_IOMUXC_RTC_BASE
#define APB_IOMUXC_RTC_BASE (0xf1860000u)
#endif
#ifndef APB_IOMUXC_SAF_BASE
#define APB_IOMUXC_SAF_BASE (0xfc500000u)
#endif
#ifndef APB_IOMUXC_SEC_BASE
#define APB_IOMUXC_SEC_BASE (0xf8500000u)
#endif
#endif

#define IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR (APB_IOMUXC_RTC_BASE)
#define IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR (APB_IOMUXC_SAF_BASE)
#define IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR (APB_IOMUXC_SEC_BASE)

/*
 * IO_PAD_CONFIG register exclude EMMC
 */
// POE - Parametric Output Enable,
#define IO_PAD_CONFIG__POE_FIELD_OFFSET 16
#define IO_PAD_CONFIG__POE_FIELD_SIZE 1
// IS - Input select, 0: input, 1: output
#define IO_PAD_CONFIG__IS_FIELD_OFFSET 12
#define IO_PAD_CONFIG__IS_FIELD_SIZE 1
// SR - Slew rate, 1: slow slew rate, 0: fast
#define IO_PAD_CONFIG__SR_FIELD_OFFSET 8
#define IO_PAD_CONFIG__SR_FIELD_SIZE 1
// DS - Driver selsect 0/1
#define IO_PAD_CONFIG__DS_FIELD_OFFSET 4
#define IO_PAD_CONFIG__DS_FIELD_SIZE 2
// PS - Pull select, 1: Pull up, 0: pull down
#define IO_PAD_CONFIG__PS_FIELD_OFFSET 1
#define IO_PAD_CONFIG__PS_FIELD_SIZE 1
// PE - Pull enable
#define IO_PAD_CONFIG__PE_FIELD_OFFSET 0
#define IO_PAD_CONFIG__PE_FIELD_SIZE 1

/*
 * IO_PAD_CONFIG register define for EMMC pins
 *
 */
// SP - PAD Drive Strenth for P
#define IO_PAD_CONFIG_EMMC__SP_FIELD_OFFSET 20
#define IO_PAD_CONFIG_EMMC__SP_FIELD_SIZE 4
// SP - PAD Drive Strenth for N
#define IO_PAD_CONFIG_EMMC__SN_FIELD_OFFSET 16
#define IO_PAD_CONFIG_EMMC__SN_FIELD_SIZE 4
// RXSEL - PAD input select
#define IO_PAD_CONFIG_EMMC__RXSEL_FIELD_OFFSET 12
#define IO_PAD_CONFIG_EMMC__RXSEL_FIELD_SIZE 3
// TXPREP - PAD Slew-Rate control for P
#define IO_PAD_CONFIG_EMMC__TXPREP_FIELD_OFFSET 8
#define IO_PAD_CONFIG_EMMC__TXPREP_FIELD_SIZE 4
// TXPREN - PAD Slew-Rate control for N
#define IO_PAD_CONFIG_EMMC__TXPREN_FIELD_OFFSET 4
#define IO_PAD_CONFIG_EMMC__TXPREN_FIELD_SIZE 4
// PULL-EN, 0: OFF, 1:PU, 2:PD, 3:RESERVED
#define IO_PAD_CONFIG_EMMC__PULL_EN_FIELD_OFFSET 0
#define IO_PAD_CONFIG_EMMC__PULL_EN_FIELD_SIZE 2

/*
 * PIN_MUX_CONFIG registers defination for all
 */
// FV - Pad input value when FIN=2
#define PIN_MUX_CONFIG__FV_FIELD_OFFSET 12
#define PIN_MUX_CONFIG__FV_FIELD_SIZE 1
// FIN - Force input ON/OFF
#define PIN_MUX_CONFIG__FIN_FIELD_OFFSET 8
#define PIN_MUX_CONFIG__FIN_FIELD_SIZE 2
// ODE - Open-drain enable
#define PIN_MUX_CONFIG__ODE_FIELD_OFFSET 4
#define PIN_MUX_CONFIG__ODE_FIELD_SIZE 1
// MUX_MODE - Mux mode bits to select IP function
#define PIN_MUX_CONFIG__MUX_FIELD_OFFSET 0
#define PIN_MUX_CONFIG__MUX_FIELD_SIZE 3

/*
 * INPUT_SOURCE_SELECT registers defination
 */
// SEL: <alt_func_name> inut pad selection
#define INPUT_SOURCE_SELECT__SEL_FIELD_OFFSET 0
//#define INPUT_SOURCE_SELECT__SEL_FIELD_SIZE (X-1:0)

/*
 * RTC & NON-GPIO PINS
 */

// 0  -   SYS_MODE0
// 1  -   SYS_MODE1
// 2  -   SYS_RESET_N
// 3  -   AP_RESET_N
// 4  -   RTC_RESET_N
// 5  -   SYS_PWR_ON

// 6  -   SYS_WAKEUP0
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x18<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x214<<0))
//MUX0:
//MUX1: PMU_WAKEUP0
//MUX2: TM_TAMPER4
//MUX3: CANFD1_RX_1  // SEL: 0
#define REG_AP_APB_IOMUX_CTRL_SAFETY_INPUT_SOURCE_SELECT_CANFD1_RX (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x400<<10))
#define INPUT_SOURCE_SELECT_CANFD1_RX_SRC_SEL_FIELD_OFFSET 0
#define INPUT_SOURCE_SELECT_CANFD1_RX_SRC_SEL_FIELD_SIZE 2
//MUX4:
//MUX5:
//MUX6:
//MUX7:

// 7  -   SYS_WAKEUP1
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x1c<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x218<<0))
//MUX0:
//MUX1: PMU_WAKEUP1
//MUX2: TM_TAMPER5
//MUX3: CANFD2_RX_1   // SEL: 0
#define REG_AP_APB_IOMUX_CTRL_SAFETY_INPUT_SOURCE_SELECT_CANFD2_RX (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x404<<10))
#define INPUT_SOURCE_SELECT_CANFD2_RX_SRC_SEL_FIELD_OFFSET 0
#define INPUT_SOURCE_SELECT_CANFD2_RX_SRC_SEL_FIELD_SIZE 2
//MUX4: PMU_EXT_RESET_REQ0
//MUX5:
//MUX6:
//MUX7:

// 8  -   SYS_CTRL0
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_CTRL0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x20<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_CTRL0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x21c<<0))
//MUX0:
//MUX1: PMU_PWR_CTRL0
//MUX2: TM_TAMPER0
//MUX3: CANFD1_TX
//MUX4:
//MUX5:
//MUX6:
//MUX7:

// 9  -   SYS_CTRL1
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_CTRL1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x24<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_CTRL1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x220<<0))
//MUX0:
//MUX1: PMU_PWR_CTRL1
//MUX2: TM_TAMPER1
//MUX3: CANFD2_TX
//MUX4:
//MUX5:
//MUX6:
//MUX7:

// 10 -   SYS_CTRL2
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_CTRL2 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x28<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_CTRL2 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x224<<0))
//MUX0:
//MUX1: PMU_PWR_CTRL2
//MUX2: TM_TAMPER2
//MUX3:
//MUX4:
//MUX5:
//MUX6:
//MUX7:

// 11 -   SYS_CTRL3
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_CTRL3 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x2c<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_CTRL3 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x228<<0))
//MUX0:
//MUX1: PMU_PWR_CTRL3
//MUX2: TM_TAMPER3
//MUX3: XTAL_RTC_CLK_OUT
//MUX4: PMU_EXT_RESET_REQ1
//MUX5:
//MUX6:
//MUX7:

// 12 -   JTAG_TMS
// 13 -   JTAG_TCK
// 14 -   JTAG_TDI
// 15 -   JTAG_TDO
// 16 -   JTAG_TRST_N

#define PORT_PIN_CNT 156
#define GPIO_SAF_PAD_OFF (0xD8)

typedef struct {
    uint16_t   mux_val;
    uint16_t   input_source_sel_reg_offset;
    uint16_t   sel_size;
    uint16_t   sel_val;
} SEMIDRIVE_X9_PIN_ALT_FUNC;

typedef struct {
    uint16_t  io_pad_config_reg_offset;
    uint16_t  pin_mux_config_reg_offset;
    SEMIDRIVE_X9_PIN_ALT_FUNC alt_funcs[8];
} SEMIDRIVE_X9_PIN_DEF;

SEMIDRIVE_X9_PIN_DEF x9_pins[PORT_PIN_CNT] = {
    /* PORT 0 */
    {
        // 0   -   OSPI1_SCLK
        0x14,  // pad_config, (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x14<<10))
        0x200,  // pin_mux_config, (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x200<<10))
        {
            {0, 0, 0, 0},       //MUX0: GPIO_MUX1_IO24
            {1, 0, 0, 0},       //MUX1: OSPI1_SCLK
            {2, 0x408, 1, 0},   //MUX2: SPI4_SCLK_1 //  SEL: 0, (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x408<<10))
            {3, 0, 0, 0},       //MUX3: UART5_CTS
            {4, 0, 0, 0},       //MUX4: CANFD1_TX
            {5, 0, 0, 0},       //MUX5:
            {6, 0, 0, 0},       //MUX6:
            {7, 0, 0, 0},       //MUX7: PTB_ACK
        },
    }, // end of PORT 0

    {
        // 1   -   OSPI1_SS0
        0x18,  // pad_config,
        0x204,  // pin_mux_config,
        {
            {0, 0, 0, 0},       //MUX0: GPIO_MUX1_IO25
            {1, 0, 0, 0},       //MUX1: OSPI1_SS0
            {2, 0x40c, 1, 0},   //MUX2: SPI4_SS_1   // SEL: 0
            {3, 0, 0, 0},       //MUX3: UART5_RTS
            {4, 0x400, 2, 1},   //MUX4: CANFD1_RX_2  // SEL: 1
            {5, 0, 0, 0},       //MUX5:
            {6, 0, 0, 0},       //MUX6:
            {7, 0, 0, 0},       //MUX7: PTB_CLK
        },
    }, // end of PORT 1

    {
        // 2   -   OSPI1_DATA0
        0x1c,  // pad_config,
        0x208,  // pin_mux_config,
        {
            {0, 0, 0, 0},       //MUX0: GPIO_MUX1_IO26
            {1, 0, 0, 0},       //MUX1: OSPI1_DATA0
            {2, 0x410, 1, 0},   //MUX2: SPI4_MISO_1  // SEL: 0
            {3, 0, 0, 0},       //MUX3: UART5_TX
            {4, 0, 0, 0},       //MUX4: CANFD2_TX
            {5, 0, 0, 0},       //MUX5: TMR1_EXT_CLK
            {6, 0, 0, 0},       //MUX6:
            {7, 0, 0, 0},       //MUX7: PTB_DA0
        },
    }, // end of PORT 2

    {
        // 3   -   OSPI1_DATA1
        0x20,  // pad_config,
        0x20c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO27
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA1
            {2, 0x414, 1, 0},       //MUX2: SPI4_MOSI_1     // SEL: 0
            {3, 0x418, 1, 0},       //MUX3: UART5_RX_1      // SEL: 0
            {4, 0x404, 2, 1},       //MUX4: CANFD2_RX_2     // SEL: 1
            {5, 0, 0, 0},           //MUX5: PWM1_EXT_CLK
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA1
        },
    }, // end of PORT 3

    {
        // 4   -   OSPI1_DATA2
        0x24,  // pad_config,
        0x210,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO28
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA2
            {2, 0x41c, 3, 0},       //MUX2: ENET1_MDIO_1        // SEL: 0
            {3, 0x420, 2, 0},       //MUX3: I2C2_SCL_1          // SEL: 0
            {4, 0x424, 1, 0},       //MUX4: ENET1_AUS_IN_1_1    // SEL: 0
            {5, 0, 0, 0},           //MUX5: UART7_DE
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA2
        },
    }, // end of PORT 4

    {
        // 5   -   OSPI1_DATA3
        0x28,  // pad_config,
        0x214,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO29
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA3
            {2, 0, 0, 0},           //MUX2: ENET1_MDC
            {3, 0x428, 2, 0},       //MUX3: I2C2_SDA_1          // SEL: 0
            {4, 0x42c, 1, 0},       //MUX4: ENET1_AUS_IN_2_1    // SEL: 0
            {5, 0, 0, 0},           //MUX5: UART7_RE
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA3
        },
    }, // end of PORT 5

    {
        // 6   -   OSPI1_DATA4
        0x2c,  // pad_config,
        0x218,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO30
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA4
            {2, 0x430, 1, 0},       //MUX2: SPI3_SCLK_1         // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART7_TX
            {4, 0, 0, 0},           //MUX4: CANFD3_TX
            {5, 0x434, 1, 0},       //MUX5: TMR1_CH0_1          // SEL: 0
            {6, 0x438, 1, 0},       //MUX6: ENET1_CAP_COMP_0_1  // SEL: 0
            {7, 0, 0, 0},           //MUX7: PTB_DA4
        },
    }, // end of PORT 6

    {
        // 7   -   OSPI1_DATA5
        0x30,  // pad_config,
        0x21c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO31
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA5
            {2, 0x43c, 1, 0},       //MUX2: SPI3_MISO_1         // SEL: 0
            {3, 0x440, 1, 0},       //MUX3: UART7_RX_1          // SEL: 0
            {4, 0x444, 1, 0},       //MUX4: CANFD3_RX_1         // SEL: 0
            {5, 0x448, 1, 0},       //MUX5: TMR1_CH1_1          // SEL: 0
            {6, 0x44c, 1, 0},       //MUX6: ENET1_CAP_COMP_1_1  // SEL: 0
            {7, 0, 0, 0},           //MUX7: PTB_DA5
        },
    }, // end of PORT 7

    {
        // 8   -   OSPI1_DATA6
        0x34,  // pad_config,
        0x220,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO32
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA6
            {2, 0x450, 1, 0},       //MUX2: SPI3_MOSI_1         // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART7_CTS
            {4, 0, 0, 0},           //MUX4: CANFD4_TX
            {5, 0, 0, 0},           //MUX5: PWM1_CH0
            {6, 0x454, 1, 0},       //MUX6: ENET1_CAP_COMP_2_1  // SEL: 0
            {7, 0, 0, 0},           //MUX7: PTB_DA6
        },
    }, // end of PORT 8

    {
        // 9   -   OSPI1_DATA7,
        0x38,  // pad_config, (IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR + (0x38<<10))
        0x224,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO33
            {1, 0, 0, 0},           //MUX1: OSPI1_DATA7
            {2, 0x458, 1, 0},       //MUX2: SPI3_SS_1           // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART7_RTS
            {4, 0x45c, 1, 0},       //MUX4: CANFD4_RX_1         // SEL: 0
            {5, 0, 0, 0},           //MUX5: PWM1_CH1
            {6, 0x460, 1, 0},       //MUX6: ENET1_CAP_COMP_3_1  // SEL: 0
            {7, 0, 0, 0},           //MUX7: PTB_DA7
        },
    }, // end of PORT 9

    {
        // 10  -   OSPI1_DQS
        0x3c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x228,  // pin_mux_config,
        {
            {0, 0,  0, 0},          //MUX0: GPIO_MUX1_IO34
            {1, 0, 0, 0},           //MUX1: OSPI1_DQS
            {2, 0, 0, 0},           //MUX2: 24M_CLK_OUT
            {3, 0x464, 2, 0},       //MUX3: I2C1_SCL_1          // SEL: 0
            {4, 0x460, 1, 1},       //MUX4: ENET1_CAP_COMP_3_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: PWM1_CH2
            {6, 0x468, 1, 0},       //MUX6: ENET1_AUX_IN_0_1    // SEL: 0
            {7, 0, 0, 0},           //MUX7: PTB_DA8
        },
    }, // end of PORT 10

    {
        // 11  -   OSPI1_SS1
        0x40,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x22c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO35
            {1, 0, 0, 0},           //MUX1: OSPI1_SS1
            {2, 0, 0, 0},           //MUX2: WDT1_RESET_N
            {3, 0x46c, 2, 0},       //MUX3: I2C1_SDA_1              // SEL: 0
            {4, 0, 0, 0},           //MUX4: CKGEN_SAF_ENET_25M_125M
            {5, 0, 0, 0},           //MUX5: PWM1_CH3
            {6, 0x424, 1, 1},       //MUX6: ENET1_AUX_IN_1_2        // SEL: 1
            {7, 0, 0, 0},           //MUX7: PTB_DA9
        },
    }, // end of PORT 11

    {
        // 12  -   RGMII1_TXC
        0x44,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x230,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO36
            {1, 0, 0, 0},           //MUX1: ENET1_TXC
            {2, 0x468, 1, 1},       //MUX2: ENET1_AUS_IN_0_2    // SEL: 1
            {3, 0x470, 1, 0},       //MUX3: SPI1_SCLK_1         // SEL: 0
            {4, 0x474, 1, 0},       //MUX4: I2S_SC1_SCK_1       // SEL: 0
            {5, 0x478, 1, 0},       //MUX5: TMR1_CH2_1          // SEL: 0
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA10
        },
    }, // end of PORT 11

    {
        // 13  -   RGMII1_TXD0
        0x48,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x234,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO37
            {1, 0, 0, 0},           //MUX1: ENET1_TXD0
            {2, 0, 0, 0},           //MUX2:
            {3, 0x47c, 1, 0},       //MUX3: SPI1_MISO_1         // SEL: 0
            {4, 0x488, 2, 2},       //MUX4: I2S_SC1_SDO_SDI_1   // SEL: 2
            {5, 0x480, 1, 0},       //MUX5: TMR1_CH3_1          // SEL: 0
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA11
        },
    }, // end of PORT 13

    {
        // 14  -   RGMII1_TXD1
        0x4c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x238,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO38
            {1, 0, 0, 0},           //MUX1: ENET1_TXD1
            {2, 0, 0, 0},           //MUX2:
            {3, 0x484, 1, 0},       //MUX3: SPI1_MOSI_1         // SEL: 0
            {4, 0x488, 2, 0},       //MUX4: I2S_SC1_SDI_SDO_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA12
        },
    }, // end of PORT 14

    {
        // 15  -   RGMII1_TXD2
        0x50,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x23c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO39
            {1, 0, 0, 0},           //MUX1: ENET1_TXD2
            {2, 0x41c, 3, 1},       //MUX2: ENET1_MDIO_2    // SEL: 1
            {3, 0x48c, 1, 0},       //MUX3: SPI1_SS_1       // SEL: 0
            {4, 0x490, 1, 0},       //MUX4: I2S_SC1_WS_1    // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA13
        },
    }, // end of PORT 15

    {
        // 16  -   RGMII1_TXD3
        0x54,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x240,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO40
            {1, 0, 0, 0},           //MUX1: ENET1_TXD3
            {2, 0, 0, 0},           //MUX2: ENET1_MDC
            {3, 0x494, 1, 0},       //MUX3: SPI2_SCLK_1     // SEL: 0
            {4, 0x498, 1, 0},       //MUX4: I2S_SC2_SCK_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA14
        },
    }, // end of PORT 16

    {
        // 17  -   RGMII1_TX_CTL
        0x58,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x244,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO41
            {1, 0, 0, 0},           //MUX1: ENET1_TXEN
            {2, 0, 0, 0},           //MUX2:
            {3, 0x49c, 1, 0},       //MUX3: SPI2_MISO_1         // SEL: 0
            {4, 0x4a4, 2, 2},       //MUX4: I2S_SC2_SDO_SDI_1   // SEL: 2
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA15
        },
    }, // end of PORT 17

    {
        // 18  -   RGMII1_RXC
        0x5c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x248,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO42
            {1, 0, 0, 0},           //MUX1: ENET1_RXC
            {2, 0x438, 1, 1},       //MUX2: ENET1_CAP_COMP_0_2  // SEL: 1
            {3, 0x4a0, 1, 0},       //MUX3: SPI2_MOSI_1         // SEL: 0
            {4, 0x4a4, 2, 0},       //MUX4: I2S_SC2_SDI_SDO_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA16
        },
    }, // end of PORT 18

    {
        // 19  -   RGMII1_RXD0
        0x60,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x24c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO43
            {1, 0, 0, 0},           //MUX1: ENET1_RXD0
            {2, 0, 0, 0},           //MUX2:
            {3, 0x4a8, 1, 0},       //MUX3: SPI2_SS_1       // SEL: 0
            {4, 0x4ac, 1, 0},       //MUX4: I2S_SC2_WS_1    // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA17
        },
    }, // end of PORT 19

    {
        // 20  -   RGMII1_RXD1
        0x64,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x250,  // pin_mux_config,
        {
            {0, 0,  0, 0},          //MUX0: GPIO_MUX1_IO44
            {1, 0, 0, 0},           //MUX1: ENET1_RXD1
            {2, 0, 0, 0},           //MUX2:
            {3, 0x4b0, 2, 0},       //MUX3: I2C3_SCL_1  // SEL: 0
            {4, 0, 0, 0},           //MUX4:
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA18
        },
    }, // end of PORT 20

    {
        // 21  -   RGMII1_RXD2
        0x68,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x254,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO45
            {1, 0, 0, 0},           //MUX1: ENET1_RXD2
            {2, 0, 0, 0},           //MUX2: ENET1_RMII_REF_CLK
            {3, 0x4b4, 2, 0},       //MUX3: I2C3_SDA_1          // SEL: 0
            {4, 0x4b8, 1, 0},       //MUX4: TMR2_CH0_1          // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA19
        },
    }, // end of PORT 21

    {
        // 22  -   RGMII1_RXD3
        0x6c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x258,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO46
            {1, 0, 0, 0},           //MUX1: ENET1_RXD3
            {2, 0, 0, 0},           //MUX2: ENET1_RX_ER
            {3, 0x4bc, 2, 0},       //MUX3: I2C4_SCL_1  // SEL: 0
            {4, 0x4c0, 1, 0},       //MUX4: TMR2_CH1_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA20
        },
    }, // end of PORT 22

    {
        // 23  -   RGMII1_RX_CTL
        0x70,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x25c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO47
            {1, 0, 0, 0},           //MUX1: ENET1_RXDV
            {2, 0, 0, 0},           //MUX2:
            {3, 0x4c4, 2, 0},       //MUX3: I2C4_SDA_1  // SEL: 0
            {4, 0, 0, 0},           //MUX4:
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: PTB_DA21
        },
    }, // end of PORT 23

    {
        // 24  -   GPIO_A0
        0x74,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x260,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO0
            {1, 0x464, 2, 1},       //MUX1: I2C1_SCL_2      // SEL: 1
            {2, 0x494, 1, 1},       //MUX2: SPI2_SCLK_2     // SEL: 1
            {3, 0, 0, 0},           //MUX3: CANFD3_TX
            {4, 0x474, 1, 1},       //MUX4: I2S_SC1_SCK_2   // SEL: 1
            {5, 0x4c8, 1, 0},       //MUX5: TMR2_CH2_1      // SEL: 0
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D0
            {7, 0, 0, 0},           //MUX7: PTB_DA22
        },
    }, // end of PORT 24

    {
        // 25  -   GPIO_A1
        0x78,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x264,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO1
            {1, 0x46c, 2, 1},       //MUX1: I2C1_SDA_2      // SEL: 1
            {2, 0x49c, 1, 1},       //MUX2: SPI2_MISO_2     // SEL: 1
            {3, 0x444, 1, 1},       //MUX3: CANFD3_RX_2     // SEL: 1
            {4, 0x490, 1, 1},       //MUX4: I2S_SC1_WS_2    // SEL: 1
            {5, 0x4cc, 1, 0},       //MUX5: TMR2_CH3_1      // SEL: 0
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D1
            {7, 0, 0, 0},           //MUX7: PTB_DA23
        },
    }, // end of PORT 25

    {
        // 26  -   GPIO_A2
        0x7c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x268,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO2
            {1, 0x420, 2, 1},       //MUX1: I2C2_SCL_2          // SEL: 1
            {2, 0x4a0, 1, 1},       //MUX2: SPI2_MOSI_2         // SEL: 1
            {3, 0, 0, 0},           //MUX3: CANFD4_TX
            {4, 0x488, 2, 3},       //MUX4: I2S_SC1_SDO_SDI_2   // SEL: 3
            {5, 0, 0, 0},           //MUX5: UART2_DE
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D2
            {7, 0, 0, 0},           //MUX7: PTB_DA24
        },
    }, // end of PORT 26

    {
        // 27  -   GPIO_A3
        0x80,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x26c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO3
            {1, 0x428, 2, 1},       //MUX1: I2C2_SDA_2          // SEL: 1
            {2, 0x4a8, 1, 1},       //MUX2: SPI2_SS_2           // SEL: 1
            {3, 0x45c, 1, 1},       //MUX3: CANFD4_RX_2         // SEL: 1
            {4, 0x488, 2, 1},       //MUX4: I2S_SC1_SDI_SDO_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: UART2_RE
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D3
            {7, 0, 0, 0},           //MUX7: PTB_DA25
        },
    }, // end of PORT 24

    {
        // 28  -   GPIO_A4
        0x84,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x270,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO4
            {1, 0, 0, 0},           //MUX1: UART1_TX
            {2, 0x4b0, 2, 1},       //MUX2: I2C3_SCL_2          // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART2_RTS
            {4, 0x42c, 1, 1},       //MUX4: ENET1_AUS_IN_2_2    // SEL: 1
            {5, 0, 0, 0},           //MUX5: PWM2_CH0
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D4
            {7, 0, 0, 0},           //MUX7: PTB_DA26
        },
    }, // end of PORT 28

    {
        // 29  -   GPIO_A5
        0x88,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x274,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO5
            {1, 0, 0, 0},           //MUX1: UART1_RX
            {2, 0x4b4, 2, 1},       //MUX2: I2C3_SDA_2  // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART2_CTS
            {4, 0, 0, 0},           //MUX4: ENET1_AUS_IN_3
            {5, 0, 0, 0},           //MUX5: PWM2_CH1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D5
            {7, 0, 0, 0},           //MUX7: PTB_DA27
        },
    }, // end of PORT 29

    {
        // 30  -   GPIO_A6
        0x8c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x278,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO6
            {1, 0, 0, 0},           //MUX1: UART2_TX
            {2, 0x4bc, 2, 1},       //MUX2: I2C4_SCL_2      // SEL: 1
            {3, 0x41c, 3, 2},       //MUX3: ENET1_MODIO_3,  // SEL: 2
            {4, 0, 0, 0},           //MUX4: CKGEN_SAF_I2S_MCLK1
            {5, 0, 0, 0},           //MUX5: PWM2_CH2
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D6
            {7, 0, 0, 0},           //MUX7: PTB_DA28
        },
    }, // end of PORT 30

    {
        // 31  -   GPIO_A7
        0x90,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x27c,  // pin_mux_config,
        {
            {0, 0,  0, 0},          //MUX0: GPIO_MUX1_IO7
            {1, 0, 0, 0},           //MUX1: UART2_RX
            {2, 0x4c4, 2, 1},       //MUX2: I2C4_SDA_2  // SEL: 1
            {3, 0, 0, 0},           //MUX3: ENET1_MDC
            {4, 0, 0, 0},           //MUX4: WDT1_RESET_N
            {5, 0, 0, 0},           //MUX5: PWM2_CH3
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D7
            {7, 0, 0, 0},           //MUX7: PTB_DA29
        },
    }, // end of PORT 31

    /* PORT 1 */

    {
        // 32  -   GPIO_A8
        0x94,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x280,  // pin_mux_config,
        {
            {0, 0, 0, 0},               //MUX0: GPIO_MUX1_IO8
            {1, 0x470, 1, 1},           //MUX1: SPI1_SCLK_2                 // SEL: 1
            {2, 0, 0, 0},               //MUX2: UART3_TX
            {3, 0, 0, 0},               //MUX3: UART4_RTS
            {4, 0, 0, 0},               //MUX4: CKGEN_SAF_ENET_25M_125M
            {5, 0x4b8, 1, 1},           //MUX5: TMR2_CH0_2                  // SEL: 1
            {6, 0, 0, 0},               //MUX6: OBSERVER_MUX1_D8
            {7, 0, 0, 0},               //MUX7: PTB_DA30
        },
    }, // end of PORT 32

    {
        // 33  -   GPIO_A9
        0x98,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x284,  // pin_mux_config,
        {
            {0, 0, 0, 0},               //MUX0: GPIO_MUX1_IO9
            {1, 0x47c, 1, 1},           //MUX1: SPI1_MISO_2  // SEL: 1
            {2, 0, 0, 0},               //MUX2: UART3_RX
            {3, 0, 0, 0},               //MUX3: UART4_CTS
            {4, 0x44c, 1, 1},           //MUX4: ENET1_CAP_COMP_1_2  // SEL: 1
            {5, 0x4c0, 1, 1},           //MUX5: TMR2_CH1_2  // SEL: 1
            {6, 0, 0, 0},               //MUX6: OBSERVER_MUX1_D9
            {7, 0, 0, 0},               //MUX7: PTB_DA31
        },
    }, // end of PORT 33

    {
        // 34  -   GPIO_A10
        0x9c,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x288,  // pin_mux_config,
        {
            {0, 0, 0, 0},          //MUX0: GPIO_MUX1_IO10
            {1, 0x484, 1, 1},           //MUX1: SPI1_MOSI_2  // SEL: 1
            {2, 0, 0, 0},       //MUX2: UART4_TX
            {3, 0x41c, 3, 3},           //MUX3: ENET1_MDIO_4  // SEL: 3
            {4, 0, 0, 0},           //MUX4: PWM1_CH0
            {5, 0x4c8, 1, 1},           //MUX5: TMR2_CH2_2  // SEL: 1
            {6, 0, 0, 0},       //MUX6: OBSERVER_MUX1_D10
            {7, 0, 0, 0},           //MUX7: PTB_CMD0
        },
    }, // end of PORT 34

    {
        // 35  -   GPIO_A11
        0xa0,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x28c,  // pin_mux_config,
        {
            {0, 0, 0, 0},          //MUX0: GPIO_MUX1_IO11
            {1, 0x48c, 1, 1},       //MUX1: SPI1_SS_2       // SEL: 1
            {2, 0, 0, 0},           //MUX2: UART4_RX
            {3, 0, 0, 0},           //MUX3: ENET1_MDC
            {4, 0, 0, 0},           //MUX4: WDT1_RESET_N
            {5, 0x4cc, 1, 1},       //MUX5: TMR2_CH3_2      // SEL: 1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D11
            {7, 0, 0, 0},           //MUX7: PTB_CMD1
        },
    }, // end of PORT 35

    {
        // 36  -   GPIO_B0
        0xa4,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x290,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO12
            {1, 0x4b0, 2, 2},       //MUX1: I2C3_SCL_3          // SEL: 2
            {2, 0x408, 1, 1},       //MUX2: SPI4_SCLK_2         // SEL: 1
            {3, 0, 0, 0},           //MUX3: CANFD1_TX
            {4, 0x498, 1, 1},       //MUX4: I2S_SC2_SCK_2       // SEL: 1
            {5, 0, 0, 0},           //MUX5: TMR2_EXT_CLK
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D12
            {7, 0, 0, 0},           //MUX7: PTB_CMD2
        },
    }, // end of PORT 36

    {
        // 37  -   GPIO_B1
        0xa8,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x294,  // pin_mux_config,
        {
            {0, 0, 0, 0},               //MUX0: GPIO_MUX1_IO13
            {1, 0x4b4, 2, 2},           //MUX1: I2C3_SDA_3      // SEL: 2
            {2, 0x410, 1, 1},           //MUX2: SPI4_MISO_2     // SEL: 1
            {3, 0x400, 2, 2},           //MUX3: CANFD1_RX_3     // SEL: 2
            {4, 0x4ac, 1, 1},           //MUX4: I2S_SC2_WS_2    // SEL: 1
            {5, 0, 0, 0},               //MUX5:
            {6, 0, 0, 0},               //MUX6: OBSERVER_MUX1_D13
            {7, 0, 0, 0},               //MUX7: PTB_CMD3
        },
    }, // end of PORT 37

    {
        // 38  -   GPIO_B2
        0xac,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x298,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO14
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
            {1, 0x4bc, 2, 2},       //MUX1: I2C4_SCL
#else
            {1, 0, 0, 0},           //MUX1: I2C4_SCL
#endif
            {2, 0x414, 1, 1},       //MUX2: SPI4_MOSI_2         // SEL: 1
            {3, 0, 0, 0},           //MUX3: CANFD2_TX
            {4, 0x4a4, 2, 1},       //MUX4: I2S_SC2_SDI_SDO_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5:
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D14
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA5
        },
    }, // end of PORT 38

    {
        // 39  -   GPIO_B3
        0xb0,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x29c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO15
            {1, 0x4c4, 2, 2},       //MUX1: I2C4_SDA_3          // SEL: 2
            {2, 0x40c, 1, 1},       //MUX2: SPI4_SS_2           // SEL: 1
            {3, 0x404, 2, 2},       //MUX3: CANFD2_RX_3         // SEL: 2
            {4, 0x4a4, 2, 3},       //MUX4: I2S_SC2_SDO_SDI_2   // SEL: 3
            {5, 0, 0, 0},           //MUX5: PWM2_EXT_CLK
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D15
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA6
        },
    }, // end of PORT 39

    {
        // 40  -   GPIO_B4
        0xb4,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2a0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO16
            {1, 0, 0, 0},           //MUX1: UART5_TX
            {2, 0x464, 2, 2},       //MUX2: I2C1_SCL_3  // SEL: 2
            {3, 0, 0, 0},           //MUX3: UART6_RTS
            {4, 0, 0, 0},           //MUX4: WDT1_EXT_CLK
            {5, 0, 0, 0},           //MUX5: PWM2_CH2
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA7
        },
    }, // end of PORT 40

    {
        // 41  -   GPIO_B5
        0xb8,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2a4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO17
            {1, 0x418, 1, 1},       //MUX1: UART5_RX_2      // SEL: 1
            {2, 0x46c, 2, 2},       //MUX2: I2C1_SDA_3      // SEL: 2
            {3, 0, 0, 0},           //MUX3: UART6_CTS
            {4, 0, 0, 0},           //MUX4: OSPI1_RESET
            {5, 0, 0, 0},           //MUX5: PWM2_CH3
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA8
        },
    }, // end of PORT 41

    {
        // 42  -   GPIO_B6
        0xbc,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2a8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO18
            {1, 0, 0, 0},           //MUX1: UART6_TX
            {2, 0x420, 2, 2},       //MUX2: I2C2_SCL_3      // SEL: 2
            {3, 0x41c, 3, 4},       //MUX3: ENET1_MDIO_5    // SEL: 4
            {4, 0, 0, 0},           //MUX4: CKGEN_SAF_I2S_MCLK1
            {5, 0, 0, 0},           //MUX5: OSPI1_ECC_FAIL
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA9
        },
    }, // end of PORT 42

    {
        // 43  -   GPIO_B7
        0xc0,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2ac,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO19
            {1, 0, 0, 0},           //MUX1: UART6_RX
            {2, 0x428, 2, 2},       //MUX2: I2C2_SDA_3  // SEL: 2
            {3, 0, 0, 0},           //MUX3: ENET1_MDC
            {4, 0, 0, 0},           //MUX4: WDT1_RESET_N
            {5, 0, 0, 0},           //MUX5: OSPI1_INTB
            {6, 0, 0, 0},           //MUX6: DC1_CRC_EXT_CLK
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA10
        },
    }, // end of PORT 43

    {
        // 44  -   GPIO_B8
        0xc4,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2b0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO20
            {1, 0x430, 1, 1},       //MUX1: SPI3_SCLK_2         // SEL: 1
            {2, 0, 0, 0},           //MUX2: UART7_TX
            {3, 0, 0, 0},           //MUX3: UART8_RTS
            {4, 0, 0, 0},           //MUX4: CKGEN_SAF_ENET_25M_125M
            {5, 0x434, 1, 1},       //MUX5: TMR1_CH0_2          // SEL: 1
            {6, 0, 0, 0},           //MUX6: DC2_CRC_EXT_CLK
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA11
        },
    }, // end of PORT 44

    {
        // 45  -   GPIO_B9
        0xc8,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2b4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO21
            {1, 0x43c, 1, 1},       //MUX1: SPI3_MISO_2         // SEL: 1
            {2, 0x440, 1, 1},       //MUX2: UART7_RX_2          // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART8_CTS
            {4, 0x454, 1, 1},       //MUX4: ENET1_CAP_COMP_2_2  // SEL: 1
            {5, 0x448, 1, 1},       //MUX5: TMR1_CH1_2  // SEL: 1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX1_D0
            {7, 0, 0, 0},           //MUX7: PTB_DA22
        },
    }, // end of PORT 45

    {
        // 46  -   GPIO_B10
        0xcc,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2b8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO22
            {1, 0x450, 1, 1},       //MUX1: SPI3_MOSI_2     // SEL: 1
            {2, 0, 0, 0},           //MUX2: UART8_TX
            {3, 0x41c, 3, 5},       //MUX3: ENET1_MDIO_6    // SEL: 5
            {4, 0, 0, 0},           //MUX4: PWM1_CH1
            {5, 0x478, 1, 1},       //MUX5: TMR1_CH2_2      // SEL: 1
            {6, 0, 0, 0},           //MUX6: DC4_CRC_EXT_CLK
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA13
        },
    }, // end of PORT 46

    {
        // 47  -   GPIO_B11
        0xd0,  // pad_config, IOMUX_CTRL_SAFETY_APB_AB0_BASE_ADDR
        0x2bc,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX1_IO23
            {1, 0x458, 1, 1},       //MUX1: SPI3_SS_2   // SEL: 1
            {2, 0, 0, 0},           //MUX2: UART8_RX
            {3, 0, 0, 0},           //MUX3: ENET1_MDC
            {4, 0, 0, 0},           //MUX4: 24M_CLK_OUT
            {5, 0x480, 1, 1},       //MUX5: TMR1_CH3_2  // SEL: 1
            {6, 0, 0, 0},           //MUX6: DC5_CRC_EXT_CLK
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA14
        },
    }, // end of PORT 47

    {
        // 48  -   GPIO_C0
        0x0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x200,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: X9_IOMUXC_GPIO_C0__GPIO_MUX2_IO0_1
            {1, 0x400, 1, 0},       //MUX1: X9_IOMUXC_GPIO_C0__I2C5_SCL_1   // SEL: 0
            {2, 0x404, 1, 0},       //MUX2: GPIO_C0__SPI6_SCLK_1            // SEL: 0
            {3, 0, 0, 0},           //MUX3: GPIO_C0__CANFD5_TX_1
            {4, 0x408, 1, 0},       //MUX4: GPIO_C0__MSHC4_CARD_DET_N_1     // SEL: 0
            {5, 0, 0, 0},           //MUX5: GPIO_C0__USB1_PWR_1
            {6, 0, 0, 0},           //MUX6: GPIO_C0__TMR3_CH0_1
            {7, 0, 0, 0},           //MUX7: GPIO_C0__DFM_DDR_PWROKIN_1
        },
    }, // end of PORT 48

    {
        // 49  -   GPIO_C1
        0x4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x204,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO1_1
            {1, 0x40c, 1, 0},       //MUX1: I2C5_SDA_1      // SEL: 0
            {2, 0x410, 1, 0},       //MUX2: SPI6_MISO_1     // SEL: 0
            {3, 0x414, 2, 0},       //MUX3: CANFD5_RX_1     // SEL: 0
            {4, 0x418, 1, 0},       //MUX4: MSHC4_WP_1      // SEL: 0
            {5, 0x41c, 3, 0},       //MUX5: USB1_OC_1       //  SEL: 0
            {6, 0, 0, 0},           //MUX6: TMR3_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_RESET_1
        },
    }, // end of PORT 49

    {
        // 50  -   GPIO_C2
        0x8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x208,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO2_1
            {1, 0x420, 1, 0},       //MUX1: I2C6_SCL_1      // SEL: 0
            {2, 0x424, 1, 0},       //MUX2: SPI6_MOSI_1     // SEL: 0
            {3, 0, 0, 0},           //MUX3: CANFD6_TX_1
            {4, 0, 0, 0},           //MUX4: MSHC4_VOLT_SW_1
            {5, 0, 0, 0},           //MUX5: USB2_PWR_1
            {6, 0, 0, 0},           //MUX6: TMR3_CH2_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DWC_DDRPHY_DTO_1
        },
    }, // end of PORT 50

    {
        // 51  -   GPIO_C3
        0xc,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x20c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO3_1
            {1, 0x428, 1, 0},       //MUX1: I2C6_SDA_1      // SEL: 0
            {2, 0x42c, 1, 0},       //MUX2: SPI6_SS_1       // SEL: 0
            {3, 0x430, 2, 0},       //MUX3: CANFD6_RX_1     // SEL: 0
            {4, 0, 0, 0},           //MUX4: MSHC4_LED_CTRL_1
            {5, 0x434, 3, 0},       //MUX5: USB2_OC_1       // SEL: 0
            {6, 0, 0, 0},           //MUX6: TMR3_CH3_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_WSI_1
        },
    }, // end of PORT 51

    {
        // 52  -   GPIO_C4
        0x10,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x210,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO4_1
            {1, 0, 0, 0},           //MUX1: UART9_TX_1
            {2, 0x438, 2, 0},       //MUX2: I2C7_SCL_1          // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART10_RTS_1
            {4, 0x43c, 1, 0},       //MUX4: MSHC3_CARD_DET_N_1  // SEL: 0
            {5, 0x440, 2, 0},       //MUX5: PCIE1_CLKREQ_N_1/PCIEX2 input source    // SEL: 0
            {6, 0, 0, 0},           //MUX6: TMR4_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_TDRCLK_1
        },
    }, // end of PORT 52

    {
        // 53  -   GPIO_C5
        0x14,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x214,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO5_1
            {1, 0, 0, 0},           //MUX1: UART9_RX_1
            {2, 0x444, 2, 0},       //MUX2: I2C7_SDA_1          // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART10_CTS_1
            {4, 0x448, 1, 0},       //MUX4: MSHC3_WP_1          // SEL: 0
            {5, 0x44c, 2, 0},       //MUX5: PCIE2_CLKREQ_N_1/PCIX1 input source    // SEL: 0
            {6, 0, 0, 0},           //MUX6: TMR4_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_WRSTN_1
        },
    }, // end of PORT 53

    {
        // 54  -   GPIO_C6
        0x18,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x218,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO6_1
            {1, 0, 0, 0},           //MUX1: UART10_TX_1
            {2, 0x450, 2, 0},       //MUX2: I2C8_SCL_1      // SEL: 0
            {3, 0x454, 3, 0},       //MUX3: ENET2_MDIO_1    // SEL: 0
            {4, 0x458, 1, 0},       //MUX4: MSHC3_VOLT_SW_1 // SEL: 0
            {5, 0, 0, 0},           //MUX5: UART12_DE_1
            {6, 0, 0, 0},           //MUX6: TMR4_CH2_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRCMDTDRSHIFTEN_1
        },
    }, // end of PORT 54

    {
        // 55  -   GPIO_C7
        0x1c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x21c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO7_1
            {1, 0, 0, 0},           //MUX1: UART10_RX_1
            {2, 0x45c, 2, 0},       //MUX2: I2C8_SDA_1  // SEL: 0
            {3, 0, 0, 0},           //MUX3: ENET2_MDC_1
            {4, 0, 0, 0},           //MUX4: MSHC3_LED_CTRL_1
            {5, 0, 0, 0},           //MUX5: UART12_RE_1
            {6, 0, 0, 0},           //MUX6: TMR4_CH3_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRCMDTDRCAPTUREEN_1
        },
    }, // end of PORT 55

    {
        // 56  -   GPIO_C8
        0x20,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x220,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO8_1
            {1, 0x460, 2, 0},       //MUX1: SPI5_SCLK_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: UART11_TX_1
            {3, 0, 0, 0},           //MUX3: UART12_RTS_1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_ENET_25M_125M_1
            {5, 0, 0, 0},           //MUX5: WDT5_RESET_N_1
            {6, 0, 0, 0},           //MUX6: PWM3_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRCMDTDRUPDATEEN_1
        },
    }, // end of PORT 56

    {
        // 57  -   GPIO_C9
        0x24,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x224,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO9_1
            {1, 0x464, 2, 0},       //MUX1: SPI5_MISO_1  // SEL: 0
            {2, 0x468, 2, 0},       //MUX2: UART11_RX_1  // SEL: 0
            {3, 0, 0, 0},           //MUX3: _UART12_CTS_1
            {4, 0, 0, 0},           //MUX4: ENET2_CAP_COMP_1_1
            {5, 0, 0, 0},           //MUX5: WDT6_RESET_N_1
            {6, 0, 0, 0},           //MUX6: PWM3_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRCMDTDR_TDO_1
        },
    }, // end of PORT 57

    {
        // 58  -   GPIO_C10
        0x28,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x228,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO10_1
            {1, 0x46c, 2, 0},       //MUX1: SPI5_MOSI_1     // SEL: 0
            {2, 0, 0, 0},           //MUX2: UART12_TX_1
            {3, 0x454, 3, 1},       //MUX3: ENET2_MDIO_2    // SEL: 1
            {4, 0, 0, 0},           //MUX4: TMR5_CH2_1
            {5, 0, 0, 0},           //MUX5: WDT3_RESET_N_1
            {6, 0, 0, 0},           //MUX6: PWM3_CH2_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRRDDATATDRSHIFTEN_1
        },
    }, // end of PORT 58

    {
        // 59  -   GPIO_C11
        0x2c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x22c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO11_1
            {1, 0x470, 2, 0},       //MUX1: SPI5_SS_1       // SEL: 0
            {2, 0x474, 1, 0},       //MUX2: UART12_RX_1     // SEL: 0
            {3, 0, 0, 0},           //MUX3: ENET2_MDC_2
            {4, 0, 0, 0},           //MUX4: TMR5_CH3_1
            {5, 0, 0, 0},           //MUX5: WDT4_RESET_N_1
            {6, 0, 0, 0},           //MUX6: PWM3_CH3_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRRDDATATDRCAPTUREEN_1
        },
    }, // end of PORT 59

    {
        // 60  -   GPIO_C12
        0x30,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x230,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: X9_IOMUXC_GPIO_C12__GPIO_MUX2_IO12_1
            {1, 0x478, 2, 0},       //MUX1: I2C13_SCL_1         // SEL: 0
            {2, 0, 0, 0},           //MUX2: USB1_PWR_2
            {3, 0, 0, 0},           //MUX3: CANFD7_TX_1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_I2S_MCLK2_1
            {5, 0x47c, 2, 0},       //MUX5: PERST_N_1/PCIEX1 input souce     // SEL 0
            {6, 0, 0, 0},           //MUX6: PWM4_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRRDDATATDRUPDATEEN_1
        },
    }, // end of PORT 60

    {
        // 61  -   GPIO_C13
        0x34,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x234,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO13_1
            {1, 0x480, 2, 0},       //MUX1: I2C13_SDA_1     // SEL 0
            {2, 0x41c, 3, 1},       //MUX2: USB1_OC_2       // SEL: 1
            {3, 0x484, 2, 0},       //MUX3: CANFD7_RX_1     // SEL 0
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_CSI_MCLK1_1
            {5, 0x488, 2, 0},       //MUX5: WAKE_N_1/PCIEX1 input source  // SEL: 0
            {6, 0, 0, 0},           //MUX6: PWM4_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHYCSRRDDATATDR_TDO_1
        },
    }, // end of PORT 61

    {
        //  62  -   GPIO_C14
        0x38,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x238,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO14_1
            {1, 0x48c, 2, 0},       //MUX1: I2C14_SCL_1         // SEL 0
            {2, 0, 0, 0},           //MUX2: USB2_PWR_2
            {3, 0, 0, 0},           //MUX3: CANFD8_TX_1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_I2S_MCLK3_1
            {5, 0x490, 2, 0},       //MUX5: PCIE1_PERST_N_1/PCIEX2 input source  // SEL: 0
            {6, 0, 0, 0},           //MUX6: PWM4_CH2_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHY_TEST_RSTN_1
        },
    }, // end of PORT 62

    {
        //  63  -   GPIO_C15
        0x3c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x23c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO15_1
            {1, 0x494, 2, 0},       //MUX1: I2C14_SDA_1     // SEL 0
            {2, 0x434, 3, 1},       //MUX2: USB2_OC_2       // SEL: 1
            {3, 0x498, 2, 0},       //MUX3: CANFD8_RX_1     // SEL: 0
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_CSI_MCLK2_1
            {5, 0x49c, 2, 0},       //MUX5: PCIE1_WAKE_N_1/PCIEX2 input source  // SEL 0
            {6, 0, 0, 0},           //MUX6: PWM4_CH3_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHY_TEST_APBRESETN_1
        },
    }, // end of PORT 63

    /* PORT 2 */

    {
        // 64  -   GPIO_D0
        0x40,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x240,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO16_1
            {1, 0, 0, 0},           //MUX1: I2C9_SCL_1
            {2, 0x4a0, 2, 0},       //MUX2: SPI8_SCLK_1          // SEL: 0
            {3, 0, 0, 0},           //MUX3: CANFD5_TX_2
            {4, 0x408, 1, 1},       //MUX4: _MSHC4_CARD_DET_N_2  // SEL: 1
            {5, 0x47c, 2, 1},       //MUX5: PCIE2_PERST_N_2/PCIEX1 input source      // SEL 1
            {6, 0, 0, 0},           //MUX6: TMR5_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_DDR_DDRPHY_TEST_DFI_RSTN_1
        },
    }, // end of PORT 64

    {
        //  65  -   GPIO_D1
        0x44,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x244,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO17_1
            {1, 0, 0, 0},           //MUX1: _I2C9_SDA_1
            {2, 0x4a4, 2, 0},       //MUX2: SPI8_MISO_1     // SEL: 0
            {3, 0x414, 2, 1},       //MUX3: CANFD5_RX_2     // SEL: 1
            {4, 0x418, 1, 1},       //MUX4: _MSHC4_WP_2     // SEL: 1
            {5, 0x488, 2, 1},       //MUX5: PCIE2_WAKE_N_2/PCIEX1 input source  // SEL: 1
            {6, 0, 0, 0},           //MUX6: TMR5_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSMODEENAC_1
        },
    }, // end of PORT 65

    {
        // 66  -   GPIO_D2
        0x48,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x248,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO18_1
            {1, 0, 0, 0},           //MUX1: _I2C10_SCL_1
            {2, 0x4a8, 2, 0},       //MUX2: SPI8_MOSI_1         // SEL: 0
            {3, 0, 0, 0},           //MUX3: CANFD6_TX_2
            {4, 0, 0, 0},           //MUX4: MSHC4_VOLT_SW_2
            {5, 0x490, 2, 1},       //MUX5: PCIE1_PERST_N_2/PCIEX2 input source  // SEL: 1
            {6, 0x4ac, 1, 0},       //MUX6: SPDIF3_IN_1         // SEL: 0
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTENAC_1
        },
    }, // end of PORT 66

    {
        // 67  -   GPIO_D3
        0x4c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x24c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO19_1
            {1, 0, 0, 0},           //MUX1: I2C10_SDA_1
            {2, 0x4b0, 2, 0},       //MUX2: SPI8_SS_1       // SEL: 0
            {3, 0x430, 2, 1},       //MUX3: CANFD6_RX_2     // SEL: 1
            {4, 0, 0, 0},           //MUX4: MSHC4_LED_CTRL_2
            {5, 0x49c, 2, 1},       //MUX5: PCIE1_WAKE_N_2/PCIEX2 input source  // SEL 1
            {6, 0, 0, 0},           //MUX6: SPDIF3_OUT_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTDATAAC_1
        },
    }, // end of PORT 67

    {
        // 68  -   GPIO_D4
        0x50,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x250,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO20_1
            {1, 0, 0, 0},           //MUX1: UART13_TX_1
            {2, 0, 0, 0},           //MUX2: I2C11_SCL_1
            {3, 0, 0, 0},           //MUX3: UART14_RTS_1
            {4, 0x43c, 1, 1},       //MUX4: MSHC3_CARD_DET_N_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: USB1_PWR_3
            {6, 0, 0, 0},           //MUX6: TMR6_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BAPASSMODEENDAT_1
        },
    }, // end of PORT 68

    {
        // 69  -   GPIO_D5
        0x54,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x254,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO21_1
            {1, 0x4b4, 2, 0},       //MUX1: UART13_RX_1     // SEL: 0
            {2, 0, 0, 0},           //MUX2: I2C11_SDA_1
            {3, 0x4b8, 1, 0},       //MUX3: UART14_CTS_1    // SEL: 0
            {4, 0x448, 1, 1},       //MUX4: MSHC3_WP_2      // SEL: 1
            {5, 0x41c, 3, 2},       //MUX5: USB1_OC_3       // SEL: 2
            {6, 0, 0, 0},           //MUX6: TMR6_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTENDAT_1
        },
    }, // end of PORT 69

    {
        // 70  -   GPIO_D6
        0x58,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x258,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO22_1
            {1, 0, 0, 0},           //MUX1: UART14_TX_1
            {2, 0x4bc, 1, 0},       //MUX2: I2C12_SCL_1         // SEL: 0
            {3, 0x454, 3, 2},       //MUX3: ENET2_MDIO_3        // SEL: 2
            {4, 0x458, 1, 1},       //MUX4: MSHC3_VOLT_SW_2     // SEL: 1
            {5, 0, 0, 0},           //MUX5: USB2_PWR_3
            {6, 0x4c0, 1, 0},       //MUX6: SPDIF4_IN_1         // SEL: 0
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTDATADAT_1
        },
    }, // end of PORT 70

    {
        // 71  -   GPIO_D7
        0x5c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x25c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO23_1
            {1, 0x4c4, 1, 0},       //MUX1: UART14_RX_1     // SEL: 0
            {2, 0x4c8, 1, 0},       //MUX2: I2C12_SDA_1     // SEL: 0
            {3, 0, 0, 0},           //MUX3: ENET2_MDC_3
            {4, 0, 0, 0},           //MUX4: MSHC3_LED_CTRL_2
            {5, 0x434, 3, 2},       //MUX5: USB2_OC_3       // SEL: 2
            {6, 0, 0, 0},           //MUX6: SPDIF4_OUT_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSMODEENMASTER_1
        },
    }, // end of PORT 71

    {
        // 72  -   GPIO_D8
        0x60,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x260,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO24_1
            {1, 0x4cc, 1, 0},       //MUX1: SPI7_SCLK_1   // SEL: 0
            {2, 0, 0, 0},           //MUX2: UART15_TX_1
            {3, 0, 0, 0},           //MUX3: UART16_RTS_1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_ENET_25M_125M_2
            {5, 0, 0, 0},           //MUX5: WDT5_RESET_N_2
            {6, 0, 0, 0},           //MUX6: PWM5_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTENMASTER_1
        },
    }, // end of PORT 72

    {
        // 73  -   GPIO_D9
        0x64,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x264,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO25_1
            {1, 0x4d0, 1, 0},       //MUX1: SPI7_MISO_1   // SEL: 0
            {2, 0x4d4, 2, 0},       //MUX2: UART15_RX_1   // SEL: 0
            {3, 0x4d8, 2, 0},       //MUX3: UART16_CTS_1  // SEL: 0
            {4, 0, 0, 0},           //MUX4: ENET2_CAP_COMP_2_1
            {5, 0, 0, 0},           //MUX5: WDT6_RESET_N_2
            {6, 0, 0, 0},           //MUX6: PWM5_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSOUTDATAMASTER_1
        },
    }, // end of PORT 73

    {
        // 74  -   GPIO_D10
        0x68,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x268,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO26_1
            {1, 0x4dc, 1, 0},       //MUX1: SPI7_MOSI_1     // SEL: 0
            {2, 0, 0, 0},           //MUX2: UART16_TX_1
            {3, 0x454, 3, 3},       //MUX3: ENET2_MDIO_4    // SEL: 3
            {4, 0x4e0, 1, 0},       //MUX4: TMR6_CH2_1      // SEL: 0
            {5, 0, 0, 0},           //MUX5: WDT3_RESET_N_2
            {6, 0, 0, 0},           //MUX6: PWM3_EXT_CLK_1
            {7, 0, 0, 0},           //MUX7: DFM_DDRPHY_BYPASS_IN_DATA_SEL_0_1
        },
    }, // end of PORT 74

    {
        // 75  -   GPIO_D11
        0x6c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x26c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO27_1
            {1, 0x4e4, 1, 0},       //MUX1: SPI7_SS_1       // SEL: 0
            {2, 0x4e8, 2, 0},       //MUX2: UART16_RX_1     // SEL: 0
            {3, 0, 0, 0},           //MUX3: ENET2_MDC_4
            {4, 0x4ec, 1, 0},       //MUX4: TMR6_CH3_1      // SEL: 0
            {5, 0, 0, 0},           //MUX5: WDT4_RESET_N_2
            {6, 0, 0, 0},           //MUX6: PWM4_EXT_CLK_1
            {7, 0, 0, 0},           //MUX7: DFM_DDRPHY_BYPASS_IN_DATA_SEL_1_1
        },
    }, // end of PORT 75

    {
        // 76  -   GPIO_D12
        0x70,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x270,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO28_1
            {1, 0x4f0, 2, 0},       //MUX1: I2C15_SCL_1         // SEL: 0
            {2, 0, 0, 0},           //MUX2: USB1_PWR_4
            {3, 0, 0, 0},           //MUX3: CANFD7_TX_2
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_I2S_MCLK2_2
            {5, 0x440, 2, 1},       //MUX5: PCIE1_CLKREQ_N_2/PCIEX2 input source // SEL: 1
            {6, 0, 0, 0},           //MUX6: PWM6_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_DDRPHY_BYPASS_IN_DATA_SEL_2_1
        },
    }, // end of PORT 76

    {
        // 77  -   GPIO_D13
        0x74,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x274,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO29_1
            {1, 0x4f4, 2, 0},       //MUX1: I2C15_SDA_1     // SEL: 0
            {2, 0x41c, 3, 3},       //MUX2: USB1_OC_4       // SEL: 3
            {3, 0x484, 2, 1},       //MUX3: CANFD7_RX_2     // SEL 1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_CSI_MCLK1_2
            {5, 0x44c, 2, 1},       //MUX5: PCIE2_CLKREQ_N_2/PCIEX1 input source  // SEL: 1
            {6, 0, 0, 0},           //MUX6: PWM6_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA0_1
        },
    }, // end of PORT 77

    {
        // 78  -   GPIO_D14
        0x78,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x278,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO30_1
            {1, 0x4f8, 2, 0},       //MUX1: I2C16_SCL_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: USB2_PWR_4
            {3, 0, 0, 0},           //MUX3: _CANFD8_TX_2
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_I2S_MCLK3_2
            {5, 0, 0, 0},           //MUX5: OSPI2_ECC_FAIL_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D0_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA1_1
        },
    }, // end of PORT 78

    {
        // 79  -   GPIO_D15
        0x7c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x27c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO31_1
            {1, 0x4fc, 2, 0},       //MUX1: I2C16_SDA_1     // SEL: 0
            {2, 0x434, 3, 3},       //MUX2: USB2_OC_4       // SEL: 3
            {3, 0x498, 2, 1},       //MUX3: CANFD8_RX_2     // SEL: 1
            {4, 0, 0, 0},           //MUX4: CKGEN_SEC_CSI_MCLK2_2
            {5, 0, 0, 0},           //MUX5: OSPI2_INTB_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D1_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA2_1
        },
    }, // end of PORT 79

    {
        // 80  -   OSPI2_SCLK
        0x80,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x280,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO32_1
            {1, 0, 0, 0},           //MUX1: OSPI2_SCLK_1
            {2, 0x460, 2, 1},       //MUX2: SPI5_SCLK_2     // SEL: 1
            {3, 0, 0, 0},           //MUX3: SHC2_CARD_DET_N_1
            {4, 0x500, 2, 0},       //MUX4: I2S_MC2_SCKO_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_CLK_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D2_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA3_1
        },
    }, // end of PORT 80

    {
        // 81  -   OSPI2_SS0
        0x84,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x284,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO33_1
            {1, 0, 0, 0},           //MUX1: OSPI2_SS0_1
            {2, 0x470, 2, 1},       //MUX2: SPI5_SS_2       // SEL: 1
            {3, 0, 0, 0},           //MUX3: MSHC2_WP_1
            {4, 0x504, 2, 0},       //MUX4: I2S_MC2_WSO_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_HSYNC_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D3_1
            {7, 0, 0, 0},           //MUX7: DFM_PHY_BYPASSINDATA4_1
        },
    }, // end of PORT 81

    {
        // 82  -   OSPI2_DATA0
        0x88,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x288,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO34_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA0_1
            {2, 0x464, 2, 1},       //MUX2: SPI5_MISO_2          // SEL: 1
            {3, 0, 0, 0},           //MUX3: MSHC2_VOLT_SW_1
            {4, 0x508, 2, 0},       //MUX4: I2S_MC2_SDI0_SDO0_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_VSYNC_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D4_1
            {7, 0, 0, 0},           //MUX7: DFM_CKGEN_SAF_TESTOUT_1
        },
    }, // end of PORT 82

    {
        // 83  -   OSPI2_DATA1
        0x8c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x28c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO35_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA1_1
            {2, 0x46c, 2, 1},       //MUX2: SPI5_MOSI_2         // SEL: 1
            {3, 0, 0, 0},           //MUX3: MSHC2_LED_CTRL_1
            {4, 0x50c, 2, 0},       //MUX4: I2S_MC2_SDI1_SDO1_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_ENABLE_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D5_1
            {7, 0, 0, 0},           //MUX7: DFM_CKGEN_SEC_TESTOUT_1
        },
    }, // end of PORT 83

    {
        // 84  -   OSPI2_DATA2
        0x90,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x290,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO36_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA2_1
            {2, 0x454, 3, 4},       //MUX2: ENET2_MDIO_5        // SEL: 4
            {3, 0x4bc, 1, 1},       //MUX3: I2C12_SCL_2         // SEL: 1
            {4, 0x510, 2, 0},       //MUX4: I2S_MC2_SDI2_SDO2_1 // SEL: 0
            {5, 0, 0, 0},           //MUX5: _CSI3_CH0_DATA0_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D6_1
            {7, 0, 0, 0},           //MUX7: DFM_CKGEN_HPI_TESTOUT_1
        },
    }, // end of PORT 84

    {
        // 85  -   OSPI2_DATA3
        0x94,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x294,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO37_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA3_1
            {2, 0, 0, 0},           //MUX2: ENET2_MDC_5
            {3, 0x4c8, 1, 1},       //MUX3: I2C12_SDA_2            // SEL: 1
            {4, 0x514, 2, 0},       //MUX4: I2S_MC2_SDI3_SDO3_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA1_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D7_1
            {7, 0, 0, 0},           //MUX7: DFM_CKGEN_DISP_TESTOUT_1
        },
    }, // end of PORT 85

    {
        // 86  -   OSPI2_DATA4
        0x98,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x298,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO38_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA4_1
            {2, 0, 0, 0},           //MUX2: UART11_TX_2
            {3, 0, 0, 0},           //MUX3: MSHC1_CARD_DET_N_1
            {4, 0x518, 2, 0},       //MUX4: I2S_MC2_SDI4_SDO4_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA2_1
            {6, 0, 0, 0},           //MUX6: MSHC4_CLK_1
            {7, 0, 0, 0},           //MUX7: DFM_RTCXTAL_FOUT_1
        },
    }, // end of PORT 86

    {
        // 87  -   OSPI2_DATA5
        0x9c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x29c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO39_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA5_1
            {2, 0x468, 2, 1},       //MUX2: _UART11_RX_2           // SEL: 1
            {3, 0, 0, 0},           //MUX3: MSHC1_WP_1
            {4, 0x51c, 2, 0},       //MUX4: I2S_MC2_SDI5_SDO5_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA3_1
            {6, 0, 0, 0},           //MUX6: MSHC4_CMD_1
            {7, 0, 0, 0},           //MUX7: DFM_RTC2M_FOUT_1
        },
    }, // end of PORT 87

    {
        // 88  -   OSPI2_DATA6
        0xa0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2a0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO40_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA6_1
            {2, 0, 0, 0},           //MUX2: UART11_CTS_1
            {3, 0, 0, 0},           //MUX3: _MSHC1_VOLT_SW_1
            {4, 0x520, 2, 0},       //MUX4: I2S_MC2_SDI6_SDO6_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA4_1
            {6, 0, 0, 0},           //MUX6: MSHC4_DATA0_1
            {7, 0, 0, 0},           //MUX7: DFM_XTAL24M_FOUT_1
        },
    }, // end of PORT 88

    {
        // 89  -   OSPI2_DATA7
        0xa4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2a4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO41_1
            {1, 0, 0, 0},           //MUX1: OSPI2_DATA7_1
            {2, 0, 0, 0},           //MUX2: UART11_RTS_1
            {3, 0, 0, 0},           //MUX3: MSHC1_LED_CTRL_1
            {4, 0x524, 2, 0},       //MUX4: I2S_MC2_SDI7_SDO7_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA5_1
            {6, 0, 0, 0},           //MUX6: MSHC4_DATA1_1
            {7, 0, 0, 0},           //MUX7: DFM_XTALSOC_FOUT_1
        },
    }, // end of PORT 89

    {
        // 90  -   OSPI2_DQS
        0xa8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2a8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO42_1
            {1, 0x528, 1, 0},       //MUX1: SPI2_DQS_1   // SEL: 0
            {2, 0, 0, 0},           //MUX2: USB1_PWR_5
            {3, 0, 0, 0},           //MUX3: USB2_PWR_5
            {4, 0x52c, 2, 0},       //MUX4: I2S_MC2_WSI_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA6_1
            {6, 0, 0, 0},           //MUX6: MSHC4_DATA2_1
            {7, 0, 0, 0},           //MUX7: DFM_RC24M_FOUT_1
        },
    }, // end of PORT 90

    {
        // 91  -   OSPI2_SS1
        0xac,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2ac,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO43_1
            {1, 0, 0, 0},           //MUX1: OSPI2_SS1_1
            {2, 0x41c, 3, 4},       //MUX2: USB1_OC_5       // SEL: 4
            {3, 0x434, 3, 4},       //MUX3: USB2_OC_5       // SEL: 4
            {4, 0x530, 2, 0},       //MUX4: I2S_MC2_SCKI_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH0_DATA7_1
            {6, 0, 0, 0},           //MUX6: MSHC4_DATA3_1
            {7, 0, 0, 0},           //MUX7: DFM_USB_PORTRESET_1
        },
    }, // end of PORT 91

    {
        // 92  -   RGMII2_TXC
        0xb0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2b0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO44_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXC_1
            {2, 0, 0, 0},           //MUX2: ENET2_AUS_IN_0_1
            {3, 0, 0, 0},           //MUX3: UART15_CTS_1
            {4, 0x534, 1, 0},       //MUX4: I2S_SC3_SCK_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_CLK_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D8_1
            {7, 0, 0, 0},           //MUX7: DFM_USB_ATERESET_1
        },
    }, // end of PORT 92

    {
        // 93  -   RGMII2_TXD0
        0xb4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2b4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO45_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXD0_1
            {2, 0, 0, 0},           //MUX2: TMR8_CH0_1
            {3, 0, 0, 0},           //MUX3: UART15_TX_2
            {4, 0x538, 1, 0},       //MUX4: I2S_SC3_WS_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_HSYNC_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D9_1
            {7, 0, 0, 0},           //MUX7: DFM_USB_PHY_RESET_1
        },
    }, // end of PORT 93

    {
        // 94  -   RGMII2_TXD1
        0xb8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2b8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO46_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXD1_1
            {2, 0, 0, 0},           //MUX2: MR8_CH1_1
            {3, 0x4d4, 2, 1},       //MUX3: UART15_RX_2  // SEL: 1
            {4, 0, 0, 0},           //MUX4: I2S_SC3_SDO_SDI_1
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_VSYNC_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D10_1
            {7, 0, 0, 0},           //MUX7: DFM_USB_PIPE_RESET_N_1
        },
    }, // end of PORT 94

    {
        // 95  -   RGMII2_TXD2
        0xbc,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2bc,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO47_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXD2_1
            {2, 0x454, 3, 5},       //MUX2: ENET2_MDIO_6        // SEL: 5
            {3, 0, 0, 0},           //MUX3: UART15_RTS_1
            {4, 0x53c, 2, 0},       //MUX4: I2S_SC3_SDI_SDO_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_ENABLE_1
            {6, 0, 0, 0},           //MUX6: CKGEN_SEC_I2S_MCLK2_3
            {7, 0, 0, 0},           //MUX7: DFM_PCIE_PHY_RESET_1
        },
    }, // end of PORT 95

    /* PORT 3 */

    {
        // 96  -   RGMII2_TXD3
        0xc0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2c0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO48_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXD3_1
            {2, 0, 0, 0},           //MUX2: ENET2_MDC_6
            {3, 0x438, 2, 1},       //MUX3: I2C7_SCL_2  // SEL: 1
            {4, 0, 0, 0},           //MUX4:
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA0_1
            {6, 0, 0, 0},           //MUX6: CKGEN_SEC_I2S_MCLK3_3
            {7, 0, 0, 0},           //MUX7: DFM_PCIE_DTB_OUT0_1
        },
    }, // end of PORT 96

    {
        // 97  -   RGMII2_TX_CTL
        0xc4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2c4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO49_1
            {1, 0, 0, 0},           //MUX1: ENET2_TXEN_1
            {2, 0, 0, 0},           //MUX2:TMR8_EXT_CLK_1
            {3, 0x444, 2, 1},       //MUX3: I2C7_SDA_2  // SEL: 1
            {4, 0, 0, 0},           //MUX4:
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA1_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D11_1
            {7, 0, 0, 0},           //MUX7: DFM_PCIE_DTB_OUT1_1
        },
    }, // end of PORT 97

    {
        // 98  -   RGMII2_RXC
        0xc8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2c8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO50_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXC_1
            {2, 0, 0, 0},           //MUX2: ENET2_CAP_COMP_0_1
            {3, 0x4d8, 2, 1},       //MUX3: UART16_CTS_2   // SEL: 1
            {4, 0x540, 1, 0},       //MUX4: I2S_SC4_SCK_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA2_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D12_1
            {7, 0, 0, 0},           //MUX7: DFM_EMMC_TEST_DLOUT_EN_1
        },
    }, // end of PORT 98

    {
        // 99  -   RGMII2_RXD0
        0xcc,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2cc,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO51_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXD0_1
            {2, 0, 0, 0},           //MUX2: TMR8_CH2_1
            {3, 0, 0, 0},           //MUX3: UART16_TX_2
            {4, 0x544, 1, 0},       //MUX4: I2S_SC4_WS_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA3_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D13_1
            {7, 0, 0, 0},           //MUX7: DFM_EMMC_PHY_TEST_INTF_SEL_1
        },
    }, // end of PORT 99

    {
        // 100 -   RGMII2_RXD1
        0xd0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2d0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO52_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXD1_1
            {2, 0, 0, 0},           //MUX2: TMR8_CH3_1
            {3, 0x4e8, 2, 1},       //MUX3: UART16_RX_2   // SEL: 1
            {4, 0, 0, 0},           //MUX4: I2S_SC4_SDO_SDI_1
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA4_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D14_1
            {7, 0, 0, 0},           //MUX7: DFM_EMMC_TEST_DL_CLK_1
        },
    }, // end of PORT 100

    {
        // 101 -   RGMII2_RXD2
        0xd4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2d4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO53_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXD2_1
            {2, 0, 0, 0},           //MUX2: ENET2_RMII_REF_CLK_1
            {3, 0, 0, 0},           //MUX3: UART16_RTS_2
            {4, 0x548, 2, 0},       //MUX4: I2S_SC4_SDI_SDO_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA5_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D15_1
            {7, 0, 0, 0},           //MUX7: DFM_EMMC2_JTAGIF_SEL_1
        },
    }, // end of PORT 101

    {
        // 102 -   RGMII2_RXD3
        0xd8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2d8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO54_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXD3_1
            {2, 0, 0, 0},           //MUX2: ENET2_RX_ER_1
            {3, 0x450, 2, 1},       //MUX3: I2C8_SCL_2  // SEL: 1
            {4, 0, 0, 0},           //MUX4: TMR4_EXT_CLK_1
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA6_1
            {6, 0, 0, 0},           //MUX6: OBSERVER_MUX2_D16_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_PHY_SEL_0_1
        },
    }, // end of PORT 102

    {
        // 103 -   RGMII2_RX_CTL
        0xdc,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2dc,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO55_1
            {1, 0, 0, 0},           //MUX1: ENET2_RXDV_1
            {2, 0, 0, 0},           //MUX2:
            {3, 0x45c, 2, 1},       //MUX3: I2C8_SDA_2   // SEL: 1
            {4, 0, 0, 0},           //MUX4: TMR5_EXT_CLK_1
            {5, 0, 0, 0},           //MUX5: CSI3_CH1_DATA7_1
            {6, 0, 0, 0},           //MUX6:
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_PHY_SEL_1_1
        },
    }, // end of PORT 103

    {
        // 104 -   I2S_SC3_SCK
        0xe0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2e0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO56_1
            {1, 0x534, 1, 1},       //MUX1: I2S_SC3_SCK_2  // SEL: 1
            {2, 0, 0, 0},           //MUX2: ADC_CSEL6_1
            {3, 0x478, 2, 1},       //MUX3: I2C13_SCL_2     // SEL: 1
            {4, 0x530, 2, 1},       //MUX4: I2S_MC2_SCKI_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_CLK_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACECLK_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_PHY_SEL_2_1
        },
    }, // end of PORT 104

    {
        // 105 -   I2S_SC3_WS
        0xe4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2e4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO57_1
            {1, 0x538, 1, 1},       //MUX1: I2S_SC3_WS_2    // SEL: 1
            {2, 0, 0, 0},           //MUX2: ADC_CSEL5_1
            {3, 0x480, 2, 1},       //MUX3: I2C13_SDA_2     // SEL: 1
            {4, 0x52c, 2, 1},       //MUX4: I2S_MC2_WSI_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_HSYNC_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACECTRL_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_SHUTDOWNZ_1
        },
    }, // end of PORT 105

    {
        // 106 -   I2S_SC3_SD
        0xe8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2e8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO58_1
            {1, 0, 0, 0},           //MUX1: I2S_SC3_SDO_SDI_2
            {2, 0x548, 2, 1},       //MUX2: I2S_SC4_SDI_SDO_2  // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART13_TX_2
            {4, 0x508, 2, 1},       //MUX4: I2S_MC2_SDI0_SDO0_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_VSYNC_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA0_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_RSTZ_1
        },
    }, // end of PORT 106

    {
        // 107 -   I2S_SC4_SCK
        0xec,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2ec,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO59_1
            {1, 0x540, 1, 1},       //MUX1: I2S_SC4_SCK_2   // SEL: 1
            {2, 0x528, 1, 1},       //MUX2: OSPI2_DQS_2     // SEL: 1
            {3, 0x478, 2, 2},       //MUX3: I2C13_SCL_3     // SEL: 2
            {4, 0x50c, 2, 1},       //MUX4: I2S_MC2_SDI1_SDO1_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_ENABLE_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA1_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_0_1
        },
    }, // end of PORT 107

    {
        // 108 -   I2S_SC4_WS
        0xf0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2f0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO60_1
            {1, 0x544, 1, 1},       //MUX1: I2S_SC4_WS_2    // SEL: 1
            {2, 0, 0, 0},           //MUX2: PWM5_EXT_CLK_1
            {3, 0x480, 2, 2},       //MUX3: I2C13_SDA_3     // SEL 2
            {4, 0x510, 2, 1},       //MUX4: I2S_MC2_SDI2_SDO2_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA0_CANFD9_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA2_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_1_1
        },
    }, // end of PORT 108

    {
        // 109 -   I2S_SC4_SD
        0xf4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2f4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO61_1
#if (TARGET_REFERENCE_D9 || TARGET_REFERENCE_D9P)
            {1, 0x548, 2, 3},       //MUX1: I2S_SC4_SDO_SDI_2
#else
            {1, 0, 0, 0},           //MUX1: I2S_SC4_SDO_SDI_2
#endif
            {2, 0x53c, 2, 1},       //MUX2: I2S_SC3_SDI_SDO_2   // SEL: 1
            {3, 0x4b4, 2, 1},       //MUX3: UART13_RX_2         // SEL: 1
            {4, 0x514, 2, 1},       //MUX4: I2S_MC2_SDI3_SDO3_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA1_CANFD9_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA3_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_2_1
        },
    }, // end of PORT 109

    {
        // 110 -   I2S_SC5_SCK
        0xf8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2f8,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO62_1
            {1, 0, 0, 0},           //MUX1: I2S_SC5_SCK_1
            {2, 0, 0, 0},           //MUX2: PWM6_EXT_CLK_1
            {3, 0x48c, 2, 1},       //MUX3: I2C14_SCL_2         // SEL 1
            {4, 0x518, 2, 1},       //MUX4: I2S_MC2_SDI4_SDO4_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA2_CANFD10_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA4_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_3_1
        },
    }, // end of PORT 110

    {
        // 111 -   I2S_SC5_WS
        0xfc,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x2fc,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO63_1
            {1, 0, 0, 0},           //MUX1: I2S_SC5_WS_1
            {2, 0, 0, 0},           //MUX2: PWM7_EXT_CLK_1
            {3, 0x494, 2, 1},       //MUX3: I2C14_SDA_2  // SEL 1
            {4, 0x51c, 2, 1},       //MUX4: I2S_MC2_SDI5_SDO5_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA3_CANFD10_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA5_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_4_1
        },
    }, // end of PORT 111

    {
        // 112 -   I2S_SC5_SD
        0x100,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x300,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO64_1
            {1, 0x550, 1, 1},       //MUX1: I2S_SC5_SDO_SDI_1  //  SEL: 1
            {2, 0x54c, 1, 0},       //MUX2: I2S_SC6_SDI_SDO_1  // SEL 0
            {3, 0, 0, 0},           //MUX3: PWM6_CH2_1
            {4, 0x520, 2, 1},       //MUX4: I2S_MC2_SDI6_SDO6_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA4_CANFD11_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA6_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_5_1
        },
    }, // end of PORT 112

    {
        // 113 -   I2S_SC6_SCK
        0x104,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x304,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO65_1
            {1, 0, 0, 0},           //MUX1: I2S_SC6_SCK_1
            {2, 0, 0, 0},           //MUX2: TMR6_EXT_CLK_1
            {3, 0x48c, 2, 2},       //MUX3: I2C14_SCL_3         // SEL 2
            {4, 0x524, 2, 1},       //MUX4: I2S_MC2_SDI7_SDO7_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA5_CANFD11_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA7_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_6_1
        },
    }, // end of PORT 113

    {
        // 114 -   I2S_SC6_WS
        0x108,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x308,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO66_1
            {1, 0, 0, 0},           //MUX1: I2S_SC6_WS_1
            {2, 0, 0, 0},           //MUX2: TMR7_EXT_CLK_1
            {3, 0x494, 2, 2},       //MUX3: I2C14_SDA_3             // SEL 2
            {4, 0x504, 2, 1},       //MUX4: I2S_MC2_WSO_2           // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA6_CANFD12_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA8_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDIN_7_1
        },
    }, // end of PORT 114

    {
        // 115 -   I2S_SC6_SD
        0x10c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x30c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO67_1
            {1, 0x54c, 1, 0},       //MUX1: I2S_SC6_SDO_SDI_1  // SEL: 0
            {2, 0x550, 1, 0},       //MUX2: I2S_SC5_SDI_SDO_1  // SEL: 0
            {3, 0, 0, 0},           //MUX3: PWM6_CH3_1
            {4, 0x500, 2, 1},       //MUX4: I2S_MC2_SCKO_2      // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA7_CANFD12_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA9_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_0_1
        },
    }, // end of PORT 115

    {
        // 116 -   I2S_SC7_SCK
        0x110,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x310,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO68_1
            {1, 0x554, 1, 0},       //MUX1: I2S_SC7_SCK_1       // SEL: 0
            {2, 0, 0, 0},           //MUX2: ADC_CSEL4_1
            {3, 0, 0, 0},           //MUX3: CKGEN_SEC_I2S_MCLK2_4
            {4, 0x558, 2, 0},       //MUX4: I2S_MC1_SCKO_1      // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA8_CANFD13_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA10_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_1_1
        },
    }, // end of PORT 116

    {
        // 117 -   I2S_SC7_WS
        0x114,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x314,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO69_1
            {1, 0x55c, 1, 0},       //MUX1: I2S_SC7_WS_1    // SEL: 0
            {2, 0, 0, 0},           //MUX2: ADC_CSEL3_1
            {3, 0, 0, 0},           //MUX3: CKGEN_SEC_I2S_MCLK3_4
            {4, 0x560, 2, 0},       //MUX4: I2S_MC1_WSO_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA9_CANFD13_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA11_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_2_1
        },
    }, // end of PORT 117

    {
        // 118 -   I2S_SC7_SD
        0x118,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x318,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO70_1
            {1, 0, 0, 0},           //MUX1: I2S_SC7_SDO_SDI_1
            {2, 0x564, 3, 0},       //MUX2: I2S_SC8_SDI_SDO_1  // SEL: 0
            {3, 0, 0, 0},           //MUX3: UART15_TX_3
            {4, 0x568, 2, 0},       //MUX4: I2S_MC1_SDI4_SDO4_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA10_CANFD14_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA12_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_3_1
        },
    }, // end of PORT 118

    {
        // 119 -   I2S_SC8_SCK
        0x11c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x31c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO71_1
            {1, 0x56c, 2, 0},       //MUX1: I2S_SC8_SCK_1       // SEL: 0
            {2, 0, 0, 0},           //MUX2: ADC_EXT_CLK_1
            {3, 0x4f0, 2, 1},       //MUX3: I2C15_SCL_2         // SEL: 1
            {4, 0x570, 2, 0},       //MUX4: I2S_MC1_SDI5_SDO5_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA11_CANFD14_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA13_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_4_1
        },
    }, // end of PORT 119

    {
        // 120 -   I2S_SC8_WS
        0x120,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x320,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO72_1
            {1, 0x574, 2, 0},       //MUX1: I2S_SC8_WS_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: PWM8_EXT_CLK_1
            {3, 0x4f4, 2, 1},       //MUX3: I2C15_SDA_2  // SEL: 1
            {4, 0x578, 2, 0},       //MUX4: I2S_MC1_SDI6_SDO6_1  // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA12_CANFD15_TX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA14_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_5_1
        },
    }, // end of PORT 120

    {
        // 121 -   I2S_SC8_SD
        0x124,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x324,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO73_1
            {1, 0, 0, 0},           //MUX1: I2S_SC8_SDO_SDI_1
            {2, 0x57c, 2, 0},       //MUX2: I2S_SC7_SDI_SDO_1  // SEL: 0
            {3, 0x4d4, 2, 2},       //MUX3: UART15_RX_3         // SEL: 2
            {4, 0x580, 2, 0},       //MUX4: I2S_MC1_SDI7_SDO7_1   // SEL: 0
            {5, 0, 0, 0},           //MUX5: DISP_DATA13_CANFD15_RX_1
            {6, 0, 0, 0},           //MUX6: CSSYS_TRACEDATA15_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_6_1
        },
    }, // end of PORT 121

    {
        // 122 -   I2S_MC_SCK
        0x128,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x328,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO74_1
            {1, 0x558, 2, 1},       //MUX1: I2S_MC1_SCKO_2  // SEL: 1
            {2, 0x584, 2, 0},       //MUX2: I2S_MC1_SCKI_1   // SEL: 0
            {3, 0x4f0, 2, 2},       //MUX3: I2C15_SCL_3     // SEL: 2
            {4, 0x56c, 2, 1},       //MUX4: I2S_SC8_SCK_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA14_CANFD16_TX_1
            {6, 0, 0, 0},           //MUX6: PWM7_CH0_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTDOUT_7_1
        },
    }, // end of PORT 122

    {
        // 123 -   I2S_MC_WS
        0x12c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x32c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO75_1
            {1, 0x560, 2, 1},       //MUX1: I2S_MC1_WSO_2  // SEL: 1
            {2, 0x588, 2, 0},       //MUX2: I2S_MC1_WSI_1  // SEL: 0
            {3, 0x4f4, 2, 2},       //MUX3: I2C15_SDA_3  // SEL: 2
            {4, 0x574, 2, 1},       //MUX4: I2S_SC8_WS_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA15_CANFD16_RX_1
            {6, 0, 0, 0},           //MUX6: PWM7_CH1_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTEN_1
        },
    }, // end of PORT 123

    {
        // 124 -   I2S_MC_SD0
        0x130,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x330,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO76_1
            {1, 0x58c, 1, 0},       //MUX1: I2S_MC1_SDI0_SDO0_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: CKGEN_SEC_I2S_MCLK2_5
            {3, 0, 0, 0},           //MUX3: TMR7_CH0_1
            {4, 0x564, 3, 1},       //MUX4: I2S_SC8_SDI_SDO_2   // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA16_CANFD17_TX_1
            {6, 0, 0, 0},           //MUX6: PWM7_CH2_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTCLK_1
        },
    }, // end of PORT 124

    {
        // 125 -   I2S_MC_SD1
        0x134,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x334,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO77_1
            {1, 0x590, 1, 0},       //MUX1: I2S_MC1_SDI1_SDO1_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: CKGEN_SEC_I2S_MCLK3_5
            {3, 0, 0, 0},           //MUX3: TMR7_CH1_1
            {4, 0, 0, 0},           //MUX4: I2S_SC8_SDO_SDI_2
            {5, 0, 0, 0},           //MUX5: DISP_DATA17_CANFD17_RX_1
            {6, 0, 0, 0},           //MUX6: PWM7_CH3_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TESTCLR_1
        },
    }, // end of PORT 125

    {
        // 126 -   I2S_MC_SD2
        0x138,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x338,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO78_1
            {1, 0x594, 1, 0},       //MUX1: I2S_MC1_SDI2_SDO2_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: SPDIF1_IN_1
            {3, 0x4f8, 2, 1},       //MUX3: I2C16_SCL_2         // SEL: 1
            {4, 0, 0, 0},           //MUX4: TMR7_CH2_1
            {5, 0, 0, 0},           //MUX5: DISP_DATA18_CANFD18_TX_1
            {6, 0, 0, 0},           //MUX6: CKGEN_SEC_CSI_MCLK1_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_EN_1
        },
    }, // end of PORT 126

    {
        // 127 -   I2S_MC_SD3
        0x13c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x33c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO79_1
            {1, 0x598, 1, 0},       //MUX1: I2S_MC1_SDI3_SDO3_1  // SEL: 0
            {2, 0, 0, 0},           //MUX2: SPDIF1_OUT_1
            {3, 0x4fc, 2, 1},       //MUX3: I2C16_SDA_2         // SEL: 1
            {4, 0, 0, 0},           //MUX4: TMR7_CH3_1
            {5, 0, 0, 0},           //MUX5: DISP_DATA19_CANFD18_RX_1
            {6, 0, 0, 0},           //MUX6: CKGEN_SEC_CSI_MCLK2_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_0_1
        },
    }, // end of PORT 127

    /* PORT 4 */

    {
        // 128 -   I2S_MC_SD4
        0x140,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x340,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO80_1
            {1, 0x568, 2, 1},       //MUX1: I2S_MC1_SDI4_SDO4_2  // SEL: 1
            {2, 0, 0, 0},           //MUX2: SPDIF2_IN_1
            {3, 0x44c, 2, 2},       //MUX3: PCIE2_CLKREQ_N_3/PCIEX1 input source  // SEL: 2
            {4, 0x4a0, 2, 1},       //MUX4: SPI8_SCLK_2         // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA20_CANFD19_TX_1
            {6, 0, 0, 0},           //MUX6: PU_DBG1_TRST_N_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_1_1
        },
    }, // end of PORT 128

    {
        // 129 -   I2S_MC_SD5
        0x144,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x344,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO81_1
            {1, 0x570, 2, 1},       //MUX1: I2S_MC1_SDI5_SDO5_2  // SEL: 1
            {2, 0, 0, 0},           //MUX2: SPDIF2_OUT_1
            {3, 0x440, 2, 2},       //MUX3: PCIE1_CLKREQ_N_3/PCIEX2 input source    // SEL: 2
            {4, 0x4a4, 2, 1},       //MUX4: SPI8_MISO_2         // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA21_CANFD19_RX_1
            {6, 0, 0, 0},           //MUX6: PU_DBG1_TMS_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_2_1
        },
    }, // end of PORT 129

    {
        // 130 -   I2S_MC_SD6
        0x148,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x348,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO82_1
            {1, 0x578, 2, 1},       //MUX1: I2S_MC1_SDI6_SDO6_2     // SEL: 1
            {2, 0x584, 2, 1},       //MUX2: I2S_MC1_SCKI_2          // SEL: 1
            {3, 0x4f8, 2, 2},       //MUX3: I2C16_SCL_3             // SEL: 2
            {4, 0x4a8, 2, 1},       //MUX4: SPI8_MOSI_2             // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA22_CANFD20_TX_1
            {6, 0, 0, 0},           //MUX6: PU_DBG1_TDI_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_3_1
        },
    }, // end of PORT 130

    {
        // 131 -   I2S_MC_SD7
        0x14c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x34c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO83_1
            {1, 0x580, 2, 1},       //MUX1: I2S_MC1_SDI7_SDO7_2  // SEL: 1
            {2, 0x588, 2, 1},       //MUX2: I2S_MC1_WSI_2       // SEL: 1
            {3, 0x4fc, 2, 2},       //MUX3: I2C16_SDA_3         // SEL: 2
            {4, 0x4b0, 2, 1},       //MUX4: SPI8_SS_2           // SEL: 1
            {5, 0, 0, 0},           //MUX5: DISP_DATA23_CANFD20_RX_1
            {6, 0, 0, 0},           //MUX6: PU_DBG1_JTAG_TCK_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_4_1
        },
    }, // end of PORT 131

    {
        // 132 -   EMMC1_CLK
        0x150,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x350,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO84_1
            {1, 0, 0, 0},           //MUX1: MSHC1_CLK_1
            {2, 0x4cc, 1, 1},       //MUX2: SPI7_SCLK_2     // SEL: 1
            {3, 0x4b8, 1, 1},       //MUX3: UART14_CTS_2    // SEL: 1
            {4, 0x558, 2, 2},       //MUX4: I2S_MC1_SCKO_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: UART16_TX_3
            {6, 0, 0, 0},           //MUX6: PU_DBG1_TDO_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_5_1
        },
    }, // end of PORT 132

    {
        // 133 -   EMMC1_CMD
        0x154,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x354,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO85_1
            {1, 0, 2, 1},           //MUX1: MSHC1_CMD_1
            {2, 0x4d0, 1, 1},       //MUX2: SPI7_MISO_2     // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART14_TX_2
            {4, 0x560, 2, 2},       //MUX4: I2S_MC1_WSO_3   // SEL: 2
            {5, 0x4e8, 2, 2},       //MUX5: UART16_RX_3     // SEL: 2
            {6, 0, 0, 0},           //MUX6: PU_DBG2_TRST_N_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_6_1
        },
    }, // end of PORT 133

    {
        // 134 -   EMMC1_DATA0
        0x158,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x358,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO86_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_0_1
            {2, 0x4dc, 1, 1},       //MUX2: SPI7_MOSI_2         // SEL: 1
            {3, 0x4c4, 1, 1},       //MUX3: UART14_RX_2         // SEL: 1
            {4, 0x58c, 1, 1},       //MUX4: I2S_MC1_SDI0_SDO0_2  // SEL: 1
            {5, 0x4d8, 2, 2},       //MUX5: UART16_CTS_3        // SEL: 2
            {6, 0, 0, 0},           //MUX6: PU_DBG2_TMS_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_7_1
        },
    }, // end of PORT 134

    {
        // 135 -   EMMC1_DATA1
        0x15c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x35c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO87_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_1_1
            {2, 0x4e4, 1, 1},       //MUX2: SPI7_SS_2           // SEL: 1
            {3, 0, 0, 0},           //MUX3: UART14_RTS_2
            {4, 0x590, 1, 1},       //MUX4: I2S_MC1_SDI1_SDO1_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: UART16_RTS_3
            {6, 0, 0, 0},           //MUX6: PU_DBG2_TDI_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_8_1
        },
    }, // end of PORT 135

    {
        // 136 -   EMMC1_DATA2
        0x160,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x360,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO88_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_2_1
            {2, 0, 0, 0},           //MUX2: UART11_TX_3
            {3, 0x400, 1, 1},       //MUX3: I2C5_SCL_2              // SEL: 1
            {4, 0x594, 1, 1},       //MUX4: I2S_MC1_SDI2_SDO2_2     // SEL: 1
            {5, 0x4ac, 1, 1},       //MUX5: SPDIF3_IN_2             // SEL: 1
            {6, 0, 0, 0},           //MUX6: PU_DBG2_JTAG_TCK_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_9_1
        },
    }, // end of PORT 136

    {
        // 137 -   EMMC1_DATA3
        0x164,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x364,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO89_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_3_1
            {2, 0x468, 2, 2},       //MUX2: UART11_RX_3         // SEL: 2
            {3, 0x40c, 1, 1},       //MUX3: I2C5_SDA_2          // SEL: 1
            {4, 0x598, 1, 1},       //MUX4: I2S_MC1_SDI3_SDO3_2  // SEL: 1
            {5, 0, 0, 0},           //MUX5: SPDIF3_OUT_2
            {6, 0, 0, 0},           //MUX6: PU_DBG2_TDO_1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_CONT_DATA_10_1
        },
    }, // end of PORT 137

    {
        // 138 -   EMMC1_DATA4
        0x168,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x368,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO90_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_4_1
            {2, 0x4a0, 2, 2},       //MUX2: SPI8_SCLK_3             // SEL: 2
            {3, 0, 0, 0},           //MUX3: UART13_CTS_1
            {4, 0x568, 2, 2},       //MUX4: I2S_MC1_SDI4_SDO4_3     // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM8_CH0_1
            {6, 0x4e0, 1, 1},       //MUX6: TMR6_CH2_2              // SEL: 1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_STOPSTATEDATA_0_1
        },
    }, // end of PORT 138

    {
        // 139 -   EMMC1_DATA5
        0x16c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x36c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO91_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_5_1
            {2, 0x4a4, 2, 2},       //MUX2: SPI8_MISO_3         // SEL: 2
            {3, 0, 0, 0},           //MUX3: UART13_TX_3
            {4, 0x570, 2, 2},       //MUX4: I2S_MC1_SDI5_SDO5_3   // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM8_CH1_1
            {6, 0x4ec, 1, 1},       //MUX6: TMR6_CH3_2             // SEL: 1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_STOPSTATEDATA_1_1
        },
    }, // end of PORT 139

    {
        // 140 -   EMMC1_DATA6
        0x170,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x370,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO92_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_6_1
            {2, 0x4a8, 2, 2},       //MUX2: SPI8_MOSI_3         // SEL: 2
            {3, 0x4b4, 2, 2},       //MUX3: UART13_RX_3         // SEL: 2
            {4, 0x578, 2, 2},       //MUX4: I2S_MC1_SDI6_SDO6_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM8_CH2_1
            {6, 0x564, 3, 2},       //MUX6: I2S_SC8_SDI_SDO_3   // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_STOPSTATEDATA_2_1
        },
    }, // end of PORT 140

    {
        // 141 -   EMMC1_DATA7
        0x174,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x374,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO93_1
            {1, 0, 0, 0},           //MUX1: MSHC1_DATA_7_1
            {2, 0x4b0, 2, 2},       //MUX2: SPI8_SS_3           // SEL: 2
            {3, 0, 0, 0},           //MUX3: UART13_RTS_1
            {4, 0x580, 2, 2},       //MUX4: I2S_MC1_SDI7_SDO7_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM8_CH3_1
            {6, 0x56c, 2, 2},       //MUX6: I2S_SC8_SCK_3       // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_STOPSTATEDATA_3_1
        },
    }, // end of PORT 141

    {
        // 142 -   EMMC1_STROBE
        0x178,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x378,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO94_1
            {1, 0, 0, 0},           //MUX1: MSHC1_STB_1
            {2, 0, 0, 0},           //MUX2: UART12_TX_2
            {3, 0x420, 1, 1},       //MUX3: I2C6_SCL_2          // SEL: 1
            {4, 0x588, 2, 2},       //MUX4: I2S_MC1_WSI_3       // SEL: 2
            {5, 0x4c0, 1, 1},       //MUX5: SPDIF4_IN_2         // SEL: 1
            {6, 0x574, 2, 2},       //MUX6: I2S_SC8_WS_3        // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_STOPSTATECLK_1
        },
    }, // end of PORT 142

    {
        // 143 -   EMMC1_RESET_N
        0x17c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x37c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO95_1
            {1, 0, 0, 0},           //MUX1: MSHC1_RST_N_1
            {2, 0x474, 1, 1},       //MUX2: UART12_RX_2         // SEL: 1
            {3, 0x428, 1, 1},       //MUX3: I2C6_SDA_2          // SEL: 1
            {4, 0x584, 2, 2},       //MUX4: I2S_MC1_SCKI_3      // SEL: 2
            {5, 0, 0, 0},           //MUX5: SPDIF4_OUT_2
            {6, 0, 0, 0},           //MUX6: I2S_SC8_SDO_SDI_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_FORCERXMODE_0_1
        },
    }, // end of PORT 143

    {
        // 144 -   EMMC2_CLK
        0x180,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x380,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO96_1
            {1, 0, 0, 0},           //MUX1: MSHC2_CLK_1
            {2, 0, 0, 0},           //MUX2: USB1_PWR_6
            {3, 0x438, 2, 2},       //MUX3: I2C7_SCL_3      // SEL: 2
            {4, 0x500, 2, 2},       //MUX4: I2S_MC2_SCKO_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM5_CH2_1
            {6, 0, 0, 0},           //MUX6: CANFD5_TX_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_FORCERXMODE_1_1
        },
    }, // end of PORT 144

    {
        // 145 -   EMMC2_CMD
        0x184,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x384,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO97_1
            {1, 0, 0, 0},           //MUX1: MSHC2_CMD_1
            {2, 0x41c, 3, 5},       //MUX2: USB1_OC_6       // SEL: 5
            {3, 0x444, 2, 2},       //MUX3: I2C7_SDA_3      // SEL: 2
            {4, 0x504, 2, 2},       //MUX4: I2S_MC2_WSO_3   // SEL: 2
            {5, 0, 0, 0},           //MUX5: PWM5_CH3_1
            {6, 0x414, 2, 2},       //MUX6: CANFD5_RX_3     // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_FORCERXMODE_2_1
        },
    }, // end of PORT 145

    {
        // 146 -   EMMC2_DATA0
        0x188,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x388,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO98_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_0_1
            {2, 0, 0, 0},           //MUX2: USB2_PWR_6
            {3, 0x404, 1, 1},       //MUX3: SPI6_SCLK_2          // SEL: 1
            {4, 0x508, 2, 2},       //MUX4: I2S_MC2_SDI0_SDO0_3  // SEL: 2
            {5, 0x47c, 2, 2},       //MUX5: PCIE2_PERST_N_3/PCIEX1 input source  // SEL 2
            {6, 0, 0, 0},           //MUX6: CANFD6_TX_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_RX_FORCERXMODE_3_1
        },
    }, // end of PORT 146

    {
        // 147 -   EMMC2_DATA1
        0x18c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x38c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO99_1
            {1, 0, 2, 1},           //MUX1: MSHC2_DATA_1_1
            {2, 0x434, 3, 5},       //MUX2: USB2_OC_6           // SEL: 5
            {3, 0x410, 1, 1},       //MUX3: SPI6_MISO_2         // SEL: 1
            {4, 0x50c, 2, 2},       //MUX4: I2S_MC2_SDI1_SDO1_3  // SEL:2
            {5, 0x488, 2, 2},       //MUX5: PCIE2_WAKE_N_3/PCIEX1 input souce  // SEL: 2
            {6, 0x430, 2, 2},       //MUX6: CANFD6_RX_3          // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_ENABLE_0_1
        },
    }, // end of PORT 147

    {
        // 148 -   EMMC2_DATA2
        0x190,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x390,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO100_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_2_1
            {2, 0, 0, 0},           //MUX2: I2C15_SCL_4
            {3, 0x424, 1, 1},       //MUX3: SPI6_MOSI_2         // SEL: 1
            {4, 0x510, 2, 2},       //MUX4: I2S_MC2_SDI2_SDO2_3  // SEL: 2
            {5, 0x490, 2, 2},       //MUX5: PCIE1_PERST_N_3/PCIEX2 input souce     // SEL: 2
            {6, 0, 0, 0},           //MUX6: CANFD7_TX_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_ENABLE_1_1
        },
    }, // end of PORT 148

    {
        // 149 -   EMMC2_DATA3
        0x194,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x394,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO101_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_3_1
            {2, 0x4f4, 2, 3},       //MUX2: I2C15_SDA_4         // SEL: 3
            {3, 0x42c, 1, 1},       //MUX3: SPI6_SS_2           // SEL: 1
            {4, 0x514, 2, 2},       //MUX4: I2S_MC2_SDI3_SDO3_3  // SEL: 2
            {5, 0x49c, 2, 2},       //MUX5: PCIE1_WAKE_N_3/PCIEX2 input source      // SEL 2
            {6, 0x484, 2, 2},       //MUX6: CANFD7_RX_3         // SEL 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_ENABLE_2_1
        },
    }, // end of PORT 149

    {
        // 150 -   EMMC2_DATA4
        0x198,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x398,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO102_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_4_1
            {2, 0, 0, 0},           //MUX2: MSHC3_DATA_3_1
            {3, 0x460, 2, 2},       //MUX3: SPI5_SCLK_3         // SEL: 2
            {4, 0x518, 2, 2},       //MUX4: I2S_MC2_SDI4_SDO4_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: USB1_PWR_7
            {6, 0, 0, 0},           //MUX6: CANFD8_TX_3
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_ENABLE_3_1
        },
    }, // end of PORT 150

    {
        // 151 -   EMMC2_DATA5
        0x19c,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x39c,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO103_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_5_1
            {2, 0, 0, 0},           //MUX2: MSHC3_DATA_2_1
            {3, 0x464, 2, 2},       //MUX3: SPI5_MISO_3         // SEL: 2
            {4, 0x51c, 2, 2},       //MUX4: I2S_MC2_SDI5_SDO5_3  // SEL: 2
            {5, 0x41c, 3, 6},       //MUX5: USB1_OC_7           // SEL: 6
            {6, 0x498, 2, 2},       //MUX6: CANFD8_RX_3         // SEL: 2
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_ENABLECLK_1
        },
    }, // end of PORT 151

    {
        // 152 -   EMMC2_DATA6
        0x1a0,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x3a0,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO104_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_6_1
            {2, 0, 0, 0},           //MUX2: MSHC3_DATA_1_1
            {3, 0x46c, 2, 2},       //MUX3: SPI5_MOSI_3         // SEL: 2
            {4, 0x520, 2, 2},       //MUX4: I2S_MC2_SDI6_SDO6_3  // SEL: 2
            {5, 0, 0, 0},           //MUX5: USB2_PWR_7
            {6, 0x57c, 2, 1},       //MUX6: I2S_SC7_SDI_SDO_2   // SEL: 1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_BASEDIR_0_1
        },
    }, // end of PORT 152

    {
        // 153 -   EMMC2_DATA7
        0x1a4,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x3a4,  // pin_mux_config,
        {
            {0, 0, 0, 0},           //MUX0: GPIO_MUX2_IO105_1
            {1, 0, 0, 0},           //MUX1: MSHC2_DATA_7_1
            {2, 0, 0, 0},           //MUX2: MSHC3_DATA_0_1
            {3, 0x470, 2, 2},       //MUX3: SPI5_SS_3           // SEL: 2
            {4, 0x524, 2, 2},       //MUX4: I2S_MC2_SDI7_SDO7_3  // SEL: 2
            {5, 0x434, 3, 6},       //MUX5: USB2_OC_7           // SEL: 6
            {6, 0x554, 1, 1},       //MUX6: I2S_SC7_SCK_2       // SEL: 1
            {7, 0, 0, 0},           //MUX7: DFM_MIPI_TX_TXCLKLOCK_1
        },
    }, // end of PORT 153

    {
        // 154 -   EMMC2_STROBE
        0x1a8,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x3a8,  // pin_mux_config,
        {
            {0, 0, 0, 0},               //MUX0: GPIO_MUX2_IO106_1
            {1, 0, 0, 0},               //MUX1: MSHC2_STB_1
            {2, 0, 0, 0},               //MUX2: MSHC3_CLK_1
            {3, 0x450, 2, 2},           //MUX3: I2C8_SCL_3          // SEL: 2
            {4, 0x52c, 2, 2},           //MUX4: I2S_MC2_WSI_3       // SEL: 2
            {5, 0x440, 2, 3},           //MUX5: PCIE1_CLKREQ_N_4/PCIEX2 input source    // SEL: 3
            {6, 0x55c, 1, 1},           //MUX6: I2S_SC7_WS_2        // SEL: 1
            {7, 0, 0, 0},               //MUX7: DFM_MIPI_TX_BISTON_1
        },
    }, // end of PORT 154

    {
        // 155 -   EMMC2_RESET_N
        0x1ac,  // pad_config, IOMUX_CTRL_AP_APB_IOMUX_CTRL_AP_AB0_BASE_ADDR
        0x3ac,  // pin_mux_config,
        {
            {0, 0, 0, 0},               //MUX0: GPIO_MUX2_IO107_1
            {1, 0, 0, 0},               //MUX1: MSHC2_RST_N_1
            {2, 0, 0, 0},               //MUX2: MSHC3_CMD_1
            {3, 0x45c, 2, 2},           //MUX3: I2C8_SDA_3          // SEL: 2
            {4, 0x530, 2, 2},           //MUX4: I2S_MC2_SCKI_3      // SEL: 2
            {5, 0x44c, 2, 3},           //MUX5: PCIE2_CLKREQ_N_4/PCIEX1 input source // SEL: 3
            {6, 0, 0, 0},               //MUX6: I2S_SC7_SDO_SDI_2
            {7, 0, 0, 0},               //MUX7: DFM_MIPI_TX_CLKOUT_GP_1
        },
    }, // end of PORT 155


};  // end of SEMIDRIVE_X9_PIN_DEF x9_pins[156]




#endif  /* __REGS_IOMUX_H */
