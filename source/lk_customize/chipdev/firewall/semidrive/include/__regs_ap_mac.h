/*
 * Copyright (c) Semidrive
 */

#ifndef _AP_APB_MAC_REG_H
#define _AP_APB_MAC_REG_H

//--------------------------------------------------------------------------
// IP Ref Info     : REG_AP_APB_MAC
// RTL version     :
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Address Block Name : MAC_APB_MAC_APB_AB0
// Description        :
//--------------------------------------------------------------------------
#define MAC_APB_MAC_APB_AB0_BASE_ADDR 0x0
//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GLB_CTL
// Register Offset : 0x0
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GLB_CTL (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x0)
#define GLB_CTL_DOM_CFG_LOCK_FIELD_OFFSET 5
#define GLB_CTL_DOM_CFG_LOCK_FIELD_SIZE 1
#define GLB_CTL_DOM_CFG_MODE_FIELD_OFFSET 4
#define GLB_CTL_DOM_CFG_MODE_FIELD_SIZE 1
#define GLB_CTL_PERCK_DIS_LOCK_FIELD_OFFSET 3
#define GLB_CTL_PERCK_DIS_LOCK_FIELD_SIZE 1
#define GLB_CTL_PERCK_DIS_FIELD_OFFSET 2
#define GLB_CTL_PERCK_DIS_FIELD_SIZE 1
#define GLB_CTL_DOM_PRO_LOCK_FIELD_OFFSET 1
#define GLB_CTL_DOM_PRO_LOCK_FIELD_SIZE 1
#define GLB_CTL_DOM_PRO_EN_FIELD_OFFSET 0
#define GLB_CTL_DOM_PRO_EN_FIELD_SIZE 1

#define BIT_AP_APB_MAC_GLB_CTL_DOM_CFG_LOCK    (BIT_(5))
#define BIT_AP_APB_MAC_GLB_CTL_DOM_CFG_MODE    (BIT_(4))
#define BIT_AP_APB_MAC_GLB_CTL_PERCK_DIS_LOCK    (BIT_(3))
#define BIT_AP_APB_MAC_GLB_CTL_PERCK_DIS    (BIT_(2))
#define BIT_AP_APB_MAC_GLB_CTL_DOM_PRO_LOCK    (BIT_(1))
#define BIT_AP_APB_MAC_GLB_CTL_DOM_PRO_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_RES_MGR
// Register Offset : 0x4
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_RES_MGR (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x4)
#define RES_MGR_MID_LOCK_FIELD_OFFSET 17
#define RES_MGR_MID_LOCK_FIELD_SIZE 1
#define RES_MGR_MID_EN_FIELD_OFFSET 16
#define RES_MGR_MID_EN_FIELD_SIZE 1
#define RES_MGR_PRI_PER_LOCK_FIELD_OFFSET 15
#define RES_MGR_PRI_PER_LOCK_FIELD_SIZE 1
#define RES_MGR_PRI_PER_FIELD_OFFSET 13
#define RES_MGR_PRI_PER_FIELD_SIZE 2
#define RES_MGR_PRI_PER_EN_FIELD_OFFSET 12
#define RES_MGR_PRI_PER_EN_FIELD_SIZE 1
#define RES_MGR_SEC_PER_LOCK_FIELD_OFFSET 11
#define RES_MGR_SEC_PER_LOCK_FIELD_SIZE 1
#define RES_MGR_SEC_PER_FIELD_OFFSET 9
#define RES_MGR_SEC_PER_FIELD_SIZE 2
#define RES_MGR_SEC_PER_EN_FIELD_OFFSET 8
#define RES_MGR_SEC_PER_EN_FIELD_SIZE 1
#define RES_MGR_DID_LOCK_FIELD_OFFSET 7
#define RES_MGR_DID_LOCK_FIELD_SIZE 1
#define RES_MGR_DID_FIELD_OFFSET 3
#define RES_MGR_DID_FIELD_SIZE 4
#define RES_MGR_DID_EN_FIELD_OFFSET 2
#define RES_MGR_DID_EN_FIELD_SIZE 1
#define RES_MGR_RES_MGR_EN_LOCK_FIELD_OFFSET 1
#define RES_MGR_RES_MGR_EN_LOCK_FIELD_SIZE 1
#define RES_MGR_RES_MGR_EN_FIELD_OFFSET 0
#define RES_MGR_RES_MGR_EN_FIELD_SIZE 1

