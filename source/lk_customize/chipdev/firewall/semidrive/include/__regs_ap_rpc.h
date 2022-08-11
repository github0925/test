/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _AP_APB_RPC_REG_H
#define _AP_APB_RPC_REG_H
//--------------------------------------------------------------------------
// IP Ref Info     : REG_AP_APB_RPC
// RTL version     :
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Address Block Name : RPC_APB_AB0
// Description        :
//--------------------------------------------------------------------------
#define RPC_APB_AB0_BASE_ADDR 0x0
//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG0_DOM
// Register Offset : 0x0
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG0_DOM (RPC_APB_AB0_BASE_ADDR + (0x0<<0))
#define REGG0_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG0_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG0_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG0_DOM_PER_SEL_FIELD_SIZE 2
#define REGG0_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG0_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG0_DOM_DID_FIELD_OFFSET 0
#define REGG0_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG0_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG0_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG0_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG0_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG0_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG0_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG0_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG0_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG1_DOM
// Register Offset : 0x400 * 0x400(1024) --> 0x100000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG1_DOM (RPC_APB_AB0_BASE_ADDR + (0x100000<<0))
#define REGG1_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG1_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG1_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG1_DOM_PER_SEL_FIELD_SIZE 2
#define REGG1_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG1_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG1_DOM_DID_FIELD_OFFSET 0
#define REGG1_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG1_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG1_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG1_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG1_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG1_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG1_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG1_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG1_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG2_DOM
// Register Offset : 0x800 * 0x400(1024) --> 0x200000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG2_DOM (RPC_APB_AB0_BASE_ADDR + (0x200000<<0))
#define REGG2_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG2_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG2_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG2_DOM_PER_SEL_FIELD_SIZE 2
#define REGG2_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG2_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG2_DOM_DID_FIELD_OFFSET 0
#define REGG2_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG2_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG2_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG2_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG2_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG2_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG2_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG2_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG2_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG3_DOM
// Register Offset : 0xc00 * 0x400(1024) --> 0x300000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG3_DOM (RPC_APB_AB0_BASE_ADDR + (0x300000<<0))
#define REGG3_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG3_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG3_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG3_DOM_PER_SEL_FIELD_SIZE 2
#define REGG3_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG3_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG3_DOM_DID_FIELD_OFFSET 0
#define REGG3_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG3_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG3_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG3_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG3_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG3_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG3_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG3_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG3_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG4_DOM
// Register Offset : 0x1000 * 0x400(1024) --> 0x400000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG4_DOM (RPC_APB_AB0_BASE_ADDR + (0x400000<<0))
#define REGG4_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG4_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG4_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG4_DOM_PER_SEL_FIELD_SIZE 2
#define REGG4_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG4_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG4_DOM_DID_FIELD_OFFSET 0
#define REGG4_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG4_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG4_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG4_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG4_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG4_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG4_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG4_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG4_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG5_DOM
// Register Offset : 0x1400 * 0x400(1024) --> 0x500000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG5_DOM (RPC_APB_AB0_BASE_ADDR + (0x500000<<0))
#define REGG5_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG5_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG5_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG5_DOM_PER_SEL_FIELD_SIZE 2
#define REGG5_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG5_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG5_DOM_DID_FIELD_OFFSET 0
#define REGG5_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG5_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG5_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG5_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG5_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG5_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG5_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG5_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG5_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG6_DOM
// Register Offset : 0x1800 * 0x400(1024) --> 0x600000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG6_DOM (RPC_APB_AB0_BASE_ADDR + (0x600000<<0))
#define REGG6_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG6_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG6_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG6_DOM_PER_SEL_FIELD_SIZE 2
#define REGG6_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG6_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG6_DOM_DID_FIELD_OFFSET 0
#define REGG6_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG6_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG6_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG6_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG6_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG6_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG6_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG6_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG6_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_REGG7_DOM
// Register Offset : 0x1c00 * 0x400(1024) --> 0x700000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_REGG7_DOM (RPC_APB_AB0_BASE_ADDR + (0x700000<<0))
#define REGG7_DOM_PER_SEL_LOCK_FIELD_OFFSET 7
#define REGG7_DOM_PER_SEL_LOCK_FIELD_SIZE 1
#define REGG7_DOM_PER_SEL_FIELD_OFFSET 5
#define REGG7_DOM_PER_SEL_FIELD_SIZE 2
#define REGG7_DOM_DID_LOCK_FIELD_OFFSET 4
#define REGG7_DOM_DID_LOCK_FIELD_SIZE 1
#define REGG7_DOM_DID_FIELD_OFFSET 0
#define REGG7_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_REGG7_DOM_PER_SEL_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_REGG7_DOM_PER_SEL_1    (BIT_(6))
#define BIT_AP_APB_RPC_REGG7_DOM_PER_SEL_0    (BIT_(5))
#define BIT_AP_APB_RPC_REGG7_DOM_DID_LOCK    (BIT_(4))
#define BIT_AP_APB_RPC_REGG7_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_REGG7_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_REGG7_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_REGG7_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GLB_CTL
// Register Offset : 0x2000 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) --> 0x7fe000
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GLB_CTL (RPC_APB_AB0_BASE_ADDR + (0x7fe000<<0))
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

#define BIT_AP_APB_RPC_GLB_CTL_DOM_CFG_LOCK    (BIT_(5))
#define BIT_AP_APB_RPC_GLB_CTL_DOM_CFG_MODE    (BIT_(4))
#define BIT_AP_APB_RPC_GLB_CTL_PERCK_DIS_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_GLB_CTL_PERCK_DIS    (BIT_(2))
#define BIT_AP_APB_RPC_GLB_CTL_DOM_PRO_LOCK    (BIT_(1))
#define BIT_AP_APB_RPC_GLB_CTL_DOM_PRO_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RES_MGR
// Register Offset : 0x2004 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x4 --> 0x7fe004
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RES_MGR (RPC_APB_AB0_BASE_ADDR + (0x7fe004<<0))
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

#define BIT_AP_APB_RPC_RES_MGR_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_RPC_RES_MGR_MID_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RES_MGR_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RES_MGR_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_RES_MGR_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_RES_MGR_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_RPC_RES_MGR_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_RES_MGR_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RES_MGR_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RES_MGR_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RES_MGR_DID_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RES_MGR_DID_3    (BIT_(6))
#define BIT_AP_APB_RPC_RES_MGR_DID_2    (BIT_(5))
#define BIT_AP_APB_RPC_RES_MGR_DID_1    (BIT_(4))
#define BIT_AP_APB_RPC_RES_MGR_DID_0    (BIT_(3))
#define BIT_AP_APB_RPC_RES_MGR_DID_EN    (BIT_(2))
#define BIT_AP_APB_RPC_RES_MGR_RES_MGR_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_RPC_RES_MGR_RES_MGR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RES_MGR_MA0
// Register Offset : 0x2008 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x8 --> 0x7fe008
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RES_MGR_MA0 (RPC_APB_AB0_BASE_ADDR + (0x7fe008<<0))
#define RES_MGR_MA0_MID_FIELD_OFFSET 0
#define RES_MGR_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RES_MGR_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RES_MGR_MA1
// Register Offset : 0x200c -->0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0xc --> 0x7fe00c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RES_MGR_MA1 (RPC_APB_AB0_BASE_ADDR + (0x7fe00c<<0))
#define RES_MGR_MA1_MID_FIELD_OFFSET 0
#define RES_MGR_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RES_MGR_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RES_MGR_MA2
// Register Offset : 0x2010 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x10 --> 0x7fe010
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RES_MGR_MA2 (RPC_APB_AB0_BASE_ADDR + (0x7fe010<<0))
#define RES_MGR_MA2_MID_FIELD_OFFSET 0
#define RES_MGR_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RES_MGR_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RES_MGR_MA3
// Register Offset : 0x2014 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x14 --> 0x7fe014
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RES_MGR_MA3 (RPC_APB_AB0_BASE_ADDR + (0x7fe014<<0))
#define RES_MGR_MA3_MID_FIELD_OFFSET 0
#define RES_MGR_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RES_MGR_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GRP_MGR
// Register Offset : 0x2100 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x100 --> 0x7fe100
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GRP_MGR (RPC_APB_AB0_BASE_ADDR + (0x7fe100<<0))
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

