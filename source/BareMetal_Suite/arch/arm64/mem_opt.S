/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"
#include "lnk_sym.h"
#include <arch/asm_macros.h>

/* void memcpy_aligned(uint64_t *dst, const uint64_t * src, size_t sz) */
FUNCTION(memcpy_aligned)
quad8_cp:
    cmp x2, #64
    blt quad_cp
    ldp x4, x5, [x1], #16
    ldp x6, x7, [x1], #16
    ldp x8, x9, [x1], #16
    ldp x10, x11, [x1], #16
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    stp x8, x9, [x0], #16
    stp x10, x11, [x0], #16
    sub x2, x2, #64
    b quad8_cp
quad_cp:
	cmp x2, #0
    beq done
    ldr x4, [x1], #8
    str x4, [x0], #8
    sub x2, x2, #8
    b quad_cp
done:
    ret

/* void mem_wr_only_aligned(uint64_t *dst, size_t sz)*/
FUNCTION(mem_wr_only_aligned)
    add x2, x0, x1
1:
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    stp x8, x9, [x0], #16
    stp x10, x11, [x0], #16
    cmp x0, x2
    blt 1b
    ret

/* void mem_rd_only_aligned(uint64_t *dst, size_t sz)*/
FUNCTION(mem_rd_only_aligned)
    add x2, x0, x1
2:
    ldp x4, x5, [x0], #16
    ldp x6, x7, [x0], #16
    ldp x8, x9, [x0], #16
    ldp x10, x11, [x0], #16
    cmp x0, x2
    blt 2b
    ret

/* void memset_aligned(uint64_t *dst, uint8_t v, size_t sz)*/
FUNCTION(memset_aligned)
    orr x1, x1, x1, LSL #8
    orr x1, x1, x1, LSL #16
    orr x1, x1, x1, LSL #32
    mov x4, x1
    mov x5, x1
    mov x6, x1
    mov x7, x1
    mov x8, x1
    mov x9, x1
    mov x10, x1
    mov x11, x1
quad8_set:
    cmp x2, #64
    blt quad_set
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    stp x8, x9, [x0], #16
    stp x10, x11, [x0], #16
    sub x2, x2, #64
    b quad8_set
quad_set:
	cmp x2, #0
    beq set_done
    str x4, [x0], #8
    sub x2, x2, #8
    b quad_set
set_done:
    ret

/* void memclr_aligned(uint64_t *dst, size_t sz)*/
FUNCTION(memclr_aligned)
    stp x29,x30,[sp,#-16]!
    mov x2, x1
    mov x1, #0
    bl memset_aligned
    ldp x29,x30,[sp],#16
    ret