#define BIT_AP_APB_MAC_RES_MGR_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_MAC_RES_MGR_MID_EN    (BIT_(16))
#define BIT_AP_APB_MAC_RES_MGR_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_MAC_RES_MGR_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_MAC_RES_MGR_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_MAC_RES_MGR_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_MAC_RES_MGR_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_MAC_RES_MGR_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_MAC_RES_MGR_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_MAC_RES_MGR_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_MAC_RES_MGR_DID_LOCK    (BIT_(7))
#define BIT_AP_APB_MAC_RES_MGR_DID_3    (BIT_(6))
#define BIT_AP_APB_MAC_RES_MGR_DID_2    (BIT_(5))
#define BIT_AP_APB_MAC_RES_MGR_DID_1    (BIT_(4))
#define BIT_AP_APB_MAC_RES_MGR_DID_0    (BIT_(3))
#define BIT_AP_APB_MAC_RES_MGR_DID_EN    (BIT_(2))
#define BIT_AP_APB_MAC_RES_MGR_RES_MGR_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_MAC_RES_MGR_RES_MGR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_RES_MGR_MA0
// Register Offset : 0x8
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_RES_MGR_MA0 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x8)
#define RES_MGR_MA0_MID_FIELD_OFFSET 0
#define RES_MGR_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_RES_MGR_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_RES_MGR_MA1
// Register Offset : 0xc
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_RES_MGR_MA1 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0xc)
#define RES_MGR_MA1_MID_FIELD_OFFSET 0
#define RES_MGR_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_RES_MGR_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_RES_MGR_MA2
// Register Offset : 0x10
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_RES_MGR_MA2 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x10)
#define RES_MGR_MA2_MID_FIELD_OFFSET 0
#define RES_MGR_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_RES_MGR_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_RES_MGR_MA3
// Register Offset : 0x14
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_RES_MGR_MA3 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x14)
#define RES_MGR_MA3_MID_FIELD_OFFSET 0
#define RES_MGR_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_RES_MGR_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GRP_MGR
// Register Offset : 0x100
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GRP_MGR (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x100)
#define GRP_MGR_MID_LOCK_FIELD_OFFSET 17
#define GRP_MGR_MID_LOCK_FIELD_SIZE 1
#define GRP_MGR_MID_EN_FIELD_OFFSET 16
#define GRP_MGR_MID_EN_FIELD_SIZE 1
#define GRP_MGR_PRI_PER_LOCK_FIELD_OFFSET 15
#define GRP_MGR_PRI_PER_LOCK_FIELD_SIZE 1
#define GRP_MGR_PRI_PER_FIELD_OFFSET 13
#define GRP_MGR_PRI_PER_FIELD_SIZE 2
#define GRP_MGR_PRI_PER_EN_FIELD_OFFSET 12
#define GRP_MGR_PRI_PER_EN_FIELD_SIZE 1
#define GRP_MGR_SEC_PER_LOCK_FIELD_OFFSET 11
#define GRP_MGR_SEC_PER_LOCK_FIELD_SIZE 1
#define GRP_MGR_SEC_PER_FIELD_OFFSET 9
#define GRP_MGR_SEC_PER_FIELD_SIZE 2
#define GRP_MGR_SEC_PER_EN_FIELD_OFFSET 8
#define GRP_MGR_SEC_PER_EN_FIELD_SIZE 1
#define GRP_MGR_DID_LOCK_FIELD_OFFSET 7
#define GRP_MGR_DID_LOCK_FIELD_SIZE 1
#define GRP_MGR_DID_FIELD_OFFSET 3
#define GRP_MGR_DID_FIELD_SIZE 4
#define GRP_MGR_DID_EN_FIELD_OFFSET 2
#define GRP_MGR_DID_EN_FIELD_SIZE 1
#define GRP_MGR_GRP_MGR_EN_LOCK_FIELD_OFFSET 1
#define GRP_MGR_GRP_MGR_EN_LOCK_FIELD_SIZE 1
#define GRP_MGR_GRP_MGR_EN_FIELD_OFFSET 0
#define GRP_MGR_GRP_MGR_EN_FIELD_SIZE 1

