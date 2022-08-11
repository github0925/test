//*****************************************************************************
//
// cmd_clkgen.c - app for the clkgen test Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include "clkgen_hal_ip_test.h"
#include "chip_res.h"
#include "ckgen_cfg.h"

/****************globale res manage id to clkgen res index****************/
/*fsrefclk global resource index*/
const uint32_t g_CmdRefclkResGlbResIdx[] = {
    RES_SCR_L16_SAF_FSREFCLK_SAF,
    RES_SCR_L16_SEC_FSREFCLK_SEC,
    RES_SCR_L16_SEC_FSREFCLK_HPI,
    RES_SCR_L16_SEC_FSREFCLK_CPU1,
    RES_SCR_L16_SEC_FSREFCLK_CPU2,
    RES_SCR_L16_SEC_FSREFCLK_GPU1,
    RES_SCR_L16_SEC_FSREFCLK_GPU2,
    RES_SCR_L16_SEC_FSREFCLK_HIS,
    RES_SCR_L16_SEC_FSREFCLK_VPU,
    RES_SCR_L16_SEC_FSREFCLK_VSN,
    RES_SCR_L16_SEC_FSREFCLK_DISP,
    RES_SCR_L16_SEC_FSREFCLK_DDR_SS,
};

/*safety ip bus slice and gating global resource index*/
const uint32_t g_CmdSafIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SAF_CE1,
    RES_IP_SLICE_SAF_I2C_SAF,
    RES_IP_SLICE_SAF_SPI_SAF,
    RES_IP_SLICE_SAF_UART_SAF,
    RES_IP_SLICE_SAF_I2S_MCLK1,
    RES_IP_SLICE_SAF_I2S_SC1,
    RES_IP_SLICE_SAF_I2S_SC2,
    RES_IP_SLICE_SAF_ENET1_TX,
    RES_IP_SLICE_SAF_ENET1_RMII,
    RES_IP_SLICE_SAF_ENET1_PHY_REF,
    RES_IP_SLICE_SAF_ENET1_TIMER_SEC,
    RES_IP_SLICE_SAF_OSPI1,
    RES_IP_SLICE_SAF_TIMER1,
    RES_IP_SLICE_SAF_TIMER2,
    RES_IP_SLICE_SAF_PWM1,
    RES_IP_SLICE_SAF_PWM2,
    RES_IP_SLICE_SAF_CAN_1_TO_4,
};

const uint32_t g_CmdSafBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SAF_SAF_PLAT_CTL,
};

const uint32_t g_CmdSafGatingGlbResIdx[] = {
    RES_GATING_EN_SAF_SAF_PLAT,
    RES_GATING_EN_SAF_SAF_PLAT_NOC,
    RES_GATING_EN_SAF_NOC_MAINB,
    RES_GATING_EN_SAF_MAC_RDC,
    RES_GATING_EN_SAF_RTC_PCLK,
    RES_GATING_EN_SAF_GIC1,
    RES_GATING_EN_SAF_DMA1,
    RES_GATING_EN_SAF_ENTE1_TX,
    RES_GATING_EN_SAF_ENET1_PHY_REF,
    RES_GATING_EN_SAF_ENET1_TIMER_SEC,
    RES_GATING_EN_SAF_OSPI1,
    RES_GATING_EN_SAF_IRAM1,
    RES_GATING_EN_SAF_ROMC1,
    RES_GATING_EN_SAF_EFUSEC,
    RES_GATING_EN_SAF_CE1,
    RES_GATING_EN_SAF_I2C1,
    RES_GATING_EN_SAF_I2C2,
    RES_GATING_EN_SAF_I2C3,
    RES_GATING_EN_SAF_I2C4,
    RES_GATING_EN_SAF_SPI1,
    RES_GATING_EN_SAF_SPI2,
    RES_GATING_EN_SAF_SPI3,
    RES_GATING_EN_SAF_SPI4,
    RES_GATING_EN_SAF_UART1,
    RES_GATING_EN_SAF_UART2,
    RES_GATING_EN_SAF_UART3,
    RES_GATING_EN_SAF_UART4,
    RES_GATING_EN_SAF_UART5,
    RES_GATING_EN_SAF_UART6,
    RES_GATING_EN_SAF_UART7,
    RES_GATING_EN_SAF_UART8,
    RES_GATING_EN_SAF_I2S_SC1,
    RES_GATING_EN_SAF_I2S_SC2,
    RES_GATING_EN_SAF_TBU11,
    RES_GATING_EN_SAF_TBU12,
    RES_GATING_EN_SAF_TBU13,
    RES_GATING_EN_SAF_TIMER1,
    RES_GATING_EN_SAF_TIMER2,
    RES_GATING_EN_SAF_PWM1,
    RES_GATING_EN_SAF_PWM2,
    RES_GATING_EN_SAF_CAN1,
    RES_GATING_EN_SAF_CAN2,
    RES_GATING_EN_SAF_CAN3,
    RES_GATING_EN_SAF_CAN4,
    RES_GATING_EN_SAF_GPIO1,
    RES_GATING_EN_SAF_SCR_SAF,
    RES_GATING_EN_SAF_PVT_SENS_SAF,
    RES_GATING_EN_SAF_PLL1,
    RES_GATING_EN_SAF_PLL2,
    RES_GATING_EN_SAF_CKGEN_SAF,
    RES_GATING_EN_SAF_RSTGEN_SAF,
    RES_GATING_EN_SAF_SEM1,
    RES_GATING_EN_SAF_SEM2,
    RES_GATING_EN_SAF_EIC_SAF,
    RES_GATING_EN_SAF_RPC_SAF,
    RES_GATING_EN_SAF_XTAL_SAF,
    RES_GATING_EN_SAF_BIPC_ENET1,
};

