/*
* dsi_reg.h
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: dc register defs file.
*
* Revision History:
* -----------------
* 011, 12/23/2019 BI create this file
*/
#ifndef __DSI_REG_H__
#define __DSI_REG_H__
#include <__regs_ap_dwc_mipi_dsi_host.h>
#include <__regs_base.h>

#define DSI1_BASE                   APB_MIPI_DSI1_BASE
#define DSI2_BASE                   APB_MIPI_DSI2_BASE

#define REG(x)                      (x)

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VERSION
// Register Offset : 0x0
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VERSION           REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VERSION)

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PWR_UP
// Register Offset : 0x4
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PWR_UP            REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PWR_UP)
#define PWR_UP_SHUTDOWNZ_SHIFT     PWR_UP_SHUTDOWNZ_FIELD_OFFSET
#define PWR_UP_SHUTDOWNZ_MASK      0x1 << PWR_UP_SHUTDOWNZ_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_CLKMSG_CFG
// Register Offset : 0x8
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_CLKMSG_CFG        REG(REG_AP_APB_DWC_MIPI_DSI_HOST_CLKMGR_CFG)
#define TO_CLK_DIVISION_SHIFT      CLKMGR_CFG_TO_CLK_DIVISION_FIELD_OFFSET
#define TO_CLK_DIVISION_MASK       0xFF << TO_CLK_DIVISION_SHIFT
#define TX_ESC_CLK_DIVISION_SHIFT  CLKMGR_CFG_TX_ESC_CLK_DIVISION_FIELD_OFFSET
#define TX_ESC_CLK_DIVISION_MASK   0xFF << TX_ESC_CLK_DIVISION_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_VCID
// Register Offset : 0xc
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_VCID          REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_VCID)
#define DPI_VCID_SHIFT             DPI_VCID_DPI_VCID_FIELD_OFFSET
#define DPI_VCID_MASK              0x2 << DPI_VCID_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_COLOR_CODING
// Register Offset : 0x10
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_COLOR_CODING  REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_COLOR_CODING)
#define LOOSELY18_EN_SHIFT         DPI_COLOR_CODING_LOOSELY18_EN_FIELD_OFFSET
#define LOOSELY18_EN_MASK          0x1 << LOOSELY18_EN_SHIFT
#define DPI_COLOR_CODING_SHIFT     DPI_COLOR_CODING_DPI_COLOR_CODING_FIELD_OFFSET
#define DPI_COLOR_CODING_MASK      0xF << DPI_COLOR_CODING_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_CFG_POL
// Register Offset : 0x14
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_CFG_POL       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_CFG_POL)
#define COLORM_ACTIVE_LOW_SHIFT    DPI_CFG_POL_COLORM_ACTIVE_LOW_FIELD_OFFSET
#define COLORM_ACTIVE_LOW_MASK     0x1 << COLORM_ACTIVE_LOW_SHIFT
#define SHUTD_ACTIVE_LOW_SHIFT     DPI_CFG_POL_SHUTD_ACTIVE_LOW_FIELD_OFFSET
#define SHUTD_ACTIVE_LOW_MASK      0x1 << SHUTD_ACTIVE_LOW_SHIFT
#define HSYNC_ACTIVE_LOW_SHIFT     DPI_CFG_POL_HSYNC_ACTIVE_LOW_FIELD_OFFSET
#define HSYNC_ACTIVE_LOW_MASK      0x1 << HSYNC_ACTIVE_LOW_SHIFT
#define VSYNC_ACTIVE_LOW_SHIFT     DPI_CFG_POL_VSYNC_ACTIVE_LOW_FIELD_OFFSET
#define VSYNC_ACTIVE_LOW_MASK      0x1 << VSYNC_ACTIVE_LOW_SHIFT
#define DATAEN_ACTIVE_LOW_SHIFT    DPI_CFG_POL_DATAEN_ACTIVE_LOW_FIELD_OFFSET
#define DATAEN_ACTIVE_LOW_MASK     0x1 << DATAEN_ACTIVE_LOW_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_LP_CMD_TIM
// Register Offset : 0x18
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_LP_CMD_TIM    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_LP_CMD_TIM)
#define OUTVACT_LPCMD_TIME_SHIFT   DPI_LP_CMD_TIM_OUTVACT_LPCMD_TIME_FIELD_OFFSET
#define OUTVACT_LPCMD_TIME_MASK    0xFF << OUTVACT_LPCMD_TIME_SHIFT
#define INVACT_LPCMD_TIME_SHIFT    DPI_LP_CMD_TIM_INVACT_LPCMD_TIME_FIELD_OFFSET
#define INVACT_LPCMD_TIME_MASK     0xFF << INVACT_LPCMD_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PCKHDL_CFG
// Register Offset : 0x2c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PCKHDL_CFG        REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PCKHDL_CFG)
#define EOTP_TX_LP_EN_SHIFT        PCKHDL_CFG_EOTP_TX_LP_EN_FIELD_OFFSET
#define EOTP_TX_LP_EN_MASK         0x1 << EOTP_TX_LP_EN_SHIFT
#define CRC_RX_EN_SHIFT            PCKHDL_CFG_CRC_RX_EN_FIELD_OFFSET
#define CRC_RX_EN_MASK             0x1 << CRC_RX_EN_SHIFT
#define ECC_RX_EN_SHIFT            PCKHDL_CFG_ECC_RX_EN_FIELD_OFFSET
#define ECC_RX_EN_MASK             0x1 << ECC_RX_EN_SHIFT
#define BTA_EN_SHIFT               PCKHDL_CFG_BTA_EN_FIELD_OFFSET
#define BTA_EN_MASK                0x1 << BTA_EN_SHIFT
#define EOTP_RX_EN_SHIFT           PCKHDL_CFG_EOTP_RX_EN_FIELD_OFFSET
#define EOTP_RX_EN_MASK            0x1 << EOTP_RX_EN_SHIFT
#define EOTP_TX_EN_SHIFT           PCKHDL_CFG_EOTP_TX_EN_FIELD_OFFSET
#define EOTP_TX_EN_MASK            0x1 << EOTP_TX_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_GEN_VCID
// Register Offset : 0x30
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_GEN_VCID         REG(REG_AP_APB_DWC_MIPI_DSI_HOST_GEN_VCID)
#define GEN_VCID_TX_AUTO_SHIFT    GEN_VCID_GEN_VCID_TX_AUTO_FIELD_OFFSET
#define GEN_VCID_TX_AUTO_MASK     0x2 << GEN_VCID_TX_AUTO_SHIFT
#define VCID_TEAR_AUTO_SHIFT      GEN_VCID_GEN_VCID_TEAR_AUTO_FIELD_OFFSET
#define VCID_TEAR_AUTO_MASK       0x2 << VCID_TEAR_AUTO_SHIFT
#define GEN_VCID_RX_SHIFT         GEN_VCID_GEN_VCID_RX_FIELD_OFFSET
#define GEN_VCID_RX_MASK          0x2 << GEN_VCID_RX_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_MODE_CFG
// Register Offset : 0x34
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_MODE_CFG         REG(REG_AP_APB_DWC_MIPI_DSI_HOST_MODE_CFG)
#define CMD_VIDEO_MODE_SHIFT      MODE_CFG_CMD_VIDEO_MODE_FIELD_OFFSET
#define CMD_VIDEO_MODE_MASK       0x1 << CMD_VIDEO_MODE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_MODE_CFG
// Register Offset : 0x38
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_MODE_CFG     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_MODE_CFG)
#define VPG_ORIENTATION_SHIFT     VID_MODE_CFG_VPG_ORIENTATION_FIELD_OFFSET
#define VPG_ORIENTATION_MASK      0x1 << VPG_ORIENTATION_SHIFT
#define VPG_MODE_SHIFT            VID_MODE_CFG_VPG_MODE_FIELD_OFFSET
#define VPG_MODE_MASK             0x1 << VPG_MODE_SHIFT
#define VPG_EN_SHIFT              VID_MODE_CFG_VPG_EN_FIELD_OFFSET
#define VPG_EN_MASK               0x1 << VPG_EN_SHIFT
#define LP_CMD_EN_SHIFT           VID_MODE_CFG_LP_CMD_EN_FIELD_OFFSET
#define LP_CMD_EN_MASK            0x1 << LP_CMD_EN_SHIFT
#define FRAME_BTA_ACK_EN_SHIFT    VID_MODE_CFG_FRAME_BTA_ACK_EN_FIELD_OFFSET
#define FRAME_BTA_ACK_EN_MASK     0x1 << FRAME_BTA_ACK_EN_SHIFT
#define LP_HFP_EN_SHIFT           VID_MODE_CFG_LP_HFP_EN_FIELD_OFFSET
#define LP_HFP_EN_MASK            0x1 << LP_HFP_EN_SHIFT
#define LP_HBP_EN_SHIFT           VID_MODE_CFG_LP_HBP_EN_FIELD_OFFSET
#define LP_HBP_EN_MASK            0x1 << LP_HBP_EN_SHIFT
#define LP_VACT_EN_SHIFT          VID_MODE_CFG_LP_VACT_EN_FIELD_OFFSET
#define LP_VACT_EN_MASK           0x1 << LP_VACT_EN_SHIFT
#define LP_VFP_EN_SHIFT           VID_MODE_CFG_LP_VFP_EN_FIELD_OFFSET
#define LP_VFP_EN_MASK            0x1 << LP_VFP_EN_SHIFT
#define LP_VBP_EN_SHIFT           VID_MODE_CFG_LP_VBP_EN_FIELD_OFFSET
#define LP_VBP_EN_MASK            0x1 << LP_VBP_EN_SHIFT
#define LP_VSA_EN_SHIFT           VID_MODE_CFG_LP_VSA_EN_FIELD_OFFSET
#define LP_VSA_EN_MASK            0x1 << LP_VSA_EN_SHIFT
#define VID_MODE_TYPE_SHIFT       VID_MODE_CFG_VID_MODE_TYPE_FIELD_OFFSET
#define VID_MODE_TYPE_MASK        0x3 << VID_MODE_TYPE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_PKT_SIZE
// Register Offset : 0x3c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_PKT_SIZE     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_PKT_SIZE)
#define VID_PKT_SIZE_SHIFT        VID_PKT_SIZE_VID_PKT_SIZE_FIELD_OFFSET
#define VID_PKT_SIZE_MASK         0x3FFF << VID_PKT_SIZE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_NUM_CHUNKS
// Register Offset : 0x40
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_NUM_CHUNKS   REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_NUM_CHUNKS)
#define VID_NUM_CHUNKS_SHIFT      VID_NUM_CHUNKS_VID_NUM_CHUNKS_FIELD_OFFSET
#define VID_NUM_CHUNKS_MASK       0x1FFF << VID_NUM_CHUNKS_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_NULL_SIZE
// Register Offset : 0x44
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_NULL_SIZE    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_NULL_SIZE)
#define VID_NULL_SIZE_SHIFT       VID_NULL_SIZE_VID_NULL_SIZE_FIELD_OFFSET
#define VID_NULL_SIZE_MASK        0x1FFF << VID_NULL_SIZE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_HSA_TIME
// Register Offset : 0x48
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HSA_TIME     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HSA_TIME)
#define VID_HSA_TIME_SHIFT        VID_HSA_TIME_VID_HSA_TIME_FIELD_OFFSET
#define VID_HSA_TIME_MASK         0xFFF << VID_HSA_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_HBP_TIME
// Register Offset : 0x4c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HBP_TIME     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HBP_TIME)
#define VID_HBP_TIME_SHIFT        VID_HBP_TIME_VID_HBP_TIME_FIELD_OFFSET
#define VID_HBP_TIME_MASK         0xFFF << VID_HBP_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_HLINE_TIME
// Register Offset : 0x50
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HLINE_TIME   REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HLINE_TIME)
#define VID_HLINE_TIME_SHIFT      VID_HLINE_TIME_VID_HLINE_TIME_FIELD_OFFSET
#define VID_HLINE_TIME_MASK       0x7FFF << VID_HLINE_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VSA_LINES
// Register Offset : 0x54
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VSA_LINES    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VSA_LINES)
#define VSA_LINES_SHIFT           VID_VSA_LINES_VSA_LINES_FIELD_OFFSET
#define VSA_LINES_MASK            0x3FF << VSA_LINES_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VBP_LINES
// Register Offset : 0x58
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VBP_LINES    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VBP_LINES)
#define VBP_LINES_SHIFT           VID_VBP_LINES_VBP_LINES_FIELD_OFFSET
#define VBP_LINES_MASK            0x3FF << VBP_LINES_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VFP_LINES
// Register Offset : 0x5c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VFP_LINES    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VFP_LINES)
#define VFP_LINES_SHIFT           VID_VFP_LINES_VFP_LINES_FIELD_OFFSET
#define VFP_LINES_MASK            0x3FF << VFP_LINES_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VACTIVE_LINES
// Register Offset : 0x60
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VACTIVE_LINES REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VACTIVE_LINES)
#define V_ACTIVE_LINES_SHIFT      VID_VACTIVE_LINES_V_ACTIVE_LINES_FIELD_OFFSET
#define V_ACTIVE_LINES_MASK       0x3FFF << V_ACTIVE_LINES_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_CMD_MODE_CFG
// Register Offset : 0x68
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_CMD_MODE_CFG     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_CMD_MODE_CFG)
#define MAX_RD_PKT_SIZE_SHIFT     CMD_MODE_CFG_MAX_RD_PKT_SIZE_FIELD_OFFSET
#define MAX_RD_PKT_SIZE_MASK      0x1 << MAX_RD_PKT_SIZE_SHIFT
#define DCS_LW_TX_SHIFT           CMD_MODE_CFG_DCS_LW_TX_FIELD_OFFSET
#define DCS_LW_TX_MASK            0x1 << DCS_LW_TX_SHIFT
#define DCS_SR_0P_TX_SHIFT        CMD_MODE_CFG_DCS_SR_0P_TX_FIELD_OFFSET
#define DCS_SR_0P_TX_MASK         0x1 << DCS_SR_0P_TX_SHIFT
#define DCS_SW_1P_TX_SHIFT        CMD_MODE_CFG_DCS_SW_1P_TX_FIELD_OFFSET
#define DCS_SW_1P_TX_MASK         0x1 << DCS_SW_1P_TX_SHIFT
#define DCS_SW_0P_TX_SHIFT        CMD_MODE_CFG_DCS_SW_0P_TX_FIELD_OFFSET
#define DCS_SW_0P_TX_MASK         0x1 << DCS_SW_0P_TX_SHIFT
#define GEN_LW_TX_SHIFT           CMD_MODE_CFG_GEN_LW_TX_FIELD_OFFSET
#define GEN_LW_TX_MASK            0x1 << GEN_LW_TX_SHIFT
#define GEN_SR_2P_TX_SHIFT        CMD_MODE_CFG_GEN_SR_2P_TX_FIELD_OFFSET
#define GEN_SR_2P_TX_MASK         0x1 << GEN_SR_2P_TX_SHIFT
#define GEN_SR_1P_TX_SHIFT        CMD_MODE_CFG_GEN_SR_1P_TX_FIELD_OFFSET
#define GEN_SR_1P_TX_MASK         0x1 << GEN_SR_1P_TX_SHIFT
#define GEN_SR_0P_TX_SHIFT        CMD_MODE_CFG_GEN_SR_0P_TX_FIELD_OFFSET
#define GEN_SR_0P_TX_MASK         0x1 << GEN_SR_0P_TX_SHIFT
#define GEN_SW_2P_TX_SHIFT        CMD_MODE_CFG_GEN_SW_2P_TX_FIELD_OFFSET
#define GEN_SW_2P_TX_MASK         0x1 << GEN_SW_2P_TX_SHIFT
#define GEN_SW_1P_TX_SHIFT        CMD_MODE_CFG_GEN_SW_1P_TX_FIELD_OFFSET
#define GEN_SW_1P_TX_MASK         0x1 << GEN_SW_1P_TX_SHIFT
#define GEN_SW_0P_TX_SHIFT        CMD_MODE_CFG_GEN_SW_0P_TX_FIELD_OFFSET
#define GEN_SW_0P_TX_MASK         0x1 << GEN_SW_0P_TX_SHIFT
#define ACK_RQST_EN_SHIFT         CMD_MODE_CFG_ACK_RQST_EN_FIELD_OFFSET
#define ACK_RQST_EN_MASK          0x1 << ACK_RQST_EN_SHIFT
#define TEAR_FX_EN_SHIFT          CMD_MODE_CFG_TEAR_FX_EN_FIELD_OFFSET
#define TEAR_FX_EN_MASK           0x1 << TEAR_FX_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_GEN_HDR
// Register Offset : 0x6c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_GEN_HDR          REG(REG_AP_APB_DWC_MIPI_DSI_HOST_GEN_HDR)
#define GEN_WC_MSBYTE_SHIFT       GEN_HDR_GEN_WC_MSBYTE_FIELD_OFFSET
#define GEN_WC_MSBYTE_MASK        0xFF << GEN_WC_MSBYTE_SHIFT
#define GEN_WC_LSBYTE_SHIFT       GEN_HDR_GEN_WC_LSBYTE_FIELD_OFFSET
#define GEN_WC_LSBYTE_MASK        0xFF << GEN_WC_LSBYTE_SHIFT
#define GEN_VC_SHIFT              GEN_HDR_GEN_VC_FIELD_OFFSET
#define GEN_VC_MASK               0x2 << GEN_VC_SHIFT
#define GEN_DT_SHIFT              GEN_HDR_GEN_DT_FIELD_OFFSET
#define GEN_DT_MASK               0x3F << GEN_DT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_GEN_PLD_DATA
// Register Offset : 0x70
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_GEN_PLD_DATA     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_GEN_PLD_DATA)
#define GEN_PLD_B4_SHIFT          GEN_PLD_DATA_GEN_PLD_B4_FIELD_OFFSET
#define GEN_PLD_B4_MASK           0xFF << GEN_PLD_B4_SHIFT
#define GEN_PLD_B3_SHIFT          GEN_PLD_DATA_GEN_PLD_B3_FIELD_OFFSET
#define GEN_PLD_B3_MASK           0xFF << GEN_PLD_B3_SHIFT
#define GEN_PLD_B2_SHIFT          GEN_PLD_DATA_GEN_PLD_B2_FIELD_OFFSET
#define GEN_PLD_B2_MASK           0xFF << GEN_PLD_B2_SHIFT
#define GEN_PLD_B1_SHIFT          GEN_PLD_DATA_GEN_PLD_B1_FIELD_OFFSET
#define GEN_PLD_B1_MASK           0xFF << GEN_PLD_B1_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_CMD_PKT_STATUS
// Register Offset : 0x74
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_CMD_PKT_STATUS   REG(REG_AP_APB_DWC_MIPI_DSI_HOST_CMD_PKT_STATUS)
#define GEN_BUFF_PLD_FULL_SHIFT   CMD_PKT_STATUS_GEN_BUFF_PLD_FULL_FIELD_OFFSET
#define GEN_BUFF_PLD_FULL_MASK    0x1 << GEN_BUFF_PLD_FULL_SHIFT
#define GEN_BUFF_PLD_EMPTY_SHIFT  CMD_PKT_STATUS_GEN_BUFF_PLD_EMPTY_FIELD_OFFSET
#define GEN_BUFF_PLD_EMPTY_MASK   0x1 << GEN_BUFF_PLD_EMPTY_SHIFT
#define GEN_BUFF_CMD_FULL_SHIFT   CMD_PKT_STATUS_GEN_BUFF_CMD_FULL_FIELD_OFFSET
#define GEN_BUFF_CMD_FULL_MASK    0x1 << GEN_BUFF_CMD_FULL_SHIFT
#define GEN_BUFF_CMD_EMPTY_SHIFT  CMD_PKT_STATUS_GEN_BUFF_CMD_EMPTY_FIELD_OFFSET
#define GEN_BUFF_CMD_EMPTY_MASK   0x1 << GEN_BUFF_CMD_EMPTY_SHIFT
#define GEN_RD_CMD_BUSY_SHIFT     CMD_PKT_STATUS_GEN_RD_CMD_BUSY_FIELD_OFFSET
#define GEN_RD_CMD_BUSY_MASK      0x1 << GEN_RD_CMD_BUSY_SHIFT
#define GEN_PLD_R_FULL_SHIFT      CMD_PKT_STATUS_GEN_PLD_R_FULL_FIELD_OFFSET
#define GEN_PLD_R_FULL_MASK       0x1 << GEN_PLD_R_FULL_SHIFT
#define GEN_PLD_R_EMPTY_SHIFT     CMD_PKT_STATUS_GEN_PLD_R_EMPTY_FIELD_OFFSET
#define GEN_PLD_R_EMPTY_MASK      0x1 << GEN_PLD_R_EMPTY_SHIFT
#define GEN_PLD_W_FULL_SHIFT      CMD_PKT_STATUS_GEN_PLD_W_FULL_FIELD_OFFSET
#define GEN_PLD_W_FULL_MASK       0x1 << GEN_PLD_W_FULL_SHIFT
#define GEN_PLD_W_EMPTY_SHIFT     CMD_PKT_STATUS_GEN_PLD_W_EMPTY_FIELD_OFFSET
#define GEN_PLD_W_EMPTY_MASK      0x1 << GEN_PLD_W_EMPTY_SHIFT
#define GEN_CMD_FULL_SHIFT        CMD_PKT_STATUS_GEN_CMD_FULL_FIELD_OFFSET
#define GEN_CMD_FULL_MASK         0x1 << GEN_CMD_FULL_SHIFT
#define GEN_CMD_EMPTY_SHIFT       CMD_PKT_STATUS_GEN_CMD_EMPTY_FIELD_OFFSET
#define GEN_CMD_EMPTY_MASK        0x1 << GEN_CMD_EMPTY_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_TO_CNT_CFG
// Register Offset : 0x78
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_TO_CNT_CFG       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_TO_CNT_CFG)
#define HSTX_TO_CNT_SHIFT         TO_CNT_CFG_HSTX_TO_CNT_FIELD_OFFSET
#define HSTX_TO_CNT_MASK          0xFFFF << HSTX_TO_CNT_SHIFT
#define LPRX_TO_CNT_SHIFT         TO_CNT_CFG_LPRX_TO_CNT_FIELD_OFFSET
#define LPRX_TO_CNT_MASK          0xFFFF << LPRX_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_HS_RD_TO_CNT
// Register Offset : 0x7c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_HS_RD_TO_CNT     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_HS_RD_TO_CNT)
#define HS_RD_TO_CNT_SHIFT        HS_RD_TO_CNT_HS_RD_TO_CNT_FIELD_OFFSET
#define HS_RD_TO_CNT_MASK         0xFFFF << HS_RD_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_LP_RD_TO_CNT
// Register Offset : 0x80
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_LP_RD_TO_CNT     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_LP_RD_TO_CNT)
#define LP_RD_TO_CNT_SHIFT        LP_RD_TO_CNT_LP_RD_TO_CNT_FIELD_OFFSET
#define LP_RD_TO_CNT_MASK         0xFFFF << LP_RD_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_HS_WR_TO_CNT
// Register Offset : 0x84
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_HS_WR_TO_CNT     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_HS_WR_TO_CNT)
#define PRESP_TO_MODE_SHIFT       HS_WR_TO_CNT_RESERVED_24_FIELD_OFFSET
#define PRESP_TO_MODE_MASK        0x1 << PRESP_TO_MODE_SHIFT
#define HS_WR_TO_CNT_SHIFT        HS_WR_TO_CNT_HS_WR_TO_CNT_FIELD_OFFSET
#define HS_WR_TO_CNT_MASK         0xFFFF << HS_WR_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_LP_WR_TO_CNT
// Register Offset : 0x88
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_LP_WR_TO_CNT     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_LP_WR_TO_CNT)
#define LP_WR_TO_CNT_SHIFT        LP_WR_TO_CNT_LP_WR_TO_CNT_FIELD_OFFSET
#define LP_WR_TO_CNT_MASK         0xFFFF << LP_WR_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_BTA_TO_CNT
// Register Offset : 0x8c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_BTA_TO_CNT       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_BTA_TO_CNT)
#define BTA_TO_CNT_SHIFT          BTA_TO_CNT_BTA_TO_CNT_FIELD_OFFSET
#define BTA_TO_CNT_MASK           0xFFFF << BTA_TO_CNT_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_SDF_3D
// Register Offset : 0x90
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_SDF_3D           REG(REG_AP_APB_DWC_MIPI_DSI_HOST_SDF_3D)
#define SEND_3D_CFG_SHIFT         SDF_3D_SEND_3D_CFG_FIELD_OFFSET
#define SEND_3D_CFG_MASK          0x1 << SEND_3D_CFG_SHIFT
#define RIGHT_FIRST_SHIFT         SDF_3D_RIGHT_FIRST_FIELD_OFFSET
#define RIGHT_FIRST_MASK          0x1 << RIGHT_FIRST_SHIFT
#define SECOND_VSYNC_SHIFT        SDF_3D_SECOND_VSYNC_FIELD_OFFSET
#define SECOND_VSYNC_MASK         0x1 << SECOND_VSYNC_SHIFT
#define FORMAT_3D_SHIFT           SDF_3D_FORMAT_3D_FIELD_OFFSET
#define FORMAT_3D_MASK            0x2 << FORMAT_3D_SHIFT
#define MODE_3D_SHIFT             SDF_3D_MODE_3D_FIELD_OFFSET
#define MODE_3D_MASK              0x2 << MODE_3D_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_LPCLK_CTRL
// Register Offset : 0x94
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_LPCLK_CTRL       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_LPCLK_CTRL)
#define AUTO_CLKLANE_CTRL_SHIFT   LPCLK_CTRL_AUTO_CLKLANE_CTRL_FIELD_OFFSET
#define AUTO_CLKLANE_CTRL_MASK    0x1 << AUTO_CLKLANE_CTRL_SHIFT
#define PHY_TXREQUESTCLKHS_SHIFT  LPCLK_CTRL_PHY_TXREQUESTCLKHS_FIELD_OFFSET
#define PHY_TXREQUESTCLKHS_MASK   0x1 << PHY_TXREQUESTCLKHS_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TMR_LPCLK_CFG
// Register Offset : 0x98
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TMR_LPCLK_CFG REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TMR_LPCLK_CFG)
#define PHY_CLKHS2LP_TIME_SHIFT   PHY_TMR_LPCLK_CFG_PHY_CLKHS2LP_TIME_FIELD_OFFSET
#define PHY_CLKHS2LP_TIME_MASK    0x3FF << PHY_CLKHS2LP_TIME_SHIFT
#define PHY_CLKLP2HS_TIME_SHIFT   PHY_TMR_LPCLK_CFG_PHY_CLKLP2HS_TIME_FIELD_OFFSET
#define PHY_CLKLP2HS_TIME_MASK    0x3FF << PHY_CLKLP2HS_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TMR_CFG
// Register Offset : 0x9c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TMR_CFG      REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TMR_CFG)
#define PHY_HS2LP_TIME_SHIFT      PHY_TMR_CFG_PHY_HS2LP_TIME_FIELD_OFFSET
#define PHY_HS2LP_TIME_MASK       0x3FF << PHY_HS2LP_TIME_SHIFT
#define PHY_LP2HS_TIME_SHIFT      PHY_TMR_CFG_PHY_LP2HS_TIME_FIELD_OFFSET
#define PHY_LP2HS_TIME_MASK       0x3FF << PHY_LP2HS_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_RSTZ
// Register Offset : 0xa0
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_RSTZ         REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_RSTZ)
#define PHY_FORCEPLL_SHIFT        PHY_RSTZ_PHY_FORCEPLL_FIELD_OFFSET
#define PHY_FORCEPLL_MASK         0x1 << PHY_FORCEPLL_SHIFT
#define PHY_ENABLECLK_SHIFT       PHY_RSTZ_PHY_ENABLECLK_FIELD_OFFSET
#define PHY_ENABLECLK_MASK        0x1 << PHY_ENABLECLK_SHIFT
#define PHY_RSTZ_SHIFT            PHY_RSTZ_PHY_RSTZ_FIELD_OFFSET
#define PHY_RSTZ_MASK             0x1 << PHY_RSTZ_SHIFT
#define PHY_SHUTDOWNZ_SHIFT       PHY_RSTZ_PHY_SHUTDOWNZ_FIELD_OFFSET
#define PHY_SHUTDOWNZ_MASK        0x1 << PHY_SHUTDOWNZ_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_IF_CFG
// Register Offset : 0xa4
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_IF_CFG       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_IF_CFG)
#define PHY_STOP_WAIT_TIME_SHIFT  PHY_IF_CFG_PHY_STOP_WAIT_TIME_FIELD_OFFSET
#define PHY_STOP_WAIT_TIME_MASK   0xFF << PHY_STOP_WAIT_TIME_SHIFT
#define N_LANES_SHIFT             PHY_IF_CFG_N_LANES_FIELD_OFFSET
#define N_LANES_MASK              0x3 << N_LANES_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_ULPS_CTRL
// Register Offset : 0xa8
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_ULPS_CTRL    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_ULPS_CTRL)
#define PHY_TXEXITULPSLAN_SHIFT   PHY_ULPS_CTRL_PHY_TXEXITULPSLAN_FIELD_OFFSET
#define PHY_TXEXITULPSLAN_MASK    0x1 << PHY_TXEXITULPSLAN_SHIFT
#define PHY_TXREQULPSLAN_SHIFT    PHY_ULPS_CTRL_PHY_TXREQULPSLAN_FIELD_OFFSET
#define PHY_TXREQULPSLAN_MASK     0x1 << PHY_TXREQULPSLAN_SHIFT
#define PHY_TXEXITULPSCLK_SHIFT   PHY_ULPS_CTRL_PHY_TXEXITULPSCLK_FIELD_OFFSET
#define PHY_TXEXITULPSCLK_MASK    0x1 << PHY_TXEXITULPSCLK_SHIFT
#define PHY_TXREQULPSCLK_SHIFT    PHY_ULPS_CTRL_PHY_TXREQULPSCLK_FIELD_OFFSET
#define PHY_TXREQULPSCLK_MASK     0x1 << PHY_TXREQULPSCLK_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TX_TRIGGERS
// Register Offset : 0xac
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TX_TRIGGERS  REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TX_TRIGGERS)
#define PHY_TX_TRIGGERS_SHIFT     PHY_TX_TRIGGERS_PHY_TX_TRIGGERS_FIELD_OFFSET
#define PHY_TX_TRIGGERS_MASK      0xF << PHY_TX_TRIGGERS_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_STATUS
// Register Offset : 0xb0
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_STATUS       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_STATUS)
#define PHY_ULPSACTIVENOT3LANE_SHIFT PHY_STATUS_PHY_ULPSACTIVENOT3LANE_FIELD_OFFSET
#define PHY_ULPSACTIVENOT3LANE_MASK  0x1 << PHY_ULPSACTIVENOT3LANE_SHIFT
#define PHY_STOPSTATE3LANE_SHIFT  PHY_STATUS_PHY_STOPSTATE3LANE_FIELD_OFFSET
#define PHY_STOPSTATE3LANE_MASK   0x1 << PHY_STOPSTATE3LANE_SHIFT
#define PHY_ULPSACTIVENOT2LANE_SHIFT PHY_STATUS_PHY_ULPSACTIVENOT2LANE_FIELD_OFFSET
#define PHY_ULPSACTIVENOT2LANE_MASK  0x1 << PHY_ULPSACTIVENOT2LANE_SHIFT
#define PHY_STOPSTATE2LANE_SHIFT  PHY_STATUS_PHY_STOPSTATE2LANE_FIELD_OFFSET
#define PHY_STOPSTATE2LANE_MASK   0x1 << PHY_STOPSTATE2LANE_SHIFT
#define PHY_ULPSACTIVENOT1LANE_SHIFT PHY_STATUS_PHY_ULPSACTIVENOT1LANE_FIELD_OFFSET
#define PHY_ULPSACTIVENOT1LANE_MASK  0x1 << PHY_ULPSACTIVENOT1LANE_SHIFT
#define PHY_STOPSTATE1LANE_SHIFT  PHY_STATUS_PHY_STOPSTATE1LANE_FIELD_OFFSET
#define PHY_STOPSTATE1LANE_MASK   0x1 << PHY_STOPSTATE1LANE_SHIFT
#define PHY_RXULPSESC0LANE_SHIFT  PHY_STATUS_PHY_RXULPSESC0LANE_FIELD_OFFSET
#define PHY_RXULPSESC0LANE_MASK   0x1 << PHY_RXULPSESC0LANE_SHIFT
#define PHY_ULPSACTIVENOT0LANE_SHIFT PHY_STATUS_PHY_ULPSACTIVENOT0LANE_FIELD_OFFSET
#define PHY_ULPSACTIVENOT0LANE_MASK  0x1 << PHY_ULPSACTIVENOT0LANE_SHIFT
#define PHY_STOPSTATE0LANE_SHIFT  PHY_STATUS_PHY_STOPSTATE0LANE_FIELD_OFFSET
#define PHY_STOPSTATE0LANE_MASK   0x1 << PHY_STOPSTATE0LANE_SHIFT
#define PHY_ULPSACTIVENOTCLK_SHIFT   PHY_STATUS_PHY_ULPSACTIVENOTCLK_FIELD_OFFSET
#define PHY_ULPSACTIVENOTCLK_MASK    0x1 << PHY_ULPSACTIVENOTCLK_SHIFT
#define PHY_STOPSTATECLKLANE_SHIFT   PHY_STATUS_PHY_STOPSTATECLKLANE_FIELD_OFFSET
#define PHY_STOPSTATECLKLANE_MASK    0x1 << PHY_STOPSTATECLKLANE_SHIFT
#define PHY_DIRECTION_SHIFT       PHY_STATUS_PHY_DIRECTION_FIELD_OFFSET
#define PHY_DIRECTION_MASK        0x1 << PHY_DIRECTION_SHIFT
#define PHY_LOCK_SHIFT            PHY_STATUS_PHY_LOCK_FIELD_OFFSET
#define PHY_LOCK_MASK             0x1 << PHY_LOCK_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TST_CTRL0
// Register Offset : 0xb4
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TST_CTRL0    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TST_CTRL0)
#define PHY_TESTCLK_SHIFT         PHY_TST_CTRL0_PHY_TESTCLK_FIELD_OFFSET
#define PHY_TESTCLK_MASK          0x1 << PHY_TESTCLK_SHIFT
#define PHY_TESTCLR_SHIFT         PHY_TST_CTRL0_PHY_TESTCLR_FIELD_OFFSET
#define PHY_TESTCLR_MASK          0x1 << PHY_TESTCLR_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TST_CTRL1
// Register Offset : 0xb8
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TST_CTRL1    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TST_CTRL1)
#define PHY_TESTEN_SHIFT          PHY_TST_CTRL1_PHY_TESTEN_FIELD_OFFSET
#define PHY_TESTEN_MASK           0x1 << PHY_TESTEN_SHIFT
#define PHT_TESTDOUT_SHIFT        PHY_TST_CTRL1_PHT_TESTDOUT_FIELD_OFFSET
#define PHT_TESTDOUT_MASK         0xFF << PHT_TESTDOUT_SHIFT
#define PHY_TESTDIN_SHIFT         PHY_TST_CTRL1_PHY_TESTDIN_FIELD_OFFSET
#define PHY_TESTDIN_MASK          0xFF << PHY_TESTDIN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_ST0
// Register Offset : 0xbc
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_ST0          REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_ST0)
#define DPHY_ERRORS_4_SHIFT       INT_ST0_DPHY_ERRORS_4_FIELD_OFFSET
#define DPHY_ERRORS_4_MASK        0x1 << DPHY_ERRORS_4_SHIFT
#define DPHY_ERRORS_3_SHIFT       INT_ST0_DPHY_ERRORS_3_FIELD_OFFSET
#define DPHY_ERRORS_3_MASK        0x1 << DPHY_ERRORS_3_SHIFT
#define DPHY_ERRORS_2_SHIFT       INT_ST0_DPHY_ERRORS_2_FIELD_OFFSET
#define DPHY_ERRORS_2_MASK        0x1 << DPHY_ERRORS_2_SHIFT
#define DPHY_ERRORS_1_SHIFT       INT_ST0_DPHY_ERRORS_1_FIELD_OFFSET
#define DPHY_ERRORS_1_MASK        0x1 << DPHY_ERRORS_1_SHIFT
#define DPHY_ERRORS_0_SHIFT       INT_ST0_DPHY_ERRORS_0_FIELD_OFFSET
#define DPHY_ERRORS_0_MASK        0x1 << DPHY_ERRORS_0_SHIFT
#define ACK_WITH_ERR_15_SHIFT     INT_ST0_ACK_WITH_ERR_15_FIELD_OFFSET
#define ACK_WITH_ERR_15_MASK      0x1 << ACK_WITH_ERR_15_SHIFT
#define ACK_WITH_ERR_14_SHIFT     INT_ST0_ACK_WITH_ERR_14_FIELD_OFFSET
#define ACK_WITH_ERR_14_MASK      0x1 << ACK_WITH_ERR_14_SHIFT
#define ACK_WITH_ERR_13_SHIFT     INT_ST0_ACK_WITH_ERR_13_FIELD_OFFSET
#define ACK_WITH_ERR_13_MASK      0x1 << ACK_WITH_ERR_13_SHIFT
#define ACK_WITH_ERR_12_SHIFT     INT_ST0_ACK_WITH_ERR_12_FIELD_OFFSET
#define ACK_WITH_ERR_12_MASK      0x1 << ACK_WITH_ERR_12_SHIFT
#define ACK_WITH_ERR_11_SHIFT     INT_ST0_ACK_WITH_ERR_11_FIELD_OFFSET
#define ACK_WITH_ERR_11_MASK      0x1 << ACK_WITH_ERR_11_SHIFT
#define ACK_WITH_ERR_10_SHIFT     INT_ST0_ACK_WITH_ERR_10_FIELD_OFFSET
#define ACK_WITH_ERR_10_MASK      0x1 << ACK_WITH_ERR_10_SHIFT
#define ACK_WITH_ERR_9_SHIFT      INT_ST0_ACK_WITH_ERR_9_FIELD_OFFSET
#define ACK_WITH_ERR_9_MASK       0x1 << ACK_WITH_ERR_9_SHIFT
#define ACK_WITH_ERR_8_SHIFT      INT_ST0_ACK_WITH_ERR_8_FIELD_OFFSET
#define ACK_WITH_ERR_8_MASK       0x1 << ACK_WITH_ERR_8_SHIFT
#define ACK_WITH_ERR_7_SHIFT      INT_ST0_ACK_WITH_ERR_7_FIELD_OFFSET
#define ACK_WITH_ERR_7_MASK       0x1 << ACK_WITH_ERR_7_SHIFT
#define ACK_WITH_ERR_6_SHIFT      INT_ST0_ACK_WITH_ERR_6_FIELD_OFFSET
#define ACK_WITH_ERR_6_MASK       0x1 << ACK_WITH_ERR_6_SHIFT
#define ACK_WITH_ERR_5_SHIFT      INT_ST0_ACK_WITH_ERR_5_FIELD_OFFSET
#define ACK_WITH_ERR_5_MASK       0x1 << ACK_WITH_ERR_5_SHIFT
#define ACK_WITH_ERR_4_SHIFT      INT_ST0_ACK_WITH_ERR_4_FIELD_OFFSET
#define ACK_WITH_ERR_4_MASK       0x1 << ACK_WITH_ERR_4_SHIFT
#define ACK_WITH_ERR_3_SHIFT      INT_ST0_ACK_WITH_ERR_3_FIELD_OFFSET
#define ACK_WITH_ERR_3_MASK       0x1 << ACK_WITH_ERR_3_SHIFT
#define ACK_WITH_ERR_2_SHIFT      INT_ST0_ACK_WITH_ERR_2_FIELD_OFFSET
#define ACK_WITH_ERR_2_MASK       0x1 << ACK_WITH_ERR_2_SHIFT
#define ACK_WITH_ERR_1_SHIFT      INT_ST0_ACK_WITH_ERR_1_FIELD_OFFSET
#define ACK_WITH_ERR_1_MASK       0x1 << ACK_WITH_ERR_1_SHIFT
#define ACK_WITH_ERR_0_SHIFT      INT_ST0_ACK_WITH_ERR_0_FIELD_OFFSET
#define ACK_WITH_ERR_0_MASK       0x1 << ACK_WITH_ERR_0_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_ST1
// Register Offset : 0xc0
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_ST1          REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_ST1)
#define TEAR_REQUEST_ERR_SHIFT    INT_ST1_RESERVED_20_FIELD_OFFSET
#define TEAR_REQUEST_ERR_MASK     0x1 << TEAR_REQUEST_ERR_SHIFT
#define DPI_BUFF_PLD_UNDER_SHIFT  INT_ST1_DPI_BUFF_PLD_UNDER_FIELD_OFFSET
#define DPI_BUFF_PLD_UNDER_MASK   0x1 << DPI_BUFF_PLD_UNDER_SHIFT
#define GEN_PLD_RECEV_ERR_SHIFT   INT_ST1_GEN_PLD_RECEV_ERR_FIELD_OFFSET
#define GEN_PLD_RECEV_ERR_MASK    0x1 << GEN_PLD_RECEV_ERR_SHIFT
#define GEN_PLD_RD_ERR_SHIFT      INT_ST1_GEN_PLD_RD_ERR_FIELD_OFFSET
#define GEN_PLD_RD_ERR_MASK       0x1 << GEN_PLD_RD_ERR_SHIFT
#define GEN_PLD_SEND_ERR_SHIFT    INT_ST1_GEN_PLD_SEND_ERR_FIELD_OFFSET
#define GEN_PLD_SEND_ERR_MASK     0x1 << GEN_PLD_SEND_ERR_SHIFT
#define GEN_PLD_WR_ERR_SHIFT      INT_ST1_GEN_PLD_WR_ERR_FIELD_OFFSET
#define GEN_PLD_WR_ERR_MASK       0x1 << GEN_PLD_WR_ERR_SHIFT
#define GEN_CMD_WR_ERR_SHIFT      INT_ST1_GEN_CMD_WR_ERR_FIELD_OFFSET
#define GEN_CMD_WR_ERR_MASK       0x1 << GEN_CMD_WR_ERR_SHIFT
#define DPI_PLD_WR_ERR_SHIFT      INT_ST1_DPI_PLD_WR_ERR_FIELD_OFFSET
#define DPI_PLD_WR_ERR_MASK       0x1 << DPI_PLD_WR_ERR_SHIFT
#define EOPT_ERR_SHIFT            INT_ST1_EOPT_ERR_FIELD_OFFSET
#define EOPT_ERR_MASK             0x1 << EOPT_ERR_SHIFT
#define PKT_SIZE_ERR_SHIFT        INT_ST1_PKT_SIZE_ERR_FIELD_OFFSET
#define PKT_SIZE_ERR_MASK         0x1 << PKT_SIZE_ERR_SHIFT
#define CRC_ERR_SHIFT             INT_ST1_CRC_ERR_FIELD_OFFSET
#define CRC_ERR_MASK              0x1 << CRC_ERR_SHIFT
#define ECC_MILTI_ERR_SHIFT       INT_ST1_ECC_MILTI_ERR_FIELD_OFFSET
#define ECC_MILTI_ERR_MASK        0x1 << ECC_MILTI_ERR_SHIFT
#define ECC_SINGLE_ERR_SHIFT      INT_ST1_ECC_SINGLE_ERR_FIELD_OFFSET
#define ECC_SINGLE_ERR_MASK       0x1 << ECC_SINGLE_ERR_SHIFT
#define TO_LP_RX_SHIFT            INT_ST1_TO_LP_RX_FIELD_OFFSET
#define TO_LP_RX_MASK             0x1 << TO_LP_RX_SHIFT
#define TO_HS_TX_SHIFT            INT_ST1_TO_HS_TX_FIELD_OFFSET
#define TO_HS_TX_MASK             0x1 << TO_HS_TX_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_MSK0
// Register Offset : 0xc4
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_MSK0         REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_MSK0)
/*The same as MIPI_DSI_INT_ST0*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_MSK1
// Register Offset : 0xc8
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_MSK1         REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_MSK1)
/*The same as MIPI_DSI_INT_ST1*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_CAL
// Register Offset : 0xcc
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_CAL          REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_CAL)
#define TXSKEWCALHS_SHIFT         PHY_CAL_TXSKEWCALHS_FIELD_OFFSET
#define TXSKEWCALHS_MASK          0x1 << TXSKEWCALHS_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_FORCE0
// Register Offset : 0xd8
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_FORCE0       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_FORCE0)
/*The same as MIPI_DSI_INT_ST0*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_INT_FORCE1
// Register Offset : 0xdc
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_INT_FORCE1       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_INT_FORCE1)
/*The same as MIPI_DSI_INT_ST1*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DSC_PARAMETER
// Register Offset : 0xf0
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DSC_PARAMETER    REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DSC_PARAMETER)
#define PPS_SEL_SHIFT             DSC_PARAMETER_PPS_SEL_FIELD_OFFSET
#define PPS_SEL_MASK              0x2 << PPS_SEL_SHIFT
#define COMPRESS_ALGO_SHIFT       DSC_PARAMETER_COMPRESS_ALGO_FIELD_OFFSET
#define COMPRESS_ALGO_MASK        0x2 << COMPRESS_ALGO_SHIFT
#define COMPRESSION_MODE_SHIFT    DSC_PARAMETER_COMPRESSION_MODE_FIELD_OFFSET
#define COMPRESSION_MODE_MASK     0x1 << COMPRESSION_MODE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_PHY_TMR_RD_CFG
// Register Offset : 0xf4
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_PHY_TMR_RD_CFG   REG(REG_AP_APB_DWC_MIPI_DSI_HOST_PHY_TMR_RD_CFG)
#define MAX_RD_TIME_SHIFT         PHY_TMR_RD_CFG_MAX_RD_TIME_FIELD_OFFSET
#define MAX_RD_TIME_MASK          0x7FFF << MAX_RD_TIME_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_SHADOW_CTRL
// Register Offset : 0x100
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_SHADOW_CTRL  REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_SHADOW_CTRL)
#define VID_SHADOW_PIN_REQ_SHIFT  VID_SHADOW_CTRL_VID_SHADOW_PIN_REQ_FIELD_OFFSET
#define VID_SHADOW_PIN_REQ_MASK   0x1 << VID_SHADOW_PIN_REQ_SHIFT
#define VID_SHADOW_REQ_SHIFT      VID_SHADOW_CTRL_VID_SHADOW_REQ_FIELD_OFFSET
#define VID_SHADOW_REQ_MASK       0x1 << VID_SHADOW_REQ_SHIFT
#define VID_SHADOW_EN_SHIFT       VID_SHADOW_CTRL_VID_SHADOW_EN_FIELD_OFFSET
#define VID_SHADOW_EN_MASK        0x1 << VID_SHADOW_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_VCID_ACT
// Register Offset : 0x10c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_VCID_ACT     REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_VCID_ACT)
/*The same as MIPI_DSI_DPI_VCID*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_COLOR_CODING_ACT
// Register Offset : 0x110
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_COLOR_CODING_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_COLOR_CODING_ACT)
/*The same as MIPI_DSI_DPI_COLOR_CODING*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_DPI_LP_CMD_TIM_ACT
// Register Offset : 0x118
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_DPI_LP_CMD_TIM_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_DPI_LP_CMD_TIM_ACT)
/*The same as MIPI_DSI_DPI_LP_CMD_TIM*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_MODE_CFG_ACT
// Register Offset : 0x138
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_MODE_CFG_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_MODE_CFG_ACT)
#define ACT_LP_CMD_EN_SHIFT       VID_MODE_CFG_ACT_LP_CMD_EN_FIELD_OFFSET
#define ACT_LP_CMD_EN_MASK        0x1 << ACT_LP_CMD_EN_SHIFT
#define ACT_FRAME_BTA_ACK_EN_SHIFT VID_MODE_CFG_ACT_FRAME_BTA_ACK_EN_FIELD_OFFSET
#define ACT_FRAME_BTA_ACK_EN_MASK  0x1 << ACT_FRAME_BTA_ACK_EN_SHIFT
#define ACT_LP_HFP_EN_SHIFT       VID_MODE_CFG_ACT_LP_HFP_EN_FIELD_OFFSET
#define ACT_LP_HFP_EN_MASK        0x1 << ACT_LP_HFP_EN_SHIFT
#define ACT_LP_HBP_EN_SHIFT       VID_MODE_CFG_ACT_LP_HBP_EN_FIELD_OFFSET
#define ACT_LP_HBP_EN_MASK        0x1 << ACT_LP_HBP_EN_SHIFT
#define ACT_LP_VACT_EN_SHIFT      VID_MODE_CFG_ACT_LP_VACT_EN_FIELD_OFFSET
#define ACT_LP_VACT_EN_MASK       0x1 << ACT_LP_VACT_EN_SHIFT
#define ACT_LP_VFP_EN_SHIFT       VID_MODE_CFG_ACT_LP_VFP_EN_FIELD_OFFSET
#define ACT_LP_VFP_EN_MASK        0x1 << ACT_LP_VFP_EN_SHIFT
#define ACT_LP_VBP_EN_SHIFT       VID_MODE_CFG_ACT_LP_VBP_EN_FIELD_OFFSET
#define ACT_LP_VBP_EN_MASK        0x1 << ACT_LP_VBP_EN_SHIFT
#define ACT_LP_VSA_EN_SHIFT       VID_MODE_CFG_ACT_LP_VSA_EN_FIELD_OFFSET
#define ACT_LP_VSA_EN_MASK        0x1 << ACT_LP_VSA_EN_SHIFT
#define ACT_VID_MODE_TYPE_SHIFT   VID_MODE_CFG_ACT_VID_MODE_TYPE_FIELD_OFFSET
#define ACT_VID_MODE_TYPE_MASK    0x2 << ACT_VID_MODE_TYPE_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_PKT_SIZE_ACT
// Register Offset : 0x13c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_PKT_SIZE_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_PKT_SIZE_ACT)
/*The same as MIPI_DSI_VID_PKT_SIZE*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_NUM_CHUNKS_ACT
// Register Offset : 0x140
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_NUM_CHUNKS_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_NUM_CHUNKS_ACT)
/*The same as MIPI_DSI_VID_NUM_CHUNKS*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_NULL_SIZE_ACT
// Register Offset : 0x144
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_NULL_SIZE_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_NULL_SIZE_ACT)
/*The same as MIPI_DSI_VID_NULL_SIZE*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_HSA_TIME_ACT
// Register Offset : 0x148
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HSA_TIME_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HSA_TIME_ACT)
/*The same as MIPI_DSI_VID_HSA_TIME*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_HBP_TIME_ACT
// Register Offset : 0x14c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HBP_TIME_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HBP_TIME_ACT)
/*The same as MIPI_DSI_VID_HBP_TIME_*/

