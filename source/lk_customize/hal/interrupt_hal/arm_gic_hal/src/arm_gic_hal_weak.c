//*****************************************************************************
//
// arm_gic_hal_weak.c - hal stub of arm gic module if driver excluded.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <arm_gic_hal.h>

bool hal_arm_gic_create_handle(void **handle, uint32_t res_glb_idx)
{
    return true;
}

bool hal_arm_gic_release_handle(void *handle)
{
    return true;
}

bool hal_arm_gic_init(void *handle)
{
    return true;
}

bool hal_arm_gic_enable_interrupt(void *handle, uint32_t irq_number)
{
    return true;
}

bool hal_arm_gic_disable_interrupt(void *handle, uint32_t irq_number)
{
    return true;
}

bool hal_arm_gic_register_interrupt(void *handle, uint32_t irq_number, uint8_t priority, int_handler irq_handler, void* arg)
{
    return true;
}

bool hal_arm_gic_set_interrupt_priority(void *handle, uint32_t irq_number, uint8_t priority)
{
    return true;
}

bool hal_arm_gic_get_interrupt_priority(void *handle, uint32_t irq_number, uint8_t* priority)
{
    return true;
}

bool hal_arm_gic_register_interrupt(void *handle,
    uint32_t irq_number,
    uint8_t priority,
    int_handler irq_handler,
    void* arg)
{
    return true;
}

bool hal_arm_gic_set_trigger_mode(void *handle,uint32_t irq_number, enum ARM_GIC_INTERRUPT_TRIGGER_MODE mode)
{
    return true;
}