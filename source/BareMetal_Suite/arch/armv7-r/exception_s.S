/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/

#include "asm_def.h"

.align 3

FUNCTION(default_exception_hdlr_s)
    /* save spsr and r14 onto the svc stack */
	srsdb #0x13!
	/* change to svc mode, disable interrupt */
	cpsid i, #0x13	// we are in svc now
	push {r0-r12, lr}
	mov r0, sp
	bl default_exception_hdlr
	b .		// default_exception_hdlr will not return, add a loop here in case.
