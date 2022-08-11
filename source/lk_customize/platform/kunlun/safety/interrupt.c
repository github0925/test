#include <reg.h>
#include <err.h>
#include <stdio.h>
#include <trace.h>
#include <lk_wrapper.h>
#include <platform/interrupts.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <irq.h>
#include <__regs_base.h>
#include <target_res.h>
#include <arm_gic_hal.h>

/******************* Macro Definition *******************/
#define INTERRUPT_DEBUG_LEVEL INFO
#define INTERRUPT_ERROR_LEVEL ALWAYS


/* Be careful to enable bottom half model.
 * Bottom-half will fowarding all registered handler to task level to be
 * executed. For this reason, any contiguous trigger level from interrupt should
 * not use this model.
 */
#define USE_BOTTOM_HALF_MODEL   0

#define DEFAULT_INTERRUPT_PRIORITY GIC_INTERRUPT_PRI_LOWEST
/********************************************************/

/******************* Internal Data structure *******************/
enum InterruptMode
{
    INT_MODE_PASS_THROUGH,
    INT_MODE_BOTTOM_HALF,
};

#if USE_BOTTOM_HALF_MODEL

typedef struct InterruptMeta_t
{
    uint16_t ulVector : 10;
    uint16_t ucPriority : 5;
    uint16_t ulMode : 1;
}InterruptMeta_t;

static struct InterruptControlBlock_t
{
    InterruptMeta_t usMeta;
    int_handler pxHandler;
    void* pvArg;

} InterruptControlBlock[MAX_INT+1];

static spin_lock_t ICB_lock = SPIN_LOCK_INITIAL_VALUE;

/************************************************************************/

InterruptMeta_t pvPrivilegedInterruptTable[] =
/********************************** Pass Through IRQ table **********************************/
{
    /* kernel tick handler following freertos, pass through to freertos IRQ handler. */
    {INT_KERNEL_TICK, DEFAULT_INTERRUPT_PRIORITY,INT_MODE_PASS_THROUGH},
    {MAX_INT+1, 0,INT_MODE_PASS_THROUGH},
};
/********************************************************************************************/


static void vSetInterruptControlBlock(struct InterruptControlBlock_t* ICB, void* pvHandler, void* pvArg, uint32_t vector)
{
    if(!ICB)
        return;

    ICB->usMeta.ulVector = vector;
    ICB->pxHandler = pvHandler;
    ICB->pvArg = pvArg;
    ICB->usMeta.ucPriority = DEFAULT_INTERRUPT_PRIORITY;
    ICB->usMeta.ulMode = INT_MODE_BOTTOM_HALF;

    InterruptMeta_t* pPrivilegedInterrupt = &pvPrivilegedInterruptTable[0];

    while(pPrivilegedInterrupt->ulVector < (MAX_INT+1))
    {
        if(pPrivilegedInterrupt->ulVector == vector)
        {
            ICB->usMeta.ulMode = INT_MODE_PASS_THROUGH;
            ICB->usMeta.ucPriority = pPrivilegedInterrupt->ucPriority;
            break;
        }

        pPrivilegedInterrupt++;
    }

    return;
}

/*************** Bottom-Half scetion of interrupt handler ***************/
/* This function is to impl Semidrive bottom-half section callback
 * This function is called in task context.
 */

static void vInterruptBottomHalf(void* parameter1, uint32_t parameter2)
{
    struct InterruptControlBlock_t* ICB = (struct InterruptControlBlock_t*)parameter1;
    /* avoid commpiler warning */
    (void)parameter2;

    dprintf(INTERRUPT_DEBUG_LEVEL,
                "Bottom-half %u :start excute handler %p arg %p.\n",ICB->usMeta.ulVector,ICB->pxHandler,ICB->pvArg);

    if(ICB->pxHandler)
        ICB->pxHandler(ICB->pvArg);

}

/*************** Top-Half scetion of interrupt handler ***************/
/* This function is to register Semidrive bottom-half section
 * to FreeRTOS daemon task, and do context switch if needed.
 * This function is called in interrupt context.
 * If the handler is pass-through, handler should own its resched
 * state.
 */



static enum handler_return uInterruptTopHalf(void* arg)
{
    struct InterruptControlBlock_t* ICB = (struct InterruptControlBlock_t*)arg;

    if(ICB->usMeta.ulMode == INT_MODE_BOTTOM_HALF)
    /* We defer to call the interrupt within task context.*/
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        /* Pass ulVector as r2 for debug */
        xTimerPendFunctionCallFromISR(vInterruptBottomHalf,
                                    ICB,
                                    ICB->usMeta.ulVector,
                                    &xHigherPriorityTaskWoken);


        dprintf(INTERRUPT_DEBUG_LEVEL,
            "Fowarding bottom-half INT:%u.\n",ICB->usMeta.ulVector);

        /* Switch to bottom section task (timer task) */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    /* We invoke the interrupt handler directly. handler owner
     * should maintain its resched state.
     */
    else if(ICB->usMeta.ulMode == INT_MODE_PASS_THROUGH)

    {
        if(ICB->pxHandler)
            ICB->pxHandler(ICB->pvArg);
    }

    return INT_RESCHEDULE;

}

#endif


void register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
    void* gic_handle = NULL;
    if(false == hal_arm_gic_create_handle(&gic_handle,GIC_RES_ID) )
    {
        dprintf(INTERRUPT_ERROR_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else if(vector > MAX_INT)
    {
        dprintf(INTERRUPT_ERROR_LEVEL,
            "Vector to be registered is overflowed:%d\n",vector);
            return;
    }
    else
    {
#if USE_BOTTOM_HALF_MODEL
        struct InterruptControlBlock_t* ICB = &InterruptControlBlock[vector];

        spin_lock_saved_state_t states;

        spin_lock_irqsave(&ICB_lock,states);
        vSetInterruptControlBlock(ICB,handler,arg,vector);
        spin_unlock_irqrestore(&ICB_lock,states);

        hal_arm_gic_register_interrupt(gic_handle, ICB->usMeta.ulVector, ICB->usMeta.ucPriority, uInterruptTopHalf, ICB);
#else
        hal_arm_gic_register_interrupt(gic_handle, vector, DEFAULT_INTERRUPT_PRIORITY, handler, arg);
#endif
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
        dprintf(INTERRUPT_ERROR_LEVEL,
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
        dprintf(INTERRUPT_ERROR_LEVEL,
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
        dprintf(INTERRUPT_ERROR_LEVEL,
                "Create GIC handle fail. RES id:0x%x\n",GIC_RES_ID);
    }
    else
    {
        hal_arm_gic_init(gic_handle);
        hal_arm_gic_release_handle(gic_handle);
    }

    return;
}

/* FreeRTOS IRQ implementation. */
void vApplicationIRQHandler(uint32_t ulICCIAR)
{
    arm_gic_platform_irq_spotless(ulICCIAR);

    return;
}
