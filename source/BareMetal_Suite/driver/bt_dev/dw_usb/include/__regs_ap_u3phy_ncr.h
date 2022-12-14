/*
* __regs_ap_u3phy_ncr.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: synopsys usb registers header file
*
* Revision History:
* -----------------
* 011, 3/8/2019 chenqing create this file
*/
#ifndef _AP_APB_U3PHY_NCR_REG_H
#define _AP_APB_U3PHY_NCR_REG_H
//--------------------------------------------------------------------------
// IP Ref Info     : REG_AP_APB_U3PHY_NCR
// RTL version     :
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Address Block Name : U3PHY_NCR_APB_AB0
// Description        :
//--------------------------------------------------------------------------
#define U3PHY_NCR_APB_AB0_BASE_ADDR 0x10000
//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_0
// Register Offset : 0x0
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_0 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x0)
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_TEST_BURNIN    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_TX_VBOOST_LVL_2    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_TX_VBOOST_LVL_1    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_TX_VBOOST_LVL_0    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_RANGE_2    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_RANGE_1    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_RANGE_0    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_EN    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_RTUNE_REQ    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_USE_XO    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_XO_EN    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_REPEAT_CLK_EN    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_CLKDIV2    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_USE_PAD    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_REF_SSP_EN    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_8    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_7    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_6    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_5    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_4    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_3    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_2    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_1    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_SSC_REF_CLK_SEL_0    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_ALT_CLK_EN    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PHYCLKDEBUG_SRC_3    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PHYCLKDEBUG_SRC_2    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PHYCLKDEBUG_SRC_1    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PHYCLKDEBUG_SRC_0    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PIPE_RESET    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PORT_RESET    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_0_PHY_RESET    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_1
// Register Offset : 0x4
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_1 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x4)
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_COMPDISTUNE0_2    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_COMPDISTUNE0_1    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_COMPDISTUNE0_0    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_BIAS_2    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_BIAS_1    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_BIAS_0    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_LEVEL_4    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_LEVEL_3    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_LEVEL_2    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_LEVEL_1    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_LOS_LEVEL_0    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_TEST_POWERDOWN_SSP    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_TEST_POWERDOWN_HSP    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_5    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_4    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_3    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_2    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_1    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_FSEL_0    (BIT_(13))
#define FS_AP_APB_U3PHY_NCR_CTRL_1_FSEL         13
#define FM_AP_APB_U3PHY_NCR_CTRL_1_FSEL \
    (0x1fu << FS_AP_APB_U3PHY_NCR_CTRL_1_FSEL)
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_ACJT_LEVEL_4    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_ACJT_LEVEL_3    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_ACJT_LEVEL_2    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_ACJT_LEVEL_1    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_ACJT_LEVEL_0    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_REFSSC_CLK_EN    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_1_MPLL_MULTIPLIER_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_2
// Register Offset : 0x8
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_2 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x8)
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_CHRGSEL0    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_LANE0_TX2RX_LOOPBK    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_LANE0_EXT_PCLK_REQ    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_5    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_4    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_3    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_2    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_1    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_6DB_0    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_5    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_4    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_3    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_2    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_1    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_DEEMPH_3P5DB_0    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_9    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_8    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_7    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_6    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_5    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_4    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_3    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_2    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_1    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_RX_LOS_MASK_VAL_0    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_2_PCS_TX_SWING_FULL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_3
// Register Offset : 0xc
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_3 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0xc)
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_VATESTENB    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_SQRXTUNE0_2    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_SQRXTUNE0_1    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_SQRXTUNE0_0    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_RX0LOSLFPSEN    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_RETENABLEN    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_REFCLKSEL_1    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_REFCLKSEL_0    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_OTGTUNE0_2    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_OTGTUNE0_1    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_OTGTUNE0_0    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_OTGDISABLE0    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LOOPBACKENB0    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_HSXCVREXTCTL0    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_CHRGSRCPUENB0_1    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_CHRGSRCPUENB0_0    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_AUTORSMENB0    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_ATERESET    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_FSSE0EXT0    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_FSDATAEXT0    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_TXENABLEN0    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_TXBITSTUFFENH0    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_TXBITSTUFFEN0    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_IDPULLUP0    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_VDATSRCENB0    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_VDATDETENB0    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_DCDENB0    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LANE0_TX_TERM_OFFSET_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LANE0_TX_TERM_OFFSET_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LANE0_TX_TERM_OFFSET_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LANE0_TX_TERM_OFFSET_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_3_LANE0_TX_TERM_OFFSET_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_4
// Register Offset : 0x10
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_4 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x10)
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_REF_CLK_SW_32K    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_CR_CKDIV_2    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_CR_CKDIV_1    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_CR_CKDIV_0    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_CR_MODE    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_COMMONONN    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_BYPASSSEL0    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_BYPASSDPEN0    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_BYPASSDPDATA0    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_BYPASSDMEN0    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_BYPASSDMDATA0    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_VBUSVLDEXTSEL0    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_VBUSVLDEXT0    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_VDATREFTUNE0_1    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_VDATREFTUNE0_0    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXVREFTUNE0_3    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXVREFTUNE0_2    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXVREFTUNE0_1    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXVREFTUNE0_0    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXRISETUNE0_1    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXRISETUNE0_0    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXRESTUNE0_1    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXRESTUNE0_0    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXPREEMPPULSETUNE0    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXPREEMPAMPTUNE0_1    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXPREEMPAMPTUNE0_0    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXHSXVTUNE0_1    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXHSXVTUNE0_0    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXFSLSTUNE0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXFSLSTUNE0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXFSLSTUNE0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_4_TXFSLSTUNE0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_5
// Register Offset : 0x14
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_5 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x14)
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_5_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_6
// Register Offset : 0x18
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_6 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x18)
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_6_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_CTRL_7
// Register Offset : 0x1c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_CTRL_7 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x1c)
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_CTRL_7_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_0
// Register Offset : 0x80
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_0 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x80)
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_19    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_18    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_17    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_16    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_15    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_14    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_13    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_12    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_11    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_10    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_9    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_8    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_7    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_6    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_5    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_4    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_3    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_2    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_1    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RSVD0_0    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_0_BVALID0    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_0_AVALID0    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_0_OTGSESSVLD0    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_0_IDDIG0    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_0_HSSQUELCH0    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_0_HSRXDAT0    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_0_FSVPLUS0    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_0_FSVMINUS0    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_0_FSLSRCV0    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_0_RTUNE_ACK    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_0_REF_CLKREQ_N    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_0_ALT_CLK_REQ    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_1
// Register Offset : 0x84
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_1 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x84)
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_1_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_2
// Register Offset : 0x88
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_2 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x88)
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_2_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_3
// Register Offset : 0x8c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_3 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x8c)
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_3_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_4
// Register Offset : 0x90
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_4 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x90)
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_4_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_5
// Register Offset : 0x94
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_5 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x94)
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_5_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_6
// Register Offset : 0x98
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_6 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x98)
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_6_RSVD0_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_U3PHY_NCR_STS_7
// Register Offset : 0x9c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_U3PHY_NCR_STS_7 (U3PHY_NCR_APB_AB0_BASE_ADDR + 0x9c)
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_31    (BIT_(31))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_30    (BIT_(30))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_29    (BIT_(29))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_28    (BIT_(28))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_27    (BIT_(27))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_26    (BIT_(26))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_25    (BIT_(25))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_24    (BIT_(24))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_23    (BIT_(23))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_22    (BIT_(22))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_21    (BIT_(21))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_20    (BIT_(20))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_19    (BIT_(19))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_18    (BIT_(18))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_17    (BIT_(17))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_16    (BIT_(16))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_15    (BIT_(15))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_14    (BIT_(14))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_13    (BIT_(13))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_12    (BIT_(12))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_11    (BIT_(11))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_10    (BIT_(10))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_9    (BIT_(9))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_8    (BIT_(8))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_7    (BIT_(7))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_6    (BIT_(6))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_5    (BIT_(5))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_4    (BIT_(4))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_3    (BIT_(3))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_2    (BIT_(2))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_1    (BIT_(1))
#define BIT_AP_APB_U3PHY_NCR_STS_7_RSVD0_0    (BIT_(0))



#endif
