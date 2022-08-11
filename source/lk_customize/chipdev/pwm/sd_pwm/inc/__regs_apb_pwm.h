
#ifndef __REGS_APB_PWM_H__
#define __REGS_APB_PWM_H__

//--------------------------------------------------------------------------
// IP Ref Info     : REG_AP_APB_PWM
// RTL version     :
//--------------------------------------------------------------------------
#ifndef BIT_
#define BIT_(x)	(1 << x)
#endif

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_INT_STA
// Register Offset : 0x0
// Description     :
//--------------------------------------------------------------------------
#define INT_STA_FIFO_UNDERRUN_FIELD_OFFSET 2
#define INT_STA_FIFO_UNDERRUN_FIELD_SIZE 1
#define INT_STA_CNT_G0_OVF_FIELD_OFFSET 1
#define INT_STA_CNT_G0_OVF_FIELD_SIZE 1
#define INT_STA_CMP_EVENT_FIELD_OFFSET 0
#define INT_STA_CMP_EVENT_FIELD_SIZE 1

#define BIT_AP_APB_PWM_INT_STA_FIFO_UNDERRUN    (BIT_(2))
#define BIT_AP_APB_PWM_INT_STA_CNT_G0_OVF    (BIT_(1))
#define BIT_AP_APB_PWM_INT_STA_CMP_EVENT    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_INT_STA_EN
// Register Offset : 0x4
// Description     :
//--------------------------------------------------------------------------
#define INT_STA_EN_FIFO_UNDERRUN_FIELD_OFFSET 2
#define INT_STA_EN_FIFO_UNDERRUN_FIELD_SIZE 1
#define INT_STA_EN_CNT_G0_OVF_FIELD_OFFSET 1
#define INT_STA_EN_CNT_G0_OVF_FIELD_SIZE 1
#define INT_STA_EN_CMP_EVENT_FIELD_OFFSET 0
#define INT_STA_EN_CMP_EVENT_FIELD_SIZE 1

#define BIT_AP_APB_PWM_INT_STA_EN_FIFO_UNDERRUN    (BIT_(2))
#define BIT_AP_APB_PWM_INT_STA_EN_CNT_G0_OVF    (BIT_(1))
#define BIT_AP_APB_PWM_INT_STA_EN_CMP_EVENT    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_INT_SIG_EN
// Register Offset : 0x8
// Description     :
//--------------------------------------------------------------------------
#define INT_SIG_EN_FIFO_UNDERRUN_FIELD_OFFSET 2
#define INT_SIG_EN_FIFO_UNDERRUN_FIELD_SIZE 1
#define INT_SIG_EN_CNT_G0_OVF_FIELD_OFFSET 1
#define INT_SIG_EN_CNT_G0_OVF_FIELD_SIZE 1
#define INT_SIG_EN_CMP_EVENT_FIELD_OFFSET 0
#define INT_SIG_EN_CMP_EVENT_FIELD_SIZE 1

#define BIT_AP_APB_PWM_INT_SIG_EN_FIFO_UNDERRUN    (BIT_(2))
#define BIT_AP_APB_PWM_INT_SIG_EN_CNT_G0_OVF    (BIT_(1))
#define BIT_AP_APB_PWM_INT_SIG_EN_CMP_EVENT    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CNT_G0_CONFIG
// Register Offset : 0xc
// Description     :
//--------------------------------------------------------------------------
#define CNT_G0_CONFIG_DIV_NUM_FIELD_OFFSET 16
#define CNT_G0_CONFIG_DIV_NUM_FIELD_SIZE 16
#define CNT_G0_CONFIG_INT_CLR_FIELD_OFFSET 4
#define CNT_G0_CONFIG_INT_CLR_FIELD_SIZE 1
#define CNT_G0_CONFIG_EXT_CLR_EN_FIELD_OFFSET 3
#define CNT_G0_CONFIG_EXT_CLR_EN_FIELD_SIZE 1
#define CNT_G0_CONFIG_FRC_RLD_FIELD_OFFSET 2
#define CNT_G0_CONFIG_FRC_RLD_FIELD_SIZE 1
#define CNT_G0_CONFIG_SRC_CLK_SEL_FIELD_OFFSET 0
#define CNT_G0_CONFIG_SRC_CLK_SEL_FIELD_SIZE 2

#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_15    (BIT_(31))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_14    (BIT_(30))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_13    (BIT_(29))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_12    (BIT_(28))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_11    (BIT_(27))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_10    (BIT_(26))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_9    (BIT_(25))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_8    (BIT_(24))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_7    (BIT_(23))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_6    (BIT_(22))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_5    (BIT_(21))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_4    (BIT_(20))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_3    (BIT_(19))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_2    (BIT_(18))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_1    (BIT_(17))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_DIV_NUM_0    (BIT_(16))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_INT_CLR    (BIT_(4))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_EXT_CLR_EN    (BIT_(3))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_FRC_RLD    (BIT_(2))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_SRC_CLK_SEL_1    (BIT_(1))
#define BIT_AP_APB_PWM_CNT_G0_CONFIG_SRC_CLK_SEL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CLK_CONFIG
// Register Offset : 0x10
// Description     :
//--------------------------------------------------------------------------
#define CLK_CONFIG_CLK1_DIV_FIELD_OFFSET 16
#define CLK_CONFIG_CLK1_DIV_FIELD_SIZE 16
#define CLK_CONFIG_CLK0_DIV_FIELD_OFFSET 0
#define CLK_CONFIG_CLK0_DIV_FIELD_SIZE 16