/*display ip bus slice and gating global resource index*/
const uint32_t g_CmdDispIpSliceGlbResIdx[] = {
    RES_IP_SLICE_DISP_MIPI_CSI1_PIX,
    RES_IP_SLICE_DISP_MIPI_CSI2_PIX,
    RES_IP_SLICE_DISP_MIPI_CSI3_PIX,
    RES_IP_SLICE_DISP_DP1,
    RES_IP_SLICE_DISP_DP2,
    RES_IP_SLICE_DISP_DP3,
    RES_IP_SLICE_DISP_DC5,
    RES_IP_SLICE_DISP_DC1,
    RES_IP_SLICE_DISP_DC2,
    RES_IP_SLICE_DISP_DC3,
    RES_IP_SLICE_DISP_DC4,
    RES_IP_SLICE_DISP_SPARE1,
    RES_IP_SLICE_DISP_SPARE2,
    RES_IP_SLICE_DISP_EXT_AUD1,
    RES_IP_SLICE_DISP_EXT_AUD2,
    RES_IP_SLICE_DISP_EXT_AUD3,
    RES_IP_SLICE_DISP_EXT_AUD4,
};

const uint32_t g_CmdDispBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_DISP_DISP_BUS_CTL,
};

const uint32_t g_CmdDispGatingGlbResIdx[] = {
    RES_GATING_EN_DISP_DISP_BUS,
    RES_GATING_EN_DISP_LVDS_CLK_1,
    RES_GATING_EN_DISP_LVDS_CLK_2,
    RES_GATING_EN_DISP_IRAM5,
    RES_GATING_EN_DISP_XTAL_AP,
    RES_GATING_EN_DISP_PLL_LVDS1,
    RES_GATING_EN_DISP_PLL_LVDS2,
    RES_GATING_EN_DISP_PLL_LVDS3,
    RES_GATING_EN_DISP_PLL_LVDS4,
    RES_GATING_EN_DISP_CKGEN_DISP,
    RES_GATING_EN_DISP_DC1,
    RES_GATING_EN_DISP_DC2,
    RES_GATING_EN_DISP_DC3,
    RES_GATING_EN_DISP_DC4,
    RES_GATING_EN_DISP_DC5,
    RES_GATING_EN_DISP_DP1,
    RES_GATING_EN_DISP_DP2,
    RES_GATING_EN_DISP_DP3,
    RES_GATING_EN_DISP_SPARE1,
    RES_GATING_EN_DISP_SPARE2,
    RES_GATING_EN_DISP_MIPI_DSI1,
    RES_GATING_EN_DISP_MIPI_DSI2,
    RES_GATING_EN_DISP_LVDS_SS,
    RES_GATING_EN_DISP_MIPI_CSI1_PIX_CSI,
    RES_GATING_EN_DISP_MIPI_CSI2_PIX_CSI,
    RES_GATING_EN_DISP_MIPI_CSI3_PIX_SCI,
    RES_GATING_EN_DISP_MIPI_CSI1_PIX,
    RES_GATING_EN_DISP_MIPI_CSI2_PIX,
    RES_GATING_EN_DISP_MIPI_CSI3_PIX,
};

/*secure ip bus core slice and gating global resource index*/
const uint32_t g_CmdSecIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SEC_CE2,
    RES_IP_SLICE_SEC_ADC,
    RES_IP_SLICE_SEC_I2C_SEC0,
    RES_IP_SLICE_SEC_I2C_SEC1,
    RES_IP_SLICE_SEC_SPI_SEC0,
    RES_IP_SLICE_SEC_SPI_SEC1,
    RES_IP_SLICE_SEC_UART_SEC0,
    RES_IP_SLICE_SEC_UART_SEC1,
    RES_IP_SLICE_SEC_EMMC1,
    RES_IP_SLICE_SEC_EMMC2,
    RES_IP_SLICE_SEC_EMMC3,
    RES_IP_SLICE_SEC_EMMC4,
    RES_IP_SLICE_SEC_ENET2_TX,
    RES_IP_SLICE_SEC_ENET2_RMII,
    RES_IP_SLICE_SEC_ENET2_PHY_REF,
    RES_IP_SLICE_SEC_ENET2_TIMER_SEC,
    RES_IP_SLICE_SEC_SPDIF1,
    RES_IP_SLICE_SEC_SPDIF2,
    RES_IP_SLICE_SEC_SPDIF3,
    RES_IP_SLICE_SEC_SPDIF4,
    RES_IP_SLICE_SEC_OSPI2,
    RES_IP_SLICE_SEC_TIMER3,
    RES_IP_SLICE_SEC_TIMER4,
    RES_IP_SLICE_SEC_TIMER5,
    RES_IP_SLICE_SEC_TIMER6,
    RES_IP_SLICE_SEC_TIMER7,
    RES_IP_SLICE_SEC_TIMER8,
    RES_IP_SLICE_SEC_PWM3,
    RES_IP_SLICE_SEC_PWM4,
    RES_IP_SLICE_SEC_PWM5,
    RES_IP_SLICE_SEC_PWM6,
    RES_IP_SLICE_SEC_PWM7,
    RES_IP_SLICE_SEC_PWM8,
    RES_IP_SLICE_SEC_I2S_MCLK2,
    RES_IP_SLICE_SEC_I2S_MCLK3,
    RES_IP_SLICE_SEC_I2S_MC1,
    RES_IP_SLICE_SEC_I2S_MC2,
    RES_IP_SLICE_SEC_I2S_SC3,
    RES_IP_SLICE_SEC_I2S_SC4,
    RES_IP_SLICE_SEC_I2S_SC5,
    RES_IP_SLICE_SEC_I2S_SC6,
    RES_IP_SLICE_SEC_I2S_SC7,
    RES_IP_SLICE_SEC_I2S_SC8,
    RES_IP_SLICE_SEC_CSI_MCLK1,
    RES_IP_SLICE_SEC_CSI_MCLK2,
    RES_IP_SLICE_SEC_GIC4_GIC5,
    RES_IP_SLICE_SEC_CAN5_CAN20,
    RES_IP_SLICE_SEC_TRACE,
    RES_IP_SLICE_SEC_SYS_CNT,
    RES_IP_SLICE_SEC_MSHC_TIMER,
    RES_IP_SLICE_SEC_HPI_CLK600,
    RES_IP_SLICE_SEC_HPI_CLK800,
};

const uint32_t g_CmdSecBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SEC_SEC_PLAT_CTL,
};

const uint32_t g_CmdSecCoreSliceGlbResIdx[] = {
    RES_CORE_SLICE_SEC_MP_PLAT,
};