#define BIT_AP_APB_MAC_GRP_MGR_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_MAC_GRP_MGR_MID_EN    (BIT_(16))
#define BIT_AP_APB_MAC_GRP_MGR_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_MAC_GRP_MGR_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_MAC_GRP_MGR_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_MAC_GRP_MGR_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_MAC_GRP_MGR_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_MAC_GRP_MGR_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_MAC_GRP_MGR_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_MAC_GRP_MGR_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_MAC_GRP_MGR_DID_LOCK    (BIT_(7))
#define BIT_AP_APB_MAC_GRP_MGR_DID_3    (BIT_(6))
#define BIT_AP_APB_MAC_GRP_MGR_DID_2    (BIT_(5))
#define BIT_AP_APB_MAC_GRP_MGR_DID_1    (BIT_(4))
#define BIT_AP_APB_MAC_GRP_MGR_DID_0    (BIT_(3))
#define BIT_AP_APB_MAC_GRP_MGR_DID_EN    (BIT_(2))
#define BIT_AP_APB_MAC_GRP_MGR_GRP_MGR_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_MAC_GRP_MGR_GRP_MGR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GRP_MGR_MA0
// Register Offset : 0x104
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GRP_MGR_MA0 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x104)
#define GRP_MGR_MA0_MID_FIELD_OFFSET 0
#define GRP_MGR_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_GRP_MGR_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GRP_MGR_MA1
// Register Offset : 0x108
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GRP_MGR_MA1 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x108)
#define GRP_MGR_MA1_MID_FIELD_OFFSET 0
#define GRP_MGR_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_GRP_MGR_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GRP_MGR_MA2
// Register Offset : 0x10c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GRP_MGR_MA2 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x10c)
#define GRP_MGR_MA2_MID_FIELD_OFFSET 0
#define GRP_MGR_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_GRP_MGR_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_GRP_MGR_MA3
// Register Offset : 0x110
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_GRP_MGR_MA3 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x110)
#define GRP_MGR_MA3_MID_FIELD_OFFSET 0
#define GRP_MGR_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_GRP_MGR_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_GID
// Register Offset : 0x200
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_GID (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x200)
#define DOM_GID_LOCK_FIELD_OFFSET 3
#define DOM_GID_LOCK_FIELD_SIZE 1
#define DOM_GID_GID_FIELD_OFFSET 0
#define DOM_GID_GID_FIELD_SIZE 3

#define BIT_AP_APB_MAC_DOM_GID_LOCK    (BIT_(3))
#define BIT_AP_APB_MAC_DOM_GID_GID_2    (BIT_(2))
#define BIT_AP_APB_MAC_DOM_GID_GID_1    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_GID_GID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_OWN
// Register Offset : 0x204
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_OWN (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x204)
#define DOM_OWN_MID_LOCK_FIELD_OFFSET 17
#define DOM_OWN_MID_LOCK_FIELD_SIZE 1
#define DOM_OWN_MID_EN_FIELD_OFFSET 16
#define DOM_OWN_MID_EN_FIELD_SIZE 1
#define DOM_OWN_PRI_PER_LOCK_FIELD_OFFSET 15
#define DOM_OWN_PRI_PER_LOCK_FIELD_SIZE 1
#define DOM_OWN_PRI_PER_FIELD_OFFSET 13
#define DOM_OWN_PRI_PER_FIELD_SIZE 2
#define DOM_OWN_PRI_PER_EN_FIELD_OFFSET 12
#define DOM_OWN_PRI_PER_EN_FIELD_SIZE 1
#define DOM_OWN_SEC_PER_LOCK_FIELD_OFFSET 11
#define DOM_OWN_SEC_PER_LOCK_FIELD_SIZE 1
#define DOM_OWN_SEC_PER_FIELD_OFFSET 9
#define DOM_OWN_SEC_PER_FIELD_SIZE 2
#define DOM_OWN_SEC_PER_EN_FIELD_OFFSET 8
#define DOM_OWN_SEC_PER_EN_FIELD_SIZE 1
#define DOM_OWN_DOM_OWN_EN_LOCK_FIELD_OFFSET 1
#define DOM_OWN_DOM_OWN_EN_LOCK_FIELD_SIZE 1
#define DOM_OWN_DOM_OWN_EN_FIELD_OFFSET 0
#define DOM_OWN_DOM_OWN_EN_FIELD_SIZE 1

