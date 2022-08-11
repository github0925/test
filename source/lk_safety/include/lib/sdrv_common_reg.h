/*
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __SDRV_COMMON_REG_H__
#define __SDRV_COMMON_REG_H__

#include <reg.h>
#include "__regs_base.h"
#include "lib/reg.h"

#ifndef SDRV_GENERAL_REG
#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + (u32)((0x50 + (n)*4) << 10))
#endif

#define SDRV_REG_ROM 0
#define SDRV_REG_REMAP 1
#define SDRV_REG_SDPE 2
#define SDRV_REG_MCAL 3
#define SDRV_REG_BOOTREASON 4
#define SDRV_REG_HWID 5
#define SDRV_REG_STATUS 6
#define SDRV_REG_UNUSED 7

/* Fields for SDRV_REG_STATUS Register */
/* The NOC has been initialized */
#define SDRV_REG_STATUS_NOC_INIT_DONE (1<<0)
/* The DDR has been initialized */
#define SDRV_REG_STATUS_DDR_INIT_DONE (1<<1)
/* The firewall early config done */
#define SDRV_REG_STATUS_FIREWALL_EARLY_DONE (1<<2)
/* The firewall formal config done */
#define SDRV_REG_STATUS_FIREWALL_CFG_DONE (1<<3)
/* The sdpe load done */
#define SDRV_REG_STATUS_SDPE_LD_DONE (1<<4)
/* The safety handover done */
#define SDRV_REG_STATUS_HANDOVER_DONE (1<<5)

static inline uint32_t sdrv_common_reg_get_u32(uint32_t reg)
{
    return readl(_ioaddr(SDRV_GENERAL_REG(reg)));
}

static inline void sdrv_common_reg_set_u32(uint32_t value, uint32_t reg)
{
    writel(value, _ioaddr(SDRV_GENERAL_REG(reg)));
}

static inline uint32_t sdrv_common_reg_get_value(uint32_t reg, uint32_t mask)
{
    return readl(_ioaddr(SDRV_GENERAL_REG(reg))) & mask;
}

static inline void sdrv_common_reg_set_value(uint32_t reg, uint32_t value,
        uint32_t mask)
{
    uint32_t tmp_value;
    tmp_value = readl(_ioaddr(SDRV_GENERAL_REG(reg))) & ~mask;
    tmp_value |= value & mask;
    writel(tmp_value, _ioaddr(SDRV_GENERAL_REG(reg)));
}

#endif
