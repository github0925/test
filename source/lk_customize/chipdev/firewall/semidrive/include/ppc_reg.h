/*
 * Copyright (c) Semidrive
 */

#ifndef _PPC_REG_H
#define _PPC_REG_H

#include "__regs_ap_ppc.h"

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_DOM
// Register Offset : 0x0 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_CTRL_JUMP                       0x38
#define PPC_DOM_(i)                         (REG_AP_APB_PPC_DOM + (i) * PPC_CTRL_JUMP)
#define PPC_DOM_LOCK_SHIFT                  DOM_LOCK_FIELD_OFFSET
#define PPC_DOM_LOCK_MASK                   1UL << PPC_DOM_LOCK_SHIFT
#define PPC_DOM_SET_SHIFT                   DOM_SET_FIELD_OFFSET
#define PPC_DOM_SET_MASK                    1UL << PPC_DOM_SET_SHIFT
#define PPC_DOM_DID_SHIFT                   DOM_DID_FIELD_OFFSET
#define PPC_DOM_DID_MASK                    0xF << PPC_DOM_DID_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_DOM_PER0
// Register Offset : 0x4 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_DOM_PER0_(i)                    (REG_AP_APB_PPC_DOM_PER0 + (i) * PPC_CTRL_JUMP)
#define PPC_DOM_PER0_DOM7_LOCK_SHIFT        DOM_PER0_DOM7_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM7_LOCK_MASK         1UL << PPC_DOM_PER0_DOM7_LOCK_SHIFT
#define PPC_DOM_PER0_DOM7_PER_SHIFT         DOM_PER0_DOM7_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM7_PER_MASK          3UL << PPC_DOM_PER0_DOM7_PER_SHIFT
#define PPC_DOM_PER0_DOM7_EN_SHIFT          DOM_PER0_DOM7_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM7_EN_MASK           1UL << PPC_DOM_PER0_DOM7_EN_SHIFT
#define PPC_DOM_PER0_DOM6_LOCK_SHIFT        DOM_PER0_DOM6_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM6_LOCK_MASK         1UL << PPC_DOM_PER0_DOM6_LOCK_SHIFT
#define PPC_DOM_PER0_DOM6_PER_SHIFT         DOM_PER0_DOM6_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM6_PER_MASK          3UL << PPC_DOM_PER0_DOM6_PER_SHIFT
#define PPC_DOM_PER0_DOM6_EN_SHIFT          DOM_PER0_DOM6_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM6_EN_MASK           1UL << PPC_DOM_PER0_DOM6_EN_SHIFT
#define PPC_DOM_PER0_DOM5_LOCK_SHIFT        DOM_PER0_DOM5_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM5_LOCK_MASK         1UL << PPC_DOM_PER0_DOM5_LOCK_SHIFT
#define PPC_DOM_PER0_DOM5_PER_SHIFT         DOM_PER0_DOM5_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM5_PER_MASK          3UL << PPC_DOM_PER0_DOM5_PER_SHIFT
#define PPC_DOM_PER0_DOM5_EN_SHIFT          DOM_PER0_DOM5_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM5_EN_MASK           1UL << PPC_DOM_PER0_DOM5_EN_SHIFT
#define PPC_DOM_PER0_DOM4_LOCK_SHIFT        DOM_PER0_DOM4_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM4_LOCK_MASK         1UL << PPC_DOM_PER0_DOM4_LOCK_SHIFT
#define PPC_DOM_PER0_DOM4_PER_SHIFT         DOM_PER0_DOM4_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM4_PER_MASK          3UL << PPC_DOM_PER0_DOM4_PER_SHIFT
#define PPC_DOM_PER0_DOM4_EN_SHIFT          DOM_PER0_DOM4_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM4_EN_MASK           1UL << PPC_DOM_PER0_DOM4_EN_SHIFT
#define PPC_DOM_PER0_DOM3_LOCK_SHIFT        DOM_PER0_DOM3_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM3_LOCK_MASK         1UL << PPC_DOM_PER0_DOM3_LOCK_SHIFT
#define PPC_DOM_PER0_DOM3_PER_SHIFT         DOM_PER0_DOM3_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM3_PER_MASK          3UL << PPC_DOM_PER0_DOM3_PER_SHIFT
#define PPC_DOM_PER0_DOM3_EN_SHIFT          DOM_PER0_DOM3_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM3_EN_MASK           1UL << PPC_DOM_PER0_DOM3_EN_SHIFT
#define PPC_DOM_PER0_DOM2_LOCK_SHIFT        DOM_PER0_DOM2_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM2_LOCK_MASK         1UL << PPC_DOM_PER0_DOM2_LOCK_SHIFT
#define PPC_DOM_PER0_DOM2_PER_SHIFT         DOM_PER0_DOM2_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM2_PER_MASK          3UL << PPC_DOM_PER0_DOM2_PER_SHIFT
#define PPC_DOM_PER0_DOM2_EN_SHIFT          DOM_PER0_DOM2_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM2_EN_MASK           1UL << PPC_DOM_PER0_DOM2_EN_SHIFT
#define PPC_DOM_PER0_DOM1_LOCK_SHIFT        DOM_PER0_DOM1_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM1_LOCK_MASK         1UL << PPC_DOM_PER0_DOM1_LOCK_SHIFT
#define PPC_DOM_PER0_DOM1_PER_SHIFT         DOM_PER0_DOM1_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM1_PER_MASK          3UL << PPC_DOM_PER0_DOM1_PER_SHIFT
#define PPC_DOM_PER0_DOM1_EN_SHIFT          DOM_PER0_DOM1_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM1_EN_MASK           1UL << PPC_DOM_PER0_DOM1_EN_SHIFT
#define PPC_DOM_PER0_DOM0_LOCK_SHIFT        DOM_PER0_DOM0_LOCK_FIELD_OFFSET
#define PPC_DOM_PER0_DOM0_LOCK_MASK         1UL << PPC_DOM_PER0_DOM0_LOCK_SHIFT
#define PPC_DOM_PER0_DOM0_PER_SHIFT         DOM_PER0_DOM0_PER_FIELD_OFFSET
#define PPC_DOM_PER0_DOM0_PER_MASK          3UL << PPC_DOM_PER0_DOM0_PER_SHIFT
#define PPC_DOM_PER0_DOM0_EN_SHIFT          DOM_PER0_DOM0_EN_FIELD_OFFSET
#define PPC_DOM_PER0_DOM0_EN_MASK           1UL << PPC_DOM_PER0_DOM0_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_DOM_PER1
// Register Offset : 0x8 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_DOM_PER1_(i)                    (REG_AP_APB_PPC_DOM_PER1 + (i) * PPC_CTRL_JUMP)
#define PPC_DOM_PER1_DOM15_LOCK_SHIFT       DOM_PER1_DOM15_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM15_LOCK_MASK        1UL << PPC_DOM_PER1_DOM15_LOCK_SHIFT
#define PPC_DOM_PER1_DOM15_PER_SHIFT        DOM_PER1_DOM15_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM15_PER_MASK         3UL << PPC_DOM_PER1_DOM15_PER_SHIFT
#define PPC_DOM_PER1_DOM15_EN_SHIFT         DOM_PER1_DOM15_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM15_EN_MASK          1UL << PPC_DOM_PER1_DOM15_EN_SHIFT
#define PPC_DOM_PER1_DOM14_LOCK_SHIFT       DOM_PER1_DOM14_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM14_LOCK_MASK        1UL << PPC_DOM_PER1_DOM14_LOCK_SHIFT
#define PPC_DOM_PER1_DOM14_PER_SHIFT        DOM_PER1_DOM14_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM14_PER_MASK         3UL << PPC_DOM_PER1_DOM14_PER_SHIFT
#define PPC_DOM_PER1_DOM14_EN_SHIFT         DOM_PER1_DOM14_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM14_EN_MASK          1UL << PPC_DOM_PER1_DOM14_EN_SHIFT
#define PPC_DOM_PER1_DOM13_LOCK_SHIFT       DOM_PER1_DOM13_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM13_LOCK_MASK        1UL << PPC_DOM_PER1_DOM13_LOCK_SHIFT
#define PPC_DOM_PER1_DOM13_PER_SHIFT        DOM_PER1_DOM13_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM13_PER_MASK         3UL << PPC_DOM_PER1_DOM13_PER_SHIFT
#define PPC_DOM_PER1_DOM13_EN_SHIFT         DOM_PER1_DOM13_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM13_EN_MASK          1UL << PPC_DOM_PER1_DOM13_EN_SHIFT
#define PPC_DOM_PER1_DOM12_LOCK_SHIFT       DOM_PER1_DOM12_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM12_LOCK_MASK        1UL << PPC_DOM_PER1_DOM12_LOCK_SHIFT
#define PPC_DOM_PER1_DOM12_PER_SHIFT        DOM_PER1_DOM12_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM12_PER_MASK         3UL << PPC_DOM_PER1_DOM12_PER_SHIFT
#define PPC_DOM_PER1_DOM12_EN_SHIFT         DOM_PER1_DOM12_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM12_EN_MASK          1UL << PPC_DOM_PER1_DOM12_EN_SHIFT
#define PPC_DOM_PER1_DOM11_LOCK_SHIFT       DOM_PER1_DOM11_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM11_LOCK_MASK        1UL << PPC_DOM_PER1_DOM11_LOCK_SHIFT
#define PPC_DOM_PER1_DOM11_PER_SHIFT        DOM_PER1_DOM11_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM11_PER_MASK         3UL << PPC_DOM_PER1_DOM11_PER_SHIFT
#define PPC_DOM_PER1_DOM11_EN_SHIFT         DOM_PER1_DOM11_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM11_EN_MASK          1UL << PPC_DOM_PER1_DOM11_EN_SHIFT
#define PPC_DOM_PER1_DOM10_LOCK_SHIFT       DOM_PER1_DOM10_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM10_LOCK_MASK        1UL << PPC_DOM_PER1_DOM10_LOCK_SHIFT
#define PPC_DOM_PER1_DOM10_PER_SHIFT        DOM_PER1_DOM10_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM10_PER_MASK         3UL << PPC_DOM_PER1_DOM10_PER_SHIFT
#define PPC_DOM_PER1_DOM10_EN_SHIFT         DOM_PER1_DOM10_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM10_EN_MASK          1UL << PPC_DOM_PER1_DOM10_EN_SHIFT
#define PPC_DOM_PER1_DOM9_LOCK_SHIFT        DOM_PER1_DOM9_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM9_LOCK_MASK         1UL << PPC_DOM_PER1_DOM9_LOCK_SHIFT
#define PPC_DOM_PER1_DOM9_PER_SHIFT         DOM_PER1_DOM9_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM9_PER_MASK          3UL << PPC_DOM_PER1_DOM9_PER_SHIFT
#define PPC_DOM_PER1_DOM9_EN_SHIFT          DOM_PER1_DOM9_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM9_EN_MASK           1UL << PPC_DOM_PER1_DOM9_EN_SHIFT
#define PPC_DOM_PER1_DOM8_LOCK_SHIFT        DOM_PER1_DOM8_LOCK_FIELD_OFFSET
#define PPC_DOM_PER1_DOM8_LOCK_MASK         1UL << PPC_DOM_PER1_DOM8_LOCK_SHIFT
#define PPC_DOM_PER1_DOM8_PER_SHIFT         DOM_PER1_DOM8_PER_FIELD_OFFSET
#define PPC_DOM_PER1_DOM8_PER_MASK          3UL << PPC_DOM_PER1_DOM8_PER_SHIFT
#define PPC_DOM_PER1_DOM8_EN_SHIFT          DOM_PER1_DOM8_EN_FIELD_OFFSET
#define PPC_DOM_PER1_DOM8_EN_MASK           1UL << PPC_DOM_PER1_DOM8_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_SEC_PER0
// Register Offset : 0xc + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_SEC_PER0_(i)                    (REG_AP_APB_PPC_SEC_PER0 + (i) * PPC_CTRL_JUMP)
#define PPC_SEC_PER0_DOM3_LOCK_SHIFT        SEC_PER0_DOM3_LOCK_FIELD_OFFSET
#define PPC_SEC_PER0_DOM3_LOCK_MASK         1UL << PPC_SEC_PER0_DOM3_LOCK_SHIFT
#define PPC_SEC_PER0_DOM3_NSE_PER_SHIFT     SEC_PER0_DOM3_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM3_NSE_PER_MASK      3UL << PPC_SEC_PER0_DOM3_NSE_PER_SHIFT
#define PPC_SEC_PER0_DOM3_SEC_PER_SHIFT     SEC_PER0_DOM3_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM3_SEC_PER_MASK      3UL << PPC_SEC_PER0_DOM3_SEC_PER_SHIFT
#define PPC_SEC_PER0_DOM3_SEC_EN_SHIFT      SEC_PER0_DOM3_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER0_DOM3_SEC_EN_MASK       1UL << PPC_SEC_PER0_DOM3_SEC_EN_SHIFT
#define PPC_SEC_PER0_DOM2_LOCK_SHIFT        SEC_PER0_DOM2_LOCK_FIELD_OFFSET
#define PPC_SEC_PER0_DOM2_LOCK_MASK         1UL << PPC_SEC_PER0_DOM2_LOCK_SHIFT
#define PPC_SEC_PER0_DOM2_NSE_PER_SHIFT     SEC_PER0_DOM2_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM2_NSE_PER_MASK      3UL << PPC_SEC_PER0_DOM2_NSE_PER_SHIFT
#define PPC_SEC_PER0_DOM2_SEC_PER_SHIFT     SEC_PER0_DOM2_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM2_SEC_PER_MASK      3UL << PPC_SEC_PER0_DOM2_SEC_PER_SHIFT
#define PPC_SEC_PER0_DOM2_SEC_EN_SHIFT      SEC_PER0_DOM2_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER0_DOM2_SEC_EN_MASK       1UL << PPC_SEC_PER0_DOM2_SEC_EN_SHIFT
#define PPC_SEC_PER0_DOM1_LOCK_SHIFT        SEC_PER0_DOM1_LOCK_FIELD_OFFSET
#define PPC_SEC_PER0_DOM1_LOCK_MASK         1UL << PPC_SEC_PER0_DOM1_LOCK_SHIFT
#define PPC_SEC_PER0_DOM1_NSE_PER_SHIFT     SEC_PER0_DOM1_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM1_NSE_PER_MASK      3UL << PPC_SEC_PER0_DOM1_NSE_PER_SHIFT
#define PPC_SEC_PER0_DOM1_SEC_PER_SHIFT     SEC_PER0_DOM1_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM1_SEC_PER_MASK      3UL << PPC_SEC_PER0_DOM1_SEC_PER_SHIFT
#define PPC_SEC_PER0_DOM1_SEC_EN_SHIFT      SEC_PER0_DOM1_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER0_DOM1_SEC_EN_MASK       1UL << PPC_SEC_PER0_DOM1_SEC_EN_SHIFT
#define PPC_SEC_PER0_DOM0_LOCK_SHIFT        SEC_PER0_DOM0_LOCK_FIELD_OFFSET
#define PPC_SEC_PER0_DOM0_LOCK_MASK         1UL << PPC_SEC_PER0_DOM0_LOCK_SHIFT
#define PPC_SEC_PER0_DOM0_NSE_PER_SHIFT     SEC_PER0_DOM0_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM0_NSE_PER_MASK      3UL << PPC_SEC_PER0_DOM0_NSE_PER_SHIFT
#define PPC_SEC_PER0_DOM0_SEC_PER_SHIFT     SEC_PER0_DOM0_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER0_DOM0_SEC_PER_MASK      3UL << PPC_SEC_PER0_DOM0_SEC_PER_SHIFT
#define PPC_SEC_PER0_DOM0_SEC_EN_SHIFT      SEC_PER0_DOM0_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER0_DOM0_SEC_EN_MASK       1UL << PPC_SEC_PER0_DOM0_SEC_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_SEC_PER1
// Register Offset : 0x10 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_SEC_PER1_(i)                    (REG_AP_APB_PPC_SEC_PER1 + (i) * PPC_CTRL_JUMP)
#define PPC_SEC_PER1_DOM7_LOCK_SHIFT        SEC_PER1_DOM7_LOCK_FIELD_OFFSET
#define PPC_SEC_PER1_DOM7_LOCK_MASK         1UL << PPC_SEC_PER1_DOM7_LOCK_SHIFT
#define PPC_SEC_PER1_DOM7_NSE_PER_SHIFT     SEC_PER1_DOM7_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM7_NSE_PER_MASK      3UL << PPC_SEC_PER1_DOM7_NSE_PER_SHIFT
#define PPC_SEC_PER1_DOM7_SEC_PER_SHIFT     SEC_PER1_DOM7_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM7_SEC_PER_MASK      3UL << PPC_SEC_PER1_DOM7_SEC_PER_SHIFT
#define PPC_SEC_PER1_DOM7_SEC_EN_SHIFT      SEC_PER1_DOM7_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER1_DOM7_SEC_EN_MASK       1UL << PPC_SEC_PER1_DOM7_SEC_EN_SHIFT
#define PPC_SEC_PER1_DOM6_LOCK_SHIFT        SEC_PER1_DOM6_LOCK_FIELD_OFFSET
#define PPC_SEC_PER1_DOM6_LOCK_MASK         1UL << PPC_SEC_PER1_DOM6_LOCK_SHIFT
#define PPC_SEC_PER1_DOM6_NSE_PER_SHIFT     SEC_PER1_DOM6_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM6_NSE_PER_MASK      3UL << PPC_SEC_PER1_DOM6_NSE_PER_SHIFT
#define PPC_SEC_PER1_DOM6_SEC_PER_SHIFT     SEC_PER1_DOM6_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM6_SEC_PER_MASK      3UL << PPC_SEC_PER1_DOM6_SEC_PER_SHIFT
#define PPC_SEC_PER1_DOM6_SEC_EN_SHIFT      SEC_PER1_DOM6_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER1_DOM6_SEC_EN_MASK       1UL << PPC_SEC_PER1_DOM6_SEC_EN_SHIFT
#define PPC_SEC_PER1_DOM5_LOCK_SHIFT        SEC_PER1_DOM5_LOCK_FIELD_OFFSET
#define PPC_SEC_PER1_DOM5_LOCK_MASK         1UL << PPC_SEC_PER1_DOM5_LOCK_SHIFT
#define PPC_SEC_PER1_DOM5_NSE_PER_SHIFT     SEC_PER1_DOM5_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM5_NSE_PER_MASK      3UL << PPC_SEC_PER1_DOM5_NSE_PER_SHIFT
#define PPC_SEC_PER1_DOM5_SEC_PER_SHIFT     SEC_PER1_DOM5_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM5_SEC_PER_MASK      3UL << PPC_SEC_PER1_DOM5_SEC_PER_SHIFT
#define PPC_SEC_PER1_DOM5_SEC_EN_SHIFT      SEC_PER1_DOM5_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER1_DOM5_SEC_EN_MASK       1UL << PPC_SEC_PER1_DOM5_SEC_EN_SHIFT
#define PPC_SEC_PER1_DOM4_LOCK_SHIFT        SEC_PER1_DOM4_LOCK_FIELD_OFFSET
#define PPC_SEC_PER1_DOM4_LOCK_MASK         1UL << PPC_SEC_PER1_DOM4_LOCK_SHIFT
#define PPC_SEC_PER1_DOM4_NSE_PER_SHIFT     SEC_PER1_DOM4_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM4_NSE_PER_MASK      3UL << PPC_SEC_PER1_DOM4_NSE_PER_SHIFT
#define PPC_SEC_PER1_DOM4_SEC_PER_SHIFT     SEC_PER1_DOM4_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER1_DOM4_SEC_PER_MASK      3UL << PPC_SEC_PER1_DOM4_SEC_PER_SHIFT
#define PPC_SEC_PER1_DOM4_SEC_EN_SHIFT      SEC_PER1_DOM4_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER1_DOM4_SEC_EN_MASK       1UL << PPC_SEC_PER1_DOM4_SEC_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_SEC_PER2
// Register Offset : 0x14 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_SEC_PER2_(i)                    (REG_AP_APB_PPC_SEC_PER2 + (i) * PPC_CTRL_JUMP)
#define PPC_SEC_PER2_DOM11_LOCK_SHIFT       SEC_PER2_DOM11_LOCK_FIELD_OFFSET
#define PPC_SEC_PER2_DOM11_LOCK_MASK        1UL << PPC_SEC_PER2_DOM11_LOCK_SHIFT
#define PPC_SEC_PER2_DOM11_NSE_PER_SHIFT    SEC_PER2_DOM11_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM11_NSE_PER_MASK     3UL << PPC_SEC_PER2_DOM11_NSE_PER_SHIFT
#define PPC_SEC_PER2_DOM11_SEC_PER_SHIFT    SEC_PER2_DOM11_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM11_SEC_PER_MASK     3UL << PPC_SEC_PER2_DOM11_SEC_PER_SHIFT
#define PPC_SEC_PER2_DOM11_SEC_EN_SHIFT     SEC_PER2_DOM11_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER2_DOM11_SEC_EN_MASK      1UL << PPC_SEC_PER2_DOM11_SEC_EN_SHIFT
#define PPC_SEC_PER2_DOM10_LOCK_SHIFT       SEC_PER2_DOM10_LOCK_FIELD_OFFSET
#define PPC_SEC_PER2_DOM10_LOCK_MASK        1UL << PPC_SEC_PER2_DOM10_LOCK_SHIFT
#define PPC_SEC_PER2_DOM10_NSE_PER_SHIFT    SEC_PER2_DOM10_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM10_NSE_PER_MASK     3UL << PPC_SEC_PER2_DOM10_NSE_PER_SHIFT
#define PPC_SEC_PER2_DOM10_SEC_PER_SHIFT    SEC_PER2_DOM10_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM10_SEC_PER_MASK     3UL << PPC_SEC_PER2_DOM10_SEC_PER_SHIFT
#define PPC_SEC_PER2_DOM10_SEC_EN_SHIFT     SEC_PER2_DOM10_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER2_DOM10_SEC_EN_MASK      1UL << PPC_SEC_PER2_DOM10_SEC_EN_SHIFT
#define PPC_SEC_PER2_DOM9_LOCK_SHIFT        SEC_PER2_DOM9_LOCK_FIELD_OFFSET
#define PPC_SEC_PER2_DOM9_LOCK_MASK         1UL << PPC_SEC_PER2_DOM9_LOCK_SHIFT
#define PPC_SEC_PER2_DOM9_NSE_PER_SHIFT     SEC_PER2_DOM9_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM9_NSE_PER_MASK      3UL << PPC_SEC_PER2_DOM9_NSE_PER_SHIFT
#define PPC_SEC_PER2_DOM9_SEC_PER_SHIFT     SEC_PER2_DOM9_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM9_SEC_PER_MASK      3UL << PPC_SEC_PER2_DOM9_SEC_PER_SHIFT
#define PPC_SEC_PER2_DOM9_SEC_EN_SHIFT      SEC_PER2_DOM9_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER2_DOM9_SEC_EN_MASK       1UL << PPC_SEC_PER2_DOM9_SEC_EN_SHIFT
#define PPC_SEC_PER2_DOM8_LOCK_SHIFT        SEC_PER2_DOM8_LOCK_FIELD_OFFSET
#define PPC_SEC_PER2_DOM8_LOCK_MASK         1UL << PPC_SEC_PER2_DOM8_LOCK_SHIFT
#define PPC_SEC_PER2_DOM8_NSE_PER_SHIFT     SEC_PER2_DOM8_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM8_NSE_PER_MASK      3UL << PPC_SEC_PER2_DOM8_NSE_PER_SHIFT
#define PPC_SEC_PER2_DOM8_SEC_PER_SHIFT     SEC_PER2_DOM8_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER2_DOM8_SEC_PER_MASK      3UL << PPC_SEC_PER2_DOM8_SEC_PER_SHIFT
#define PPC_SEC_PER2_DOM8_SEC_EN_SHIFT      SEC_PER2_DOM8_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER2_DOM8_SEC_EN_MASK       1UL << PPC_SEC_PER2_DOM8_SEC_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_SEC_PER3
// Register Offset : 0x18 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_SEC_PER3_(i)                    (REG_AP_APB_PPC_SEC_PER3 + (i) * PPC_CTRL_JUMP)
#define PPC_SEC_PER3_DOM15_LOCK_SHIFT       SEC_PER3_DOM15_LOCK_FIELD_OFFSET
#define PPC_SEC_PER3_DOM15_LOCK_MASK        1UL << PPC_SEC_PER3_DOM15_LOCK_SHIFT
#define PPC_SEC_PER3_DOM15_NSE_PER_SHIFT    SEC_PER3_DOM15_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM15_NSE_PER_MASK     3UL << PPC_SEC_PER3_DOM15_NSE_PER_SHIFT
#define PPC_SEC_PER3_DOM15_SEC_PER_SHIFT    SEC_PER3_DOM15_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM15_SEC_PER_MASK     3UL << PPC_SEC_PER3_DOM15_SEC_PER_SHIFT
#define PPC_SEC_PER3_DOM15_SEC_EN_SHIFT     SEC_PER3_DOM15_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER3_DOM15_SEC_EN_MASK      1UL << PPC_SEC_PER3_DOM15_SEC_EN_SHIFT
#define PPC_SEC_PER3_DOM14_LOCK_SHIFT       SEC_PER3_DOM14_LOCK_FIELD_OFFSET
#define PPC_SEC_PER3_DOM14_LOCK_MASK        1UL << PPC_SEC_PER3_DOM14_LOCK_SHIFT
#define PPC_SEC_PER3_DOM14_NSE_PER_SHIFT    SEC_PER3_DOM14_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM14_NSE_PER_MASK     3UL << PPC_SEC_PER3_DOM14_NSE_PER_SHIFT
#define PPC_SEC_PER3_DOM14_SEC_PER_SHIFT    SEC_PER3_DOM14_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM14_SEC_PER_MASK     3UL << PPC_SEC_PER3_DOM14_SEC_PER_SHIFT
#define PPC_SEC_PER3_DOM14_SEC_EN_SHIFT     SEC_PER3_DOM14_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER3_DOM14_SEC_EN_MASK      1UL << PPC_SEC_PER3_DOM14_SEC_EN_SHIFT
#define PPC_SEC_PER3_DOM13_LOCK_SHIFT       SEC_PER3_DOM13_LOCK_FIELD_OFFSET
#define PPC_SEC_PER3_DOM13_LOCK_MASK        1UL << PPC_SEC_PER3_DOM13_LOCK_SHIFT
#define PPC_SEC_PER3_DOM13_NSE_PER_SHIFT    SEC_PER3_DOM13_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM13_NSE_PER_MASK     3UL << PPC_SEC_PER3_DOM13_NSE_PER_SHIFT
#define PPC_SEC_PER3_DOM13_SEC_PER_SHIFT    SEC_PER3_DOM13_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM13_SEC_PER_MASK     3UL << PPC_SEC_PER3_DOM13_SEC_PER_SHIFT
#define PPC_SEC_PER3_DOM13_SEC_EN_SHIFT     SEC_PER3_DOM13_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER3_DOM13_SEC_EN_MASK      1UL << PPC_SEC_PER3_DOM13_SEC_EN_SHIFT
#define PPC_SEC_PER3_DOM12_LOCK_SHIFT       SEC_PER3_DOM12_LOCK_FIELD_OFFSET
#define PPC_SEC_PER3_DOM12_LOCK_MASK        1UL << PPC_SEC_PER3_DOM12_LOCK_SHIFT
#define PPC_SEC_PER3_DOM12_NSE_PER_SHIFT    SEC_PER3_DOM12_NSE_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM12_NSE_PER_MASK     3UL << PPC_SEC_PER3_DOM12_NSE_PER_SHIFT
#define PPC_SEC_PER3_DOM12_SEC_PER_SHIFT    SEC_PER3_DOM12_SEC_PER_FIELD_OFFSET
#define PPC_SEC_PER3_DOM12_SEC_PER_MASK     3UL << PPC_SEC_PER3_DOM12_SEC_PER_SHIFT
#define PPC_SEC_PER3_DOM12_SEC_EN_SHIFT     SEC_PER3_DOM12_SEC_EN_FIELD_OFFSET
#define PPC_SEC_PER3_DOM12_SEC_EN_MASK      1UL << PPC_SEC_PER3_DOM12_SEC_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_PRI_PER0
// Register Offset : 0x1c + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_PRI_PER0_(i)                    (REG_AP_APB_PPC_PRI_PER0 + (i) * PPC_CTRL_JUMP)
#define PPC_PRI_PER0_DOM3_LOCK_SHIFT        PRI_PER0_DOM3_LOCK_FIELD_OFFSET
#define PPC_PRI_PER0_DOM3_LOCK_MASK         1UL << PPC_PRI_PER0_DOM3_LOCK_SHIFT
#define PPC_PRI_PER0_DOM3_USE_PER_SHIFT     PRI_PER0_DOM3_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM3_USE_PER_MASK      3UL << PPC_PRI_PER0_DOM3_USE_PER_SHIFT
#define PPC_PRI_PER0_DOM3_PRI_PER_SHIFT     PRI_PER0_DOM3_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM3_PRI_PER_MASK      3UL << PPC_PRI_PER0_DOM3_PRI_PER_SHIFT
#define PPC_PRI_PER0_DOM3_PRI_EN_SHIFT      PRI_PER0_DOM3_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER0_DOM3_PRI_EN_MASK       1UL << PPC_PRI_PER0_DOM3_PRI_EN_SHIFT
#define PPC_PRI_PER0_DOM2_LOCK_SHIFT        PRI_PER0_DOM2_LOCK_FIELD_OFFSET
#define PPC_PRI_PER0_DOM2_LOCK_MASK         1UL << PPC_PRI_PER0_DOM2_LOCK_SHIFT
#define PPC_PRI_PER0_DOM2_USE_PER_SHIFT     PRI_PER0_DOM2_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM2_USE_PER_MASK      3UL << PPC_PRI_PER0_DOM2_USE_PER_SHIFT
#define PPC_PRI_PER0_DOM2_PRI_PER_SHIFT     PRI_PER0_DOM2_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM2_PRI_PER_MASK      3UL << PPC_PRI_PER0_DOM2_PRI_PER_SHIFT
#define PPC_PRI_PER0_DOM2_PRI_EN_SHIFT      PRI_PER0_DOM2_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER0_DOM2_PRI_EN_MASK       1UL << PPC_PRI_PER0_DOM2_PRI_EN_SHIFT
#define PPC_PRI_PER0_DOM1_LOCK_SHIFT        PRI_PER0_DOM1_LOCK_FIELD_OFFSET
#define PPC_PRI_PER0_DOM1_LOCK_MASK         1UL << PPC_PRI_PER0_DOM1_LOCK_SHIFT
#define PPC_PRI_PER0_DOM1_USE_PER_SHIFT     PRI_PER0_DOM1_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM1_USE_PER_MASK      3UL << PPC_PRI_PER0_DOM1_USE_PER_SHIFT
#define PPC_PRI_PER0_DOM1_PRI_PER_SHIFT     PRI_PER0_DOM1_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM1_PRI_PER_MASK      3UL << PPC_PRI_PER0_DOM1_PRI_PER_SHIFT
#define PPC_PRI_PER0_DOM1_PRI_EN_SHIFT      PRI_PER0_DOM1_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER0_DOM1_PRI_EN_MASK       1UL << PPC_PRI_PER0_DOM1_PRI_EN_SHIFT
#define PPC_PRI_PER0_DOM0_LOCK_SHIFT        PRI_PER0_DOM0_LOCK_FIELD_OFFSET
#define PPC_PRI_PER0_DOM0_LOCK_MASK         1UL << PPC_PRI_PER0_DOM0_LOCK_SHIFT
#define PPC_PRI_PER0_DOM0_USE_PER_SHIFT     PRI_PER0_DOM0_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM0_USE_PER_MASK      3UL << PPC_PRI_PER0_DOM0_USE_PER_SHIFT
#define PPC_PRI_PER0_DOM0_PRI_PER_SHIFT     PRI_PER0_DOM0_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER0_DOM0_PRI_PER_MASK      3UL << PPC_PRI_PER0_DOM0_PRI_PER_SHIFT
#define PPC_PRI_PER0_DOM0_PRI_EN_SHIFT      PRI_PER0_DOM0_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER0_DOM0_PRI_EN_MASK       1UL << PPC_PRI_PER0_DOM0_PRI_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_PRI_PER1
// Register Offset : 0x20 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_PRI_PER1_(i)                    (REG_AP_APB_PPC_PRI_PER1 + (i) * PPC_CTRL_JUMP)
#define PPC_PRI_PER1_DOM7_LOCK_SHIFT        PRI_PER1_DOM7_LOCK_FIELD_OFFSET
#define PPC_PRI_PER1_DOM7_LOCK_MASK         1UL << PPC_PRI_PER1_DOM7_LOCK_SHIFT
#define PPC_PRI_PER1_DOM7_USE_PER_SHIFT     PRI_PER1_DOM7_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM7_USE_PER_MASK      3UL << PPC_PRI_PER1_DOM7_USE_PER_SHIFT
#define PPC_PRI_PER1_DOM7_PRI_PER_SHIFT     PRI_PER1_DOM7_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM7_PRI_PER_MASK      3UL << PPC_PRI_PER1_DOM7_PRI_PER_SHIFT
#define PPC_PRI_PER1_DOM7_PRI_EN_SHIFT      PRI_PER1_DOM7_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER1_DOM7_PRI_EN_MASK       1UL << PPC_PRI_PER1_DOM7_PRI_EN_SHIFT
#define PPC_PRI_PER1_DOM6_LOCK_SHIFT        PRI_PER1_DOM6_LOCK_FIELD_OFFSET
#define PPC_PRI_PER1_DOM6_LOCK_MASK         1UL << PPC_PRI_PER1_DOM6_LOCK_SHIFT
#define PPC_PRI_PER1_DOM6_USE_PER_SHIFT     PRI_PER1_DOM6_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM6_USE_PER_MASK      3UL << PPC_PRI_PER1_DOM6_USE_PER_SHIFT
#define PPC_PRI_PER1_DOM6_PRI_PER_SHIFT     PRI_PER1_DOM6_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM6_PRI_PER_MASK      3UL << PPC_PRI_PER1_DOM6_PRI_PER_SHIFT
#define PPC_PRI_PER1_DOM6_PRI_EN_SHIFT      PRI_PER1_DOM6_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER1_DOM6_PRI_EN_MASK       1UL << PPC_PRI_PER1_DOM6_PRI_EN_SHIFT
#define PPC_PRI_PER1_DOM5_LOCK_SHIFT        PRI_PER1_DOM5_LOCK_FIELD_OFFSET
#define PPC_PRI_PER1_DOM5_LOCK_MASK         1UL << PPC_PRI_PER1_DOM5_LOCK_SHIFT
#define PPC_PRI_PER1_DOM5_USE_PER_SHIFT     PRI_PER1_DOM5_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM5_USE_PER_MASK      3UL << PPC_PRI_PER1_DOM5_USE_PER_SHIFT
#define PPC_PRI_PER1_DOM5_PRI_PER_SHIFT     PRI_PER1_DOM5_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM5_PRI_PER_MASK      3UL << PPC_PRI_PER1_DOM5_PRI_PER_SHIFT
#define PPC_PRI_PER1_DOM5_PRI_EN_SHIFT      PRI_PER1_DOM5_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER1_DOM5_PRI_EN_MASK       1UL << PPC_PRI_PER1_DOM5_PRI_EN_SHIFT
#define PPC_PRI_PER1_DOM4_LOCK_SHIFT        PRI_PER1_DOM4_LOCK_FIELD_OFFSET
#define PPC_PRI_PER1_DOM4_LOCK_MASK         1UL << PPC_PRI_PER1_DOM4_LOCK_SHIFT
#define PPC_PRI_PER1_DOM4_USE_PER_SHIFT     PRI_PER1_DOM4_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM4_USE_PER_MASK      3UL << PPC_PRI_PER1_DOM4_USE_PER_SHIFT
#define PPC_PRI_PER1_DOM4_PRI_PER_SHIFT     PRI_PER1_DOM4_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER1_DOM4_PRI_PER_MASK      3UL << PPC_PRI_PER1_DOM4_PRI_PER_SHIFT
#define PPC_PRI_PER1_DOM4_PRI_EN_SHIFT      PRI_PER1_DOM4_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER1_DOM4_PRI_EN_MASK       1UL << PPC_PRI_PER1_DOM4_PRI_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_PRI_PER2
// Register Offset : 0x24 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_PRI_PER2_(i)                    (REG_AP_APB_PPC_PRI_PER2 + (i) * PPC_CTRL_JUMP)
#define PPC_PRI_PER2_DOM11_LOCK_SHIFT       PRI_PER2_DOM11_LOCK_FIELD_OFFSET
#define PPC_PRI_PER2_DOM11_LOCK_MASK        1UL << PPC_PRI_PER2_DOM11_LOCK_SHIFT
#define PPC_PRI_PER2_DOM11_USE_PER_SHIFT    PRI_PER2_DOM11_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM11_USE_PER_MASK     3UL << PPC_PRI_PER2_DOM11_USE_PER_SHIFT
#define PPC_PRI_PER2_DOM11_PRI_PER_SHIFT    PRI_PER2_DOM11_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM11_PRI_PER_MASK     3UL << PPC_PRI_PER2_DOM11_PRI_PER_SHIFT
#define PPC_PRI_PER2_DOM11_PRI_EN_SHIFT     PRI_PER2_DOM11_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER2_DOM11_PRI_EN_MASK      1UL << PPC_PRI_PER2_DOM11_PRI_EN_SHIFT
#define PPC_PRI_PER2_DOM10_LOCK_SHIFT       PRI_PER2_DOM10_LOCK_FIELD_OFFSET
#define PPC_PRI_PER2_DOM10_LOCK_MASK        1UL << PPC_PRI_PER2_DOM10_LOCK_SHIFT
#define PPC_PRI_PER2_DOM10_USE_PER_SHIFT    PRI_PER2_DOM10_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM10_USE_PER_MASK     3UL << PPC_PRI_PER2_DOM10_USE_PER_SHIFT
#define PPC_PRI_PER2_DOM10_PRI_PER_SHIFT    PRI_PER2_DOM10_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM10_PRI_PER_MASK     3UL << PPC_PRI_PER2_DOM10_PRI_PER_SHIFT
#define PPC_PRI_PER2_DOM10_PRI_EN_SHIFT     PRI_PER2_DOM10_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER2_DOM10_PRI_EN_MASK      1UL << PPC_PRI_PER2_DOM10_PRI_EN_SHIFT
#define PPC_PRI_PER2_DOM9_LOCK_SHIFT        PRI_PER2_DOM9_LOCK_FIELD_OFFSET
#define PPC_PRI_PER2_DOM9_LOCK_MASK         1UL << PPC_PRI_PER2_DOM9_LOCK_SHIFT
#define PPC_PRI_PER2_DOM9_USE_PER_SHIFT     PRI_PER2_DOM9_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM9_USE_PER_MASK      3UL << PPC_PRI_PER2_DOM9_USE_PER_SHIFT
#define PPC_PRI_PER2_DOM9_PRI_PER_SHIFT     PRI_PER2_DOM9_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM9_PRI_PER_MASK      3UL << PPC_PRI_PER2_DOM9_PRI_PER_SHIFT
#define PPC_PRI_PER2_DOM9_PRI_EN_SHIFT      PRI_PER2_DOM9_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER2_DOM9_PRI_EN_MASK       1UL << PPC_PRI_PER2_DOM9_PRI_EN_SHIFT
#define PPC_PRI_PER2_DOM8_LOCK_SHIFT        PRI_PER2_DOM8_LOCK_FIELD_OFFSET
#define PPC_PRI_PER2_DOM8_LOCK_MASK         1UL << PPC_PRI_PER2_DOM8_LOCK_SHIFT
#define PPC_PRI_PER2_DOM8_USE_PER_SHIFT     PRI_PER2_DOM8_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM8_USE_PER_MASK      3UL << PPC_PRI_PER2_DOM8_USE_PER_SHIFT
#define PPC_PRI_PER2_DOM8_PRI_PER_SHIFT     PRI_PER2_DOM8_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER2_DOM8_PRI_PER_MASK      3UL << PPC_PRI_PER2_DOM8_PRI_PER_SHIFT
#define PPC_PRI_PER2_DOM8_PRI_EN_SHIFT      PRI_PER2_DOM8_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER2_DOM8_PRI_EN_MASK       1UL << PPC_PRI_PER2_DOM8_PRI_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_PRI_PER3
// Register Offset : 0x28 + i * PPC_CTRL_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_PRI_PER3_(i)                    (REG_AP_APB_PPC_PRI_PER3 + (i) * PPC_CTRL_JUMP)
#define PPC_PRI_PER3_DOM15_LOCK_SHIFT       PRI_PER3_DOM15_LOCK_FIELD_OFFSET
#define PPC_PRI_PER3_DOM15_LOCK_MASK        1UL << PPC_PRI_PER3_DOM15_LOCK_SHIFT
#define PPC_PRI_PER3_DOM15_USE_PER_SHIFT    PRI_PER3_DOM15_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM15_USE_PER_MASK     3UL << PPC_PRI_PER3_DOM15_USE_PER_SHIFT
#define PPC_PRI_PER3_DOM15_PRI_PER_SHIFT    PRI_PER3_DOM15_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM15_PRI_PER_MASK     3UL << PPC_PRI_PER3_DOM15_PRI_PER_SHIFT
#define PPC_PRI_PER3_DOM15_PRI_EN_SHIFT     PRI_PER3_DOM15_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER3_DOM15_PRI_EN_MASK      1UL << PPC_PRI_PER3_DOM15_PRI_EN_SHIFT
#define PPC_PRI_PER3_DOM14_LOCK_SHIFT       PRI_PER3_DOM14_LOCK_FIELD_OFFSET
#define PPC_PRI_PER3_DOM14_LOCK_MASK        1UL << PPC_PRI_PER3_DOM14_LOCK_SHIFT
#define PPC_PRI_PER3_DOM14_USE_PER_SHIFT    PRI_PER3_DOM14_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM14_USE_PER_MASK     3UL << PPC_PRI_PER3_DOM14_USE_PER_SHIFT
#define PPC_PRI_PER3_DOM14_PRI_PER_SHIFT    PRI_PER3_DOM14_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM14_PRI_PER_MASK     3UL << PPC_PRI_PER3_DOM14_PRI_PER_SHIFT
#define PPC_PRI_PER3_DOM14_PRI_EN_SHIFT     PRI_PER3_DOM14_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER3_DOM14_PRI_EN_MASK      1UL << PPC_PRI_PER3_DOM14_PRI_EN_SHIFT
#define PPC_PRI_PER3_DOM13_LOCK_SHIFT       PRI_PER3_DOM13_LOCK_FIELD_OFFSET
#define PPC_PRI_PER3_DOM13_LOCK_MASK        1UL << PPC_PRI_PER3_DOM13_LOCK_SHIFT
#define PPC_PRI_PER3_DOM13_USE_PER_SHIFT    PRI_PER3_DOM13_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM13_USE_PER_MASK     3UL << PPC_PRI_PER3_DOM13_USE_PER_SHIFT
#define PPC_PRI_PER3_DOM13_PRI_PER_SHIFT    PRI_PER3_DOM13_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM13_PRI_PER_MASK     3UL << PPC_PRI_PER3_DOM13_PRI_PER_SHIFT
#define PPC_PRI_PER3_DOM13_PRI_EN_SHIFT     PRI_PER3_DOM13_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER3_DOM13_PRI_EN_MASK      1UL << PPC_PRI_PER3_DOM13_PRI_EN_SHIFT
#define PPC_PRI_PER3_DOM12_LOCK_SHIFT       PRI_PER3_DOM12_LOCK_FIELD_OFFSET
#define PPC_PRI_PER3_DOM12_LOCK_MASK        1UL << PPC_PRI_PER3_DOM12_LOCK_SHIFT
#define PPC_PRI_PER3_DOM12_USE_PER_SHIFT    PRI_PER3_DOM12_USE_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM12_USE_PER_MASK     3UL << PPC_PRI_PER3_DOM12_USE_PER_SHIFT
#define PPC_PRI_PER3_DOM12_PRI_PER_SHIFT    PRI_PER3_DOM12_PRI_PER_FIELD_OFFSET
#define PPC_PRI_PER3_DOM12_PRI_PER_MASK     3UL << PPC_PRI_PER3_DOM12_PRI_PER_SHIFT
#define PPC_PRI_PER3_DOM12_PRI_EN_SHIFT     PRI_PER3_DOM12_PRI_EN_FIELD_OFFSET
#define PPC_PRI_PER3_DOM12_PRI_EN_MASK      1UL << PPC_PRI_PER3_DOM12_PRI_EN_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_RGN_START_ADDR
// Register Offset : 0x1400 + i * PPC_RNG_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_RNG_JUMP                        0x8
#define PPC_RGN_START_ADDR_(i)              (REG_AP_APB_PPC_RGN_START_ADDR + (i)* PPC_RNG_JUMP)
#define PPC_RGN_START_ADDR_SHIFT            RGN_START_ADDR_START_ADDR_FIELD_OFFSET
#define PPC_RGN_START_ADDR_MASK             0x3FFFFFFF << PPC_RGN_START_ADDR_SHIFT

//--------------------------------------------------------------------------
// Register Name   : REG_AP_APB_PPC_RGN_END_ADDR
// Register Offset : 0x1404 + i * PPC_RNG_JUMP
// Description     :
//--------------------------------------------------------------------------
#define PPC_RGN_END_ADDR_(i)                (REG_AP_APB_PPC_RGN_END_ADDR + (i)* PPC_RNG_JUMP)
#define PPC_RGN_LOCK_SHIFT                  RGN_END_ADDR_LOCK_FIELD_OFFSET
#define PPC_RGN_LOCK_MASK                   1UL << PPC_RGN_LOCK_SHIFT
#define PPC_RGN_RGN_EN_SHIFT                RGN_END_ADDR_RGN_EN_FIELD_OFFSET
#define PPC_RGN_RGN_EN_MASK                 1UL << PPC_RGN_RGN_EN_SHIFT
#define PPC_RGN_END_ADDR_SHIFT              RGN_END_ADDR_END_ADDR_FIELD_OFFSET
#define PPC_RGN_END_ADDR_MASK               0x3FFFFFFF << PPC_RGN_END_ADDR_SHIFT

#endif