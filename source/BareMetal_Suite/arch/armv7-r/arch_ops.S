/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"

.align 3

#if defined(ARCH_API_arch_go_idle)
FUNCTION(arch_go_idle)
 	wfi
	bx lr
#endif

#if defined(ARCH_API_arch_invalidate_tle)
FUNCTION(arch_invalidate_tlb)
    mov  r0, #0
    mcr  p15, 0, r0, c8, c7, 0
    bx   lr
#endif

/* Note: call this function after MPU been properly configured */
#if defined(ARCH_API_arch_enable_branch_predict)
FUNCTION(arch_enable_branch_predict)
	mrc p15, 0, r0, c1, c0, 1	// ACTLR
	bic r0, r0, #(0x1 << 17)	// Clear RSDIS to enable return stack
	bic r0, r0, #(0x1 << 16)	// BP=00: Normal operation, branch prediction enabled
	bic r0, r0, #(0x1 << 15)
	mcr p15, 0, r0, c1, c0, 1
	isb
	bx 	lr
#endif

FUNCTION(arch_rd_sctlr)
	mrc p15, 0, r0, c1, c0, 0
	bx lr

FUNCTION(arch_wr_sctlr)
	mcr p15, 0, r0, c1, c0, 0
	bx lr

#if defined(ARCH_API_arch_rd_sctlr)
FUNCTION(arch_rd_actlr)
	mrc p15, 0, r0, c1, c0, 1
	bx lr
#endif

#if defined(ARCH_API_arch_wr_actlr)
FUNCTION(arch_wr_actlr)
	mcr p15, 0, r0, c1, c0, 1
	bx lr
#endif

#if defined(ARCH_API_fsr)
FUNCTION(arch_rd_dfsr)
	mrc p15, 0, r0, c5, c0, 0
	bx lr

FUNCTION(arch_rd_ifsr)
	mrc p15, 0, r0, c5, c0, 1
	bx lr
#endif

#if defined(ARCH_API_far)
FUNCTION(arch_rd_ifar)
	mrc p15, 0, r0, c6, c0, 2
	bx lr

FUNCTION(arch_rd_dfar)
	mrc p15, 0, r0, c6, c0, 0
	bx lr
#endif

FUNCTION(arch_rd_mpuir)
	mrc p15, 0, r0, c0, c0, 4
	bx lr

/* Read MPU Region Base Address Register */
FUNCTION(arch_rd_mpurbar)
	mrc p15, 0, r0, c6, c1, 0
	bx lr

FUNCTION(arch_wr_mpurbar)
	mcr p15, 0, r0, c6, c1, 0
	bx lr

/* Read MPU Region Size and Enable Register */
FUNCTION(arch_rd_mpurser)
	mrc p15, 0, r0, c6, c1, 2
	bx lr

FUNCTION(arch_wr_mpurser)
	mcr p15, 0, r0, c6, c1, 2
	bx lr

/* Read MPU Region Access Control Register */
FUNCTION(arch_rd_mpuracr)
	mrc p15, 0, r0, c6, c1, 4
	bx lr

FUNCTION(arch_wr_mpuracr)
	mcr p15, 0, r0, c6, c1, 4
	bx lr

/* Read MPU Region Number Register */
FUNCTION(arch_rd_mpurgnr)
	mrc p15, 0, r0, c6, c2, 0
	bx lr

FUNCTION(arch_wr_mpurgnr)
	mcr p15, 0, r0, c6, c2, 0
	bx lr

FUNCTION(arch_rd_ccsidr)
	mrc p15, 1, r0, c0, c0, 0
	bx lr

FUNCTION(arch_disable_interrupt)
	cpsid if
	bx lr

FUNCTION(arch_enable_interrupt)
	cpsie if
	bx lr

FUNCTION(arch_disable_async_abort)
	cpsid a
	isb
	dsb
	bx lr

FUNCTION(arch_enable_async_abort)
	cpsie a
	isb
	dsb
	bx lr
