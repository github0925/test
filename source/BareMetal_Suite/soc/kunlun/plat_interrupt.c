/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#ifdef SHELL_USE_USB
#include <common_hdr.h>
#include <soc.h>
#include <include/drivers/arm/gicv2.h>
#include "plat_interrupt.h"

extern void plat_arm_gic_driver_init(void);
extern void plat_arm_gic_init(void);

static interrupt_hdlr_t hdlr_tbl[MAX_INTERRUPT_NUM];

int32_t plat_install_interrupt_hdlr(uint32_t id, void (*hdlr)(void))
{
    if (id < MAX_INTERRUPT_NUM) {
        hdlr_tbl[id].hdlr = hdlr;
    }

    return 0;
}

void plat_interrupt_hdlr(void)
{
    uint32_t id = gicv2_get_pending_interrupt_id();
    gicv2_acknowledge_interrupt();

    if (id < MAX_INTERRUPT_NUM
        && NULL != hdlr_tbl[id].hdlr) {
        hdlr_tbl[id].hdlr();
    }

    gicv2_end_of_interrupt(id);
}

void plat_interrupt_init(void)
{
    plat_arm_gic_driver_init();
    plat_arm_gic_init();
}

void plat_setup_interrupt(uint32_t id, uint32_t pri, void (*f)(void))
{
    gicv2_set_interrupt_type(id, GICV2_INTR_GROUP0);
    gicv2_set_interrupt_priority(id, pri);
    /* 00 - Level-sensitive; 1x - Edge-sensitive */
    gicv2_interrupt_set_cfg(id, 0);

    plat_install_interrupt_hdlr(id, f);
}

void plat_enable_interrupt(uint32_t id)
{
    gicv2_enable_interrupt(id);
}

void plat_disable_interrupt(uint32_t id)
{
    gicv2_disable_interrupt(id);
}

void vApplicationIRQHandler(uint32_t ulICCIAR)
{
    if (ulICCIAR < MAX_INTERRUPT_NUM
        && NULL != hdlr_tbl[ulICCIAR].hdlr) {
        hdlr_tbl[ulICCIAR].hdlr();
    }
}

#endif/* SHELL_USE_USB */