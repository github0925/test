;
 ; Copyright (c) 2008-2015 Travis Geiselbrecht
 ;
 ; Permission is hereby granted, free of charge, to any person obtaining
 ; a copy of this software and associated documentation files
 ; (the "Software"), to deal in the Software without restriction,
 ; including without limitation the rights to use, copy, modify, merge,
 ; publish, distribute, sublicense, and/or sell copies of the Software,
 ; and to permit persons to whom the Software is furnished to do so,
 ; subject to the following conditions:
 ;
 ; The above copyright notice and this permission notice shall be
 ; included in all copies or substantial portions of the Software.
 ;
 ; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 ; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 ; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 ; IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 ; CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 ; TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 ; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <asm.h>
#include <arch/asm.h>

; exception handling glue.
 ; NOTE: only usable on armv6+ cores


#define TIMESTAMP_IRQ 0
    EXTERN arm_undefined_handler
    EXTERN arm_syscall_handler
    EXTERN arm_prefetch_abort_handler
    EXTERN arm_data_abort_handler
    EXTERN platform_fiq
    EXTERN platform_irq
    SECTION .text:CODE:ROOT(2)
    PUBLIC __arm_in_handler
    PUBLIC arm_undefined
    PUBLIC arm_prefetch_abort
    PUBLIC arm_data_abort
    PUBLIC arm_reserved
    PUBLIC arm_fiq

; macros to align and unalign the stack on 8 byte boundary for ABI compliance
stack_align macro tempreg
    ; make sure the stack is aligned
    mov     tempreg, sp
    tst     sp, #4
    subeq   sp, #4
    push    { tempreg }

    ; tempreg holds the original stack
    endm

stack_restore macro tempreg
    ; restore the potentially unaligned stack
    pop     { tempreg }
    mov     sp, tempreg
    endm

; save and disable the vfp unit
vfp_save macro temp1
    ; save old fpexc
    vmrs    temp1, fpexc

    push    { temp1 }

    ; hard disable the vfp unit
    bic     temp1, #(1<<30)
    vmsr    fpexc, temp1
    endm

; restore the vfp enable/disable state
vfp_restore macro temp1
    ; restore fpexc
    pop     { temp1 }

    vmsr    fpexc, temp1
    endm

; Save callee trashed registers.
 ; At exit r0 contains a pointer to the register frame.

save macro
    ; save spsr and r14 onto the svc stack
    srsdb   #0x13!

    ; switch to svc mode, interrupts disabled
    cpsid   i,#0x13

    ; save callee trashed regs and lr
    push    { r0-r3, r12, lr }

    ; save user space sp/lr
    sub     sp, #8
    stmia   sp, { r13, r14 }^

#if ARM_WITH_VFP
    ; save and disable the vfp unit
    vfp_save    r0
#endif

    ; make sure the stack is 8 byte aligned
    stack_align r0

    ; r0 now holds the pointer to the original iframe (before alignment)
    endm

save_offset macro offset
    sub     lr, offset
    save

    endm

restore macro
    ; undo the stack alignment we did before
    stack_restore r0

#if ARM_WITH_VFP
    ; restore the old state of the vfp unit
    vfp_restore r0
#endif

    ; restore user space sp/lr
    ldmia   sp, { r13, r14 }^
    add     sp, #8

    pop     { r0-r3, r12, lr }

    ; return to whence we came from
    rfeia   sp!
    endm

; Save all registers.
 ; At exit r0 contains a pointer to the register frame.

saveall macro
    ; save spsr and r14 onto the svc stack
    srsdb   #0x13!

    ; switch to svc mode, interrupts disabled
    cpsid   i,#0x13

    ; save all regs
    push    { r0-r12, lr }

    ; save user space sp/lr
    sub     sp, #8
    stmia   sp, { r13, r14 }^

#if ARM_WITH_VFP
    ; save and disable the vfp unit
    vfp_save    r0
#endif

    ; make sure the stack is 8 byte aligned
    stack_align r0

    ; r0 now holds the pointer to the original iframe (before alignment)
    endm

saveall_offset macro offset
    sub     lr, offset
    saveall
    endm

restoreall macro
    ; undo the stack alignment we did before
    stack_restore r0

#if ARM_WITH_VFP
    ; restore the old state of the vfp unit
    vfp_restore r0
#endif

    ; restore user space sp/lr
    ldmia   sp, { r13, r14 }^
    add     sp, #8

    pop     { r0-r12, r14 }

    ; return to whence we came from
    rfeia   sp!
    endm

arm_undefined:
    save
    ; r0 now holds pointer to iframe

    bl      arm_undefined_handler

    restore

#ifndef WITH_LIB_SYSCALL
arm_syscall:
    saveall
    ; r0 now holds pointer to iframe

    bl      arm_syscall_handler

    restoreall
#endif

arm_prefetch_abort:
    saveall_offset #4
    ; r0 now holds pointer to iframe

    bl      arm_prefetch_abort_handler

    restoreall

arm_data_abort:
    saveall_offset #8
    ; r0 now holds pointer to iframe

    bl      arm_data_abort_handler

    restoreall

arm_reserved:
    b   .

arm_irq:
#if TIMESTAMP_IRQ
    ; read the cycle count
    mrc     p15, 0, sp, c9, c13, 0
    str     sp, [pc, #__irq_cycle_count - . - 8]
#endif

    save_offset    #4

    ; r0 now holds pointer to iframe

    ; track that we're inside an irq handler
    LOADCONST(r2, __arm_in_handler)
    mov     r1, #1
    str     r1, [r2]

    ; call into higher level code
    bl  platform_irq

    ; clear the irq handler status
    LOADCONST(r1, __arm_in_handler)
    mov     r2, #0
    str     r2, [r1]

    ; reschedule if the handler returns nonzero
    cmp     r0, #0
    ; blne    thread_preempt

    restore

arm_fiq:
    save_offset #4
    ; r0 now holds pointer to iframe

    bl  platform_fiq

    restore
    SECTION .data:DATA
        __arm_in_handler:   DCD   0
    END

