/*
 * Copyright (c) 2014 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 *******************************************************/

#include <stdio.h>
#include <debug.h>
#include <helper.h>
#include <arch/arm64.h>

#define SHUTDOWN_ON_FATAL 1

struct fault_handler_table_entry {
    uint64_t pc;
    uint64_t fault_handler;
};

extern struct fault_handler_table_entry __fault_handler_table_start[];
extern struct fault_handler_table_entry __fault_handler_table_end[];

static void dump_iframe(const struct arm64_iframe_long *iframe)
{
    DBG("iframe %p:\n", iframe);
    DBG("x0  0x%p x1  0x%p x2  0x%p x3  0x%p\n", iframe->r[0], iframe->r[1], iframe->r[2],
        iframe->r[3]);
    DBG("x4  0x%p x5  0x%p x6  0x%p x7  0x%p\n", iframe->r[4], iframe->r[5], iframe->r[6],
        iframe->r[7]);
    DBG("x8  0x%p x9  0x%p x10 0x%p x11 0x%p\n", iframe->r[8], iframe->r[9], iframe->r[10],
        iframe->r[11]);
    DBG("x12 0x%p x13 0x%p x14 0x%p x15 0x%p\n", iframe->r[12], iframe->r[13], iframe->r[14],
        iframe->r[15]);
    DBG("x16 0x%p x17 0x%p x18 0x%p x19 0x%p\n", iframe->r[16], iframe->r[17], iframe->r[18],
        iframe->r[19]);
    DBG("x20 0x%p x21 0x%p x22 0x%p x23 0x%p\n", iframe->r[20], iframe->r[21], iframe->r[22],
        iframe->r[23]);
    DBG("x24 0x%p x25 0x%p x26 0x%p x27 0x%p\n", iframe->r[24], iframe->r[25], iframe->r[26],
        iframe->r[27]);
    DBG("x28 0x%p x29 0x%p lr  0x%p usp 0x%p\n", iframe->r[28], iframe->r[29], iframe->lr, iframe->usp);
    DBG("elr 0x%p\n", iframe->elr);
    DBG("spsr 0x%p\n", iframe->spsr);
}

__WEAK void arm64_syscall(struct arm64_iframe_long *iframe, bool is_64bit)
{
    DBG("unhandled syscall vector\n");

    while (1);
}

void arm64_sync_exception(struct arm64_iframe_long *iframe)
{
    uint32_t esr = ARM64_READ_SYSREG(esr_el1);
    uint32_t ec __attribute__((unused)) = BITS_SHIFT(esr, 31, 26);
    uint32_t il __attribute__((unused))= BIT(esr, 25);
    uint32_t iss __attribute__((unused))= BITS(esr, 24, 0);

    DBG("ESR 0x%x: ec 0x%x, il 0x%x, iss 0x%x\n", esr, ec, il, iss);
    dump_iframe(iframe);

    while (1);
}

void arm64_invalid_exception(struct arm64_iframe_long *iframe, unsigned int which)
{
    DBG("invalid exception, which 0x%x\n", which);
    dump_iframe(iframe);

    while (1);
}
