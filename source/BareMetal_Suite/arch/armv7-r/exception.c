/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    exception.c
 * @brief   ATB exception handlers
 */

/*
 * Exceptions are not allowed on ATB. Per any exception, ATB
 * will reset the boot core.
 */

#include <common_hdr.h>
#include <soc.h>

typedef struct {
    U32 r0;
    U32 r1;
    U32 r2;
    U32 r3;
    U32 r4;
    U32 r5;
    U32 r6;
    U32 r7;
    U32 r8;
    U32 r9;
    U32 r10;
    U32 r11;
    U32 r12;
    U32 lr;
    U32 lr_saved;
    U32 spsr;
} expt_frame_t, *expt_frame_p;

static void dump_exception_ctx(expt_frame_p frame)
{
    DBG("Exception Ctx dump...\n");

    DBG(" r0: 0x%x\n", frame->r0);
    DBG(" r1: 0x%x\n", frame->r1);
    DBG(" r2: 0x%x\n", frame->r2);
    DBG(" r3: 0x%x\n", frame->r3);
    DBG(" r4: 0x%x\n", frame->r4);
    DBG(" r5: 0x%x\n", frame->r5);
    DBG(" r6: 0x%x\n", frame->r6);
    DBG(" r7: 0x%x\n", frame->r7);
    DBG(" r8: 0x%x\n", frame->r8);
    DBG(" r9: 0x%x\n", frame->r9);
    DBG(" r10: 0x%x\n", frame->r10);
    DBG(" r11: 0x%x\n", frame->r11);
    DBG(" r12: 0x%x\n", frame->r12);
    DBG(" lr: 0x%x\n", frame->lr);
    DBG(" spsr: 0x%x\n", frame->spsr);
    DBG(" lr_saved: 0x%x\n", frame->lr_saved);
    DBG(" inst@lr_saved is 0x%x\n", readl(frame->lr_saved & (~0x03u)));
}

void default_exception_hdlr(expt_frame_p frame)
{
    dump_exception_ctx(frame);

    while (1);
}
