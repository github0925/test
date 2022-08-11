/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <arch/asm_macros.h>
#include <asm.h>

FUNCTION(ce_writel_)
    set_fault_handler   .Larch_write_to_reg_fault
    str     r0, [r1, #0]
    mov     r0, #0
    bx      lr
.Larch_write_to_reg_fault:
    mov     r0, #-1
    bx      lr
FUNCTION(ce_readl_)
    set_fault_handler   .Larch_read_from_reg_fault
    ldr     r0, [r0], #0
    bx      lr
.Larch_read_from_reg_fault:
    mov     r0, #-1
    bx      lr