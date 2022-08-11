/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"
#include "memory_map.h"
#include "lnk_sym.h"
#include "arch.h"

.section ".text"
.align 3

.global	reset_hdlr
.global _start

_start:
reset_hdlr:
	mov r0, #0
	mov r1, #0
	mov r2, #0
	mov r3, #0
	mov r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	mov r9, #0
	mov r10, #0
	mov r11, #0
	mov r12, #0
	mov sp, r0
	mov lr, #0
	mrs r4, cpsr	/* save the CPSR */
	msr spsr_fsxc, r4	/*load spsr with value from cpsr*/

	ldr r0, =__stack_end

	/* Initialize banked ARM registers */
	cps #MODE_SYS
	mov sp, r0
	mov lr, #0
	cps #MODE_ABT
	mov sp, r0
	mov lr, #0
	msr spsr_fsxc, r4	/*load spsr with value from cpsr*/
	cps #MODE_UND
	mov sp, r0
	mov lr, #0
	msr spsr_fsxc, r4	/*load spsr with value from cpsr*/
	cps #MODE_IRQ
	mov sp, r0
	mov lr, #0
	msr spsr_fsxc, r4	/*load spsr with value from cpsr*/
	@ add fiq support 
	sub r0, r0, #0x1000
	cps #MODE_FIQ
	mov r8, #0
	mov r9, #0
	mov r10, #0
	mov r11, #0
	mov r12, #0
	mov sp, r0
	mov lr, #0
	msr spsr_fsxc, r4	/*load spsr with value from cpsr*/
	/* switch back to Supervisor mode */
	cps #MODE_SVC

#if defined(CFG_ARCH_FPU)
	ldr r0, =0x00F00000
	mcr p15, 0, r0, c1, c0, 2	/* Enable coprocessor access in CPACR */
	ldr r2, =0x40000000
	vmsr fpexc, r2	/* Enable FPU access so that VFP registers can be initialized */
	vmov D0, r5, r6	/* r5/6 is 0 now */
	vmov D1, r5, r6
	vmov D2, r5, r6
	vmov D3, r5, r6
	vmov D4, r5, r6
	vmov D5, r5, r6
	vmov D6, r5, r6
	vmov D7, r5, r6
	vmov D8, r5, r6
	vmov D9, r5, r6
	vmov D10, r5, r6
	vmov D11, r5, r6
	vmov D12, r5, r6
	vmov D13, r5, r6
	vmov D14, r5, r6
	vmov D15, r5, r6
#endif

#if defined(CFG_ARCH_DIS_BRANCH_PREDICT)
	/* Disable Branch prediction in case somehow it fetch from some
	 * memory which is not enabled yet */
	mrc p15, 0, r0, c1, c0, 1	// ACTLR
	orr r0, r0, #(0x1 << 17)	// Set RSDIS to disable return stack
	orr r0, r0, #(0x1 << 16)	// BP=10: branch always not taken and history table updates disabled
	bic r0, r0, #(0x1 << 15)
	mcr p15, 0, r0, c1, c0, 1
	isb
#endif

	mrc p15, 0, r0, c1, c0, 0	// SCTLR
	orr r0, r0, #(0x01 << 1)	// SCTLR.A
#if defined(CFG_ARCH_R7_MPU_BG_EN)
	orr r0, r0, #(0x01 << 17)	// SCTLR.BR	MPU background region enable
#endif
	mcr p15, 0, r0, c1, c0, 0
	isb
	dsb

#if defined(CFG_ARCH_LLPP_EN)
	mrc p15, 0, r0, c15, c0, 1
	tst r0, #(0x1F << 2)	/* Size Mask*/
	beq	.no_pp_present
	orr r0, #1				/* Enable LLPP Normal AXI interface */
	mcr p15, 0, r0, c15, c0, 1
.no_pp_present:
#endif

#if defined(CFG_ARCH_TCM)
	mrc p15, 0, r0, c0, c0, 2	// Read TCMTR (TCM Type Register)
	// r0: ATCM & BTCM availability
	mrc p15, 0, r0, c9, c1, 1	// Read ATCM Region Register
	// r0 now contains ATCM size in bits[6:2]
	mrc p15, 0, r0, c9, c1, 0	// Read BTCM Region Register
	// r0 now contains BTCM size in bits[6:2]

	// Write to TCMTR.size will be ignored
	ldr r0, =ATCM_BASE
	orr r0, r0, #1	// Enable ATCM
	mcr p15, 0, r0, c9, c1, 1
	ldr r0, =BTCM_BASE
	orr r0, r0, #1	// Enable BTCM
	mcr p15, 0, r0, c9, c1, 0
#endif

	ldr r0, =__stack_end
	mov sp, r0

	/* !!! linker script shall make sure __stack_start and size shall be 8
	 * bytes aligned
	 */
#if defined(CFG_SOC_RAM_ECC)
#if !defined(CFG_ATB_INIT_MIN_STACK)
	ldr r0, =__stack_start
#else
	ldr r0, =__stack_end
	ldr r1, =CFG_ATB_MIN_STACK_SZ
	sub r0, r1
#endif
	ldr r1, =__stack_end
	ldr r2, =0x5AA5F00F
	ldr r3, =0xA55A0FF0
	mov r4, r2
	mov r5, r3
	mov r6, r2
	mov r7, r3
	mov r8, r2
	mov r9, r3
1:
 	stmia r0!, {r2-r9}
	cmp r0, r1
	blo	1b
#endif

    mov r0, #3              /* Enable I/D cache here for better performance */
    bl arch_enable_cache

#if defined(CFG_SOC_RAM_ECC) && defined(CFG_ATB_INIT_MIN_STACK)
	ldr r0, =__stack_start
	ldr r1, =__stack_end
	ldr r2, =CFG_ATB_MIN_STACK_SZ
	sub r1, r2
	ldr r2, =0x5AA5F00F
	ldr r3, =0xA55A0FF0
	mov r4, r2
	mov r5, r3
	mov r6, r2
	mov r7, r3
	mov r8, r2
	mov r9, r3
1:
 	stmia r0!, {r2-r9}
	cmp r0, r1
	blo	1b
#endif

	/* !!! linker script shall make sure __data_start/__bss_start and their
	 * size (if not zero) shall be 32 bytes aligned
	 */
	ldr r0, =__load_data_start
	ldr r1, =__data_start
	ldr r2, =__data_end
	cmp r0, r1
	beq .skip_data_cp
.data_cp_loop:
	cmp r1, r2
	bhs	.skip_data_cp
	ldmia r0!, {r3-r10}
	stmia r1!, {r3-r10}
	b .data_cp_loop

.skip_data_cp:

    ldr r0, =__bss_start
	ldr r1, =__bss_end
	mov r2, #0
	mov r3, #0
	mov r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	mov r9, #0
.bss_clr_loop:
	cmp r0, r1
	bhs .bss_clr_done
 	stmia r0!, {r2-r9}
	b .bss_clr_loop

.bss_clr_done:
	bl arch_disable_interrupt

	mov r0, #0
	mov r1, #0
    ldr r2, =__main
	blx r2

loop:
	b loop
