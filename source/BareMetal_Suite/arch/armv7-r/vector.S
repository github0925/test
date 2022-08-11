/********************************************************
 *	        Copyright(c) 2018	Semidrive 		        *
 *******************************************************/
.section ".text.vector"
.align 3

#if defined(__ARMCC_VERSION)
.eabi_attribute Tag_ABI_align_preserved, 1
#endif

.extern excpt_hdlr_dabort
.extern default_exception_hdlr_s
.extern reset_hdlr
.extern irq_handler

_vector_tbl:
	ldr pc, reset_addr
	ldr pc, undefined_addr
	ldr pc, syscall_addr
	ldr pc, prefetch_addr
	ldr pc, data_abort_addr
	b .							// Reserved
	ldr pc, irq_addr
	ldr pc, fiq_addr
	
.global __fiq_;
.type __fiq_,"function";
__fiq_:
	push	{r0 - r8,lr}
	bl 		irq_handler
	pop	    {r0 - r8,lr}
    @ stmia	sp, {r0 - r8,lr}
	@ bl 		irq_handler
	@ ldmia	sp, {r0 - r8,lr}
	subs    pc, lr, #4


reset_addr: 		.word reset_hdlr
undefined_addr:		.word default_exception_hdlr_s
syscall_addr:		.word default_exception_hdlr_s
prefetch_addr:		.word default_exception_hdlr_s
data_abort_addr:	.word default_exception_hdlr_s
irq_addr:			.word default_exception_hdlr_s
fiq_addr:			.word __fiq_
padding:			.word 0