#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_15    (BIT_(31))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_14    (BIT_(30))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_13    (BIT_(29))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_12    (BIT_(28))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_11    (BIT_(27))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_10    (BIT_(26))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_9    (BIT_(25))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_8    (BIT_(24))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_7    (BIT_(23))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_6    (BIT_(22))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_5    (BIT_(21))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_4    (BIT_(20))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_3    (BIT_(19))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_2    (BIT_(18))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_1    (BIT_(17))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK1_DIV_0    (BIT_(16))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_15    (BIT_(15))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_14    (BIT_(14))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_13    (BIT_(13))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_12    (BIT_(12))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_11    (BIT_(11))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_10    (BIT_(10))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_9    (BIT_(9))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_8    (BIT_(8))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_7    (BIT_(7))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_6    (BIT_(6))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_5    (BIT_(5))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_4    (BIT_(4))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_3    (BIT_(3))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_2    (BIT_(2))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_1    (BIT_(1))
#define BIT_AP_APB_PWM_CLK_CONFIG_CLK0_DIV_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CNT_G0_OVF
// Register Offset : 0x14
// Description     :
//--------------------------------------------------------------------------
#define CNT_G0_OVF_VALUE_FIELD_OFFSET 0
#define CNT_G0_OVF_VALUE_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_31    (BIT_(31))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_30    (BIT_(30))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_29    (BIT_(29))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_28    (BIT_(28))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_27    (BIT_(27))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_26    (BIT_(26))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_25    (BIT_(25))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_24    (BIT_(24))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_23    (BIT_(23))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_22    (BIT_(22))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_21    (BIT_(21))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_20    (BIT_(20))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_19    (BIT_(19))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_18    (BIT_(18))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_17    (BIT_(17))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_16    (BIT_(16))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_15    (BIT_(15))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_14    (BIT_(14))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_13    (BIT_(13))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_12    (BIT_(12))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_11    (BIT_(11))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_10    (BIT_(10))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_9    (BIT_(9))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_8    (BIT_(8))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_7    (BIT_(7))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_6    (BIT_(6))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_5    (BIT_(5))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_4    (BIT_(4))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_3    (BIT_(3))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CNT_G0_OVF_VALUE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CNT_G0
// Register Offset : 0x18
// Description     :
//--------------------------------------------------------------------------
#define CNT_G0_TIMER_FIELD_OFFSET 0
#define CNT_G0_TIMER_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CNT_G0_TIMER_31    (BIT_(31))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_30    (BIT_(30))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_29    (BIT_(29))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_28    (BIT_(28))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_27    (BIT_(27))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_26    (BIT_(26))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_25    (BIT_(25))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_24    (BIT_(24))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_23    (BIT_(23))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_22    (BIT_(22))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_21    (BIT_(21))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_20    (BIT_(20))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_19    (BIT_(19))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_18    (BIT_(18))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_17    (BIT_(17))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_16    (BIT_(16))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_15    (BIT_(15))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_14    (BIT_(14))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_13    (BIT_(13))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_12    (BIT_(12))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_11    (BIT_(11))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_10    (BIT_(10))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_9    (BIT_(9))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_8    (BIT_(8))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_7    (BIT_(7))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_6    (BIT_(6))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_5    (BIT_(5))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_4    (BIT_(4))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_3    (BIT_(3))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_2    (BIT_(2))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_1    (BIT_(1))
#define BIT_AP_APB_PWM_CNT_G0_TIMER_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_VAL_UPT
// Register Offset : 0x1c
// Description     :
//--------------------------------------------------------------------------
#define CMP_VAL_UPT_UPT_FIELD_OFFSET 0
#define CMP_VAL_UPT_UPT_FIELD_SIZE 1

#define BIT_AP_APB_PWM_CMP_VAL_UPT_UPT    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP0_A_VAL
// Register Offset : 0x20
// Description     :
//--------------------------------------------------------------------------
#define CMP0_A_VAL_DATA_FIELD_OFFSET 0
#define CMP0_A_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP0_A_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP1_A_VAL
// Register Offset : 0x24
// Description     :
//--------------------------------------------------------------------------
#define CMP1_A_VAL_DATA_FIELD_OFFSET 0
#define CMP1_A_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP1_A_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP0_B_VAL
// Register Offset : 0x28
// Description     :
//--------------------------------------------------------------------------
#define CMP0_B_VAL_DATA_FIELD_OFFSET 0
#define CMP0_B_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP0_B_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP1_B_VAL
// Register Offset : 0x2c
// Description     :
//--------------------------------------------------------------------------
#define CMP1_B_VAL_DATA_FIELD_OFFSET 0
#define CMP1_B_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP1_B_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP0_C_VAL
// Register Offset : 0x30
// Description     :
//--------------------------------------------------------------------------
#define CMP0_C_VAL_DATA_FIELD_OFFSET 0
#define CMP0_C_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP0_C_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP1_C_VAL
// Register Offset : 0x34
// Description     :
//--------------------------------------------------------------------------
#define CMP1_C_VAL_DATA_FIELD_OFFSET 0
#define CMP1_C_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP1_C_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP0_D_VAL
// Register Offset : 0x38
// Description     :
//--------------------------------------------------------------------------
#define CMP0_D_VAL_DATA_FIELD_OFFSET 0
#define CMP0_D_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP0_D_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP1_D_VAL
// Register Offset : 0x3c
// Description     :
//--------------------------------------------------------------------------
#define CMP1_D_VAL_DATA_FIELD_OFFSET 0
#define CMP1_D_VAL_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP1_D_VAL_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_CONFIG
// Register Offset : 0x40
// Description     :
//--------------------------------------------------------------------------
#define CMP_CONFIG_RPT_NUM_FIELD_OFFSET 16
#define CMP_CONFIG_RPT_NUM_FIELD_SIZE 8
#define CMP_CONFIG_FIFO_WML_FIELD_OFFSET 8
#define CMP_CONFIG_FIFO_WML_FIELD_SIZE 6
#define CMP_CONFIG_DMA_EN_FIELD_OFFSET 5
#define CMP_CONFIG_DMA_EN_FIELD_SIZE 1
#define CMP_CONFIG_DUAL_CMP_MODE_FIELD_OFFSET 4
#define CMP_CONFIG_DUAL_CMP_MODE_FIELD_SIZE 1
#define CMP_CONFIG_GRP_NUM_FIELD_OFFSET 2
#define CMP_CONFIG_GRP_NUM_FIELD_SIZE 2
#define CMP_CONFIG_DATA_FORMAT_FIELD_OFFSET 0
#define CMP_CONFIG_DATA_FORMAT_FIELD_SIZE 2

