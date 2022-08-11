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

FUNCTION(arm64_exception_base_el2)

/* exceptions from current EL, using SP0 */
LOCAL_FUNCTION(arm64_sync_exc_current_el_SP0_el2)
    eret
.org 0x080
LOCAL_FUNCTION(arm64_irq_current_el_SP0_el2)
    eret
.org 0x100
LOCAL_FUNCTION(arm64_fiq_current_el_SP0_el2)
    eret
.org 0x180
LOCAL_FUNCTION(arm64_err_exc_current_el_SP0_el2)
    eret
/* exceptions from current EL, using SPx */
.org 0x200
LOCAL_FUNCTION(arm64_sync_exc_current_el_SPx_el2)
    eret
.org 0x280
LOCAL_FUNCTION(arm64_irq_current_el_SPx_el2)
    eret
.org 0x300
LOCAL_FUNCTION(arm64_fiq_current_el_SPx_el2)
    eret
.org 0x380
LOCAL_FUNCTION(arm64_err_exc_current_el_SPx_el2)
    eret
/* exceptions from lower EL, running arm64 */
.org 0x400
LOCAL_FUNCTION(arm64_sync_exc_lower_el_64_el2)
    mrs   x9, esr_el2
    lsr   x9, x9, #HSR_EC_SHIFT           /* x1 = ESR_EL2.EC */
    cmp   x9, #HSR_EC_HVC64
    bne   1f
    mov   x9, x0
    mov   x0, x1
    mov   x1, x2
    mov   x2, x3
    mov   x3, x4
    mov   x4, x5
    blr   x9
1:
    eret
.org 0x480
LOCAL_FUNCTION(arm64_irq_lower_el_64_el2)
    eret
.org 0x500
LOCAL_FUNCTION(arm64_fiq_lower_el_64_el2)
    eret
.org 0x580
LOCAL_FUNCTION(arm64_err_exc_lower_el_64_el2)
    eret
/* exceptions from lower EL, running arm32 */
.org 0x600
LOCAL_FUNCTION(arm64_sync_exc_lower_el_32_el2)
    eret
.org 0x680
LOCAL_FUNCTION(arm64_irq_lower_el_32_el2)
    eret
.org 0x700
LOCAL_FUNCTION(arm64_fiq_lower_el_32_el2)
    eret
.org 0x780
LOCAL_FUNCTION(arm64_err_exc_lower_el_32_el2)
    eret
LOCAL_FUNCTION(arm64_exc_shared_restore_long_el2)
    eret
LOCAL_FUNCTION(arm64_exc_shared_restore_short_el2)
    eret
