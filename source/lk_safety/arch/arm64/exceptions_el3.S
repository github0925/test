/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 **/

#include <asm.h>
#include <arch/asm_macros.h>

  .macro armv8_switch_to_el2, ep, dtb, tmp
          msr     cptr_el3, xzr           /* Disable coprocessor traps to EL3 */
          mov     \tmp, #0x33ff
          msr     cptr_el2, \tmp          /* Disable coprocessor traps to EL2 */
          /* Initialize Generic Timers */
          msr     cntvoff_el2, xzr
          /* Initialize SCTLR_EL2
          ¦*
          ¦* setting RES1 bits (29,28,23,22,18,16,11,5,4) to 1
          ¦* and RES0 bits (31,30,27,26,24,21,20,17,15-13,10-6) +
          ¦* EE,WXN,I,SA,C,A,M to 0
          ¦*/
          ldr     \tmp, =(0x30c50830)
          msr     sctlr_el2, \tmp

          mov     \tmp, sp
          msr     sp_el2, \tmp            /* Migrate SP */
          mrs     \tmp, vbar_el1
          msr     vbar_el2, \tmp          /* Migrate VBAR */

          ldr     \tmp, =(0x000005b1)

          msr     scr_el3, \tmp

          /* Return to the EL2_SP2 mode from EL3 */
          ldr     \tmp, =(0x000003c9)
          msr     spsr_el3, \tmp
          msr     elr_el3, \ep
          mov     x0, \dtb
  .endm

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
    mrs    x30, esr_el3
    ubfx   x30, x30, #26, #6
    cmp    x30, #19    /*arch32*/
    beq    2f
    cmp    x30, #23    /*arch64*/
    beq    1f
    b      2f
1:
    mov    x12, #0xc300
    lsl    x12, x12, #16
    add    x12, x12, 1
    cmp    x0, x12
    bne    2f
    armv8_switch_to_el2 x1, x2, x3

2:
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