#define BIT_AP_APB_RPC_GRP_MGR_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_RPC_GRP_MGR_MID_EN    (BIT_(16))
#define BIT_AP_APB_RPC_GRP_MGR_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_GRP_MGR_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_GRP_MGR_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_GRP_MGR_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_RPC_GRP_MGR_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_GRP_MGR_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_GRP_MGR_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_GRP_MGR_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_RPC_GRP_MGR_DID_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_GRP_MGR_DID_3    (BIT_(6))
#define BIT_AP_APB_RPC_GRP_MGR_DID_2    (BIT_(5))
#define BIT_AP_APB_RPC_GRP_MGR_DID_1    (BIT_(4))
#define BIT_AP_APB_RPC_GRP_MGR_DID_0    (BIT_(3))
#define BIT_AP_APB_RPC_GRP_MGR_DID_EN    (BIT_(2))
#define BIT_AP_APB_RPC_GRP_MGR_GRP_MGR_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_RPC_GRP_MGR_GRP_MGR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GRP_MGR_MA0
// Register Offset : 0x2104 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x104 --> 0x7fe104
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GRP_MGR_MA0 (RPC_APB_AB0_BASE_ADDR + (0x7fe104<<0))
#define GRP_MGR_MA0_MID_FIELD_OFFSET 0
#define GRP_MGR_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_GRP_MGR_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GRP_MGR_MA1
// Register Offset : 0x2108 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x108 --> 0x7fe108
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GRP_MGR_MA1 (RPC_APB_AB0_BASE_ADDR + (0x7fe108<<0))
#define GRP_MGR_MA1_MID_FIELD_OFFSET 0
#define GRP_MGR_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_GRP_MGR_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GRP_MGR_MA2
// Register Offset : 0x210c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x10c --> 0x7fe10c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GRP_MGR_MA2 (RPC_APB_AB0_BASE_ADDR + (0x7fe10c<<0))
#define GRP_MGR_MA2_MID_FIELD_OFFSET 0
#define GRP_MGR_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_GRP_MGR_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_GRP_MGR_MA3
// Register Offset : 0x2110 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x110 --> 0x7fe110
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_GRP_MGR_MA3 (RPC_APB_AB0_BASE_ADDR + (0x7fe110<<0))
#define GRP_MGR_MA3_MID_FIELD_OFFSET 0
#define GRP_MGR_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_GRP_MGR_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_GID
// Register Offset : 0x2200 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x200 --> 0x7fe200
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_GID (RPC_APB_AB0_BASE_ADDR + (0x7fe200<<0))
#define DOM_GID_LOCK_FIELD_OFFSET 3
#define DOM_GID_LOCK_FIELD_SIZE 1
#define DOM_GID_GID_FIELD_OFFSET 0
#define DOM_GID_GID_FIELD_SIZE 3

#define BIT_AP_APB_RPC_DOM_GID_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_GID_GID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_GID_GID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_GID_GID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_OWN
// Register Offset : 0x2204 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x204 --> 0x7fe204
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_OWN (RPC_APB_AB0_BASE_ADDR + (0x7fe204<<0))
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

#define BIT_AP_APB_RPC_DOM_OWN_MID_LOCK    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_OWN_MID_EN    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_OWN_PRI_PER_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_OWN_PRI_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_OWN_PRI_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_OWN_PRI_PER_EN    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_OWN_SEC_PER_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_OWN_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_OWN_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_OWN_SEC_PER_EN    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_OWN_DOM_OWN_EN_LOCK    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_OWN_DOM_OWN_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_OWN_MA0
// Register Offset : 0x2208 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x208 --> 0x7fe208
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_OWN_MA0 (RPC_APB_AB0_BASE_ADDR + (0x7fe208<<0))
#define DOM_OWN_MA0_MID_FIELD_OFFSET 0
#define DOM_OWN_MA0_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_OWN_MA0_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_OWN_MA1
// Register Offset : 0x220c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x20c --> 0x7fe20c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_OWN_MA1 (RPC_APB_AB0_BASE_ADDR + (0x7fe20c<<0))
#define DOM_OWN_MA1_MID_FIELD_OFFSET 0
#define DOM_OWN_MA1_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_OWN_MA1_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_OWN_MA2
// Register Offset : 0x2210 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x210 --> 0x7fe210
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_OWN_MA2 (RPC_APB_AB0_BASE_ADDR + (0x7fe210<<0))
#define DOM_OWN_MA2_MID_FIELD_OFFSET 0
#define DOM_OWN_MA2_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_OWN_MA2_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_OWN_MA3
// Register Offset : 0x2214 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x214 --> 0x7fe214
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_OWN_MA3 (RPC_APB_AB0_BASE_ADDR + (0x7fe214<<0))
#define DOM_OWN_MA3_MID_FIELD_OFFSET 0
#define DOM_OWN_MA3_MID_FIELD_SIZE 32

