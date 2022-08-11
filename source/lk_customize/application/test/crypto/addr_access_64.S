/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <arch/asm_macros.h>
#include <asm.h>

FUNCTION(ce_writel_)
    set_fault_handler   .Larch_write_to_reg_fault
    str     w0, [x1]
    mov     w0, #0
    ret
    nop
.Larch_write_to_reg_fault:
    mov     w0, #-1
    ret
FUNCTION(ce_readl_)
    set_fault_handler   .Larch_read_from_reg_fault
    ldr     w0, [x0]
    ret
.Larch_read_from_reg_fault:
    mov     w0, #-1
    ret
