/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
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
#ifndef __ARM_GIC_H
#define __ARM_GIC_H

#include <sys/types.h>
#include <platform/interrupts.h>

enum {
    /* Ignore cpu_mask and forward interrupt to all CPUs other than the current cpu */
    ARM_GIC_SGI_FLAG_TARGET_FILTER_NOT_SENDER = 0x1,
    /* Ignore cpu_mask and forward interrupt to current CPU only */
    ARM_GIC_SGI_FLAG_TARGET_FILTER_SENDER = 0x2,
    ARM_GIC_SGI_FLAG_TARGET_FILTER_MASK = 0x3,

    /* Only forward the interrupt to CPUs that has the interrupt configured as group 1 (non-secure) */
    ARM_GIC_SGI_FLAG_NS = 0x4,
};

void arm_gic_init(void);
status_t arm_gic_sgi(u_int irq, u_int flags, u_int cpu_mask);

void arm_gic_register_int_handler(unsigned int vector, int_handler handler, void *arg);
status_t arm_gic_mask_interrupt(unsigned int vector);
status_t arm_gic_unmask_interrupt(unsigned int vector);
enum handler_return arm_gic_platform_irq_spotless(unsigned int ack);
uint8_t arm_gic_get_priority(u_int irq);
status_t arm_gic_set_priority(u_int irq, uint8_t priority);
status_t arm_gic_set_trigger_mode(u_int irq, uint8_t mode);
void arm_gic_disable_all_interrupts(void);
void arm_gic_enable_all_interrupts(void);
void arm_gic_igroup_init(void);

#endif