#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_31    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_30    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_29    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_28    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_27    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_26    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_25    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_24    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_23    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_22    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_21    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_20    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_19    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_18    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_17    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_16    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_15    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_14    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_13    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_12    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_11    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_10    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_9    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_8    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_7    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_6    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_5    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_4    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_3    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_OWN_MA3_MID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_PER0
// Register Offset : 0x2400 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x400 --> 0x7fe400
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7fe400<<0))
#define DOM_PER0_DOM7_LOCK_FIELD_OFFSET 31
#define DOM_PER0_DOM7_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM7_PER_FIELD_OFFSET 29
#define DOM_PER0_DOM7_PER_FIELD_SIZE 2
#define DOM_PER0_DOM7_EN_FIELD_OFFSET 28
#define DOM_PER0_DOM7_EN_FIELD_SIZE 1
#define DOM_PER0_DOM6_LOCK_FIELD_OFFSET 27
#define DOM_PER0_DOM6_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM6_PER_FIELD_OFFSET 25
#define DOM_PER0_DOM6_PER_FIELD_SIZE 2
#define DOM_PER0_DOM6_EN_FIELD_OFFSET 24
#define DOM_PER0_DOM6_EN_FIELD_SIZE 1
#define DOM_PER0_DOM5_LOCK_FIELD_OFFSET 23
#define DOM_PER0_DOM5_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM5_PER_FIELD_OFFSET 21
#define DOM_PER0_DOM5_PER_FIELD_SIZE 2
#define DOM_PER0_DOM5_EN_FIELD_OFFSET 20
#define DOM_PER0_DOM5_EN_FIELD_SIZE 1
#define DOM_PER0_DOM4_LOCK_FIELD_OFFSET 19
#define DOM_PER0_DOM4_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM4_PER_FIELD_OFFSET 17
#define DOM_PER0_DOM4_PER_FIELD_SIZE 2
#define DOM_PER0_DOM4_EN_FIELD_OFFSET 16
#define DOM_PER0_DOM4_EN_FIELD_SIZE 1
#define DOM_PER0_DOM3_LOCK_FIELD_OFFSET 15
#define DOM_PER0_DOM3_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM3_PER_FIELD_OFFSET 13
#define DOM_PER0_DOM3_PER_FIELD_SIZE 2
#define DOM_PER0_DOM3_EN_FIELD_OFFSET 12
#define DOM_PER0_DOM3_EN_FIELD_SIZE 1
#define DOM_PER0_DOM2_LOCK_FIELD_OFFSET 11
#define DOM_PER0_DOM2_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM2_PER_FIELD_OFFSET 9
#define DOM_PER0_DOM2_PER_FIELD_SIZE 2
#define DOM_PER0_DOM2_EN_FIELD_OFFSET 8
#define DOM_PER0_DOM2_EN_FIELD_SIZE 1
#define DOM_PER0_DOM1_LOCK_FIELD_OFFSET 7
#define DOM_PER0_DOM1_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM1_PER_FIELD_OFFSET 5
#define DOM_PER0_DOM1_PER_FIELD_SIZE 2
#define DOM_PER0_DOM1_EN_FIELD_OFFSET 4
#define DOM_PER0_DOM1_EN_FIELD_SIZE 1
#define DOM_PER0_DOM0_LOCK_FIELD_OFFSET 3
#define DOM_PER0_DOM0_LOCK_FIELD_SIZE 1
#define DOM_PER0_DOM0_PER_FIELD_OFFSET 1
#define DOM_PER0_DOM0_PER_FIELD_SIZE 2
#define DOM_PER0_DOM0_EN_FIELD_OFFSET 0
#define DOM_PER0_DOM0_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_DOM_PER0_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_PER0_DOM7_PER_1    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_PER0_DOM7_PER_0    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_PER0_DOM7_EN    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_PER0_DOM6_LOCK    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_PER0_DOM6_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_PER0_DOM6_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_PER0_DOM6_EN    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_PER0_DOM5_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_PER0_DOM5_PER_1    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_PER0_DOM5_PER_0    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_PER0_DOM5_EN    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_PER0_DOM4_LOCK    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_PER0_DOM4_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_PER0_DOM4_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_PER0_DOM4_EN    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_PER0_DOM3_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_PER0_DOM3_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_PER0_DOM3_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_PER0_DOM3_EN    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_PER0_DOM2_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_PER0_DOM2_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_PER0_DOM2_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_PER0_DOM2_EN    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_PER0_DOM1_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_PER0_DOM1_PER_1    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_PER0_DOM1_PER_0    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_PER0_DOM1_EN    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_PER0_DOM0_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_PER0_DOM0_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_PER0_DOM0_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_PER0_DOM0_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_PER1
// Register Offset : 0x2404 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x404 --> 0x7fe404
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7fe404<<0))
#define DOM_PER1_DOM15_LOCK_FIELD_OFFSET 31
#define DOM_PER1_DOM15_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM15_PER_FIELD_OFFSET 29
#define DOM_PER1_DOM15_PER_FIELD_SIZE 2
#define DOM_PER1_DOM15_EN_FIELD_OFFSET 28
#define DOM_PER1_DOM15_EN_FIELD_SIZE 1
#define DOM_PER1_DOM14_LOCK_FIELD_OFFSET 27
#define DOM_PER1_DOM14_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM14_PER_FIELD_OFFSET 25
#define DOM_PER1_DOM14_PER_FIELD_SIZE 2
#define DOM_PER1_DOM14_EN_FIELD_OFFSET 24
#define DOM_PER1_DOM14_EN_FIELD_SIZE 1
#define DOM_PER1_DOM13_LOCK_FIELD_OFFSET 23
#define DOM_PER1_DOM13_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM13_PER_FIELD_OFFSET 21
#define DOM_PER1_DOM13_PER_FIELD_SIZE 2
#define DOM_PER1_DOM13_EN_FIELD_OFFSET 20
#define DOM_PER1_DOM13_EN_FIELD_SIZE 1
#define DOM_PER1_DOM12_LOCK_FIELD_OFFSET 19
#define DOM_PER1_DOM12_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM12_PER_FIELD_OFFSET 17
#define DOM_PER1_DOM12_PER_FIELD_SIZE 2
#define DOM_PER1_DOM12_EN_FIELD_OFFSET 16
#define DOM_PER1_DOM12_EN_FIELD_SIZE 1
#define DOM_PER1_DOM11_LOCK_FIELD_OFFSET 15
#define DOM_PER1_DOM11_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM11_PER_FIELD_OFFSET 13
#define DOM_PER1_DOM11_PER_FIELD_SIZE 2
#define DOM_PER1_DOM11_EN_FIELD_OFFSET 12
#define DOM_PER1_DOM11_EN_FIELD_SIZE 1
#define DOM_PER1_DOM10_LOCK_FIELD_OFFSET 11
#define DOM_PER1_DOM10_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM10_PER_FIELD_OFFSET 9
#define DOM_PER1_DOM10_PER_FIELD_SIZE 2
#define DOM_PER1_DOM10_EN_FIELD_OFFSET 8
#define DOM_PER1_DOM10_EN_FIELD_SIZE 1
#define DOM_PER1_DOM9_LOCK_FIELD_OFFSET 7
#define DOM_PER1_DOM9_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM9_PER_FIELD_OFFSET 5
#define DOM_PER1_DOM9_PER_FIELD_SIZE 2
#define DOM_PER1_DOM9_EN_FIELD_OFFSET 4
#define DOM_PER1_DOM9_EN_FIELD_SIZE 1
#define DOM_PER1_DOM8_LOCK_FIELD_OFFSET 3
#define DOM_PER1_DOM8_LOCK_FIELD_SIZE 1
#define DOM_PER1_DOM8_PER_FIELD_OFFSET 1
#define DOM_PER1_DOM8_PER_FIELD_SIZE 2
#define DOM_PER1_DOM8_EN_FIELD_OFFSET 0
#define DOM_PER1_DOM8_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_DOM_PER1_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_DOM_PER1_DOM15_PER_1    (BIT_(30))
#define BIT_AP_APB_RPC_DOM_PER1_DOM15_PER_0    (BIT_(29))
#define BIT_AP_APB_RPC_DOM_PER1_DOM15_EN    (BIT_(28))
#define BIT_AP_APB_RPC_DOM_PER1_DOM14_LOCK    (BIT_(27))
#define BIT_AP_APB_RPC_DOM_PER1_DOM14_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_DOM_PER1_DOM14_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_DOM_PER1_DOM14_EN    (BIT_(24))
#define BIT_AP_APB_RPC_DOM_PER1_DOM13_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_DOM_PER1_DOM13_PER_1    (BIT_(22))
#define BIT_AP_APB_RPC_DOM_PER1_DOM13_PER_0    (BIT_(21))
#define BIT_AP_APB_RPC_DOM_PER1_DOM13_EN    (BIT_(20))
#define BIT_AP_APB_RPC_DOM_PER1_DOM12_LOCK    (BIT_(19))
#define BIT_AP_APB_RPC_DOM_PER1_DOM12_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_DOM_PER1_DOM12_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_DOM_PER1_DOM12_EN    (BIT_(16))
#define BIT_AP_APB_RPC_DOM_PER1_DOM11_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_DOM_PER1_DOM11_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_DOM_PER1_DOM11_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_DOM_PER1_DOM11_EN    (BIT_(12))
#define BIT_AP_APB_RPC_DOM_PER1_DOM10_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_DOM_PER1_DOM10_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_DOM_PER1_DOM10_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_DOM_PER1_DOM10_EN    (BIT_(8))
#define BIT_AP_APB_RPC_DOM_PER1_DOM9_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_DOM_PER1_DOM9_PER_1    (BIT_(6))
#define BIT_AP_APB_RPC_DOM_PER1_DOM9_PER_0    (BIT_(5))
#define BIT_AP_APB_RPC_DOM_PER1_DOM9_EN    (BIT_(4))
#define BIT_AP_APB_RPC_DOM_PER1_DOM8_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_PER1_DOM8_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_PER1_DOM8_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_PER1_DOM8_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER0
// Register Offset : 0x2408 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x408 -->0x7fe408
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_SEC_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7fe408<<0))
#define SEC_PER0_DOM3_LOCK_FIELD_OFFSET 31
#define SEC_PER0_DOM3_LOCK_FIELD_SIZE 1
#define SEC_PER0_DOM3_NSE_PER_FIELD_OFFSET 27
#define SEC_PER0_DOM3_NSE_PER_FIELD_SIZE 2
#define SEC_PER0_DOM3_SEC_PER_FIELD_OFFSET 25
#define SEC_PER0_DOM3_SEC_PER_FIELD_SIZE 2
#define SEC_PER0_DOM3_SEC_EN_FIELD_OFFSET 24
#define SEC_PER0_DOM3_SEC_EN_FIELD_SIZE 1
#define SEC_PER0_DOM2_LOCK_FIELD_OFFSET 23
#define SEC_PER0_DOM2_LOCK_FIELD_SIZE 1
#define SEC_PER0_DOM2_NSE_PER_FIELD_OFFSET 19
#define SEC_PER0_DOM2_NSE_PER_FIELD_SIZE 2
#define SEC_PER0_DOM2_SEC_PER_FIELD_OFFSET 17
#define SEC_PER0_DOM2_SEC_PER_FIELD_SIZE 2
#define SEC_PER0_DOM2_SEC_EN_FIELD_OFFSET 16
#define SEC_PER0_DOM2_SEC_EN_FIELD_SIZE 1
#define SEC_PER0_DOM1_LOCK_FIELD_OFFSET 15
#define SEC_PER0_DOM1_LOCK_FIELD_SIZE 1
#define SEC_PER0_DOM1_NSE_PER_FIELD_OFFSET 11
#define SEC_PER0_DOM1_NSE_PER_FIELD_SIZE 2
#define SEC_PER0_DOM1_SEC_PER_FIELD_OFFSET 9
#define SEC_PER0_DOM1_SEC_PER_FIELD_SIZE 2
#define SEC_PER0_DOM1_SEC_EN_FIELD_OFFSET 8
#define SEC_PER0_DOM1_SEC_EN_FIELD_SIZE 1
#define SEC_PER0_DOM0_LOCK_FIELD_OFFSET 7
#define SEC_PER0_DOM0_LOCK_FIELD_SIZE 1
#define SEC_PER0_DOM0_NSE_PER_FIELD_OFFSET 3
#define SEC_PER0_DOM0_NSE_PER_FIELD_SIZE 2
#define SEC_PER0_DOM0_SEC_PER_FIELD_OFFSET 1
#define SEC_PER0_DOM0_SEC_PER_FIELD_SIZE 2
#define SEC_PER0_DOM0_SEC_EN_FIELD_OFFSET 0
#define SEC_PER0_DOM0_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_SEC_PER0_DOM3_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_SEC_PER0_DOM3_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_SEC_PER0_DOM3_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_SEC_PER0_DOM3_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_SEC_PER0_DOM3_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_SEC_PER0_DOM3_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_SEC_PER0_DOM2_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_SEC_PER0_DOM1_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_SEC_PER0_DOM0_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER1
// Register Offset : 0x240c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x40c --> 0x7fe40c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_SEC_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7fe40c<<0))
#define SEC_PER1_DOM7_LOCK_FIELD_OFFSET 31
#define SEC_PER1_DOM7_LOCK_FIELD_SIZE 1
#define SEC_PER1_DOM7_NSE_PER_FIELD_OFFSET 27
#define SEC_PER1_DOM7_NSE_PER_FIELD_SIZE 2
#define SEC_PER1_DOM7_SEC_PER_FIELD_OFFSET 25
#define SEC_PER1_DOM7_SEC_PER_FIELD_SIZE 2
#define SEC_PER1_DOM7_SEC_EN_FIELD_OFFSET 24
#define SEC_PER1_DOM7_SEC_EN_FIELD_SIZE 1
#define SEC_PER1_DOM6_LOCK_FIELD_OFFSET 23
#define SEC_PER1_DOM6_LOCK_FIELD_SIZE 1
#define SEC_PER1_DOM6_NSE_PER_FIELD_OFFSET 19
#define SEC_PER1_DOM6_NSE_PER_FIELD_SIZE 2
#define SEC_PER1_DOM6_SEC_PER_FIELD_OFFSET 17
#define SEC_PER1_DOM6_SEC_PER_FIELD_SIZE 2
#define SEC_PER1_DOM6_SEC_EN_FIELD_OFFSET 16
#define SEC_PER1_DOM6_SEC_EN_FIELD_SIZE 1
#define SEC_PER1_DOM5_LOCK_FIELD_OFFSET 15
#define SEC_PER1_DOM5_LOCK_FIELD_SIZE 1
#define SEC_PER1_DOM5_NSE_PER_FIELD_OFFSET 11
#define SEC_PER1_DOM5_NSE_PER_FIELD_SIZE 2
#define SEC_PER1_DOM5_SEC_PER_FIELD_OFFSET 9
#define SEC_PER1_DOM5_SEC_PER_FIELD_SIZE 2
#define SEC_PER1_DOM5_SEC_EN_FIELD_OFFSET 8
#define SEC_PER1_DOM5_SEC_EN_FIELD_SIZE 1
#define SEC_PER1_DOM4_LOCK_FIELD_OFFSET 7
#define SEC_PER1_DOM4_LOCK_FIELD_SIZE 1
#define SEC_PER1_DOM4_NSE_PER_FIELD_OFFSET 3
#define SEC_PER1_DOM4_NSE_PER_FIELD_SIZE 2
#define SEC_PER1_DOM4_SEC_PER_FIELD_OFFSET 1
#define SEC_PER1_DOM4_SEC_PER_FIELD_SIZE 2
#define SEC_PER1_DOM4_SEC_EN_FIELD_OFFSET 0
#define SEC_PER1_DOM4_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_SEC_PER1_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_SEC_PER1_DOM7_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_SEC_PER1_DOM7_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_SEC_PER1_DOM7_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_SEC_PER1_DOM7_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_SEC_PER1_DOM7_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_SEC_PER1_DOM6_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_SEC_PER1_DOM5_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_SEC_PER1_DOM4_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER2
// Register Offset : 0x2410 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x410 --> 0x7fe410
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_SEC_PER2 (RPC_APB_AB0_BASE_ADDR + (0x7fe410<<0))
#define SEC_PER2_DOM11_LOCK_FIELD_OFFSET 31
#define SEC_PER2_DOM11_LOCK_FIELD_SIZE 1
#define SEC_PER2_DOM11_NSE_PER_FIELD_OFFSET 27
#define SEC_PER2_DOM11_NSE_PER_FIELD_SIZE 2
#define SEC_PER2_DOM11_SEC_PER_FIELD_OFFSET 25
#define SEC_PER2_DOM11_SEC_PER_FIELD_SIZE 2
#define SEC_PER2_DOM11_SEC_EN_FIELD_OFFSET 24
#define SEC_PER2_DOM11_SEC_EN_FIELD_SIZE 1
#define SEC_PER2_DOM10_LOCK_FIELD_OFFSET 23
#define SEC_PER2_DOM10_LOCK_FIELD_SIZE 1
#define SEC_PER2_DOM10_NSE_PER_FIELD_OFFSET 19
#define SEC_PER2_DOM10_NSE_PER_FIELD_SIZE 2
#define SEC_PER2_DOM10_SEC_PER_FIELD_OFFSET 17
#define SEC_PER2_DOM10_SEC_PER_FIELD_SIZE 2
#define SEC_PER2_DOM10_SEC_EN_FIELD_OFFSET 16
#define SEC_PER2_DOM10_SEC_EN_FIELD_SIZE 1
#define SEC_PER2_DOM9_LOCK_FIELD_OFFSET 15
#define SEC_PER2_DOM9_LOCK_FIELD_SIZE 1
#define SEC_PER2_DOM9_NSE_PER_FIELD_OFFSET 11
#define SEC_PER2_DOM9_NSE_PER_FIELD_SIZE 2
#define SEC_PER2_DOM9_SEC_PER_FIELD_OFFSET 9
#define SEC_PER2_DOM9_SEC_PER_FIELD_SIZE 2
#define SEC_PER2_DOM9_SEC_EN_FIELD_OFFSET 8
#define SEC_PER2_DOM9_SEC_EN_FIELD_SIZE 1
#define SEC_PER2_DOM8_LOCK_FIELD_OFFSET 7
#define SEC_PER2_DOM8_LOCK_FIELD_SIZE 1
#define SEC_PER2_DOM8_NSE_PER_FIELD_OFFSET 3
#define SEC_PER2_DOM8_NSE_PER_FIELD_SIZE 2
#define SEC_PER2_DOM8_SEC_PER_FIELD_OFFSET 1
#define SEC_PER2_DOM8_SEC_PER_FIELD_SIZE 2
#define SEC_PER2_DOM8_SEC_EN_FIELD_OFFSET 0
#define SEC_PER2_DOM8_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_SEC_PER2_DOM11_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_SEC_PER2_DOM11_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_SEC_PER2_DOM11_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_SEC_PER2_DOM11_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_SEC_PER2_DOM11_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_SEC_PER2_DOM11_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_SEC_PER2_DOM10_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_SEC_PER2_DOM9_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_SEC_PER2_DOM8_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_SEC_PER3
// Register Offset : 0x2414 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x414 --> 0x7fe414
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_SEC_PER3 (RPC_APB_AB0_BASE_ADDR + (0x7fe414<<0))
#define SEC_PER3_DOM15_LOCK_FIELD_OFFSET 31
#define SEC_PER3_DOM15_LOCK_FIELD_SIZE 1
#define SEC_PER3_DOM15_NSE_PER_FIELD_OFFSET 27
#define SEC_PER3_DOM15_NSE_PER_FIELD_SIZE 2
#define SEC_PER3_DOM15_SEC_PER_FIELD_OFFSET 25
#define SEC_PER3_DOM15_SEC_PER_FIELD_SIZE 2
#define SEC_PER3_DOM15_SEC_EN_FIELD_OFFSET 24
#define SEC_PER3_DOM15_SEC_EN_FIELD_SIZE 1
#define SEC_PER3_DOM14_LOCK_FIELD_OFFSET 23
#define SEC_PER3_DOM14_LOCK_FIELD_SIZE 1
#define SEC_PER3_DOM14_NSE_PER_FIELD_OFFSET 19
#define SEC_PER3_DOM14_NSE_PER_FIELD_SIZE 2
#define SEC_PER3_DOM14_SEC_PER_FIELD_OFFSET 17
#define SEC_PER3_DOM14_SEC_PER_FIELD_SIZE 2
#define SEC_PER3_DOM14_SEC_EN_FIELD_OFFSET 16
#define SEC_PER3_DOM14_SEC_EN_FIELD_SIZE 1
#define SEC_PER3_DOM13_LOCK_FIELD_OFFSET 15
#define SEC_PER3_DOM13_LOCK_FIELD_SIZE 1
#define SEC_PER3_DOM13_NSE_PER_FIELD_OFFSET 11
#define SEC_PER3_DOM13_NSE_PER_FIELD_SIZE 2
#define SEC_PER3_DOM13_SEC_PER_FIELD_OFFSET 9
#define SEC_PER3_DOM13_SEC_PER_FIELD_SIZE 2
#define SEC_PER3_DOM13_SEC_EN_FIELD_OFFSET 8
#define SEC_PER3_DOM13_SEC_EN_FIELD_SIZE 1
#define SEC_PER3_DOM12_LOCK_FIELD_OFFSET 7
#define SEC_PER3_DOM12_LOCK_FIELD_SIZE 1
#define SEC_PER3_DOM12_NSE_PER_FIELD_OFFSET 3
#define SEC_PER3_DOM12_NSE_PER_FIELD_SIZE 2
#define SEC_PER3_DOM12_SEC_PER_FIELD_OFFSET 1
#define SEC_PER3_DOM12_SEC_PER_FIELD_SIZE 2
#define SEC_PER3_DOM12_SEC_EN_FIELD_OFFSET 0
#define SEC_PER3_DOM12_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_SEC_PER3_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_SEC_PER3_DOM15_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_SEC_PER3_DOM15_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_SEC_PER3_DOM15_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_SEC_PER3_DOM15_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_SEC_PER3_DOM15_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_SEC_PER3_DOM14_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_SEC_PER3_DOM13_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_SEC_PER3_DOM12_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER0
// Register Offset : 0x2418 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x418 --> 0x7fe418
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_PRI_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7fe418<<0))
#define PRI_PER0_DOM3_LOCK_FIELD_OFFSET 31
#define PRI_PER0_DOM3_LOCK_FIELD_SIZE 1
#define PRI_PER0_DOM3_USE_PER_FIELD_OFFSET 27
#define PRI_PER0_DOM3_USE_PER_FIELD_SIZE 2
#define PRI_PER0_DOM3_PRI_PER_FIELD_OFFSET 25
#define PRI_PER0_DOM3_PRI_PER_FIELD_SIZE 2
#define PRI_PER0_DOM3_PRI_EN_FIELD_OFFSET 24
#define PRI_PER0_DOM3_PRI_EN_FIELD_SIZE 1
#define PRI_PER0_DOM2_LOCK_FIELD_OFFSET 23
#define PRI_PER0_DOM2_LOCK_FIELD_SIZE 1
#define PRI_PER0_DOM2_USE_PER_FIELD_OFFSET 19
#define PRI_PER0_DOM2_USE_PER_FIELD_SIZE 2
#define PRI_PER0_DOM2_PRI_PER_FIELD_OFFSET 17
#define PRI_PER0_DOM2_PRI_PER_FIELD_SIZE 2
#define PRI_PER0_DOM2_PRI_EN_FIELD_OFFSET 16
#define PRI_PER0_DOM2_PRI_EN_FIELD_SIZE 1
#define PRI_PER0_DOM1_LOCK_FIELD_OFFSET 15
#define PRI_PER0_DOM1_LOCK_FIELD_SIZE 1
#define PRI_PER0_DOM1_USE_PER_FIELD_OFFSET 11
#define PRI_PER0_DOM1_USE_PER_FIELD_SIZE 2
#define PRI_PER0_DOM1_PRI_PER_FIELD_OFFSET 9
#define PRI_PER0_DOM1_PRI_PER_FIELD_SIZE 2
#define PRI_PER0_DOM1_PRI_EN_FIELD_OFFSET 8
#define PRI_PER0_DOM1_PRI_EN_FIELD_SIZE 1
#define PRI_PER0_DOM0_LOCK_FIELD_OFFSET 7
#define PRI_PER0_DOM0_LOCK_FIELD_SIZE 1
#define PRI_PER0_DOM0_USE_PER_FIELD_OFFSET 3
#define PRI_PER0_DOM0_USE_PER_FIELD_SIZE 2
#define PRI_PER0_DOM0_PRI_PER_FIELD_OFFSET 1
#define PRI_PER0_DOM0_PRI_PER_FIELD_SIZE 2
#define PRI_PER0_DOM0_PRI_EN_FIELD_OFFSET 0
#define PRI_PER0_DOM0_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_PRI_PER0_DOM3_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_PRI_PER0_DOM3_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_PRI_PER0_DOM3_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_PRI_PER0_DOM3_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_PRI_PER0_DOM3_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_PRI_PER0_DOM3_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_PRI_PER0_DOM2_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_PRI_PER0_DOM1_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_PRI_PER0_DOM0_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER1
// Register Offset : 0x241c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x41c --> 0x7fe41c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_PRI_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7fe41c<<0))
#define PRI_PER1_DOM7_LOCK_FIELD_OFFSET 31
#define PRI_PER1_DOM7_LOCK_FIELD_SIZE 1
#define PRI_PER1_DOM7_USE_PER_FIELD_OFFSET 27
#define PRI_PER1_DOM7_USE_PER_FIELD_SIZE 2
#define PRI_PER1_DOM7_PRI_PER_FIELD_OFFSET 25
#define PRI_PER1_DOM7_PRI_PER_FIELD_SIZE 2
#define PRI_PER1_DOM7_PRI_EN_FIELD_OFFSET 24
#define PRI_PER1_DOM7_PRI_EN_FIELD_SIZE 1
#define PRI_PER1_DOM6_LOCK_FIELD_OFFSET 23
#define PRI_PER1_DOM6_LOCK_FIELD_SIZE 1
#define PRI_PER1_DOM6_USE_PER_FIELD_OFFSET 19
#define PRI_PER1_DOM6_USE_PER_FIELD_SIZE 2
#define PRI_PER1_DOM6_PRI_PER_FIELD_OFFSET 17
#define PRI_PER1_DOM6_PRI_PER_FIELD_SIZE 2
#define PRI_PER1_DOM6_PRI_EN_FIELD_OFFSET 16
#define PRI_PER1_DOM6_PRI_EN_FIELD_SIZE 1
#define PRI_PER1_DOM5_LOCK_FIELD_OFFSET 15
#define PRI_PER1_DOM5_LOCK_FIELD_SIZE 1
#define PRI_PER1_DOM5_USE_PER_FIELD_OFFSET 11
#define PRI_PER1_DOM5_USE_PER_FIELD_SIZE 2
#define PRI_PER1_DOM5_PRI_PER_FIELD_OFFSET 9
#define PRI_PER1_DOM5_PRI_PER_FIELD_SIZE 2
#define PRI_PER1_DOM5_PRI_EN_FIELD_OFFSET 8
#define PRI_PER1_DOM5_PRI_EN_FIELD_SIZE 1
#define PRI_PER1_DOM4_LOCK_FIELD_OFFSET 7
#define PRI_PER1_DOM4_LOCK_FIELD_SIZE 1
#define PRI_PER1_DOM4_USE_PER_FIELD_OFFSET 3
#define PRI_PER1_DOM4_USE_PER_FIELD_SIZE 2
#define PRI_PER1_DOM4_PRI_PER_FIELD_OFFSET 1
#define PRI_PER1_DOM4_PRI_PER_FIELD_SIZE 2
#define PRI_PER1_DOM4_PRI_EN_FIELD_OFFSET 0
#define PRI_PER1_DOM4_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_PRI_PER1_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_PRI_PER1_DOM7_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_PRI_PER1_DOM7_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_PRI_PER1_DOM7_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_PRI_PER1_DOM7_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_PRI_PER1_DOM7_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_PRI_PER1_DOM6_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_PRI_PER1_DOM5_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_PRI_PER1_DOM4_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER2
// Register Offset : 0x2420 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x420 --> 0x7fe420
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_PRI_PER2 (RPC_APB_AB0_BASE_ADDR + (0x7fe420<<0))
#define PRI_PER2_DOM11_LOCK_FIELD_OFFSET 31
#define PRI_PER2_DOM11_LOCK_FIELD_SIZE 1
#define PRI_PER2_DOM11_USE_PER_FIELD_OFFSET 27
#define PRI_PER2_DOM11_USE_PER_FIELD_SIZE 2
#define PRI_PER2_DOM11_PRI_PER_FIELD_OFFSET 25
#define PRI_PER2_DOM11_PRI_PER_FIELD_SIZE 2
#define PRI_PER2_DOM11_PRI_EN_FIELD_OFFSET 24
#define PRI_PER2_DOM11_PRI_EN_FIELD_SIZE 1
#define PRI_PER2_DOM10_LOCK_FIELD_OFFSET 23
#define PRI_PER2_DOM10_LOCK_FIELD_SIZE 1
#define PRI_PER2_DOM10_USE_PER_FIELD_OFFSET 19
#define PRI_PER2_DOM10_USE_PER_FIELD_SIZE 2
#define PRI_PER2_DOM10_PRI_PER_FIELD_OFFSET 17
#define PRI_PER2_DOM10_PRI_PER_FIELD_SIZE 2
#define PRI_PER2_DOM10_PRI_EN_FIELD_OFFSET 16
#define PRI_PER2_DOM10_PRI_EN_FIELD_SIZE 1
#define PRI_PER2_DOM9_LOCK_FIELD_OFFSET 15
#define PRI_PER2_DOM9_LOCK_FIELD_SIZE 1
#define PRI_PER2_DOM9_USE_PER_FIELD_OFFSET 11
#define PRI_PER2_DOM9_USE_PER_FIELD_SIZE 2
#define PRI_PER2_DOM9_PRI_PER_FIELD_OFFSET 9
#define PRI_PER2_DOM9_PRI_PER_FIELD_SIZE 2
#define PRI_PER2_DOM9_PRI_EN_FIELD_OFFSET 8
#define PRI_PER2_DOM9_PRI_EN_FIELD_SIZE 1
#define PRI_PER2_DOM8_LOCK_FIELD_OFFSET 7
#define PRI_PER2_DOM8_LOCK_FIELD_SIZE 1
#define PRI_PER2_DOM8_USE_PER_FIELD_OFFSET 3
#define PRI_PER2_DOM8_USE_PER_FIELD_SIZE 2
#define PRI_PER2_DOM8_PRI_PER_FIELD_OFFSET 1
#define PRI_PER2_DOM8_PRI_PER_FIELD_SIZE 2
#define PRI_PER2_DOM8_PRI_EN_FIELD_OFFSET 0
#define PRI_PER2_DOM8_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_PRI_PER2_DOM11_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_PRI_PER2_DOM11_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_PRI_PER2_DOM11_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_PRI_PER2_DOM11_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_PRI_PER2_DOM11_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_PRI_PER2_DOM11_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_PRI_PER2_DOM10_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_PRI_PER2_DOM9_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_PRI_PER2_DOM8_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_PRI_PER3
// Register Offset : 0x2424 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x424 --> 0x7fe424
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_PRI_PER3 (RPC_APB_AB0_BASE_ADDR + (0x7fe424<<0))
#define PRI_PER3_DOM15_LOCK_FIELD_OFFSET 31
#define PRI_PER3_DOM15_LOCK_FIELD_SIZE 1
#define PRI_PER3_DOM15_USE_PER_FIELD_OFFSET 27
#define PRI_PER3_DOM15_USE_PER_FIELD_SIZE 2
#define PRI_PER3_DOM15_PRI_PER_FIELD_OFFSET 25
#define PRI_PER3_DOM15_PRI_PER_FIELD_SIZE 2
#define PRI_PER3_DOM15_PRI_EN_FIELD_OFFSET 24
#define PRI_PER3_DOM15_PRI_EN_FIELD_SIZE 1
#define PRI_PER3_DOM14_LOCK_FIELD_OFFSET 23
#define PRI_PER3_DOM14_LOCK_FIELD_SIZE 1
#define PRI_PER3_DOM14_USE_PER_FIELD_OFFSET 19
#define PRI_PER3_DOM14_USE_PER_FIELD_SIZE 2
#define PRI_PER3_DOM14_PRI_PER_FIELD_OFFSET 17
#define PRI_PER3_DOM14_PRI_PER_FIELD_SIZE 2
#define PRI_PER3_DOM14_PRI_EN_FIELD_OFFSET 16
#define PRI_PER3_DOM14_PRI_EN_FIELD_SIZE 1
#define PRI_PER3_DOM13_LOCK_FIELD_OFFSET 15
#define PRI_PER3_DOM13_LOCK_FIELD_SIZE 1
#define PRI_PER3_DOM13_USE_PER_FIELD_OFFSET 11
#define PRI_PER3_DOM13_USE_PER_FIELD_SIZE 2
#define PRI_PER3_DOM13_PRI_PER_FIELD_OFFSET 9
#define PRI_PER3_DOM13_PRI_PER_FIELD_SIZE 2
#define PRI_PER3_DOM13_PRI_EN_FIELD_OFFSET 8
#define PRI_PER3_DOM13_PRI_EN_FIELD_SIZE 1
#define PRI_PER3_DOM12_LOCK_FIELD_OFFSET 7
#define PRI_PER3_DOM12_LOCK_FIELD_SIZE 1
#define PRI_PER3_DOM12_USE_PER_FIELD_OFFSET 3
#define PRI_PER3_DOM12_USE_PER_FIELD_SIZE 2
#define PRI_PER3_DOM12_PRI_PER_FIELD_OFFSET 1
#define PRI_PER3_DOM12_PRI_PER_FIELD_SIZE 2
#define PRI_PER3_DOM12_PRI_EN_FIELD_OFFSET 0
#define PRI_PER3_DOM12_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_PRI_PER3_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_PRI_PER3_DOM15_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_PRI_PER3_DOM15_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_PRI_PER3_DOM15_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_PRI_PER3_DOM15_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_PRI_PER3_DOM15_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_PRI_PER3_DOM14_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_PRI_PER3_DOM13_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_PRI_PER3_DOM12_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_DOM_ASSIGN
// Register Offset : 0x2430 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x430 --> 0x7fe430
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_DOM_ASSIGN (RPC_APB_AB0_BASE_ADDR + (0x7fe430<<0))
#define DOM_ASSIGN_DID_FIELD_OFFSET 0
#define DOM_ASSIGN_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_DOM_ASSIGN_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_DOM_ASSIGN_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_DOM_ASSIGN_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_DOM_ASSIGN_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM
// Register Offset : 0x3100 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1100 --> 0x7ff100
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_DOM (RPC_APB_AB0_BASE_ADDR + (0x7ff100<<0))
#define RGN_DOM_LOCK_FIELD_OFFSET 31
#define RGN_DOM_LOCK_FIELD_SIZE 1
#define RGN_DOM_SET_FIELD_OFFSET 4
#define RGN_DOM_SET_FIELD_SIZE 1
#define RGN_DOM_DID_FIELD_OFFSET 0
#define RGN_DOM_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_RGN_DOM_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_DOM_SET    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_DOM_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_DOM_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_DOM_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_DOM_DID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM_PER0
// Register Offset : 0x3104 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1104 --> 0x7ff104
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_DOM_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7ff104<<0))
#define RGN_DOM_PER0_DOM7_LOCK_FIELD_OFFSET 31
#define RGN_DOM_PER0_DOM7_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM7_PER_FIELD_OFFSET 29
#define RGN_DOM_PER0_DOM7_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM7_EN_FIELD_OFFSET 28
#define RGN_DOM_PER0_DOM7_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM6_LOCK_FIELD_OFFSET 27
#define RGN_DOM_PER0_DOM6_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM6_PER_FIELD_OFFSET 25
#define RGN_DOM_PER0_DOM6_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM6_EN_FIELD_OFFSET 24
#define RGN_DOM_PER0_DOM6_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM5_LOCK_FIELD_OFFSET 23
#define RGN_DOM_PER0_DOM5_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM5_PER_FIELD_OFFSET 21
#define RGN_DOM_PER0_DOM5_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM5_EN_FIELD_OFFSET 20
#define RGN_DOM_PER0_DOM5_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM4_LOCK_FIELD_OFFSET 19
#define RGN_DOM_PER0_DOM4_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM4_PER_FIELD_OFFSET 17
#define RGN_DOM_PER0_DOM4_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM4_EN_FIELD_OFFSET 16
#define RGN_DOM_PER0_DOM4_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM3_LOCK_FIELD_OFFSET 15
#define RGN_DOM_PER0_DOM3_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM3_PER_FIELD_OFFSET 13
#define RGN_DOM_PER0_DOM3_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM3_EN_FIELD_OFFSET 12
#define RGN_DOM_PER0_DOM3_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM2_LOCK_FIELD_OFFSET 11
#define RGN_DOM_PER0_DOM2_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM2_PER_FIELD_OFFSET 9
#define RGN_DOM_PER0_DOM2_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM2_EN_FIELD_OFFSET 8
#define RGN_DOM_PER0_DOM2_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM1_LOCK_FIELD_OFFSET 7
#define RGN_DOM_PER0_DOM1_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM1_PER_FIELD_OFFSET 5
#define RGN_DOM_PER0_DOM1_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM1_EN_FIELD_OFFSET 4
#define RGN_DOM_PER0_DOM1_EN_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM0_LOCK_FIELD_OFFSET 3
#define RGN_DOM_PER0_DOM0_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER0_DOM0_PER_FIELD_OFFSET 1
#define RGN_DOM_PER0_DOM0_PER_FIELD_SIZE 2
#define RGN_DOM_PER0_DOM0_EN_FIELD_OFFSET 0
#define RGN_DOM_PER0_DOM0_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM7_PER_1    (BIT_(30))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM7_PER_0    (BIT_(29))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM7_EN    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM6_LOCK    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM6_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM6_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM6_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM5_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM5_PER_1    (BIT_(22))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM5_PER_0    (BIT_(21))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM5_EN    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM4_LOCK    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM4_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM4_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM4_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM3_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM3_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM3_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM3_EN    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM2_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM2_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM2_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM2_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM1_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM1_PER_1    (BIT_(6))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM1_PER_0    (BIT_(5))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM1_EN    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM0_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM0_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM0_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_DOM_PER0_DOM0_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM_PER1
// Register Offset : 0x3108 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1108 --> 0x7ff108
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_DOM_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7ff108<<0))
#define RGN_DOM_PER1_DOM15_LOCK_FIELD_OFFSET 31
#define RGN_DOM_PER1_DOM15_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM15_PER_FIELD_OFFSET 29
#define RGN_DOM_PER1_DOM15_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM15_EN_FIELD_OFFSET 28
#define RGN_DOM_PER1_DOM15_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM14_LOCK_FIELD_OFFSET 27
#define RGN_DOM_PER1_DOM14_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM14_PER_FIELD_OFFSET 25
#define RGN_DOM_PER1_DOM14_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM14_EN_FIELD_OFFSET 24
#define RGN_DOM_PER1_DOM14_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM13_LOCK_FIELD_OFFSET 23
#define RGN_DOM_PER1_DOM13_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM13_PER_FIELD_OFFSET 21
#define RGN_DOM_PER1_DOM13_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM13_EN_FIELD_OFFSET 20
#define RGN_DOM_PER1_DOM13_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM12_LOCK_FIELD_OFFSET 19
#define RGN_DOM_PER1_DOM12_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM12_PER_FIELD_OFFSET 17
#define RGN_DOM_PER1_DOM12_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM12_EN_FIELD_OFFSET 16
#define RGN_DOM_PER1_DOM12_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM11_LOCK_FIELD_OFFSET 15
#define RGN_DOM_PER1_DOM11_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM11_PER_FIELD_OFFSET 13
#define RGN_DOM_PER1_DOM11_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM11_EN_FIELD_OFFSET 12
#define RGN_DOM_PER1_DOM11_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM10_LOCK_FIELD_OFFSET 11
#define RGN_DOM_PER1_DOM10_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM10_PER_FIELD_OFFSET 9
#define RGN_DOM_PER1_DOM10_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM10_EN_FIELD_OFFSET 8
#define RGN_DOM_PER1_DOM10_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM9_LOCK_FIELD_OFFSET 7
#define RGN_DOM_PER1_DOM9_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM9_PER_FIELD_OFFSET 5
#define RGN_DOM_PER1_DOM9_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM9_EN_FIELD_OFFSET 4
#define RGN_DOM_PER1_DOM9_EN_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM8_LOCK_FIELD_OFFSET 3
#define RGN_DOM_PER1_DOM8_LOCK_FIELD_SIZE 1
#define RGN_DOM_PER1_DOM8_PER_FIELD_OFFSET 1
#define RGN_DOM_PER1_DOM8_PER_FIELD_SIZE 2
#define RGN_DOM_PER1_DOM8_EN_FIELD_OFFSET 0
#define RGN_DOM_PER1_DOM8_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM15_PER_1    (BIT_(30))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM15_PER_0    (BIT_(29))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM15_EN    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM14_LOCK    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM14_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM14_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM14_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM13_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM13_PER_1    (BIT_(22))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM13_PER_0    (BIT_(21))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM13_EN    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM12_LOCK    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM12_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM12_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM12_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM11_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM11_PER_1    (BIT_(14))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM11_PER_0    (BIT_(13))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM11_EN    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM10_LOCK    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM10_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM10_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM10_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM9_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM9_PER_1    (BIT_(6))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM9_PER_0    (BIT_(5))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM9_EN    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM8_LOCK    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM8_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM8_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_DOM_PER1_DOM8_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER0
// Register Offset : 0x310c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x110c --> 0x7ff10c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_SEC_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7ff10c<<0))
#define RGN_SEC_PER0_DOM3_LOCK_FIELD_OFFSET 31
#define RGN_SEC_PER0_DOM3_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM3_NSE_PER_FIELD_OFFSET 27
#define RGN_SEC_PER0_DOM3_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM3_SEC_PER_FIELD_OFFSET 25
#define RGN_SEC_PER0_DOM3_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM3_SEC_EN_FIELD_OFFSET 24
#define RGN_SEC_PER0_DOM3_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM2_LOCK_FIELD_OFFSET 23
#define RGN_SEC_PER0_DOM2_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM2_NSE_PER_FIELD_OFFSET 19
#define RGN_SEC_PER0_DOM2_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM2_SEC_PER_FIELD_OFFSET 17
#define RGN_SEC_PER0_DOM2_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM2_SEC_EN_FIELD_OFFSET 16
#define RGN_SEC_PER0_DOM2_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM1_LOCK_FIELD_OFFSET 15
#define RGN_SEC_PER0_DOM1_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM1_NSE_PER_FIELD_OFFSET 11
#define RGN_SEC_PER0_DOM1_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM1_SEC_PER_FIELD_OFFSET 9
#define RGN_SEC_PER0_DOM1_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM1_SEC_EN_FIELD_OFFSET 8
#define RGN_SEC_PER0_DOM1_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM0_LOCK_FIELD_OFFSET 7
#define RGN_SEC_PER0_DOM0_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER0_DOM0_NSE_PER_FIELD_OFFSET 3
#define RGN_SEC_PER0_DOM0_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM0_SEC_PER_FIELD_OFFSET 1
#define RGN_SEC_PER0_DOM0_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER0_DOM0_SEC_EN_FIELD_OFFSET 0
#define RGN_SEC_PER0_DOM0_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM3_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM2_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM1_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_SEC_PER0_DOM0_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER1
// Register Offset : 0x3110 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1110 --> 0x7ff110
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_SEC_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7ff110<<0))
#define RGN_SEC_PER1_DOM7_LOCK_FIELD_OFFSET 31
#define RGN_SEC_PER1_DOM7_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM7_NSE_PER_FIELD_OFFSET 27
#define RGN_SEC_PER1_DOM7_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM7_SEC_PER_FIELD_OFFSET 25
#define RGN_SEC_PER1_DOM7_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM7_SEC_EN_FIELD_OFFSET 24
#define RGN_SEC_PER1_DOM7_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM6_LOCK_FIELD_OFFSET 23
#define RGN_SEC_PER1_DOM6_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM6_NSE_PER_FIELD_OFFSET 19
#define RGN_SEC_PER1_DOM6_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM6_SEC_PER_FIELD_OFFSET 17
#define RGN_SEC_PER1_DOM6_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM6_SEC_EN_FIELD_OFFSET 16
#define RGN_SEC_PER1_DOM6_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM5_LOCK_FIELD_OFFSET 15
#define RGN_SEC_PER1_DOM5_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM5_NSE_PER_FIELD_OFFSET 11
#define RGN_SEC_PER1_DOM5_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM5_SEC_PER_FIELD_OFFSET 9
#define RGN_SEC_PER1_DOM5_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM5_SEC_EN_FIELD_OFFSET 8
#define RGN_SEC_PER1_DOM5_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM4_LOCK_FIELD_OFFSET 7
#define RGN_SEC_PER1_DOM4_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER1_DOM4_NSE_PER_FIELD_OFFSET 3
#define RGN_SEC_PER1_DOM4_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM4_SEC_PER_FIELD_OFFSET 1
#define RGN_SEC_PER1_DOM4_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER1_DOM4_SEC_EN_FIELD_OFFSET 0
#define RGN_SEC_PER1_DOM4_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM7_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM6_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM5_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_SEC_PER1_DOM4_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER2
// Register Offset : 0x3114 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1114 --> 0x7ff114
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_SEC_PER2 (RPC_APB_AB0_BASE_ADDR + (0x7ff114<<0))
#define RGN_SEC_PER2_DOM11_LOCK_FIELD_OFFSET 31
#define RGN_SEC_PER2_DOM11_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM11_NSE_PER_FIELD_OFFSET 27
#define RGN_SEC_PER2_DOM11_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM11_SEC_PER_FIELD_OFFSET 25
#define RGN_SEC_PER2_DOM11_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM11_SEC_EN_FIELD_OFFSET 24
#define RGN_SEC_PER2_DOM11_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM10_LOCK_FIELD_OFFSET 23
#define RGN_SEC_PER2_DOM10_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM10_NSE_PER_FIELD_OFFSET 19
#define RGN_SEC_PER2_DOM10_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM10_SEC_PER_FIELD_OFFSET 17
#define RGN_SEC_PER2_DOM10_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM10_SEC_EN_FIELD_OFFSET 16
#define RGN_SEC_PER2_DOM10_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM9_LOCK_FIELD_OFFSET 15
#define RGN_SEC_PER2_DOM9_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM9_NSE_PER_FIELD_OFFSET 11
#define RGN_SEC_PER2_DOM9_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM9_SEC_PER_FIELD_OFFSET 9
#define RGN_SEC_PER2_DOM9_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM9_SEC_EN_FIELD_OFFSET 8
#define RGN_SEC_PER2_DOM9_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM8_LOCK_FIELD_OFFSET 7
#define RGN_SEC_PER2_DOM8_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER2_DOM8_NSE_PER_FIELD_OFFSET 3
#define RGN_SEC_PER2_DOM8_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM8_SEC_PER_FIELD_OFFSET 1
#define RGN_SEC_PER2_DOM8_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER2_DOM8_SEC_EN_FIELD_OFFSET 0
#define RGN_SEC_PER2_DOM8_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM11_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM10_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM9_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_SEC_PER2_DOM8_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_SEC_PER3
// Register Offset : 0x3118 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1118 --> 0x7ff118
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_SEC_PER3 (RPC_APB_AB0_BASE_ADDR + (0x7ff118<<0))
#define RGN_SEC_PER3_DOM15_LOCK_FIELD_OFFSET 31
#define RGN_SEC_PER3_DOM15_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM15_NSE_PER_FIELD_OFFSET 27
#define RGN_SEC_PER3_DOM15_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM15_SEC_PER_FIELD_OFFSET 25
#define RGN_SEC_PER3_DOM15_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM15_SEC_EN_FIELD_OFFSET 24
#define RGN_SEC_PER3_DOM15_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM14_LOCK_FIELD_OFFSET 23
#define RGN_SEC_PER3_DOM14_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM14_NSE_PER_FIELD_OFFSET 19
#define RGN_SEC_PER3_DOM14_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM14_SEC_PER_FIELD_OFFSET 17
#define RGN_SEC_PER3_DOM14_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM14_SEC_EN_FIELD_OFFSET 16
#define RGN_SEC_PER3_DOM14_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM13_LOCK_FIELD_OFFSET 15
#define RGN_SEC_PER3_DOM13_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM13_NSE_PER_FIELD_OFFSET 11
#define RGN_SEC_PER3_DOM13_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM13_SEC_PER_FIELD_OFFSET 9
#define RGN_SEC_PER3_DOM13_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM13_SEC_EN_FIELD_OFFSET 8
#define RGN_SEC_PER3_DOM13_SEC_EN_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM12_LOCK_FIELD_OFFSET 7
#define RGN_SEC_PER3_DOM12_LOCK_FIELD_SIZE 1
#define RGN_SEC_PER3_DOM12_NSE_PER_FIELD_OFFSET 3
#define RGN_SEC_PER3_DOM12_NSE_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM12_SEC_PER_FIELD_OFFSET 1
#define RGN_SEC_PER3_DOM12_SEC_PER_FIELD_SIZE 2
#define RGN_SEC_PER3_DOM12_SEC_EN_FIELD_OFFSET 0
#define RGN_SEC_PER3_DOM12_SEC_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_NSE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_NSE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_SEC_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_SEC_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM15_SEC_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_NSE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_NSE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_SEC_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_SEC_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM14_SEC_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_NSE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_NSE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_SEC_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_SEC_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM13_SEC_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_NSE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_NSE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_SEC_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_SEC_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_SEC_PER3_DOM12_SEC_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER0
// Register Offset : 0x311c --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x111c --> 0x7ff11c
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_PRI_PER0 (RPC_APB_AB0_BASE_ADDR + (0x7ff11c<<0))
#define RGN_PRI_PER0_DOM3_LOCK_FIELD_OFFSET 31
#define RGN_PRI_PER0_DOM3_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM3_USE_PER_FIELD_OFFSET 27
#define RGN_PRI_PER0_DOM3_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM3_PRI_PER_FIELD_OFFSET 25
#define RGN_PRI_PER0_DOM3_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM3_PRI_EN_FIELD_OFFSET 24
#define RGN_PRI_PER0_DOM3_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM2_LOCK_FIELD_OFFSET 23
#define RGN_PRI_PER0_DOM2_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM2_USE_PER_FIELD_OFFSET 19
#define RGN_PRI_PER0_DOM2_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM2_PRI_PER_FIELD_OFFSET 17
#define RGN_PRI_PER0_DOM2_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM2_PRI_EN_FIELD_OFFSET 16
#define RGN_PRI_PER0_DOM2_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM1_LOCK_FIELD_OFFSET 15
#define RGN_PRI_PER0_DOM1_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM1_USE_PER_FIELD_OFFSET 11
#define RGN_PRI_PER0_DOM1_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM1_PRI_PER_FIELD_OFFSET 9
#define RGN_PRI_PER0_DOM1_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM1_PRI_EN_FIELD_OFFSET 8
#define RGN_PRI_PER0_DOM1_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM0_LOCK_FIELD_OFFSET 7
#define RGN_PRI_PER0_DOM0_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER0_DOM0_USE_PER_FIELD_OFFSET 3
#define RGN_PRI_PER0_DOM0_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM0_PRI_PER_FIELD_OFFSET 1
#define RGN_PRI_PER0_DOM0_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER0_DOM0_PRI_EN_FIELD_OFFSET 0
#define RGN_PRI_PER0_DOM0_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM3_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM2_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM1_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_PRI_PER0_DOM0_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER1
// Register Offset : 0x3120 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1120 --> 0x7ff120
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_PRI_PER1 (RPC_APB_AB0_BASE_ADDR + (0x7ff120<<0))
#define RGN_PRI_PER1_DOM7_LOCK_FIELD_OFFSET 31
#define RGN_PRI_PER1_DOM7_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM7_USE_PER_FIELD_OFFSET 27
#define RGN_PRI_PER1_DOM7_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM7_PRI_PER_FIELD_OFFSET 25
#define RGN_PRI_PER1_DOM7_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM7_PRI_EN_FIELD_OFFSET 24
#define RGN_PRI_PER1_DOM7_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM6_LOCK_FIELD_OFFSET 23
#define RGN_PRI_PER1_DOM6_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM6_USE_PER_FIELD_OFFSET 19
#define RGN_PRI_PER1_DOM6_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM6_PRI_PER_FIELD_OFFSET 17
#define RGN_PRI_PER1_DOM6_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM6_PRI_EN_FIELD_OFFSET 16
#define RGN_PRI_PER1_DOM6_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM5_LOCK_FIELD_OFFSET 15
#define RGN_PRI_PER1_DOM5_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM5_USE_PER_FIELD_OFFSET 11
#define RGN_PRI_PER1_DOM5_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM5_PRI_PER_FIELD_OFFSET 9
#define RGN_PRI_PER1_DOM5_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM5_PRI_EN_FIELD_OFFSET 8
#define RGN_PRI_PER1_DOM5_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM4_LOCK_FIELD_OFFSET 7
#define RGN_PRI_PER1_DOM4_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER1_DOM4_USE_PER_FIELD_OFFSET 3
#define RGN_PRI_PER1_DOM4_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM4_PRI_PER_FIELD_OFFSET 1
#define RGN_PRI_PER1_DOM4_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER1_DOM4_PRI_EN_FIELD_OFFSET 0
#define RGN_PRI_PER1_DOM4_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM7_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM6_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM5_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_PRI_PER1_DOM4_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER2
// Register Offset : 0x3124 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1124 --> 0x7ff124
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_PRI_PER2 (RPC_APB_AB0_BASE_ADDR + (0x7ff124<<0))
#define RGN_PRI_PER2_DOM11_LOCK_FIELD_OFFSET 31
#define RGN_PRI_PER2_DOM11_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM11_USE_PER_FIELD_OFFSET 27
#define RGN_PRI_PER2_DOM11_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM11_PRI_PER_FIELD_OFFSET 25
#define RGN_PRI_PER2_DOM11_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM11_PRI_EN_FIELD_OFFSET 24
#define RGN_PRI_PER2_DOM11_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM10_LOCK_FIELD_OFFSET 23
#define RGN_PRI_PER2_DOM10_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM10_USE_PER_FIELD_OFFSET 19
#define RGN_PRI_PER2_DOM10_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM10_PRI_PER_FIELD_OFFSET 17
#define RGN_PRI_PER2_DOM10_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM10_PRI_EN_FIELD_OFFSET 16
#define RGN_PRI_PER2_DOM10_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM9_LOCK_FIELD_OFFSET 15
#define RGN_PRI_PER2_DOM9_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM9_USE_PER_FIELD_OFFSET 11
#define RGN_PRI_PER2_DOM9_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM9_PRI_PER_FIELD_OFFSET 9
#define RGN_PRI_PER2_DOM9_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM9_PRI_EN_FIELD_OFFSET 8
#define RGN_PRI_PER2_DOM9_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM8_LOCK_FIELD_OFFSET 7
#define RGN_PRI_PER2_DOM8_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER2_DOM8_USE_PER_FIELD_OFFSET 3
#define RGN_PRI_PER2_DOM8_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM8_PRI_PER_FIELD_OFFSET 1
#define RGN_PRI_PER2_DOM8_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER2_DOM8_PRI_EN_FIELD_OFFSET 0
#define RGN_PRI_PER2_DOM8_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM11_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM10_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM9_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_PRI_PER2_DOM8_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_PRI_PER3
// Register Offset : 0x3128 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1128 --> 0x7ff128
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_PRI_PER3 (RPC_APB_AB0_BASE_ADDR + (0x7ff128<<0))
#define RGN_PRI_PER3_DOM15_LOCK_FIELD_OFFSET 31
#define RGN_PRI_PER3_DOM15_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM15_USE_PER_FIELD_OFFSET 27
#define RGN_PRI_PER3_DOM15_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM15_PRI_PER_FIELD_OFFSET 25
#define RGN_PRI_PER3_DOM15_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM15_PRI_EN_FIELD_OFFSET 24
#define RGN_PRI_PER3_DOM15_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM14_LOCK_FIELD_OFFSET 23
#define RGN_PRI_PER3_DOM14_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM14_USE_PER_FIELD_OFFSET 19
#define RGN_PRI_PER3_DOM14_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM14_PRI_PER_FIELD_OFFSET 17
#define RGN_PRI_PER3_DOM14_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM14_PRI_EN_FIELD_OFFSET 16
#define RGN_PRI_PER3_DOM14_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM13_LOCK_FIELD_OFFSET 15
#define RGN_PRI_PER3_DOM13_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM13_USE_PER_FIELD_OFFSET 11
#define RGN_PRI_PER3_DOM13_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM13_PRI_PER_FIELD_OFFSET 9
#define RGN_PRI_PER3_DOM13_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM13_PRI_EN_FIELD_OFFSET 8
#define RGN_PRI_PER3_DOM13_PRI_EN_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM12_LOCK_FIELD_OFFSET 7
#define RGN_PRI_PER3_DOM12_LOCK_FIELD_SIZE 1
#define RGN_PRI_PER3_DOM12_USE_PER_FIELD_OFFSET 3
#define RGN_PRI_PER3_DOM12_USE_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM12_PRI_PER_FIELD_OFFSET 1
#define RGN_PRI_PER3_DOM12_PRI_PER_FIELD_SIZE 2
#define RGN_PRI_PER3_DOM12_PRI_EN_FIELD_OFFSET 0
#define RGN_PRI_PER3_DOM12_PRI_EN_FIELD_SIZE 1