#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_CONFIG_RPT_NUM_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_5    (BIT_(13))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_4    (BIT_(12))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_3    (BIT_(11))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_2    (BIT_(10))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_1    (BIT_(9))
#define BIT_AP_APB_PWM_CMP_CONFIG_FIFO_WML_0    (BIT_(8))
#define BIT_AP_APB_PWM_CMP_CONFIG_DMA_EN    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_CONFIG_DUAL_CMP_MODE    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_CONFIG_GRP_NUM_1    (BIT_(3))
#define BIT_AP_APB_PWM_CMP_CONFIG_GRP_NUM_0    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_CONFIG_DATA_FORMAT_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_CONFIG_DATA_FORMAT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_FIFO_ENTRY
// Register Offset : 0x44
// Description     :
//--------------------------------------------------------------------------
#define FIFO_ENTRY_DATA_FIELD_OFFSET 0
#define FIFO_ENTRY_DATA_FIELD_SIZE 32

#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_31    (BIT_(31))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_30    (BIT_(30))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_29    (BIT_(29))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_28    (BIT_(28))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_27    (BIT_(27))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_26    (BIT_(26))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_25    (BIT_(25))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_24    (BIT_(24))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_23    (BIT_(23))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_22    (BIT_(22))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_21    (BIT_(21))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_20    (BIT_(20))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_19    (BIT_(19))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_18    (BIT_(18))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_17    (BIT_(17))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_16    (BIT_(16))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_15    (BIT_(15))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_14    (BIT_(14))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_13    (BIT_(13))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_12    (BIT_(12))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_11    (BIT_(11))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_10    (BIT_(10))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_9    (BIT_(9))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_8    (BIT_(8))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_7    (BIT_(7))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_6    (BIT_(6))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_5    (BIT_(5))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_4    (BIT_(4))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_3    (BIT_(3))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_2    (BIT_(2))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_1    (BIT_(1))
#define BIT_AP_APB_PWM_FIFO_ENTRY_DATA_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_FIFO_STAT
// Register Offset : 0x48
// Description     :
//--------------------------------------------------------------------------
#define FIFO_STAT_ENTRIES_FIELD_OFFSET 2
#define FIFO_STAT_ENTRIES_FIELD_SIZE 7
#define FIFO_STAT_EMPTY_FIELD_OFFSET 1
#define FIFO_STAT_EMPTY_FIELD_SIZE 1
#define FIFO_STAT_FULL_FIELD_OFFSET 0
#define FIFO_STAT_FULL_FIELD_SIZE 1

#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_6    (BIT_(8))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_5    (BIT_(7))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_4    (BIT_(6))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_3    (BIT_(5))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_2    (BIT_(4))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_1    (BIT_(3))
#define BIT_AP_APB_PWM_FIFO_STAT_ENTRIES_0    (BIT_(2))
#define BIT_AP_APB_PWM_FIFO_STAT_EMPTY    (BIT_(1))
#define BIT_AP_APB_PWM_FIFO_STAT_FULL    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_CTRL
// Register Offset : 0x4c
// Description     :
//--------------------------------------------------------------------------
#define CMP_CTRL_SW_RST_FIELD_OFFSET 31
#define CMP_CTRL_SW_RST_FIELD_SIZE 1
#define CMP_CTRL_SINGLE_MODE_FIELD_OFFSET 1
#define CMP_CTRL_SINGLE_MODE_FIELD_SIZE 1
#define CMP_CTRL_EN_FIELD_OFFSET 0
#define CMP_CTRL_EN_FIELD_SIZE 1

