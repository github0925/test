/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"
#include "lnk_sym.h"
#include <arch/asm_macros.h>
#include "armv8_mmu.h"


/* void enable_mmu_el3(uint64_t *xlat_tbl) */
FUNCTION(enable_mmu_el3)
    /* Invalidate TLB */
    tlbi    alle3
    isb
    dsb     sy

    ldr     x1, =MMU_MAIR_VAL
    msr     mair_el3, x1

    /* Initialize TCR_EL1 */
    /* set cacheable attributes on translation walk */
    /* (SMP extensions) non-shareable, inner write-back write-allocate */
    ldr     x1, =MMU_TCR_VAL
    msr     tcr_el3, x1
    isb

    msr     ttbr0_el3, x0
    /*
     * Ensure all translation table writes have drained into memory, the TLB
     * invalidation is complete, and translation register writes are
     * committed before enabling the MMU
     */
	dsb	ish
    isb

    mrs     x1, sctlr_el3
    orr     x1, x1, #0x1    /* M */
    /* SCTLR.I: If M is 0, instruction access from EL3 are to Normal WBWA */
    orr     x1, x1, #(0x01 << 2)    /* C */
    msr     sctlr_el3, x1
.Lmmu_on_pc:
    isb
    /* Jump to virtual code address */
    ldr     x1, =.Lmmu_on_vaddr
    br      x1

.Lmmu_on_vaddr:
    ret