const uint32_t g_CmdSecGatingGlbResIdx[] = {
    RES_GATING_EN_SEC_SEC_PLAT,
    RES_GATING_EN_SEC_MP_PLAT,
    RES_GATING_EN_SEC_ADC,
    RES_GATING_EN_SEC_CE2,
    RES_GATING_EN_SEC_GIC2,
    RES_GATING_EN_SEC_GIC3,
    RES_GATING_EN_SEC_EMMC1,
    RES_GATING_EN_SEC_MSHC_TIMER,
    RES_GATING_EN_SEC_EMMC2,
    RES_GATING_EN_SEC_EMMC3,
    RES_GATING_EN_SEC_EMMC4,
    RES_GATING_EN_SEC_ENET2_TX,
    RES_GATING_EN_SEC_ENET2_TIMER_SEC,
    RES_GATING_EN_SEC_OSPI2,
    RES_GATING_EN_SEC_MB,
    RES_GATING_EN_SEC_IRAM2,
    RES_GATING_EN_SEC_IRAM3,
    RES_GATING_EN_SEC_IRAM4,
    RES_GATING_EN_SEC_ROMC2,
    RES_GATING_EN_SEC_I2C5,
    RES_GATING_EN_SEC_I2C6,
    RES_GATING_EN_SEC_I2C7,
    RES_GATING_EN_SEC_I2C8,
    RES_GATING_EN_SEC_I2C9,
    RES_GATING_EN_SEC_I2C10,
    RES_GATING_EN_SEC_I2C11,
    RES_GATING_EN_SEC_I2C12,
    RES_GATING_EN_SEC_I2C13,
    RES_GATING_EN_SEC_I2C14,
    RES_GATING_EN_SEC_I2C15,
    RES_GATING_EN_SEC_I2C16,
    RES_GATING_EN_SEC_SPI5,
    RES_GATING_EN_SEC_SPI6,
    RES_GATING_EN_SEC_SPI7,
    RES_GATING_EN_SEC_SPI8,
    RES_GATING_EN_SEC_UART9,
    RES_GATING_EN_SEC_UART10,
    RES_GATING_EN_SEC_UART11,
    RES_GATING_EN_SEC_UART12,
    RES_GATING_EN_SEC_UART13,
    RES_GATING_EN_SEC_UART14,
    RES_GATING_EN_SEC_SPDIF1,
    RES_GATING_EN_SEC_SPDIF2,
    RES_GATING_EN_SEC_SPDIF3,
    RES_GATING_EN_SEC_SPDIF4,
    RES_GATING_EN_SEC_TIMER3,
    RES_GATING_EN_SEC_TIMER4,
    RES_GATING_EN_SEC_TIMER5,
    RES_GATING_EN_SEC_TIMER6,
    RES_GATING_EN_SEC_TIMER7,
    RES_GATING_EN_SEC_TIMER8,
    RES_GATING_EN_SEC_PWM3,
    RES_GATING_EN_SEC_PWM4,
    RES_GATING_EN_SEC_PWM5,
    RES_GATING_EN_SEC_PWM6,
    RES_GATING_EN_SEC_PWM7,
    RES_GATING_EN_SEC_PWM8,
    RES_GATING_EN_SEC_I2S_MC1,
    RES_GATING_EN_SEC_I2S_MC2,
    RES_GATING_EN_SEC_I2S_SC3,
    RES_GATING_EN_SEC_I2S_SC4,
    RES_GATING_EN_SEC_I2S_SC5,
    RES_GATING_EN_SEC_I2S_SC6,
    RES_GATING_EN_SEC_I2S_SC7,
    RES_GATING_EN_SEC_I2S_SC8,
    RES_GATING_EN_SEC_CANFD5,
    RES_GATING_EN_SEC_CANFD6,
    RES_GATING_EN_SEC_CANFD7,
    RES_GATING_EN_SEC_CANFD8,
    RES_GATING_EN_SEC_TRACE,
    RES_GATING_EN_SEC_SYS_CNT,
    RES_GATING_EN_SEC_TBU14,
    RES_GATING_EN_SEC_TBU15,
    RES_GATING_EN_SEC_GPIO2,
    RES_GATING_EN_SEC_GPIO3,
    RES_GATING_EN_SEC_GPIO4,
    RES_GATING_EN_SEC_GPIO5,
    RES_GATING_EN_SEC_WDT3,
    RES_GATING_EN_SEC_WDT4,
    RES_GATING_EN_SEC_WDT5,
    RES_GATING_EN_SEC_WDT6,
    RES_GATING_EN_SEC_WDT7,
    RES_GATING_EN_SEC_WDT8,
    RES_GATING_EN_SEC_PLL3,
    RES_GATING_EN_SEC_PLL4,
    RES_GATING_EN_SEC_PLL5,
    RES_GATING_EN_SEC_PLL6,
    RES_GATING_EN_SEC_PLL7,
    RES_GATING_EN_SEC_CANFD9,
    RES_GATING_EN_SEC_CANFD10,
    RES_GATING_EN_SEC_CANFD11,
    RES_GATING_EN_SEC_CANFD12,
    RES_GATING_EN_SEC_CANFD15,
    RES_GATING_EN_SEC_CANFD16,
    RES_GATING_EN_SEC_CANFD17,
    RES_GATING_EN_SEC_CANFD18,
    RES_GATING_EN_SEC_CANFD19,
    RES_GATING_EN_SEC_CANFD20,
};

/*soc ip slice and gating global resource index*/
const uint32_t g_CmdSocIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SOC_VPU1,
    RES_IP_SLICE_SOC_MJPEG,
};

const uint32_t g_CmdSocBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SOC_VPU_BUS_CTL,
    RES_BUS_SLICE_SOC_VSN_BUS_CTL,
    RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL,
    RES_BUS_SLICE_SOC_HIS_BUS_CTL,
};

const uint32_t g_CmdSocCoreSliceGlbResIdx[] = {
    RES_CORE_SLICE_SOC_CPU1A,
    RES_CORE_SLICE_SOC_CPU1B,
    RES_CORE_SLICE_SOC_CPU2,
    RES_CORE_SLICE_SOC_GPU1,
    RES_CORE_SLICE_SOC_GPU2,
    RES_CORE_SLICE_SOC_DDR,
};

