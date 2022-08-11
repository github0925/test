#ifndef __TARGET_RES_H__
#define __TARGET_RES_H__
#include "chip_res.h"
#include "irq.h"
#include "__regs_base.h"
/*bagin:global res index redefine*/
#define DEBUG_COM RES_UART_UART15
#define SYS_TIMER RES_TIMER_TIMER4
#define SYS_TIMER_HF_CLK_FREQ CKGEN_TIMER4
#define SYS_TIMER_LF_CLK_FREQ TIMER4_LF_FREQ

#define VCE_ID_GENERAL_SUPPORT_PKA RES_CE_MEM_CE2_VCE2

/*end:global res index redefine*/
#define GIC_RES_ID RES_GIC_GIC5
#define GIC_BASE GIC5_BASE
#define GICBASE(n)  (GIC5_BASE)
#define GICD_OFFSET (0x1000)
#define GICC_OFFSET (0x2000)

/*begin:int redefine*/
#define UART_INT   IRQ_GIC5_UART15_INTR_NUM
#define TIMER_INT  IRQ_GIC5_TIMER4_CHN_A_IRQ_NUM
#define MU_MESSAGE_READY_INT  (IRQ_GIC2_MU_MU_MESSAGE_READY_INT_O1_NUM)

/*system timer source clock select*/
#define TIMER_HF_SRC_CLK_TYPE 0
#define TIMER_LF_SRC_CLK_TYPE 1
#define TIMER_SEL_SRC_CLK_TYPE TIMER_LF_SRC_CLK_TYPE

#define MAX_INT     MAX_INT_NUM
/*end:int redefine*/
#endif /* __TARGET_RES_H__*/
