/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef _SOC_AP_H_
#define _SOC_AP_H_

#include "__regs_base.h"

#define UART_ROOT_CLK_FREQ  60000000u

#define SCR_BASE    APB_SCR_SEC_BASE

#define GIC_RBASE   GIC4_BASE

#define CE2_RBASE   APB_CE2_VCE1_BASE

#define MB_ID_THIS_CPU   MB_CPU_AP1 /* TODO */

extern void init_xlat_table(void);
extern void enable_mmu(void);

#endif  /* _SOC_AP_H_ */
