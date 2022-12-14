/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

/*
 * Copied from testbench r1p6.
 * Do not manually update.
 */
#ifndef _CKGATE_H_
#define _CKGATE_H_

#define CKGEN_SAF_CKGATE_CR5_CORECLK_INDEX 0
#define CKGEN_SAF_CKGATE_NOC_SAF_INDEX 1
#define CKGEN_SAF_CKGATE_NOC_MAINB_INDEX 2
#define CKGEN_SAF_CKGATE_PPC_APBDECMUX1B_INDEX 2
#define CKGEN_SAF_CKGATE_MAC_RDC_INDEX 3
#define CKGEN_SAF_CKGATE_GIC1_INDEX 6
#define CKGEN_SAF_CKGATE_DMA1_INDEX 7
#define CKGEN_SAF_CKGATE_ENET1_INDEX 8
#define CKGEN_SAF_CKGATE_OSPI1_INDEX 9
#define CKGEN_SAF_CKGATE_IRAM1_INDEX 10
#define CKGEN_SAF_CKGATE_ROMC1_INDEX 11
#define CKGEN_SAF_CKGATE_EFUSEC_INDEX 12
#define CKGEN_SAF_CKGATE_CE1_INDEX 13
#define CKGEN_SAF_CKGATE_APB_ASYNC_CE1_INDEX 13
#define CKGEN_SAF_CKGATE_I2C1_INDEX 14
#define CKGEN_SAF_CKGATE_I2C2_INDEX 15
#define CKGEN_SAF_CKGATE_I2C3_INDEX 16
#define CKGEN_SAF_CKGATE_I2C4_INDEX 17
#define CKGEN_SAF_CKGATE_SPI1_INDEX 18
#define CKGEN_SAF_CKGATE_SPI2_INDEX 19
#define CKGEN_SAF_CKGATE_SPI3_INDEX 20
#define CKGEN_SAF_CKGATE_SPI4_INDEX 21
#define CKGEN_SAF_CKGATE_UART1_INDEX 22
#define CKGEN_SAF_CKGATE_UART2_INDEX 23
#define CKGEN_SAF_CKGATE_UART3_INDEX 24
#define CKGEN_SAF_CKGATE_UART4_INDEX 25
#define CKGEN_SAF_CKGATE_UART5_INDEX 26
#define CKGEN_SAF_CKGATE_UART6_INDEX 27
#define CKGEN_SAF_CKGATE_UART7_INDEX 28
#define CKGEN_SAF_CKGATE_UART8_INDEX 29
#define CKGEN_SAF_CKGATE_I2S_SC1_INDEX 30
#define CKGEN_SAF_CKGATE_I2S_SC2_INDEX 31
#define CKGEN_SAF_CKGATE_TBU11_INDEX 32
#define CKGEN_SAF_CKGATE_TBU12_INDEX 33
#define CKGEN_SAF_CKGATE_TBU13_INDEX 34
#define CKGEN_SAF_CKGATE_WDT1_INDEX 35
#define CKGEN_SAF_CKGATE_WDT2_INDEX 36
#define CKGEN_SAF_CKGATE_TIMER1_INDEX 37
#define CKGEN_SAF_CKGATE_TIMER2_INDEX 38
#define CKGEN_SAF_CKGATE_PWM1_INDEX 39
#define CKGEN_SAF_CKGATE_PWM2_INDEX 40
#define CKGEN_SAF_CKGATE_CANFD1_INDEX 41
#define CKGEN_SAF_CKGATE_CANFD2_INDEX 42
#define CKGEN_SAF_CKGATE_CANFD3_INDEX 43
#define CKGEN_SAF_CKGATE_CANFD4_INDEX 44
#define CKGEN_SAF_CKGATE_GPIO1_INDEX 45
#define CKGEN_SAF_CKGATE_RC_24M_INDEX 46
#define CKGEN_SAF_CKGATE_SCR_SAF_INDEX 47
#define CKGEN_SAF_CKGATE_PVT_SNS_SAF_INDEX 48
#define CKGEN_SAF_CKGATE_PLL1_INDEX 49
#define CKGEN_SAF_CKGATE_PLL2_INDEX 50
#define CKGEN_SAF_CKGATE_CKGEN_SAF_INDEX 51
#define CKGEN_SAF_CKGATE_RSTGEN_SAF_INDEX 52
#define CKGEN_SAF_CKGATE_SEM1_INDEX 53
#define CKGEN_SAF_CKGATE_SEM2_INDEX 54
#define CKGEN_SAF_CKGATE_EIC_SAFE_INDEX 55
#define CKGEN_SAF_CKGATE_RPC_SAF_INDEX 56
#define CKGEN_SAF_CKGATE_XTAL_SAF_INDEX 57
#define CKGEN_SAF_CKGATE_BIPC_ENET1_INDEX 58
#define CKGEN_SEC_CKGATE_CR5_SEC_INDEX 6
#define CKGEN_SEC_CKGATE_CR5_MP_INDEX 7
#define CKGEN_SEC_CKGATE_ADSP_INDEX 8
#define CKGEN_SEC_CKGATE_CE2_INDEX 9
#define CKGEN_SEC_CKGATE_GIC2_INDEX 10
#define CKGEN_SEC_CKGATE_GIC3_INDEX 11
#define CKGEN_SEC_CKGATE_MSHC1_INDEX 12
#define CKGEN_SEC_CKGATE_MSHC2_INDEX 13
#define CKGEN_SEC_CKGATE_MSHC3_INDEX 14
#define CKGEN_SEC_CKGATE_MSHC4_INDEX 15
#define CKGEN_SEC_CKGATE_ENET2_INDEX 16
#define CKGEN_SEC_CKGATE_OSPI2_INDEX 17
#define CKGEN_SEC_CKGATE_DMA2_INDEX 18
#define CKGEN_SEC_CKGATE_DMA3_INDEX 19
#define CKGEN_SEC_CKGATE_DMA4_INDEX 20
#define CKGEN_SEC_CKGATE_DMA5_INDEX 21
#define CKGEN_SEC_CKGATE_DMA6_INDEX 22
#define CKGEN_SEC_CKGATE_DMA7_INDEX 23
#define CKGEN_SEC_CKGATE_DMA8_INDEX 24
#define CKGEN_SEC_CKGATE_MB_INDEX 25
#define CKGEN_SEC_CKGATE_IRAM2_INDEX 26
#define CKGEN_SEC_CKGATE_IRAM3_INDEX 27
#define CKGEN_SEC_CKGATE_IRAM4_INDEX 28
#define CKGEN_SEC_CKGATE_ROMC2_INDEX 29
#define CKGEN_SEC_CKGATE_I2C5_INDEX 30
#define CKGEN_SEC_CKGATE_I2C6_INDEX 31
#define CKGEN_SEC_CKGATE_I2C7_INDEX 32
#define CKGEN_SEC_CKGATE_I2C8_INDEX 33
#define CKGEN_SEC_CKGATE_I2C9_INDEX 34
#define CKGEN_SEC_CKGATE_I2C10_INDEX 35
#define CKGEN_SEC_CKGATE_I2C11_INDEX 36
#define CKGEN_SEC_CKGATE_I2C12_INDEX 37
#define CKGEN_SEC_CKGATE_I2C13_INDEX 38
#define CKGEN_SEC_CKGATE_I2C14_INDEX 39
#define CKGEN_SEC_CKGATE_I2C15_INDEX 40
#define CKGEN_SEC_CKGATE_I2C16_INDEX 41
#define CKGEN_SEC_CKGATE_SPI5_INDEX 42
#define CKGEN_SEC_CKGATE_SPI6_INDEX 43
#define CKGEN_SEC_CKGATE_SPI7_INDEX 44
#define CKGEN_SEC_CKGATE_SPI8_INDEX 45
#define CKGEN_SEC_CKGATE_UART9_INDEX 46
#define CKGEN_SEC_CKGATE_UART10_INDEX 47
#define CKGEN_SEC_CKGATE_UART11_INDEX 48
#define CKGEN_SEC_CKGATE_UART12_INDEX 49
#define CKGEN_SEC_CKGATE_UART13_INDEX 50
#define CKGEN_SEC_CKGATE_UART14_INDEX 51
#define CKGEN_SEC_CKGATE_UART15_INDEX 52
#define CKGEN_SEC_CKGATE_UART16_INDEX 53
#define CKGEN_SEC_CKGATE_SPDIF1_INDEX 54
#define CKGEN_SEC_CKGATE_SPDIF2_INDEX 55
#define CKGEN_SEC_CKGATE_SPDIF3_INDEX 56
#define CKGEN_SEC_CKGATE_SPDIF4_INDEX 57
#define CKGEN_SEC_CKGATE_TIMER3_INDEX 58
#define CKGEN_SEC_CKGATE_TIMER4_INDEX 59
#define CKGEN_SEC_CKGATE_TIMER5_INDEX 60
#define CKGEN_SEC_CKGATE_TIMER6_INDEX 61
#define CKGEN_SEC_CKGATE_TIMER7_INDEX 62
#define CKGEN_SEC_CKGATE_TIMER8_INDEX 63
#define CKGEN_SEC_CKGATE_PWM3_INDEX 64
#define CKGEN_SEC_CKGATE_PWM4_INDEX 65
#define CKGEN_SEC_CKGATE_PWM5_INDEX 66
#define CKGEN_SEC_CKGATE_PWM6_INDEX 67
#define CKGEN_SEC_CKGATE_PWM7_INDEX 68
#define CKGEN_SEC_CKGATE_PWM8_INDEX 69
#define CKGEN_SEC_CKGATE_I2S_MC1_INDEX 70
#define CKGEN_SEC_CKGATE_I2S_MC2_INDEX 71
#define CKGEN_SEC_CKGATE_I2S_SC3_INDEX 72
#define CKGEN_SEC_CKGATE_I2S_SC4_INDEX 73
#define CKGEN_SEC_CKGATE_I2S_SC5_INDEX 74
#define CKGEN_SEC_CKGATE_I2S_SC6_INDEX 75
#define CKGEN_SEC_CKGATE_I2S_SC7_INDEX 76
#define CKGEN_SEC_CKGATE_I2S_SC8_INDEX 77
#define CKGEN_SEC_CKGATE_CANFD5_INDEX 78
#define CKGEN_SEC_CKGATE_CANFD6_INDEX 79
#define CKGEN_SEC_CKGATE_CANFD7_INDEX 80
#define CKGEN_SEC_CKGATE_CANFD8_INDEX 81
#define CKGEN_SEC_CKGATE_CSSYS_INDEX 82
#define CKGEN_SEC_CKGATE_SYS_CNT_INDEX 83
#define CKGEN_SEC_CKGATE_TBU14_INDEX 84
#define CKGEN_SEC_CKGATE_TBU15_INDEX 85
#define CKGEN_SEC_CKGATE_GPIO2_INDEX 86
#define CKGEN_SEC_CKGATE_GPIO3_INDEX 87
#define CKGEN_SEC_CKGATE_GPIO4_INDEX 88
#define CKGEN_SEC_CKGATE_GPIO5_INDEX 89
#define CKGEN_SEC_CKGATE_WDT3_INDEX 90
#define CKGEN_SEC_CKGATE_WDT4_INDEX 91
#define CKGEN_SEC_CKGATE_WDT5_INDEX 92
#define CKGEN_SEC_CKGATE_WDT6_INDEX 93
#define CKGEN_SEC_CKGATE_WDT7_INDEX 94
#define CKGEN_SEC_CKGATE_WDT8_INDEX 95
#define CKGEN_SEC_CKGATE_PLL3_INDEX 96
#define CKGEN_SEC_CKGATE_PLL4_INDEX 97
#define CKGEN_SEC_CKGATE_PLL5_INDEX 98
#define CKGEN_SEC_CKGATE_PLL6_INDEX 99
#define CKGEN_SEC_CKGATE_PLL7_INDEX 100
#define CKGEN_SOC_CKGATE_TCU_INDEX 6
#define CKGEN_SOC_CKGATE_TBU0_INDEX 7
#define CKGEN_SOC_CKGATE_TBU1_INDEX 8
#define CKGEN_SOC_CKGATE_TBU2_INDEX 9
#define CKGEN_SOC_CKGATE_TBU3_INDEX 10
#define CKGEN_SOC_CKGATE_TBU4_INDEX 11
#define CKGEN_SOC_CKGATE_TBU5_INDEX 12
#define CKGEN_SOC_CKGATE_TBU6_INDEX 13
#define CKGEN_SOC_CKGATE_TBU7_INDEX 14
#define CKGEN_SOC_CKGATE_TBU8_INDEX 15
#define CKGEN_SOC_CKGATE_TBU9_INDEX 16
#define CKGEN_SOC_CKGATE_TBU10_INDEX 17
#define CKGEN_SOC_CKGATE_SCR4K_SID_INDEX 19
#define CKGEN_SOC_CKGATE_SCR4K_SSID_INDEX 20
#define CKGEN_SOC_CKGATE_BIPC_DDR_INDEX 21
#define CKGEN_SOC_CKGATE_EIC_HPI_INDEX 22
#define CKGEN_SOC_CKGATE_NOC_GICCLK_INDEX 23
#define CKGEN_SOC_CKGATE_GIC4_INDEX 24
#define CKGEN_SOC_CKGATE_GIC5_INDEX 25
#define CKGEN_SOC_CKGATE_PLL_CPU1A_INDEX 27
#define CKGEN_SOC_CKGATE_PLL_CPU1B_INDEX 27
#define CKGEN_SOC_CKGATE_CPU2_INDEX 28
#define CKGEN_SOC_CKGATE_PLL_CPU2_INDEX 29
#define CKGEN_SOC_CKGATE_GPU1_INDEX 30
#define CKGEN_SOC_CKGATE_PLL_GPU1_INDEX 31
#define CKGEN_SOC_CKGATE_GPU2_INDEX 32
#define CKGEN_SOC_CKGATE_PLL_GPU2_INDEX 33
#define CKGEN_SOC_CKGATE_VPU1_INDEX 34
#define CKGEN_SOC_CKGATE_VPU2_INDEX 35
#define CKGEN_SOC_CKGATE_MJPEG_INDEX 36
#define CKGEN_SOC_CKGATE_NNA_INDEX 37
#define CKGEN_SOC_CKGATE_PLL_VPU_INDEX 38
#define CKGEN_SOC_CKGATE_VDSP_INDEX 39
#define CKGEN_SOC_CKGATE_NOC_VSN_INDEX 40
#define CKGEN_SOC_CKGATE_PLL_VSN_INDEX 41
#define CKGEN_SOC_CKGATE_BIPC_VDSP_INDEX 42
#define CKGEN_SOC_CKGATE_EIC_VSN_INDEX 43
#define CKGEN_SOC_CKGATE_NOC_HIS_INDEX 44
#define CKGEN_SOC_CKGATE_PLL_HIS_INDEX 45
#define CKGEN_SOC_CKGATE_PCIEX2_INDEX 46
#define CKGEN_SOC_CKGATE_PCIEX1_INDEX 47
#define CKGEN_SOC_CKGATE_PCIEPHY_INDEX 48
#define CKGEN_SOC_CKGATE_PCIE_REF_INDEX 49
#define CKGEN_SOC_CKGATE_USB1_INDEX 50
#define CKGEN_SOC_CKGATE_USB2_INDEX 51
#define CKGEN_SOC_CKGATE_USB1_REF_INDEX 52
#define CKGEN_SOC_CKGATE_USB2_REF_INDEX 53
#define CKGEN_SOC_CKGATE_CPU1_INDEX 54
#define CKGEN_SOC_CKGATE_CPU1_CORE0_INDEX 54
#define CKGEN_SOC_CKGATE_CPU1_CORE1_INDEX 55
#define CKGEN_SOC_CKGATE_CPU1_CORE2_INDEX 56
#define CKGEN_SOC_CKGATE_CPU1_CORE3_INDEX 57
#define CKGEN_SOC_CKGATE_CPU1_CORE4_INDEX 58
#define CKGEN_SOC_CKGATE_CPU1_CORE5_INDEX 59
#define CKGEN_DISP_CKGATE_NOC_DISP_INDEX 0
#define CKGEN_DISP_CKGATE_MIPI_DSI1_CLKEXT_INDEX 1
#define CKGEN_DISP_CKGATE_MIPI_DSI2_CLKEXT_INDEX 2
#define CKGEN_DISP_CKGATE_IRAM5_INDEX 5
#define CKGEN_DISP_CKGATE_XTAL_AP_INDEX 6
#define CKGEN_DISP_CKGATE_PLL_LVDS1_INDEX 7
#define CKGEN_DISP_CKGATE_PLL_LVDS2_INDEX 8
#define CKGEN_DISP_CKGATE_PLL_LVDS3_INDEX 9
#define CKGEN_DISP_CKGATE_PLL_LVDS4_INDEX 10
#define CKGEN_DISP_CKGATE_CKGEN_DISP_INDEX 11
#define CKGEN_DISP_CKGATE_DC1_INDEX 12
#define CKGEN_DISP_CKGATE_DC2_INDEX 13
#define CKGEN_DISP_CKGATE_DC3_INDEX 14
#define CKGEN_DISP_CKGATE_DC4_INDEX 15
#define CKGEN_DISP_CKGATE_DC5_INDEX 16
#define CKGEN_DISP_CKGATE_DP1_INDEX 17
#define CKGEN_DISP_CKGATE_DP2_INDEX 18
#define CKGEN_DISP_CKGATE_DP3_INDEX 19
#define CKGEN_DISP_CKGATE_G2D1_INDEX 20
#define CKGEN_DISP_CKGATE_G2D2_INDEX 21
#define CKGEN_DISP_CKGATE_MIPI_DSI1_INDEX 22
#define CKGEN_DISP_CKGATE_MIPI_DSI2_INDEX 23
#define CKGEN_DISP_CKGATE_LVDS_SS_INDEX 24
#define CKGEN_DISP_CKGATE_CSI1_INDEX 25
#define CKGEN_DISP_CKGATE_CSI2_INDEX 26
#define CKGEN_DISP_CKGATE_CSI3_INDEX 27
#define CKGEN_DISP_CKGATE_DISP_MUX_INDEX 28
#define CKGEN_DISP_CKGATE_MIPI_CSI1_INDEX 29
#define CKGEN_DISP_CKGATE_MIPI_CSI2_INDEX 30
#define CKGEN_DISP_CKGATE_MIPI_CSI3_INDEX 31

#endif