// Register Name   : MIPI_DSI_VID_HLINE_TIME_ACT
// Register Offset : 0x150
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_HLINE_TIME_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_HLINE_TIME_ACT)
/*The same as MIPI_DSI_VID_HLINE_TIME*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VSA_LINES_ACT
// Register Offset : 0x154
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VSA_LINES_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VSA_LINES_ACT)
/*The same as MIPI_DSI_VID_VSA_LINES*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VBP_LINES_ACT
// Register Offset : 0x158
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VBP_LINES_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VBP_LINES_ACT)
/*The same as MIPI_DSI_VID_VBP_LINES*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VFP_LINES_ACT
// Register Offset : 0x15c
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VFP_LINES_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VFP_LINES_ACT)
/*The same as MIPI_DSI_VID_VFP_LINES*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_VACTIVE_LINES_ACT
// Register Offset : 0x160
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_VACTIVE_LINES_ACT REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_VACTIVE_LINES_ACT)
/*The same as MIPI_DSI_VID_VACTIVE_LINES*/

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_VID_PKT_STATUS
// Register Offset : 0x168
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_VID_PKT_STATUS   REG(REG_AP_APB_DWC_MIPI_DSI_HOST_VID_PKT_STATUS)
#define DPI_BUFF_PLD_FULL_SHIFT   VID_PKT_STATUS_DPI_BUFF_PLD_FULL_FIELD_OFFSET
#define DPI_BUFF_PLD_FULL_MASK    0x1 << DPI_BUFF_PLD_FULL_SHIFT
#define DPI_BUFF_PLD_EMPTY_SHIFT  VID_PKT_STATUS_DPI_BUFF_PLD_EMPTY_FIELD_OFFSET
#define DPI_BUFF_PLD_EMPTY_MASK   0x1 << DPI_BUFF_PLD_EMPTY_SHIFT
#define DPI_PLD_W_FULL_SHIFT      VID_PKT_STATUS_DPI_PLD_W_FULL_FIELD_OFFSET
#define DPI_PLD_W_FULL_MASK       0x1 << DPI_PLD_W_FULL_SHIFT
#define DPI_PLD_W_EMPTY_SHIFT     VID_PKT_STATUS_DPI_PLD_W_EMPTY_FIELD_OFFSET
#define DPI_PLD_W_EMPTY_MASK      0x1 << DPI_PLD_W_EMPTY_SHIFT
#define DPI_CMD_W_FULL_SHIFT      VID_PKT_STATUS_DPI_CMD_W_FULL_FIELD_OFFSET
#define DPI_CMD_W_FULL_MASK       0x1 << DPI_CMD_W_FULL_SHIFT
#define DPI_CMD_W_EMPTY_SHIFT     VID_PKT_STATUS_DPI_CMD_W_EMPTY_FIELD_OFFSET
#define DPI_CMD_W_EMPTY_MASK      0x1 << DPI_CMD_W_EMPTY_SHIFT

//--------------------------------------------------------------------------
// Register Name   : MIPI_DSI_SDF_3D_ACT
// Register Offset : 0x190
// Description     :
//--------------------------------------------------------------------------
#define MIPI_DSI_SDF_3D_ACT       REG(REG_AP_APB_DWC_MIPI_DSI_HOST_SDF_3D_ACT)
/*The same as MIPI_DSI_SDF_3D*/

#endif //__DSI_REG_H__