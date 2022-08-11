#ifndef _PLAT_GIC_H_
#define _PLAT_GIC_H_

#include <soc.h>

/* Priority levels for ARM platforms */
#define PLAT_RAS_PRI                0x10
#define PLAT_SDEI_CRITICAL_PRI      0x60
#define PLAT_SDEI_NORMAL_PRI        0x70

#define ARM_IRQ_SEC_SGI_0       8
#define ARM_IRQ_SEC_SGI_1       9
#define ARM_IRQ_SEC_SGI_2       10
#define ARM_IRQ_SEC_SGI_3       11
#define ARM_IRQ_SEC_SGI_4       12
#define ARM_IRQ_SEC_SGI_5       13
#define ARM_IRQ_SEC_SGI_6       14
#define ARM_IRQ_SEC_SGI_7       15

#define ARM_G1S_IRQ_PROPS(grp) \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE)
/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
  * terminology. On a GICv2 system or mode, the lists will be merged and treated
   * as Group 0 interrupts.
    */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
         ARM_G1S_IRQ_PROPS(grp), \
         INTR_PROP_DESC(0, GIC_HIGHEST_SEC_PRIORITY, (grp), \
                         GIC_INTR_CFG_LEVEL), \
         INTR_PROP_DESC(1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
                         GIC_INTR_CFG_LEVEL)

#define ARM_G0_IRQ_PROPS(grp) \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp), \
            GIC_INTR_CFG_EDGE), \
    INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
            GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G0_IRQ_PROPS(grp)  ARM_G0_IRQ_PROPS(grp)

#define PLATFORM_CORE_COUNT 1

#define PLAT_ARM_GICD_BASE  (GIC_RBASE + 0x1000u)
#define PLAT_ARM_GICC_BASE  (GIC_RBASE + 0x2000u)

static inline unsigned int plat_my_core_pos(void)
{
    return 0;
}

#endif  /* _PLAT_GIC_H_ */