#define BIT_AP_APB_PWM_CMP_CTRL_SW_RST    (BIT_(31))
#define BIT_AP_APB_PWM_CMP_CTRL_SINGLE_MODE    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_CTRL_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_A_CONFIG0
// Register Offset : 0x50
// Description     :
//--------------------------------------------------------------------------
#define CMP_A_CONFIG0_CMP1_PULSE_WID_FIELD_OFFSET 24
#define CMP_A_CONFIG0_CMP1_PULSE_WID_FIELD_SIZE 8
#define CMP_A_CONFIG0_CMP0_PULSE_WID_FIELD_OFFSET 16
#define CMP_A_CONFIG0_CMP0_PULSE_WID_FIELD_SIZE 8
#define CMP_A_CONFIG0_CMP1_OUT_MODE_FIELD_OFFSET 3
#define CMP_A_CONFIG0_CMP1_OUT_MODE_FIELD_SIZE 3
#define CMP_A_CONFIG0_CMP0_OUT_MODE_FIELD_OFFSET 0
#define CMP_A_CONFIG0_CMP0_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_7    (BIT_(31))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_6    (BIT_(30))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_5    (BIT_(29))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_4    (BIT_(28))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_3    (BIT_(27))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_2    (BIT_(26))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_1    (BIT_(25))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_PULSE_WID_0    (BIT_(24))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_OUT_MODE_2    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_OUT_MODE_1    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP1_OUT_MODE_0    (BIT_(3))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_A_CONFIG0_CMP0_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_A_CONFIG1
// Register Offset : 0x54
// Description     :
//--------------------------------------------------------------------------
#define CMP_A_CONFIG1_OVF_PULSE_WID_FIELD_OFFSET 16
#define CMP_A_CONFIG1_OVF_PULSE_WID_FIELD_SIZE 8
#define CMP_A_CONFIG1_FRC_LOW_FIELD_OFFSET 5
#define CMP_A_CONFIG1_FRC_LOW_FIELD_SIZE 1
#define CMP_A_CONFIG1_FRC_HIGH_FIELD_OFFSET 4
#define CMP_A_CONFIG1_FRC_HIGH_FIELD_SIZE 1
#define CMP_A_CONFIG1_OVF_OUT_MODE_FIELD_OFFSET 0
#define CMP_A_CONFIG1_OVF_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_FRC_LOW    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_FRC_HIGH    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_A_CONFIG1_OVF_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_B_CONFIG0
// Register Offset : 0x58
// Description     :
//--------------------------------------------------------------------------
#define CMP_B_CONFIG0_CMP1_PULSE_WID_FIELD_OFFSET 24
#define CMP_B_CONFIG0_CMP1_PULSE_WID_FIELD_SIZE 8
#define CMP_B_CONFIG0_CMP0_PULSE_WID_FIELD_OFFSET 16
#define CMP_B_CONFIG0_CMP0_PULSE_WID_FIELD_SIZE 8
#define CMP_B_CONFIG0_CMP1_OUT_MODE_FIELD_OFFSET 3
#define CMP_B_CONFIG0_CMP1_OUT_MODE_FIELD_SIZE 3
#define CMP_B_CONFIG0_CMP0_OUT_MODE_FIELD_OFFSET 0
#define CMP_B_CONFIG0_CMP0_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_7    (BIT_(31))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_6    (BIT_(30))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_5    (BIT_(29))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_4    (BIT_(28))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_3    (BIT_(27))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_2    (BIT_(26))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_1    (BIT_(25))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_PULSE_WID_0    (BIT_(24))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_OUT_MODE_2    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_OUT_MODE_1    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP1_OUT_MODE_0    (BIT_(3))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_B_CONFIG0_CMP0_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_B_CONFIG1
// Register Offset : 0x5c
// Description     :
//--------------------------------------------------------------------------
#define CMP_B_CONFIG1_OVF_PULSE_WID_FIELD_OFFSET 16
#define CMP_B_CONFIG1_OVF_PULSE_WID_FIELD_SIZE 8
#define CMP_B_CONFIG1_FRC_LOW_FIELD_OFFSET 5
#define CMP_B_CONFIG1_FRC_LOW_FIELD_SIZE 1
#define CMP_B_CONFIG1_FRC_HIGH_FIELD_OFFSET 4
#define CMP_B_CONFIG1_FRC_HIGH_FIELD_SIZE 1
#define CMP_B_CONFIG1_OVF_OUT_MODE_FIELD_OFFSET 0
#define CMP_B_CONFIG1_OVF_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_FRC_LOW    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_FRC_HIGH    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_B_CONFIG1_OVF_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_C_CONFIG0
// Register Offset : 0x60
// Description     :
//--------------------------------------------------------------------------
#define CMP_C_CONFIG0_CMP1_PULSE_WID_FIELD_OFFSET 24
#define CMP_C_CONFIG0_CMP1_PULSE_WID_FIELD_SIZE 8
#define CMP_C_CONFIG0_CMP0_PULSE_WID_FIELD_OFFSET 16
#define CMP_C_CONFIG0_CMP0_PULSE_WID_FIELD_SIZE 8
#define CMP_C_CONFIG0_CMP1_OUT_MODE_FIELD_OFFSET 3
#define CMP_C_CONFIG0_CMP1_OUT_MODE_FIELD_SIZE 3
#define CMP_C_CONFIG0_CMP0_OUT_MODE_FIELD_OFFSET 0
#define CMP_C_CONFIG0_CMP0_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_7    (BIT_(31))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_6    (BIT_(30))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_5    (BIT_(29))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_4    (BIT_(28))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_3    (BIT_(27))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_2    (BIT_(26))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_1    (BIT_(25))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_PULSE_WID_0    (BIT_(24))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_OUT_MODE_2    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_OUT_MODE_1    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP1_OUT_MODE_0    (BIT_(3))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_C_CONFIG0_CMP0_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_C_CONFIG1
// Register Offset : 0x64
// Description     :
//--------------------------------------------------------------------------
#define CMP_C_CONFIG1_OVF_PULSE_WID_FIELD_OFFSET 16
#define CMP_C_CONFIG1_OVF_PULSE_WID_FIELD_SIZE 8
#define CMP_C_CONFIG1_FRC_LOW_FIELD_OFFSET 5
#define CMP_C_CONFIG1_FRC_LOW_FIELD_SIZE 1
#define CMP_C_CONFIG1_FRC_HIGH_FIELD_OFFSET 4
#define CMP_C_CONFIG1_FRC_HIGH_FIELD_SIZE 1
#define CMP_C_CONFIG1_OVF_OUT_MODE_FIELD_OFFSET 0
#define CMP_C_CONFIG1_OVF_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_FRC_LOW    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_FRC_HIGH    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_C_CONFIG1_OVF_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_D_CONFIG0
// Register Offset : 0x68
// Description     :
//--------------------------------------------------------------------------
#define CMP_D_CONFIG0_CMP1_PULSE_WID_FIELD_OFFSET 24
#define CMP_D_CONFIG0_CMP1_PULSE_WID_FIELD_SIZE 8
#define CMP_D_CONFIG0_CMP0_PULSE_WID_FIELD_OFFSET 16
#define CMP_D_CONFIG0_CMP0_PULSE_WID_FIELD_SIZE 8
#define CMP_D_CONFIG0_CMP1_OUT_MODE_FIELD_OFFSET 3
#define CMP_D_CONFIG0_CMP1_OUT_MODE_FIELD_SIZE 3
#define CMP_D_CONFIG0_CMP0_OUT_MODE_FIELD_OFFSET 0
#define CMP_D_CONFIG0_CMP0_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_7    (BIT_(31))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_6    (BIT_(30))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_5    (BIT_(29))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_4    (BIT_(28))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_3    (BIT_(27))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_2    (BIT_(26))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_1    (BIT_(25))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_PULSE_WID_0    (BIT_(24))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_OUT_MODE_2    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_OUT_MODE_1    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP1_OUT_MODE_0    (BIT_(3))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_D_CONFIG0_CMP0_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_CMP_D_CONFIG1
// Register Offset : 0x6c
// Description     :
//--------------------------------------------------------------------------
#define CMP_D_CONFIG1_OVF_PULSE_WID_FIELD_OFFSET 16
#define CMP_D_CONFIG1_OVF_PULSE_WID_FIELD_SIZE 8
#define CMP_D_CONFIG1_FRC_LOW_FIELD_OFFSET 5
#define CMP_D_CONFIG1_FRC_LOW_FIELD_SIZE 1
#define CMP_D_CONFIG1_FRC_HIGH_FIELD_OFFSET 4
#define CMP_D_CONFIG1_FRC_HIGH_FIELD_SIZE 1
#define CMP_D_CONFIG1_OVF_OUT_MODE_FIELD_OFFSET 0
#define CMP_D_CONFIG1_OVF_OUT_MODE_FIELD_SIZE 3

