/*
* arm_gic_hal.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: semidrive arm gic hal headfile
*
* Revision History:
* -----------------
* 011, 01/24/2019 wang yongjun implement this
*/

#ifndef __ARM_GIC_HAL_H__
#define __ARM_GIC_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif
#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include <res.h>
#include <chip_res.h>
#if ENABLE_ARM_GIC
#include "arm_gic.h"
#endif

/*! ****************************************************************************
* .DEFINE
*******************************************************************************/



/************* Data structure definition *************/


enum ARM_GIC_INTERRUPT_PRIORITY_LEVEL
{
    GIC_INTERRUPT_PRI_HIGHEST = 0,

    /* In ARM GIC Architecture spec, Priority
     * numerical value MUST be less than PMR
     * (priority mask register) could be signaled
     * to processor, which means if the PMR was set
     * to MAX value(0x8F(31D)) the lowest priority
     * interrupt will not be signaled to processor
     * as it's equal to PMR. As so in HAL, we provides
     * the lowest valid priority(could be served) as
     * Default PMR-1 to make the setting interrupt could
     * be served in anycases in default GIC PMR setting.*/
    GIC_INTERRUPT_PRI_LOWEST_NS = 14,

    GIC_INTERRUPT_PRI_LOWEST = 30,
};

enum ARM_GIC_INTERRUPT_TRIGGER_MODE
{
    GIC_INTERRUPT_LEVEL_ACTIVE = 0,
    GIC_INTERRUPT_EDGE_SENSITIVE  = 1,
};

/************* Data structure definition end *************/


/************* API declaration *************/

//*****************************************************************************
//
//! hal_arm_gic_create_handle.
//!
//! \phandle gic handle reference pointer.
//! \res_glb_idx global resource idx of gic.
//!
//! This function is to create a valid gic handle.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_create_handle(void **phandle, uint32_t res_glb_idx);

//*****************************************************************************
//
//! hal_arm_gic_release_handle.
//!
//! \handle gic handle.
//!
//! This function is to release a valid gic handle.
//!
//! \return true if succ, of false if invalid input handle.
//
//*****************************************************************************
bool hal_arm_gic_release_handle(void *handle);

//*****************************************************************************
//
//! hal_arm_gic_init.
//!
//! \handle gic handle.
//!
//! This function is to initialize gic module, which will be excuted only once.
//! Be care of using a created handle for input.
//!
//! \return true if succ, of false if invalid input handle.
//
//*****************************************************************************
bool hal_arm_gic_init(void *handle);

//*****************************************************************************
//
//! hal_arm_gic_enable_interrupt.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! This function is to enable a interrupt to be detectable.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_enable_interrupt(void *handle, uint32_t irq_number);

//*****************************************************************************
//
//! hal_arm_gic_disable_interrupt.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! This function is to disable a interrupt to be undetectable.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_disable_interrupt(void *handle, uint32_t irq_number);

//*****************************************************************************
//
//! hal_arm_gic_register_interrupt.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! \irq_handler interrupt callback implementation.
//!
//! \arg argument which need to pass to handler.
//!
//! This function is to register a callback to specific irq number.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_register_interrupt(void *handle,
    uint32_t irq_number,
    uint8_t priority,
    int_handler irq_handler,
    void* arg);

//*****************************************************************************
//
//! hal_arm_gic_get_interrupt_priority.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! \priority interrupt priority reference pointer.
//!
//! This function is to get the priority of the specific interrupt number.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_get_interrupt_priority(void *handle, uint32_t irq_number, uint8_t* priority);

//*****************************************************************************
//
//! hal_arm_gic_set_interrupt_priority.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! \priority interrupt priority to be set. Priority Must be set in the
//! range of GIC_INTERRUPT_PRI_HIGHEST to GIC_INTERRUPT_PRI_LOWEST. Numerical
//! value higher, priority lower.
//!
//! This function is to set the priority of the specific interrupt number.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_set_interrupt_priority(void *handle, uint32_t irq_number, uint8_t priority);

//*****************************************************************************
//
//! hal_arm_gic_set_trigger_mode.
//!
//! \handle gic handle.
//!
//! \irq_number interrupt number.
//!
//! \mode trigger mode to be set.
//!
//! This function is to set the trigger mode of the specific interrupt number.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_set_trigger_mode(void *handle,uint32_t irq_number, enum ARM_GIC_INTERRUPT_TRIGGER_MODE mode);

//*****************************************************************************
//
//! hal_arm_gic_igroup_init.
//!
//! \handle gic handle.
//!
//! This function is to clear igroup value.
//!
//! \return true if succ, of false if failed.
//
//*****************************************************************************
bool hal_arm_gic_igroup_init(void *handle);
/************* API declaration end *************/

#ifdef __cplusplus
}
#endif
#endif
