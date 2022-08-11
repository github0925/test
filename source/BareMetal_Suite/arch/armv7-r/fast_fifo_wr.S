/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 ********************************************************/

#include "asm_def.h"

.align 3

/* void fast_fifo_wr32(U32 *fifo, U32 *data, U32 size) */
FUNCTION(fast_fifo_wr32)
	push {r4-r11}
word8_wr:
	cmp r2, #32
	blt word4_wr
	ldm r1!, {r4-r11}
	str r4, [r0]
	str r5, [r0]
	str r6, [r0]
	str r7, [r0]
	str r8, [r0]
	str r9, [r0]
	str r10, [r0]
	str r11, [r0]
	sub r2, #32
	b word8_wr
word4_wr:
	cmp r2, #16
	blt word2_wr
	ldm r1!, {r4-r7}
	str r4, [r0]
	str r5, [r0]
	str r6, [r0]
	str r7, [r0]
	sub r2, #16
	b word4_wr
word2_wr:
	cmp r2, #8
	blt word_wr
	ldm r1!, {r4-r5}
	str r4, [r0]
	str r5, [r0]
	sub r2, #8
	b word2_wr
word_wr:
	cmp r2, #0
	beq done
	ldr r4, [r1]
	str r4, [r0]
done:
	pop {r4-r11}
	bx lr
