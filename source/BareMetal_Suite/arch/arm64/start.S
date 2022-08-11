/********************************************************
 *	        Copyright(c) 2020	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"
#include "lnk_sym.h"
#include <arch/asm_macros.h>

.section .text.start

.extern arm64_exception_base

cpuid   .req x19

FUNCTION(_start)
    /* clearing general registers */
    mov x0, xzr
    mov x1, x0
    mov x2, x0
    mov x3, x0
    mov x4, x0
    mov x5, x0
    mov x6, x0
    mov x7, x0
    mov x8, x0
    mov x9, x0
    mov x10, x0
    mov x11, x0
    mov x12, x0
    mov x13, x0
    mov x14, x0
    mov x15, x0
    mov x16, x0
    mov x17, x0
    mov x18, x0
    mov x19, x0
    mov x20, x0
    mov x21, x0
    mov x22, x0
    mov x23, x0
    mov x24, x0
    mov x25, x0
    mov x26, x0
    mov x27, x0
    mov x28, x0
    mov x29, x0
    mov x30, x0     /* lr */
    mov sp, x0

    mrs x1, sctlr_el3
    orr x1, x1, #(0x01 << 12)    /* I */
    msr sctlr_el3, x1

    /* get CPU ID */
    mrs cpuid, mpidr_el1
    ubfx cpuid, cpuid, #0, #24
    lsr cpuid, cpuid, #8

    ldr x1, =__stack_end
    mov x2, #0x1000
    mul x2, x2, cpuid
    sub sp, x1, x2

    cbnz cpuid, .bss_clr_done

	/* !!! linker script shall make sure __data_start/__bass_start and the
	 * size (if not zero) shall be 32 bytes aligned
	 */
	ldr x0, =__load_data_start
	ldr x1, =__data_start
	ldr x2, =__data_end
	cmp x0, x1
	beq .skip_data_cp
.data_cp_loop:
	cmp x1, x2
	bhs	.skip_data_cp
    ldp x4, x5, [x0], #16
    ldp x6, x7, [x0], #16
    stp x4, x5, [x1], #16
    stp x6, x7, [x1], #16
	b .data_cp_loop
.skip_data_cp:
    ldr x0, =__bss_start
	ldr x1, =__bss_end
	mov x4, #0
	mov x5, #0
	mov x6, #0
	mov x7, #0
.bss_clr_loop:
	cmp x0, x1
	bhs .bss_clr_done
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
	b .bss_clr_loop
.bss_clr_done:

    ldr x0, =arm64_exception_base
    msr vbar_el3, x0
    isb sy

    mov x0, cpuid
    bl __main
    b .
