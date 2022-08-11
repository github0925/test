#ifndef __TARGET_RES_H__
#define __TARGET_RES_H__
#include "chip_res.h"
#include "irq.h"
#include "__regs_base.h"

/*bagin:global res index redefine*/
#define DEBUG_COM RES_UART_UART3 //should be uart3, z1 fixed code
#define RES_KERNEL_TIMER RES_TIMER_TIMER2
#define SYS_TIMER_HF_CLK_FREQ CKGEN_TIMER2
#define SYS_TIMER_LF_CLK_FREQ TIMER2_LF_FREQ
#define CAMERA_AVM RES_CSI_CSI1

#define INT_KERNEL_TICK TIMER2_CNT_OVF_IRQ_NUM

#define VCE_ID_GENERAL_SUPPORT_PKA RES_CE_MEM_CE1

#define GIC_RES_ID RES_GIC_GIC1
#define GIC_BASE GIC1_BASE
#define GICBASE(n)  (GIC1_BASE)
#define GICD_OFFSET (0x1000)
#define GICC_OFFSET (0x2000)
//#define GICC_OFFSET (0x4000)

/*end:global res index redefine*/

/*begin:int redefine*/
#define UART_INT   IRQ_GIC1_UART3_INTR_NUM
#define TIMER_INT  IRQ_GIC1_TIMER2_CHN_A_IRQ_NUM
#define MU_MESSAGE_READY_INT  (IRQ_GIC1_MU_MU_MESSAGE_READY_INT_O0_NUM)

#define MAX_INT MAX_INT_NUM

/*end:int redefine*/
#endif /* __TARGET_RES_H__*/