#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_LOCK    (BIT_(31))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_USE_PER_1    (BIT_(28))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_USE_PER_0    (BIT_(27))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_PRI_PER_1    (BIT_(26))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_PRI_PER_0    (BIT_(25))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM15_PRI_EN    (BIT_(24))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_LOCK    (BIT_(23))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_USE_PER_1    (BIT_(20))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_USE_PER_0    (BIT_(19))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_PRI_PER_1    (BIT_(18))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_PRI_PER_0    (BIT_(17))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM14_PRI_EN    (BIT_(16))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_LOCK    (BIT_(15))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_USE_PER_1    (BIT_(12))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_USE_PER_0    (BIT_(11))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_PRI_PER_1    (BIT_(10))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_PRI_PER_0    (BIT_(9))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM13_PRI_EN    (BIT_(8))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_LOCK    (BIT_(7))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_USE_PER_1    (BIT_(4))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_USE_PER_0    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_PRI_PER_1    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_PRI_PER_0    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_PRI_PER3_DOM12_PRI_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_RPC_RGN_DOM_ASSIGN
// Register Offset : 0x3134 --> 0x2000 * 0x400(1024) - 0x2000(8 * 1024) + 0x1134 --> 0x7ff134
// Description     :
//--------------------------------------------------------------------------
#define REG_AP_APB_RPC_RGN_DOM_ASSIGN (RPC_APB_AB0_BASE_ADDR + (0x7ff134<<0))
#define RGN_DOM_ASSIGN_DID_FIELD_OFFSET 0
#define RGN_DOM_ASSIGN_DID_FIELD_SIZE 4

#define BIT_AP_APB_RPC_RGN_DOM_ASSIGN_DID_3    (BIT_(3))
#define BIT_AP_APB_RPC_RGN_DOM_ASSIGN_DID_2    (BIT_(2))
#define BIT_AP_APB_RPC_RGN_DOM_ASSIGN_DID_1    (BIT_(1))
#define BIT_AP_APB_RPC_RGN_DOM_ASSIGN_DID_0    (BIT_(0))

#endif