/*uuu wrapper global resource index*/
const uint32_t g_CmdUuuSliceGlbResIdx[]= {
    RES_UUU_WRAP_SOC_CPU1A,
    RES_UUU_WRAP_SOC_CPU1B,
    RES_UUU_WRAP_SOC_CPU2,
    RES_UUU_WRAP_SOC_GPU1,
    RES_UUU_WRAP_SOC_GPU2,
    RES_UUU_WRAP_SOC_VPU1,
    RES_UUU_WRAP_SOC_MJPEG,
    RES_UUU_WRAP_SOC_VPU_BUS,
    RES_UUU_WRAP_SOC_VSN,
    RES_UUU_WRAP_SOC_DDR,
    RES_UUU_WRAP_SOC_HIS_BUS,
};

const uint32_t g_CmdSocGatingGlbResIdx[] = {
    RES_GATING_EN_SOC_TCU,
    RES_GATING_EN_SOC_TBU0,
    RES_GATING_EN_SOC_TBU1,
    RES_GATING_EN_SOC_TBU2,
    RES_GATING_EN_SOC_TBU3,
    RES_GATING_EN_SOC_TBU4,
    RES_GATING_EN_SOC_TBU5,
    RES_GATING_EN_SOC_TBU6,
    RES_GATING_EN_SOC_TBU8,
    RES_GATING_EN_SOC_TBU9,
    RES_GATING_EN_SOC_TBU10,
    RES_GATING_EN_SOC_SCR4K_SID,
    RES_GATING_EN_SOC_SCR4K_SSID,
    RES_GATING_EN_SOC_BIPC_DDR,
    RES_GATING_EN_SOC_EIC_HPI,
    RES_GATING_EN_SOC_GIC4_GIC5,
    RES_GATING_EN_SOC_GIC4,
    RES_GATING_EN_SOC_GIC5,
    RES_GATING_EN_SOC_CPU1A_2_PLL_CPU1A_PLL_CPU1B_PCLK,
    RES_GATING_EN_SOC_CPU2_0,
    RES_GATING_EN_SOC_CPU2_1,
    RES_GATING_EN_SOC_CPU2_PCLK_ATCLK_GICCLK,
    RES_GATING_EN_SOC_CPU2_PLL_CPU2_PCLK,
    RES_GATING_EN_SOC_GPU1_0,
    RES_GATING_EN_SOC_GPU1_1,
    RES_GATING_EN_SOC_GPU1_2,
    RES_GATING_EN_SOC_GPU2_0,
    RES_GATING_EN_SOC_GPU2_1,
    RES_GATING_EN_SOC_GPU2_2,
    RES_GATING_EN_SOC_VPU1,
    RES_GATING_EN_SOC_VPU_BUS_0_VPU1_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_VPU1_PCLK,
    RES_GATING_EN_SOC_VPU_BUS_0_VPU2_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_VPU2_PCLK,
    RES_GATING_EN_SOC_MJPEG,
    RES_GATING_EN_SOC_VPU_BUS_0_MJPEG_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_MJPEG_PCLK,
    RES_GATING_EN_SOC_VPU_BUS_1_PLL_VPU,
    RES_GATING_EN_SOC_VSN_BUS_0_VDSP_CLK,
    RES_GATING_EN_SOC_VSN_BUS_1_NOC_VSP,
    RES_GATING_EN_SOC_VSN_BUS_1_PLL_VSN,
    RES_GATING_EN_SOC_VSN_BUS_0_BIPC_VDSP_ACLK,
    RES_GATING_EN_SOC_VSN_BUS_1_BIPC_VDSP_PCLK,
    RES_GATING_EN_SOC_VSN_BUS_1_EIC_VSN,
    RES_GATING_EN_SOC_HIS_BUS_2_NOC_HIS_MAINCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_NOC_HIS_PERCLK,
    RES_GATING_EN_SOC_HIS_BUS_2_PCIE2_MSTR_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE2_PLK,
    RES_GATING_EN_SOC_HIS_BUS_2_PCIE1_MSTR_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE1_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_1,
    RES_GATING_EN_SOC_HIS_BUS_2_USB1_XM_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_CTRL_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_CTRL_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_2_USB2_XM_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_PCLK,
#if ECO_SYSTEM_CFG == 0
    RES_GATING_EN_SOC_CPU1B_0,
    RES_GATING_EN_SOC_CPU1A_2_PCLK_ATCLK_GICCLK,
    RES_GATING_EN_SOC_CPU1A_1,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5,
#endif
};

/*safety*/

#ifdef saf_ip_slice
const clkgen_ip_slice_t g_Cmdsaf_ip_slice[] = saf_ip_slice;
#else
const clkgen_ip_slice_t *g_Cmdsaf_ip_slice = NULL;
#endif

#ifdef saf_bus_slice
const clkgen_bus_slice_t g_Cmdsaf_bus_slice[] = saf_bus_slice;
#else
const clkgen_bus_slice_t *g_Cmdsaf_bus_slice = NULL;
#endif

/*secure*/
#ifdef disp_ip_slice
const clkgen_ip_slice_t g_Cmddisp_ip_slice[] = disp_ip_slice;
#else
const clkgen_ip_slice_t *g_Cmddisp_ip_slice = NULL;
#endif

#ifdef disp_bus_slice
const clkgen_bus_slice_t g_Cmddisp_bus_slice[] = disp_bus_slice;
#else
const clkgen_bus_slice_t *g_Cmddisp_bus_slice = NULL;
#endif

#ifdef sec_ip_slice
const clkgen_ip_slice_t g_Cmdsec_ip_slice[] = sec_ip_slice;
#else
const clkgen_ip_slice_t *g_Cmdsec_ip_slice = NULL;
#endif

#ifdef sec_bus_slice
const clkgen_bus_slice_t g_Cmdsec_bus_slice[] = sec_bus_slice;
#else
const clkgen_bus_slice_t *g_Cmdsec_bus_slice = NULL;
#endif

#ifdef sec_core_slice
const clkgen_core_slice_t g_Cmdsec_core_slice[] = sec_core_slice;
#else
const clkgen_core_slice_t *g_Cmdsec_core_slice = NULL;
#endif

#ifdef soc_ip_slice
const clkgen_ip_slice_t g_Cmdsoc_ip_slice[] = soc_ip_slice;
#else
const clkgen_ip_slice_t *g_Cmdsoc_ip_slice = NULL;
#endif

