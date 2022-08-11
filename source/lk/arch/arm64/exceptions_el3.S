/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 **/

#include <asm.h>
#include <arch/asm_macros.h>
#include <arch/defines.h>

.section .text.boot.vectab
.align 12

FUNCTION(arm64_exception_base_el3)

/* exceptions from current EL, using SP0 */
LOCAL_FUNCTION(arm64_sync_exc_current_el_SP0_el3)
    eret
.org 0x080
LOCAL_FUNCTION(arm64_irq_current_el_SP0_el3)
    eret
.org 0x100
LOCAL_FUNCTION(arm64_fiq_current_el_SP0_el3)
    eret
.org 0x180
LOCAL_FUNCTION(arm64_err_exc_current_el_SP0_el3)
    eret
/* exceptions from current EL, using SPx */
.org 0x200
LOCAL_FUNCTION(arm64_sync_exc_current_el_SPx_el3)
    eret
.org 0x280
LOCAL_FUNCTION(arm64_irq_current_el_SPx_el3)
    eret
.org 0x300
LOCAL_FUNCTION(arm64_fiq_current_el_SPx_el3)
    eret
.org 0x380
LOCAL_FUNCTION(arm64_err_exc_current_el_SPx_el3)
    eret
/* exceptions from lower EL, running arm64 */
.org 0x400
LOCAL_FUNCTION(arm64_sync_exc_lower_el_64_el3)

    mrs    x9, esr_el3   /* save esr to temporary registers */
    ubfx   x9, x9, #26, #6
    /* SMC instruction execution in AArch64 state */
    cmp    x9, #23
    bne    1f
    /* ----------------------------------------------
     * Detect if this is a RUN_IMAGE or other SMC.
     * ----------------------------------------------
     */
    mov    x9, #SMC_RUN_IMAGE_BL31
    cmp    x9, x0
    b.eq   smc_run_bl31
    mov    x9, #0xc4000000
    movk   x9, #0x21
    cmp    x9, x0
    bne    1f
    mrs    x10, scr_el3
    bic    x10, x10, #0x100    /* Disable SCR_EL3.HCE */
    msr    scr_el3, x10
1:
    eret
.org 0x480
LOCAL_FUNCTION(arm64_irq_lower_el_64_el3)
    eret
.org 0x500
LOCAL_FUNCTION(arm64_fiq_lower_el_64_el3)
    eret
.org 0x580
LOCAL_FUNCTION(arm64_err_exc_lower_el_64_el3)
    eret
/* exceptions from lower EL, running arm32 */
.org 0x600
LOCAL_FUNCTION(arm64_sync_exc_lower_el_32_el3)
    eret
.org 0x680
LOCAL_FUNCTION(arm64_irq_lower_el_32_el3)
    eret
.org 0x700
LOCAL_FUNCTION(arm64_fiq_lower_el_32_el3)
    eret
.org 0x780
LOCAL_FUNCTION(arm64_err_exc_lower_el_32_el3)
    eret
LOCAL_FUNCTION(arm64_exc_shared_restore_long_el3)
    eret
LOCAL_FUNCTION(arm64_exc_shared_restore_short_el3)
    eret

FUNCTION(smc_run_bl31)

    /* ------------------------------------------------
     * Make sure only Secure world reaches here. non secure world smc can not boot bl31 img
     * ------------------------------------------------
     */
    mrs x30, scr_el3
    tst x30, #SCR_NS_BIT
    b.ne unexpected_sync_exception

    /* ---------------------------------------------------------------------
     * Pass EL3 control to next BL image.
     * Here it expects X1 with the address of a entry_point_info_t
     * structure describing the next BL image entrypoint.
     * spsr_el3 value 0x1cd comefrom g_spsr = SPSR_64(MODE_EL3, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS)
     * ---------------------------------------------------------------------
     */
    mov x29, #0x1cd
    msr elr_el3, x1
    msr spsr_el3, x29

    bl disable_mmu_icache_el3
    tlbi alle3
    dsb ish /* ERET implies ISB, so it is not needed here */

    mov x0, x2 /* bl32 image addr */
    mov x1, x3 /* bl33 image addr */
    mov x2, x4 /* linux dtb base */

    eret

unexpected_sync_exception:
    mov x0, sp
    bl arm64_invalid_exception

FUNCTION(disable_mmu_icache_el3)
    mov x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
    mrs x0, sctlr_el3
    bic x0, x0, x1
    msr sctlr_el3, x0
    isb /* ensure MMU is off */
    dsb sy
    ret
