/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"

.align 3

/* void *memclr_aligned(void* dst, size_t bytes) */
FUNCTION(memclr_aligned)
	push {r4-r11}
	mov r3, r0
	mov r4, #0
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	mov r9, #0
	mov r10, #0
	mov r11, #0
word8_clr:
	cmp r1, #32
	blt word4_clr
	stm r0!, {r4-r11}
	sub r1, #32
	b word8_clr
word4_clr:
	cmp r1, #16
	blt word2_clr
	stm r0!, {r4-r7}
	sub r1, #16
	b word4_clr
word2_clr:
	cmp r1, #8
	blt word_clr
	stm r0!, {r4-r5}
	sub r1, #8
	b word2_clr
word_clr:
	cmp r1, #0
	beq done
	str r4, [r0]
done:
	mov r0, r3
	pop {r4-r11}
	bx lr