#ifdef soc_bus_slice
const clkgen_bus_slice_t g_Cmdsoc_bus_slice[] = soc_bus_slice;
#else
const clkgen_bus_slice_t *g_Cmdsoc_bus_slice = NULL;
#endif
#ifdef soc_core_slice
const clkgen_core_slice_t g_Cmdsoc_core_slice[] = soc_core_slice;
#else
const clkgen_core_slice_t *g_Cmdsoc_core_slice = NULL;
#endif

#ifdef uuu_wrapper
const clkgen_uuu_cfg_t g_Cmduuu_wrapper[] = uuu_wrapper;
#else
const clkgen_uuu_cfg_t *g_Cmduuu_wrapper = NULL;
#endif

char clkgen_ipslice_readonlyreg_check_test_help[]= {
    "do_clkgen_ipslice_readonlyreg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_ipslice_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_ipslice_readonlyreg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ipslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSafIpSliceGlbResIdx)/sizeof(g_CmdSafIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_readonlyreg_check_test(g_handle,g_CmdSafIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSafIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSafIpSliceGlbResIdx[i]);
            goto fail;
        }
    }
#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecIpSliceGlbResIdx)/sizeof(g_CmdSecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_readonlyreg_check_test(g_handle,g_CmdSecIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSecIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSecIpSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSecIpSliceGlbResIdx)/sizeof(g_CmdSecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_readonlyreg_check_test(g_handle,g_CmdSecIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSecIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSecIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocIpSliceGlbResIdx)/sizeof(g_CmdSocIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_readonlyreg_check_test(g_handle,g_CmdSocIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSocIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSocIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispIpSliceGlbResIdx)/sizeof(g_CmdDispIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_readonlyreg_check_test(g_handle,g_CmdDispIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdDispIpSliceGlbResIdx[%d]:%d\n",i,g_CmdDispIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_ipslice_readonlyreg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_busslice_readonlyreg_check_test_help[]= {
    "do_clkgen_busslice_readonlyreg_check_test:null\n" \
    "" \
};
int do_clkgen_busslice_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_busslice_readonlyreg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_busslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSafBusSliceGlbResIdx)/sizeof(g_CmdSafBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_readonlyreg_check_test(g_handle,g_CmdSafBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_readonlyreg_check_test g_CmdSafBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSafBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecBusSliceGlbResIdx)/sizeof(g_CmdSecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_readonlyreg_check_test(g_handle,g_CmdSecBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_readonlyreg_check_test g_CmdSecBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSecBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSecBusSliceGlbResIdx)/sizeof(g_CmdSecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_readonlyreg_check_test(g_handle,g_CmdSecBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_readonlyreg_check_test g_CmdSecBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSecBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocBusSliceGlbResIdx)/sizeof(g_CmdSocBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_readonlyreg_check_test(g_handle,g_CmdSocBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_readonlyreg_check_test g_CmdSocBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSocBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispBusSliceGlbResIdx)/sizeof(g_CmdDispBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_readonlyreg_check_test(g_handle,g_CmdDispBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_readonlyreg_check_test g_CmdDispBusSliceGlbResIdx[%d]:%d\n",i,g_CmdDispBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_busslice_readonlyreg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_coreslice_readonlyreg_check_test_help[]= {
    "do_clkgen_coreslice_readonlyreg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_coreslice_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_coreslice_readonlyreg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_coreslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG

#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecCoreSliceGlbResIdx)/sizeof(g_CmdSecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_readonlyreg_check_test(g_handle,g_CmdSecCoreSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_readonlyreg_check_test g_CmdSecCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSecCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSecCoreSliceGlbResIdx)/sizeof(g_CmdSecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_readonlyreg_check_test(g_handle,g_CmdSecCoreSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_readonlyreg_check_test g_CmdSecCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSecCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocCoreSliceGlbResIdx)/sizeof(g_CmdSocCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_readonlyreg_check_test(g_handle,g_CmdSocCoreSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_readonlyreg_check_test g_CmdSocCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSocCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdUuuSliceGlbResIdx)/sizeof(g_CmdUuuSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_readonlyreg_check_test(g_handle,g_CmdUuuSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_readonlyreg_check_test g_CmdUuuSliceGlbResIdx[%d]:%d\n",i,g_CmdUuuSliceGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_coreslice_readonlyreg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_other_readonlyreg_check_test_help[]= {
    "do_clkgen_other_readonlyreg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_other_readonlyreg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_other_readonlyreg_check_test creat handle failed\n");
        return false;
    }

#if SAF_SYSTEM_CFG
    ret = hal_clkgen_other_readonlyreg_check_test(g_handle,APB_CKGEN_SAF_BASE);
    if (!ret) goto fail;
#elif SEC_SYSTEM_CFG
    ret = hal_clkgen_other_readonlyreg_check_test(g_handle,APB_CKGEN_SEC_BASE);
    if (!ret) goto fail;
#else
    ret = hal_clkgen_other_readonlyreg_check_test(g_handle,APB_CKGEN_SOC_BASE);
    if (!ret) goto fail;
    ret = hal_clkgen_other_readonlyreg_check_test(g_handle,APB_CKGEN_DISP_BASE);
    if (!ret) goto fail;
#endif
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_other_readonlyreg_check_test ret:%d:\n",ret);
    return ret;
}

char clkgen_ipslice_rw_reg_check_test_help[]= {
    "do_clkgen_ipslice_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_ipslice_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_ipslice_rw_reg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ipslice_rw_reg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSafIpSliceGlbResIdx)/sizeof(g_CmdSafIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_rw_reg_check_test(g_handle,g_CmdSafIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_rw_reg_check_test g_CmdSafIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSafIpSliceGlbResIdx[i]);
            goto fail;
        }
    }
#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecIpSliceGlbResIdx)/sizeof(g_CmdSecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_rw_reg_check_test(g_handle,g_CmdSecIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_rw_reg_check_test g_CmdSecIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSecIpSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSocIpSliceGlbResIdx)/sizeof(g_CmdSocIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_rw_reg_check_test(g_handle,g_CmdSocIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_rw_reg_check_test g_CmdSocIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSocIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispIpSliceGlbResIdx)/sizeof(g_CmdDispIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_ipslice_rw_reg_check_test(g_handle,g_CmdDispIpSliceGlbResIdx[i])) {
            printf("do_clkgen_ipslice_rw_reg_check_test g_CmdDispIpSliceGlbResIdx[%d]:%d\n",i,g_CmdDispIpSliceGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_ipslice_rw_reg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_busslice_rw_reg_check_test_help[]= {
    "do_clkgen_busslice_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_busslice_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_busslice_rw_reg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_busslice_rw_reg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSafBusSliceGlbResIdx)/sizeof(g_CmdSafBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_rw_reg_check_test(g_handle,g_CmdSafBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_rw_reg_check_test g_CmdSafBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSafBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecBusSliceGlbResIdx)/sizeof(g_CmdSecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_rw_reg_check_test(g_handle,g_CmdSecBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_rw_reg_check_test g_CmdSecBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSecBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSocBusSliceGlbResIdx)/sizeof(g_CmdSocBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_busslice_rw_reg_check_test(g_handle,g_CmdSocBusSliceGlbResIdx[i])) {
            printf("do_clkgen_busslice_rw_reg_check_test g_CmdSocBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSocBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_busslice_rw_reg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_coreslice_rw_reg_check_test_help[]= {
    "do_clkgen_coreslice_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_coreslice_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    printf("do_clkgen_coreslice_rw_reg_check_test str:%s   u:%d\n",argv[1].str,argv[1].u);
    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_coreslice_rw_reg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG

#elif SEC_SYSTEM_CFG
    glb_res_idx_size = sizeof(g_CmdSecCoreSliceGlbResIdx)/sizeof(g_CmdSecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_rw_reg_check_test(g_handle,g_CmdSecCoreSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_rw_reg_check_test g_CmdSecCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSecCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }
#else
    glb_res_idx_size = sizeof(g_CmdSocCoreSliceGlbResIdx)/sizeof(g_CmdSocCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!hal_clkgen_coreslice_rw_reg_check_test(g_handle,g_CmdSocCoreSliceGlbResIdx[i])) {
            printf("do_clkgen_coreslice_rw_reg_check_test g_CmdSocCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSocCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret =true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_coreslice_rw_reg_check_test ret:%d:\n", ret);
    return ret;
}

char clkgen_lpgating_rw_reg_check_test_help[]= {
    "do_clkgen_lpgating_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_lpgating_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint8_t gating_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ipslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
#if SAF_SYSTEM_CFG
    gating_idx_size = sizeof(g_CmdSafGatingGlbResIdx)/sizeof(g_CmdSafGatingGlbResIdx[0]);
    for (uint8_t i = 0; i<gating_idx_size; i++) {
        if (!hal_clkgen_lpgating_rw_reg_check_test(g_handle,g_CmdSafGatingGlbResIdx[i])) {
            printf("do_clkgen_lpgating_rw_reg_check_test g_CmdSafGatingGlbResIdx[%d]:%d\n",i,g_CmdSafGatingGlbResIdx[i]);
            goto fail;
        }
    }

#elif SEC_SYSTEM_CFG
    gating_idx_size = sizeof(g_CmdSecGatingGlbResIdx)/sizeof(g_CmdSecGatingGlbResIdx[0]);
    for (uint8_t i = 0; i<gating_idx_size; i++) {
        if (!hal_clkgen_lpgating_rw_reg_check_test(g_handle,g_CmdSecGatingGlbResIdx[i])) {
            printf("do_clkgen_lpgating_rw_reg_check_test g_CmdSecGatingGlbResIdx[%d]:%d\n",i,g_CmdSecGatingGlbResIdx[i]);
            goto fail;
        }
    }
#else
    gating_idx_size = sizeof(g_CmdSocGatingGlbResIdx)/sizeof(g_CmdSocGatingGlbResIdx[0]);
    for (uint8_t i = 0; i<gating_idx_size; i++) {
        if (!hal_clkgen_lpgating_rw_reg_check_test(g_handle,g_CmdSocGatingGlbResIdx[i])) {
            printf("do_clkgen_lpgating_rw_reg_check_test g_CmdSocGatingGlbResIdx[%d]:%d\n",i,g_CmdSocGatingGlbResIdx[i]);
            goto fail;
        }
    }
#endif
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_lpgating_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char clkgen_uuuslice_rw_reg_check_test_help[]= {
    "do_clkgen_uuuslice_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_uuuslice_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint8_t uuu_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ipslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }
    ret = false;
    uuu_idx_size = sizeof(g_CmdUuuSliceGlbResIdx)/sizeof(g_CmdUuuSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<uuu_idx_size; i++) {
        if (!hal_clkgen_uuuslice_rw_reg_check_test(g_handle,g_CmdUuuSliceGlbResIdx[i])) {
            printf("do_clkgen_lpgating_rw_reg_check_test g_CmdUuuSliceGlbResIdx[%d]:%d\n",i,g_CmdUuuSliceGlbResIdx[i]);
            goto fail;
        }
    }
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_uuuslice_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char clkgen_other_rw_reg_check_test_help[]= {
    "do_clkgen_other_rw_reg_check_test:start_idx\n" \
    "" \
};
int do_clkgen_other_rw_reg_check_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ipslice_readonlyreg_check_test creat handle failed\n");
        return false;
    }

#if SAF_SYSTEM_CFG
    ret = hal_clkgen_other_rw_reg_check_test(g_handle,APB_CKGEN_SAF_BASE);
    if (!ret)goto fail;
#elif SEC_SYSTEM_CFG
    ret = hal_clkgen_other_rw_reg_check_test(g_handle,APB_CKGEN_SEC_BASE);
    if (!ret)goto fail;
#else
    ret = hal_clkgen_other_rw_reg_check_test(g_handle,APB_CKGEN_SOC_BASE);
    if (!ret)goto fail;
    ret = hal_clkgen_other_rw_reg_check_test(g_handle,APB_CKGEN_DISP_BASE);
    if (!ret)goto fail;
#endif
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_other_rw_reg_check_test ret:%d:\n",ret);
    return ret;
}

char clkgen_ip_clock_test_help[]= {
    "do_clkgen_ip_clock_test:start_idx\n" \
    "" \
};
int do_clkgen_ip_clock_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ip_clock_test creat handle failed\n");
        return false;
    }
    ret =false;

    glb_res_idx_size = sizeof(g_CmdSafIpSliceGlbResIdx)/sizeof(g_CmdSafIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (!g_Cmdsaf_ip_slice)break;
        if (!hal_clkgen_ip_clock_test(g_handle,g_CmdSafIpSliceGlbResIdx[i],g_Cmdsaf_ip_slice)) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSafIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSafIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSecIpSliceGlbResIdx)/sizeof(g_CmdSecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_test(g_handle,g_CmdSecIpSliceGlbResIdx[i],g_Cmdsec_ip_slice)) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSecIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSecIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocIpSliceGlbResIdx)/sizeof(g_CmdSocIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_test(g_handle,g_CmdSocIpSliceGlbResIdx[i],g_Cmdsoc_ip_slice)) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdSocIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSocIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispIpSliceGlbResIdx)/sizeof(g_CmdDispIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmddisp_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_test(g_handle,g_CmdDispIpSliceGlbResIdx[i],g_Cmddisp_ip_slice)) {
            printf("do_clkgen_ipslice_readonlyreg_check_test g_CmdDispIpSliceGlbResIdx[%d]:%d\n",i,g_CmdDispIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_ip_clock_test ret:%d:\n",ret);
    return ret;
}

char clkgen_ip_clock_div_test_help[]= {
    "do_clkgen_ip_clock_div_test:start_idx\n" \
    "" \
};
int do_clkgen_ip_clock_div_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_ip_clock_div_test creat handle failed\n");
        return false;
    }
    ret = false;

    glb_res_idx_size = sizeof(g_CmdSafIpSliceGlbResIdx)/sizeof(g_CmdSafIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsaf_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_div_test(g_handle,g_CmdSafIpSliceGlbResIdx[i],g_Cmdsaf_ip_slice)) {
            printf("do_clkgen_ip_clock_div_test g_CmdSafIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSafIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSecIpSliceGlbResIdx)/sizeof(g_CmdSecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_div_test(g_handle,g_CmdSecIpSliceGlbResIdx[i],g_Cmdsec_ip_slice)) {
            printf("do_clkgen_ip_clock_div_test g_CmdSecIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSecIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocIpSliceGlbResIdx)/sizeof(g_CmdSocIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_div_test(g_handle,g_CmdSocIpSliceGlbResIdx[i],g_Cmdsoc_ip_slice)) {
            printf("do_clkgen_ip_clock_div_test g_CmdSocIpSliceGlbResIdx[%d]:%d\n",i,g_CmdSocIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispIpSliceGlbResIdx)/sizeof(g_CmdDispIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmddisp_ip_slice==NULL)break;
        if (!hal_clkgen_ip_clock_div_test(g_handle,g_CmdDispIpSliceGlbResIdx[i],g_Cmddisp_ip_slice)) {
            printf("do_clkgen_ip_clock_div_test g_CmdDispIpSliceGlbResIdx[%d]:%d\n",i,g_CmdDispIpSliceGlbResIdx[i]);
            goto fail;
        }
    }

    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_ip_clock_div_test ret:%d:\n",ret);
    return ret;
}

char clkgen_bus_clock_test_help[]= {
    "do_clkgen_bus_clock_test:start_idx\n" \
    "" \
};
int do_clkgen_bus_clock_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_bus_clock_test creat handle failed\n");
        return false;
    }
    ret = false;

    glb_res_idx_size = sizeof(g_CmdSafBusSliceGlbResIdx)/sizeof(g_CmdSafBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsaf_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_test(g_handle,g_CmdSafBusSliceGlbResIdx[i],g_Cmdsaf_bus_slice)) {
            printf("do_clkgen_bus_clock_test g_CmdSafBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSafBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSecBusSliceGlbResIdx)/sizeof(g_CmdSecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_test(g_handle,g_CmdSecBusSliceGlbResIdx[i],g_Cmdsec_bus_slice)) {
            printf("do_clkgen_bus_clock_test g_CmdSecBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSecBusSliceGlbResIdx[i]);
            goto fail;
        }
    }


    glb_res_idx_size = sizeof(g_CmdSocBusSliceGlbResIdx)/sizeof(g_CmdSocBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_test(g_handle,g_CmdSocBusSliceGlbResIdx[i],g_Cmdsoc_bus_slice)) {
            printf("do_clkgen_bus_clock_test g_CmdSocBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSocBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispBusSliceGlbResIdx)/sizeof(g_CmdDispBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmddisp_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_test(g_handle,g_CmdDispBusSliceGlbResIdx[i],g_Cmddisp_bus_slice)) {
            printf("do_clkgen_bus_clock_test g_CmdDispBusSliceGlbResIdx[%d]:%d\n",i,g_CmdDispBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_bus_clock_test ret:%d:\n",ret);
    return ret;
}

char clkgen_bus_clock_div_test_help[]= {
    "do_clkgen_bus_clock_div_test::start_idx\n" \
    "" \
};
int do_clkgen_bus_clock_div_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_bus_clock_div_test creat handle failed\n");
        return false;
    }
    ret = false;
    glb_res_idx_size = sizeof(g_CmdSafBusSliceGlbResIdx)/sizeof(g_CmdSafBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsaf_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_div_test(g_handle,g_CmdSafBusSliceGlbResIdx[i],g_Cmdsaf_bus_slice)) {
            printf("do_clkgen_bus_clock_div_test g_CmdSafBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSafBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSecBusSliceGlbResIdx)/sizeof(g_CmdSecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_div_test(g_handle,g_CmdSecBusSliceGlbResIdx[i],g_Cmdsec_bus_slice)) {
            printf("do_clkgen_bus_clock_div_test g_CmdSecBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSecBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocBusSliceGlbResIdx)/sizeof(g_CmdSocBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_div_test(g_handle,g_CmdSocBusSliceGlbResIdx[i],g_Cmdsoc_bus_slice)) {
            printf("do_clkgen_bus_clock_div_test g_CmdSocBusSliceGlbResIdx[%d]:%d\n",i,g_CmdSocBusSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdDispBusSliceGlbResIdx)/sizeof(g_CmdDispBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmddisp_bus_slice==NULL)break;
        if (!hal_clkgen_bus_clock_div_test(g_handle,g_CmdDispBusSliceGlbResIdx[i],g_Cmddisp_bus_slice)) {
            printf("do_clkgen_bus_clock_div_test g_CmdDispBusSliceGlbResIdx[%d]:%d\n",i,g_CmdDispBusSliceGlbResIdx[i]);
            goto fail;
        }
    }
    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_bus_clock_div_test ret:%d:\n",ret);
    return ret;
}

char clkgen_core_clock_test_help[]= {
    "do_clkgen_core_clock_test:start_idx\n" \
    "" \
};
int do_clkgen_core_clock_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_core_clock_test creat handle failed\n");
        return false;
    }
    ret = false;

    glb_res_idx_size = sizeof(g_CmdSecCoreSliceGlbResIdx)/sizeof(g_CmdSecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_core_slice==NULL)break;
        if (!hal_clkgen_core_clock_test(g_handle,g_CmdSecCoreSliceGlbResIdx[i],g_Cmdsec_core_slice)) {
            printf("do_clkgen_core_clock_test g_CmdSecCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSecCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }

    glb_res_idx_size = sizeof(g_CmdSocCoreSliceGlbResIdx)/sizeof(g_CmdSocCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_core_slice==NULL)break;
        if (!hal_clkgen_core_clock_test(g_handle,g_CmdSocCoreSliceGlbResIdx[i],g_Cmdsoc_core_slice)) {
            printf("do_clkgen_core_clock_test g_CmdSocCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSocCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }

    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_core_clock_test ret:%d:\n",ret);
    return ret;
}


char clkgen_core_clock_div_test_help[]= {
    "do_clkgen_core_clock_div_test:start_idx\n" \
    "" \
};
int do_clkgen_core_clock_div_test(int argc, const cmd_args *argv)
{
    bool ret = false;
    uint16_t glb_res_idx_size = 0;
    uint32_t start_idx = argv[1].u;
    static void *g_handle;

    ret = hal_clkgen_test_creat_handle(&g_handle);
    if (!ret) {
        printf("do_clkgen_core_clock_div_test creat handle failed\n");
        return false;
    }
    ret = false;
    glb_res_idx_size = sizeof(g_CmdSecCoreSliceGlbResIdx)/sizeof(g_CmdSecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsec_core_slice==NULL)break;
        if (!hal_clkgen_core_clock_div_test(g_handle,g_CmdSecCoreSliceGlbResIdx[i],g_Cmdsec_core_slice)) {
            printf("do_clkgen_core_clock_div_test g_CmdSecCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSecCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }


    glb_res_idx_size = sizeof(g_CmdSocCoreSliceGlbResIdx)/sizeof(g_CmdSocCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i<glb_res_idx_size; i++) {
        if (g_Cmdsoc_core_slice==NULL)break;
        if (!hal_clkgen_core_clock_div_test(g_handle,g_CmdSocCoreSliceGlbResIdx[i],g_Cmdsoc_core_slice)) {
            printf("do_clkgen_core_clock_div_test g_CmdSocCoreSliceGlbResIdx[%d]:%d\n",i,g_CmdSocCoreSliceGlbResIdx[i]);
            goto fail;
        }
    }

    ret = true;
fail:
    hal_clkgen_test_release_handle(g_handle);
    printf("do_clkgen_core_clock_div_test ret:%d:\n",ret);
    return ret;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("clkgen_test1", clkgen_ipslice_readonlyreg_check_test_help, (console_cmd)&do_clkgen_ipslice_readonlyreg_check_test)
STATIC_COMMAND("clkgen_test2", clkgen_busslice_readonlyreg_check_test_help, (console_cmd)&do_clkgen_busslice_readonlyreg_check_test)
STATIC_COMMAND("clkgen_test3", clkgen_coreslice_readonlyreg_check_test_help, (console_cmd)&do_clkgen_coreslice_readonlyreg_check_test)
STATIC_COMMAND("clkgen_test4", clkgen_other_readonlyreg_check_test_help, (console_cmd)&do_clkgen_other_readonlyreg_check_test)
STATIC_COMMAND("clkgen_test5", clkgen_ipslice_rw_reg_check_test_help, (console_cmd)&do_clkgen_ipslice_rw_reg_check_test)
STATIC_COMMAND("clkgen_test6", clkgen_busslice_rw_reg_check_test_help, (console_cmd)&do_clkgen_busslice_rw_reg_check_test)
STATIC_COMMAND("clkgen_test7", clkgen_coreslice_rw_reg_check_test_help, (console_cmd)&do_clkgen_coreslice_rw_reg_check_test)
STATIC_COMMAND("clkgen_test8", clkgen_lpgating_rw_reg_check_test_help, (console_cmd)&do_clkgen_lpgating_rw_reg_check_test)
STATIC_COMMAND("clkgen_test9", clkgen_uuuslice_rw_reg_check_test_help, (console_cmd)&do_clkgen_uuuslice_rw_reg_check_test)
STATIC_COMMAND("clkgen_test10", clkgen_other_rw_reg_check_test_help, (console_cmd)&do_clkgen_other_rw_reg_check_test)
STATIC_COMMAND("clkgen_test11", clkgen_ip_clock_test_help, (console_cmd)&do_clkgen_ip_clock_test)
STATIC_COMMAND("clkgen_test12", clkgen_ip_clock_div_test_help, (console_cmd)&do_clkgen_ip_clock_div_test)
STATIC_COMMAND("clkgen_test13", clkgen_bus_clock_test_help, (console_cmd)&do_clkgen_bus_clock_test)
STATIC_COMMAND("clkgen_test14", clkgen_bus_clock_div_test_help, (console_cmd)&do_clkgen_bus_clock_div_test)
STATIC_COMMAND("clkgen_test15", clkgen_core_clock_test_help, (console_cmd)&do_clkgen_core_clock_test)
STATIC_COMMAND("clkgen_test17", clkgen_core_clock_div_test_help, (console_cmd)&do_clkgen_core_clock_div_test)
STATIC_COMMAND_END(clkgentest);
#endif
APP_START(clkgen)
.flags = 0
         APP_END