#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_7    (BIT_(23))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_6    (BIT_(22))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_5    (BIT_(21))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_4    (BIT_(20))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_3    (BIT_(19))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_2    (BIT_(18))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_1    (BIT_(17))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_PULSE_WID_0    (BIT_(16))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_FRC_LOW    (BIT_(5))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_FRC_HIGH    (BIT_(4))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_OUT_MODE_2    (BIT_(2))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_OUT_MODE_1    (BIT_(1))
#define BIT_AP_APB_PWM_CMP_D_CONFIG1_OVF_OUT_MODE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_DITHER_CTRL
// Register Offset : 0x70
// Description     :
//--------------------------------------------------------------------------
#define DITHER_CTRL_INIT_OFFSET_FIELD_OFFSET 16
#define DITHER_CTRL_INIT_OFFSET_FIELD_SIZE 16
#define DITHER_CTRL_CLIP_RSLT_FIELD_OFFSET 8
#define DITHER_CTRL_CLIP_RSLT_FIELD_SIZE 4
#define DITHER_CTRL_DROP_FIELD_OFFSET 4
#define DITHER_CTRL_DROP_FIELD_SIZE 4
#define DITHER_CTRL_IN_RSLT_FIELD_OFFSET 2
#define DITHER_CTRL_IN_RSLT_FIELD_SIZE 2
#define DITHER_CTRL_INIT_OFFSET_EN_FIELD_OFFSET 1
#define DITHER_CTRL_INIT_OFFSET_EN_FIELD_SIZE 1
#define DITHER_CTRL_DITHER_EN_FIELD_OFFSET 0
#define DITHER_CTRL_DITHER_EN_FIELD_SIZE 1

#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_15    (BIT_(31))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_14    (BIT_(30))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_13    (BIT_(29))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_12    (BIT_(28))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_11    (BIT_(27))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_10    (BIT_(26))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_9    (BIT_(25))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_8    (BIT_(24))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_7    (BIT_(23))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_6    (BIT_(22))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_5    (BIT_(21))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_4    (BIT_(20))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_3    (BIT_(19))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_2    (BIT_(18))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_1    (BIT_(17))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_0    (BIT_(16))
#define BIT_AP_APB_PWM_DITHER_CTRL_CLIP_RSLT_3    (BIT_(11))
#define BIT_AP_APB_PWM_DITHER_CTRL_CLIP_RSLT_2    (BIT_(10))
#define BIT_AP_APB_PWM_DITHER_CTRL_CLIP_RSLT_1    (BIT_(9))
#define BIT_AP_APB_PWM_DITHER_CTRL_CLIP_RSLT_0    (BIT_(8))
#define BIT_AP_APB_PWM_DITHER_CTRL_DROP_3    (BIT_(7))
#define BIT_AP_APB_PWM_DITHER_CTRL_DROP_2    (BIT_(6))
#define BIT_AP_APB_PWM_DITHER_CTRL_DROP_1    (BIT_(5))
#define BIT_AP_APB_PWM_DITHER_CTRL_DROP_0    (BIT_(4))
#define BIT_AP_APB_PWM_DITHER_CTRL_IN_RSLT_1    (BIT_(3))
#define BIT_AP_APB_PWM_DITHER_CTRL_IN_RSLT_0    (BIT_(2))
#define BIT_AP_APB_PWM_DITHER_CTRL_INIT_OFFSET_EN    (BIT_(1))
#define BIT_AP_APB_PWM_DITHER_CTRL_DITHER_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_MFC_CTRL
// Register Offset : 0x74
// Description     :
//--------------------------------------------------------------------------
#define MFC_CTRL_MFC_UP_FIELD_OFFSET 0
#define MFC_CTRL_MFC_UP_FIELD_SIZE 4

#define BIT_AP_APB_PWM_MFC_CTRL_MFC_UP_3    (BIT_(3))
#define BIT_AP_APB_PWM_MFC_CTRL_MFC_UP_2    (BIT_(2))
#define BIT_AP_APB_PWM_MFC_CTRL_MFC_UP_1    (BIT_(1))
#define BIT_AP_APB_PWM_MFC_CTRL_MFC_UP_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_SSE_CTRL
// Register Offset : 0x80
// Description     :
//--------------------------------------------------------------------------
#define SSE_CTRL_SSE_SEL_D_FIELD_OFFSET 22
#define SSE_CTRL_SSE_SEL_D_FIELD_SIZE 6
#define SSE_CTRL_SSE_SEL_C_FIELD_OFFSET 16
#define SSE_CTRL_SSE_SEL_C_FIELD_SIZE 6
#define SSE_CTRL_SSE_SEL_B_FIELD_OFFSET 10
#define SSE_CTRL_SSE_SEL_B_FIELD_SIZE 6
#define SSE_CTRL_SSE_SEL_A_FIELD_OFFSET 4
#define SSE_CTRL_SSE_SEL_A_FIELD_SIZE 6
#define SSE_CTRL_SSE_EN_D_FIELD_OFFSET 3
#define SSE_CTRL_SSE_EN_D_FIELD_SIZE 1
#define SSE_CTRL_SSE_EN_C_FIELD_OFFSET 2
#define SSE_CTRL_SSE_EN_C_FIELD_SIZE 1
#define SSE_CTRL_SSE_EN_B_FIELD_OFFSET 1
#define SSE_CTRL_SSE_EN_B_FIELD_SIZE 1
#define SSE_CTRL_SSE_EN_A_FIELD_OFFSET 0
#define SSE_CTRL_SSE_EN_A_FIELD_SIZE 1

#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_5    (BIT_(27))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_4    (BIT_(26))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_3    (BIT_(25))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_2    (BIT_(24))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_1    (BIT_(23))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_D_0    (BIT_(22))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_5    (BIT_(21))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_4    (BIT_(20))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_3    (BIT_(19))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_2    (BIT_(18))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_1    (BIT_(17))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_C_0    (BIT_(16))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_5    (BIT_(15))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_4    (BIT_(14))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_3    (BIT_(13))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_2    (BIT_(12))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_1    (BIT_(11))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_B_0    (BIT_(10))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_5    (BIT_(9))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_4    (BIT_(8))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_3    (BIT_(7))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_2    (BIT_(6))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_1    (BIT_(5))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_SEL_A_0    (BIT_(4))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_EN_D    (BIT_(3))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_EN_C    (BIT_(2))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_EN_B    (BIT_(1))
#define BIT_AP_APB_PWM_SSE_CTRL_SSE_EN_A    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_SSE_A
// Register Offset : 0x84
// Description     :
//--------------------------------------------------------------------------
#define SSE_A_SSE_FIELD_OFFSET 0
#define SSE_A_SSE_FIELD_SIZE 32

