//*****************************************************************************
//
// arm_gic_hal.c - hal of arm gic module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <arm_gic_hal.h>
#include <assert.h>


/********** Implementation **********/


bool hal_arm_gic_create_handle(void **phandle, uint32_t res_glb_idx)
{
    return true;
}

bool hal_arm_gic_release_handle(void *handle)
{
    return true;
}

bool hal_arm_gic_init(void *handle)
{
    spin_lock_t init_lock = SPIN_LOCK_INITIAL_VALUE;
    spin_lock_saved_state_t state;
    static bool initialized = false;

    spin_lock_irqsave(&init_lock,state);

    if(false == initialized)
    {
        arm_gic_init();
        initialized = true;
    }

    spin_unlock_irqrestore(&init_lock,state);

    return true;
}

bool hal_arm_gic_igroup_init(void *handle)
{
    arm_gic_igroup_init();
    return true;
}

bool hal_arm_gic_enable_interrupt(void *handle, uint32_t irq_number)
{
    bool ret = false;

    ret = (arm_gic_unmask_interrupt(irq_number) == 0 ? true : false );

    return ret;

}

bool hal_arm_gic_disable_interrupt(void *handle, uint32_t irq_number)
{
    bool ret = false;

    ret = (arm_gic_mask_interrupt(irq_number) == 0 ? true : false );

    return ret;
}


bool hal_arm_gic_set_interrupt_priority(void *handle, uint32_t irq_number, uint8_t priority)
{
    arm_gic_set_priority(irq_number, priority);

    return true;
}

bool hal_arm_gic_get_interrupt_priority(void *handle, uint32_t irq_number, uint8_t* priority)
{

    *priority = arm_gic_get_priority(irq_number);

    return true;
}

bool hal_arm_gic_register_interrupt(void *handle,
    uint32_t irq_number,
    uint8_t priority,
    int_handler irq_handler,
    void* arg)
{
    arm_gic_register_int_handler(irq_number, irq_handler, arg);
    arm_gic_set_priority(irq_number, priority);

    return true;
}


bool hal_arm_gic_set_trigger_mode(void *handle,uint32_t irq_number, enum ARM_GIC_INTERRUPT_TRIGGER_MODE mode)
{

    arm_gic_set_trigger_mode(irq_number, mode);

    return true;
}
