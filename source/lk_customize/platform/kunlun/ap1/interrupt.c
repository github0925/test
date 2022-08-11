#include <reg.h>
#include <err.h>
#include <stdio.h>
#include <trace.h>
#include <platform/interrupts.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <irq_v.h>
#include <__regs_base.h>
#include <target_res.h>
#include <arm_gic_hal.h>

#define INTERRUPT_DEBUG_LEVEL ALWAYS

void register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
    void* gic_handle = NULL;
    if(false == hal_arm_gic_create_handle(&gic_handle,GIC_RES_ID) )
    {
        dprintf(INTERRUPT_DEBUG_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else
    {
        hal_arm_gic_register_interrupt(gic_handle, vector, GIC_INTERRUPT_PRI_LOWEST, handler, arg);
        hal_arm_gic_release_handle(gic_handle);
    }

    return;
}

status_t mask_interrupt(unsigned int vector)
{
    void* gic_handle = NULL;
    status_t ret = -1;
    if(false == hal_arm_gic_create_handle(&gic_handle,GIC_RES_ID) )
    {
        ret = -1;
        dprintf(INTERRUPT_DEBUG_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else
    {
        ret = hal_arm_gic_disable_interrupt(gic_handle, vector);
        hal_arm_gic_release_handle(gic_handle);
    }

    return ret;
}
status_t unmask_interrupt(unsigned int vector)
{
    void* gic_handle = NULL;
    status_t ret = -1;
    if(false == hal_arm_gic_create_handle(&gic_handle,GIC_RES_ID) )
    {
        ret = -1;
        dprintf(INTERRUPT_DEBUG_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else
    {
        ret = hal_arm_gic_enable_interrupt(gic_handle, vector);
        hal_arm_gic_release_handle(gic_handle);
    }

    return ret;
}

void arm_gic_init_early(void)
{
    void* gic_handle = NULL;

    if(false == hal_arm_gic_create_handle(&gic_handle,GIC_RES_ID) )
    {
        dprintf(INTERRUPT_DEBUG_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else
    {
        hal_arm_gic_init(gic_handle);
        hal_arm_gic_release_handle(gic_handle);
    }

    return;
}