#define BIT_AP_APB_PWM_SSE_A_SSE_31    (BIT_(31))
#define BIT_AP_APB_PWM_SSE_A_SSE_30    (BIT_(30))
#define BIT_AP_APB_PWM_SSE_A_SSE_29    (BIT_(29))
#define BIT_AP_APB_PWM_SSE_A_SSE_28    (BIT_(28))
#define BIT_AP_APB_PWM_SSE_A_SSE_27    (BIT_(27))
#define BIT_AP_APB_PWM_SSE_A_SSE_26    (BIT_(26))
#define BIT_AP_APB_PWM_SSE_A_SSE_25    (BIT_(25))
#define BIT_AP_APB_PWM_SSE_A_SSE_24    (BIT_(24))
#define BIT_AP_APB_PWM_SSE_A_SSE_23    (BIT_(23))
#define BIT_AP_APB_PWM_SSE_A_SSE_22    (BIT_(22))
#define BIT_AP_APB_PWM_SSE_A_SSE_21    (BIT_(21))
#define BIT_AP_APB_PWM_SSE_A_SSE_20    (BIT_(20))
#define BIT_AP_APB_PWM_SSE_A_SSE_19    (BIT_(19))
#define BIT_AP_APB_PWM_SSE_A_SSE_18    (BIT_(18))
#define BIT_AP_APB_PWM_SSE_A_SSE_17    (BIT_(17))
#define BIT_AP_APB_PWM_SSE_A_SSE_16    (BIT_(16))
#define BIT_AP_APB_PWM_SSE_A_SSE_15    (BIT_(15))
#define BIT_AP_APB_PWM_SSE_A_SSE_14    (BIT_(14))
#define BIT_AP_APB_PWM_SSE_A_SSE_13    (BIT_(13))
#define BIT_AP_APB_PWM_SSE_A_SSE_12    (BIT_(12))
#define BIT_AP_APB_PWM_SSE_A_SSE_11    (BIT_(11))
#define BIT_AP_APB_PWM_SSE_A_SSE_10    (BIT_(10))
#define BIT_AP_APB_PWM_SSE_A_SSE_9    (BIT_(9))
#define BIT_AP_APB_PWM_SSE_A_SSE_8    (BIT_(8))
#define BIT_AP_APB_PWM_SSE_A_SSE_7    (BIT_(7))
#define BIT_AP_APB_PWM_SSE_A_SSE_6    (BIT_(6))
#define BIT_AP_APB_PWM_SSE_A_SSE_5    (BIT_(5))
#define BIT_AP_APB_PWM_SSE_A_SSE_4    (BIT_(4))
#define BIT_AP_APB_PWM_SSE_A_SSE_3    (BIT_(3))
#define BIT_AP_APB_PWM_SSE_A_SSE_2    (BIT_(2))
#define BIT_AP_APB_PWM_SSE_A_SSE_1    (BIT_(1))
#define BIT_AP_APB_PWM_SSE_A_SSE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_SSE_B
// Register Offset : 0x88
// Description     :
//--------------------------------------------------------------------------
#define SSE_B_SSE_FIELD_OFFSET 0
#define SSE_B_SSE_FIELD_SIZE 32

#define BIT_AP_APB_PWM_SSE_B_SSE_31    (BIT_(31))
#define BIT_AP_APB_PWM_SSE_B_SSE_30    (BIT_(30))
#define BIT_AP_APB_PWM_SSE_B_SSE_29    (BIT_(29))
#define BIT_AP_APB_PWM_SSE_B_SSE_28    (BIT_(28))
#define BIT_AP_APB_PWM_SSE_B_SSE_27    (BIT_(27))
#define BIT_AP_APB_PWM_SSE_B_SSE_26    (BIT_(26))
#define BIT_AP_APB_PWM_SSE_B_SSE_25    (BIT_(25))
#define BIT_AP_APB_PWM_SSE_B_SSE_24    (BIT_(24))
#define BIT_AP_APB_PWM_SSE_B_SSE_23    (BIT_(23))
#define BIT_AP_APB_PWM_SSE_B_SSE_22    (BIT_(22))
#define BIT_AP_APB_PWM_SSE_B_SSE_21    (BIT_(21))
#define BIT_AP_APB_PWM_SSE_B_SSE_20    (BIT_(20))
#define BIT_AP_APB_PWM_SSE_B_SSE_19    (BIT_(19))
#define BIT_AP_APB_PWM_SSE_B_SSE_18    (BIT_(18))
#define BIT_AP_APB_PWM_SSE_B_SSE_17    (BIT_(17))
#define BIT_AP_APB_PWM_SSE_B_SSE_16    (BIT_(16))
#define BIT_AP_APB_PWM_SSE_B_SSE_15    (BIT_(15))
#define BIT_AP_APB_PWM_SSE_B_SSE_14    (BIT_(14))
#define BIT_AP_APB_PWM_SSE_B_SSE_13    (BIT_(13))
#define BIT_AP_APB_PWM_SSE_B_SSE_12    (BIT_(12))
#define BIT_AP_APB_PWM_SSE_B_SSE_11    (BIT_(11))
#define BIT_AP_APB_PWM_SSE_B_SSE_10    (BIT_(10))
#define BIT_AP_APB_PWM_SSE_B_SSE_9    (BIT_(9))
#define BIT_AP_APB_PWM_SSE_B_SSE_8    (BIT_(8))
#define BIT_AP_APB_PWM_SSE_B_SSE_7    (BIT_(7))
#define BIT_AP_APB_PWM_SSE_B_SSE_6    (BIT_(6))
#define BIT_AP_APB_PWM_SSE_B_SSE_5    (BIT_(5))
#define BIT_AP_APB_PWM_SSE_B_SSE_4    (BIT_(4))
#define BIT_AP_APB_PWM_SSE_B_SSE_3    (BIT_(3))
#define BIT_AP_APB_PWM_SSE_B_SSE_2    (BIT_(2))
#define BIT_AP_APB_PWM_SSE_B_SSE_1    (BIT_(1))
#define BIT_AP_APB_PWM_SSE_B_SSE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_SSE_C
// Register Offset : 0x8c
// Description     :
//--------------------------------------------------------------------------
#define SSE_C_SSE_FIELD_OFFSET 0
#define SSE_C_SSE_FIELD_SIZE 32

