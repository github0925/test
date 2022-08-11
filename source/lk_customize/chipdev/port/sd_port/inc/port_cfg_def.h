
#ifndef PORT_CFG_DEF_H
#define PORT_CFG_DEF_H



/* AUTOSAR specification version numbers */
#define PORT_AR_RELEASE_MAJOR_VERSION  (4U)
#define PORT_AR_RELEASE_MINOR_VERSION  (4U)
#define PORT_AR_RELEASE_REVISION_VERSION (0U)


/* Vendor specific implementation version information */
#define PORT_SW_MAJOR_VERSION  (0U)
#define PORT_SW_MINOR_VERSION  (1U)
#define PORT_SW_PATCH_VERSION  (0U)

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/


/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/*
                     Container : PortGeneralConfiguration
*/
/*
Configuration: PORT_DEV_ERROR_DETECT
- if STD_ON, DET is Enabled
- if STD_OFF, DET is Disabled
*/
#define PORT_DEV_ERROR_DETECT       (STD_OFF)

/*
Configuration: PORT_VERSION_INFO_API
- if STD_ON,  Function Port_GetVersionInfo is available
- if STD_OFF, Function Port_GetVersionInfo is not available
*/
#define PORT_VERSION_INFO_API       (STD_ON)


/*
Configuration: PORT_SET_PIN_MODE_API
- if STD_ON,  Function Port_SetPinMode is available
- if STD_OFF, Function Port_SetPinMode is not available
*/

#define PORT_SET_PIN_MODE_API       (STD_ON)

/*
Configuration: PORT_SET_PIN_DIRECTION_API
- if STD_ON,  Function Port_SetPinDirection is available
- if STD_OFF, Function Port_SetPinDirection is not available
*/

#define PORT_SET_PIN_DIRECTION_API  (STD_ON)

/*
Configuration: PORT_INIT_API_MODE:
-User Mode Macro for Init API
*/
#define PORT_INIT_API_MODE                                    (PORT_MCAL_SUPERVISOR)


/*
Configuration: PortSafetyEnable
- if STD_ON, Safety is Enabled
- if STD_OFF, Safety  is Disabled
*/
#define PORT_SAFETY_ENABLE    (STD_OFF)


/*
Configuration: PortInitCheckApi
- if STD_ON, PortInitCheckApi is available
- if STD_OFF, PortInitCheckApi is not available
*/
#define PORT_INIT_CHECK_API    (STD_OFF)


/* Definition to specify the ports available on the microcontroller
   Bit value = 0 implies the port is not available
   Bit value = 1 implies the port is available
   Bit 0 is for Port 0, Bit 1 is for Port 1, ... , Bit 31 is for Port 31
*/
#define PORTS_AVAILABLE_00_04       (0x01FU)


/* Definition to specify the ports that are read only ports on the
   microcontroller
   Bit value = 0 implies the port readable/writable
   Bit value = 1 implies the port is read only port
   Bit 0 is for Port 0, Bit 1 is for Port 1, ... , Bit 31 is for Port 31
*/
#define PORTS_READONLY_00_04       (0x00U)

/* Maximum Port Number */
#define PORT_MAX_NUMBER             (4U)

/* Maximum PortPinID */
#define PORT_MAX_PIN_ID             (0x28fU)

/* Max Port Number available */
#define PORT_TOTAL_AVAILABLE_PORTS   (4U)


/* Definitions to specify the pins available in the port
   Bit value = 0 implies the pin is not available
   Bit value = 1 implies the pin is available
   Bit 0 is for Pin 0, Bit 1 is for Pin 1, ... , Bit 31 is for Pin 31
*/
#define PORT_AVAILABLE_PINS_PORT0        ((uint32_t)0xffffffffU)
#define PORT_AVAILABLE_PINS_PORT1        ((uint32_t)0xffffffffU)
#define PORT_AVAILABLE_PINS_PORT2        ((uint32_t)0xffffffffU)
#define PORT_AVAILABLE_PINS_PORT3        ((uint32_t)0xffffffffU)
#define PORT_AVAILABLE_PINS_PORT4        ((uint32_t)0xffffffffU)


/* Definition to specify the ports that support PDISC on the
   microcontroller
   Bit value = 0 implies the port supports PDISC
   Bit value = 1 implies the port does not support PDISC
   Bit 0 is for Port 0, Bit 1 is for Port 1, ... , Bit 31 is for Port 31
*/
#define PORTS_PDISC_00_31       (0x00000007U)

/* Definition to specify the ports that PDISC on the
   microcontroller
   Bit value = 0 implies the port supports PDISC
   Bit value = 1 implies the port does not support PDISC
   Bit 0 is for Port 32, Bit 1 is for Port 33, ... , Bit 8 is for Port 40
*/
#define PORTS_PDISC_32_63       (0x00000106U)
/********************************************************************************
**                      Global Symbols                                        **
*******************************************************************************/
/*
  User Defined Symbolic names of all port pins Port_ConfigSet
*/


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
#define PortConf_PIN_GPIO_SAF     (Port_PinType )(160)





/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/

/*******************************************************************************
**                      Global Constant Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Variable Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Inline Function Definitions                    **
*******************************************************************************/
#endif /* PORT_CFG_DEF_H */