#define BIT_AP_APB_MAC_DOM_OWN_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_MAC_DOM_OWN_MID_EN    (BIT_(16))
#define BIT_AP_APB_MAC_DOM_OWN_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_MAC_DOM_OWN_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_MAC_DOM_OWN_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_MAC_DOM_OWN_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_MAC_DOM_OWN_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_MAC_DOM_OWN_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_MAC_DOM_OWN_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_MAC_DOM_OWN_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_MAC_DOM_OWN_DOM_OWN_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_OWN_DOM_OWN_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_OWN_MA0
// Register Offset : 0x208
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_OWN_MA0 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x208)
#define DOM_OWN_MA0_MID_FIELD_OFFSET 0
#define DOM_OWN_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_OWN_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_OWN_MA1
// Register Offset : 0x20c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_OWN_MA1 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x20c)
#define DOM_OWN_MA1_MID_FIELD_OFFSET 0
#define DOM_OWN_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_OWN_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_OWN_MA2
// Register Offset : 0x210
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_OWN_MA2 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x210)
#define DOM_OWN_MA2_MID_FIELD_OFFSET 0
#define DOM_OWN_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_OWN_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_DOM_OWN_MA3
// Register Offset : 0x214
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_DOM_OWN_MA3 (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x214)
#define DOM_OWN_MA3_MID_FIELD_OFFSET 0
#define DOM_OWN_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_MAC_DOM_OWN_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_MDA
// Register Offset : 0x400
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_MDA (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x400)
#define MDA_LOCK_FIELD_OFFSET 31
#define MDA_LOCK_FIELD_SIZE 1
#define MDA_DID_FIELD_OFFSET 0
#define MDA_DID_FIELD_SIZE 4

#define BIT_AP_APB_MAC_MDA_LOCK    (BIT_(31))
#define BIT_AP_APB_MAC_MDA_DID_3    (BIT_(3))
#define BIT_AP_APB_MAC_MDA_DID_2    (BIT_(2))
#define BIT_AP_APB_MAC_MDA_DID_1    (BIT_(1))
#define BIT_AP_APB_MAC_MDA_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_MAC_MAA
// Register Offset : 0x404
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_MAC_MAA (MAC_APB_MAC_APB_AB0_BASE_ADDR + 0x404)
#define MAA_SRID_LOCK_FIELD_OFFSET 14
#define MAA_SRID_LOCK_FIELD_SIZE 1
#define MAA_SRID_FIELD_OFFSET 6
#define MAA_SRID_FIELD_SIZE 8
#define MAA_PRI_LOCK_FIELD_OFFSET 5
#define MAA_PRI_LOCK_FIELD_SIZE 1
#define MAA_PRI_OV_EN_FIELD_OFFSET 4
#define MAA_PRI_OV_EN_FIELD_SIZE 1
#define MAA_PRI_FIELD_OFFSET 3
#define MAA_PRI_FIELD_SIZE 1
#define MAA_SEC_LOCK_FIELD_OFFSET 2
#define MAA_SEC_LOCK_FIELD_SIZE 1
#define MAA_SEC_OV_EN_FIELD_OFFSET 1
#define MAA_SEC_OV_EN_FIELD_SIZE 1
#define MAA_SEC_FIELD_OFFSET 0
#define MAA_SEC_FIELD_SIZE 1

#define BIT_AP_APB_MAC_MAA_SRID_LOCK    (BIT_(14))
#define BIT_AP_APB_MAC_MAA_SRID_7    (BIT_(13))
#define BIT_AP_APB_MAC_MAA_SRID_6    (BIT_(12))
#define BIT_AP_APB_MAC_MAA_SRID_5    (BIT_(11))
#define BIT_AP_APB_MAC_MAA_SRID_4    (BIT_(10))
#define BIT_AP_APB_MAC_MAA_SRID_3    (BIT_(9))
#define BIT_AP_APB_MAC_MAA_SRID_2    (BIT_(8))
#define BIT_AP_APB_MAC_MAA_SRID_1    (BIT_(7))
#define BIT_AP_APB_MAC_MAA_SRID_0    (BIT_(6))
#define BIT_AP_APB_MAC_MAA_PRI_LOCK    (BIT_(5))
#define BIT_AP_APB_MAC_MAA_PRI_OV_EN    (BIT_(4))
#define BIT_AP_APB_MAC_MAA_PRI    (BIT_(3))
#define BIT_AP_APB_MAC_MAA_SEC_LOCK    (BIT_(2))
#define BIT_AP_APB_MAC_MAA_SEC_OV_EN    (BIT_(1))
#define BIT_AP_APB_MAC_MAA_SEC    (BIT_(0))

#endif