#define BIT_AP_APB_PWM_SSE_C_SSE_31    (BIT_(31))
#define BIT_AP_APB_PWM_SSE_C_SSE_30    (BIT_(30))
#define BIT_AP_APB_PWM_SSE_C_SSE_29    (BIT_(29))
#define BIT_AP_APB_PWM_SSE_C_SSE_28    (BIT_(28))
#define BIT_AP_APB_PWM_SSE_C_SSE_27    (BIT_(27))
#define BIT_AP_APB_PWM_SSE_C_SSE_26    (BIT_(26))
#define BIT_AP_APB_PWM_SSE_C_SSE_25    (BIT_(25))
#define BIT_AP_APB_PWM_SSE_C_SSE_24    (BIT_(24))
#define BIT_AP_APB_PWM_SSE_C_SSE_23    (BIT_(23))
#define BIT_AP_APB_PWM_SSE_C_SSE_22    (BIT_(22))
#define BIT_AP_APB_PWM_SSE_C_SSE_21    (BIT_(21))
#define BIT_AP_APB_PWM_SSE_C_SSE_20    (BIT_(20))
#define BIT_AP_APB_PWM_SSE_C_SSE_19    (BIT_(19))
#define BIT_AP_APB_PWM_SSE_C_SSE_18    (BIT_(18))
#define BIT_AP_APB_PWM_SSE_C_SSE_17    (BIT_(17))
#define BIT_AP_APB_PWM_SSE_C_SSE_16    (BIT_(16))
#define BIT_AP_APB_PWM_SSE_C_SSE_15    (BIT_(15))
#define BIT_AP_APB_PWM_SSE_C_SSE_14    (BIT_(14))
#define BIT_AP_APB_PWM_SSE_C_SSE_13    (BIT_(13))
#define BIT_AP_APB_PWM_SSE_C_SSE_12    (BIT_(12))
#define BIT_AP_APB_PWM_SSE_C_SSE_11    (BIT_(11))
#define BIT_AP_APB_PWM_SSE_C_SSE_10    (BIT_(10))
#define BIT_AP_APB_PWM_SSE_C_SSE_9    (BIT_(9))
#define BIT_AP_APB_PWM_SSE_C_SSE_8    (BIT_(8))
#define BIT_AP_APB_PWM_SSE_C_SSE_7    (BIT_(7))
#define BIT_AP_APB_PWM_SSE_C_SSE_6    (BIT_(6))
#define BIT_AP_APB_PWM_SSE_C_SSE_5    (BIT_(5))
#define BIT_AP_APB_PWM_SSE_C_SSE_4    (BIT_(4))
#define BIT_AP_APB_PWM_SSE_C_SSE_3    (BIT_(3))
#define BIT_AP_APB_PWM_SSE_C_SSE_2    (BIT_(2))
#define BIT_AP_APB_PWM_SSE_C_SSE_1    (BIT_(1))
#define BIT_AP_APB_PWM_SSE_C_SSE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PWM_SSE_D
// Register Offset : 0x90
// Description     :
//--------------------------------------------------------------------------
#define SSE_D_SSE_FIELD_OFFSET 0
#define SSE_D_SSE_FIELD_SIZE 32

#define BIT_AP_APB_PWM_SSE_D_SSE_31    (BIT_(31))
#define BIT_AP_APB_PWM_SSE_D_SSE_30    (BIT_(30))
#define BIT_AP_APB_PWM_SSE_D_SSE_29    (BIT_(29))
#define BIT_AP_APB_PWM_SSE_D_SSE_28    (BIT_(28))
#define BIT_AP_APB_PWM_SSE_D_SSE_27    (BIT_(27))
#define BIT_AP_APB_PWM_SSE_D_SSE_26    (BIT_(26))
#define BIT_AP_APB_PWM_SSE_D_SSE_25    (BIT_(25))
#define BIT_AP_APB_PWM_SSE_D_SSE_24    (BIT_(24))
#define BIT_AP_APB_PWM_SSE_D_SSE_23    (BIT_(23))
#define BIT_AP_APB_PWM_SSE_D_SSE_22    (BIT_(22))
#define BIT_AP_APB_PWM_SSE_D_SSE_21    (BIT_(21))
#define BIT_AP_APB_PWM_SSE_D_SSE_20    (BIT_(20))
#define BIT_AP_APB_PWM_SSE_D_SSE_19    (BIT_(19))
#define BIT_AP_APB_PWM_SSE_D_SSE_18    (BIT_(18))
#define BIT_AP_APB_PWM_SSE_D_SSE_17    (BIT_(17))
#define BIT_AP_APB_PWM_SSE_D_SSE_16    (BIT_(16))
#define BIT_AP_APB_PWM_SSE_D_SSE_15    (BIT_(15))
#define BIT_AP_APB_PWM_SSE_D_SSE_14    (BIT_(14))
#define BIT_AP_APB_PWM_SSE_D_SSE_13    (BIT_(13))
#define BIT_AP_APB_PWM_SSE_D_SSE_12    (BIT_(12))
#define BIT_AP_APB_PWM_SSE_D_SSE_11    (BIT_(11))
#define BIT_AP_APB_PWM_SSE_D_SSE_10    (BIT_(10))
#define BIT_AP_APB_PWM_SSE_D_SSE_9    (BIT_(9))
#define BIT_AP_APB_PWM_SSE_D_SSE_8    (BIT_(8))
#define BIT_AP_APB_PWM_SSE_D_SSE_7    (BIT_(7))
#define BIT_AP_APB_PWM_SSE_D_SSE_6    (BIT_(6))
#define BIT_AP_APB_PWM_SSE_D_SSE_5    (BIT_(5))
#define BIT_AP_APB_PWM_SSE_D_SSE_4    (BIT_(4))
#define BIT_AP_APB_PWM_SSE_D_SSE_3    (BIT_(3))
#define BIT_AP_APB_PWM_SSE_D_SSE_2    (BIT_(2))
#define BIT_AP_APB_PWM_SSE_D_SSE_1    (BIT_(1))
#define BIT_AP_APB_PWM_SSE_D_SSE_0    (BIT_(0))

typedef union {
    struct {
        uint32_t src_clk_sel  : 2;
        uint32_t frc_rld      : 1;
        uint32_t ext_clr_en   : 1;
        uint32_t int_clr      : 1;
        uint32_t reserve0     : 11;
        uint32_t div_num      : 16;
    };
    uint32_t val;
} sdrv_pwm_cnt_g0_config_t;

typedef union {
    struct {
        uint32_t data_format    : 2;
        uint32_t grp_num        : 2;
        uint32_t dual_cmp_mode  : 1;
        uint32_t dma_en         : 1;
        uint32_t reserve0       : 2;
        uint32_t fifo_wml       : 6;
        uint32_t reserve1       : 2;
        uint32_t rpt_num        : 8;
    };
    uint32_t val;
} sdrv_pwm_cmp_config_t;

typedef union {
    struct {
        uint32_t cmp_en         : 1;
        uint32_t single_mode    : 1;
        uint32_t reserve0       : 29;
        uint32_t sw_rst         : 1;
    };
    uint32_t val;
} sdrv_pwm_cmp_ctrl_t;

typedef union {
    struct {
        uint32_t cmp0_out_mode  : 3;
        uint32_t cmp1_out_mode  : 3;
        uint32_t reserve0       : 10;
        uint32_t cmp0_pulse_wid : 8;
        uint32_t cmp1_pulse_wid : 8;
    };
    uint32_t val;
} sdrv_pwm_cmp_ch_config0_t;

typedef union {
    struct {
        uint32_t ovf_out_mode  : 3;
        uint32_t reserve0      : 1;
        uint32_t frc_high      : 1;
        uint32_t frc_low       : 1;
        uint32_t reserve1      : 10;
        uint32_t ovf_pulse_wid : 16;
    };
    uint32_t val;
} sdrv_pwm_cmp_ch_config1_t;

typedef union {
    struct {
        uint32_t dither_en      : 1;
        uint32_t init_offset_en : 1;
        uint32_t in_rslt        : 2;
        uint32_t drop           : 4;
        uint32_t clip_rslt      : 4;
        uint32_t reserve0       : 4;
        uint32_t init_offset    : 16;
    };
    uint32_t val;
} sdrv_pwm_dither_ctrl_t;

typedef union {
    struct {
        uint32_t sse_en_a   : 1;
        uint32_t sse_en_b   : 1;
        uint32_t sse_en_c   : 1;
        uint32_t sse_en_d   : 1;
        uint32_t sse_sel_a  : 6;
        uint32_t sse_sel_b  : 6;
        uint32_t sse_sel_c  : 6;
        uint32_t sse_sel_d  : 6;
        uint32_t reserve0   : 4;
    };
    uint32_t val;
} sdrv_pwm_sse_ctrl_t;

typedef union {
    struct {
        uint32_t full      : 1;
        uint32_t empty     : 1;
        uint32_t entries   : 7;
        uint32_t reserve0  : 23;
    };
    uint32_t val;
} sdrv_pwm_fifo_stat_t;

typedef struct {
    volatile u32 int_sta;       /* offset: 0x0 */
    volatile u32 int_sta_en;    /* offset: 0x4 */
    volatile u32 int_sig_en;    /* offset: 0x8 */
    volatile sdrv_pwm_cnt_g0_config_t cnt_g0_config; /* offset: 0xc */
    volatile u32 clk_config;    /* offset: 0x10 */
    volatile u32 cnt_g0_ovf;    /* offset: 0x14 */
    volatile u32 cnt_g0;        /* offset: 0x18 */
    volatile u32 cmp_val_upt;   /* offset: 0x1c */
    volatile u32 cmp0_a_val;    /* offset: 0x20 */
    volatile u32 cmp1_a_val;    /* offset: 0x24 */
    volatile u32 cmp0_b_val;    /* offset: 0x28 */
    volatile u32 cmp1_b_val;    /* offset: 0x2c */
    volatile u32 cmp0_c_val;    /* offset: 0x30 */
    volatile u32 cmp1_c_val;    /* offset: 0x34 */
    volatile u32 cmp0_d_val;    /* offset: 0x38 */
    volatile u32 cmp1_d_val;    /* offset: 0x3c */
    volatile sdrv_pwm_cmp_config_t cmp_config;    /* offset: 0x40 */
    volatile u32 fifo_entry;    /* offset: 0x44 */
    volatile sdrv_pwm_fifo_stat_t fifo_stat;     /* offset: 0x48 */
    volatile sdrv_pwm_cmp_ctrl_t cmp_ctrl;      /* offset: 0x4c */
    volatile sdrv_pwm_cmp_ch_config0_t cmp_a_config0; /* offset: 0x50 */
    volatile sdrv_pwm_cmp_ch_config1_t cmp_a_config1; /* offset: 0x54 */
    volatile sdrv_pwm_cmp_ch_config0_t cmp_b_config0; /* offset: 0x58 */
    volatile sdrv_pwm_cmp_ch_config1_t cmp_b_config1; /* offset: 0x5c */
    volatile sdrv_pwm_cmp_ch_config0_t cmp_c_config0; /* offset: 0x60 */
    volatile sdrv_pwm_cmp_ch_config1_t cmp_c_config1; /* offset: 0x64 */
    volatile sdrv_pwm_cmp_ch_config0_t cmp_d_config0; /* offset: 0x68 */
    volatile sdrv_pwm_cmp_ch_config1_t cmp_d_config1; /* offset: 0x6c */
    volatile sdrv_pwm_dither_ctrl_t dither_ctrl;   /* offset: 0x70 */
    volatile u32 mfc_ctrl;      /* offset: 0x74 */
    volatile u8 resvd1[8];      /* offset: 0x78 */
    volatile sdrv_pwm_sse_ctrl_t sse_ctrl;      /* offset: 0x80 */
    volatile u32 sse_a;         /* offset: 0x84 */
    volatile u32 sse_b;         /* offset: 0x88 */
    volatile u32 sse_c;         /* offset: 0x8c */
    volatile u32 sse_d;         /* offset: 0x90 */
} sdrv_pwm_t;

#endif
