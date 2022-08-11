//*****************************************************************************
//
// dw_ssi_spi.c - Driver for the spi chipdev Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <debug.h>
#include <reg.h>

#include "dw_ssi_spi_uapi.h"
#include "dw_ssi_spi.h"

void dw_ssi_dump_regs(addr_t base)
{
    uint32_t ctrl0 = 0, ctrl1 = 0, ssien = 0, sen = 0, baud = 0;
    uint32_t txftl = 0, rxftl = 0, txfl = 0, rxfl = 0;
    uint32_t stat = 0, intm = 0, ints = 0, intc = 0;
    uint32_t dmac = 0, dmatdl = 0, dmardl = 0;
    ctrl0  = readl(base + DW_SSI_CTRLR0);
    ctrl1  = readl(base + DW_SSI_CTRLR1);
    ssien  = readl(base + DW_SSI_SSIENR);
    sen    = readl(base + DW_SSI_SER);
    baud   = readl(base + DW_SSI_BAUDR);
    txftl  = readl(base + DW_SSI_TXFTLR);
    rxftl  = readl(base + DW_SSI_RXFTLR);
    txfl   = readl(base + DW_SSI_TXFLR);
    rxfl   = readl(base + DW_SSI_RXFLR);
    stat   = readl(base + DW_SSI_SR);
    intm   = readl(base + DW_SSI_IMR);
    ints   = readl(base + DW_SSI_ISR);
    intc   = readl(base + DW_SSI_ICR);
    dmac   = readl(base + DW_SSI_DMACR);
    dmatdl = readl(base + DW_SSI_DMATDLR);
    dmardl = readl(base + DW_SSI_DMARDLR);
    dprintf(ALWAYS, "===dw_ssi_dump_regs start===\n");
    dprintf(ALWAYS, "===spi controller base %lx===\n", base);
    dprintf(ALWAYS,
            " ctrl0: 0x%08x\n ctrl1: 0x%08x\n ssien: 0x%08x\n sen  : 0x%08x\n baud : 0x%08x\n",
            ctrl0, ctrl1, ssien, sen, baud);
    dprintf(ALWAYS,
            " txftl: 0x%08x\n rxftl: 0x%08x\n txfl : 0x%08x\n rxfl : 0x%08x\n",
            txftl, rxftl, txfl, rxfl);
    dprintf(ALWAYS,
            " stat : 0x%08x\n intm : 0x%08x\n ints : 0x%08x\n intc : 0x%08x\n",
            stat, intm, ints, intc);
    dprintf(ALWAYS, " dmac  : 0x%08x\n dmatdl: 0x%08x\n dmardl: 0x%08x\n",
            dmac, dmatdl, dmardl);
    dprintf(ALWAYS, "===dw_ssi_dump_regs end===\n");
}

void dw_ssi_write_ctrl0(addr_t base, uint32_t val)
{
    writel(val, base + DW_SSI_CTRLR0);
}

void dw_ssi_write_data(addr_t base, uint32_t val)
{
    writel(val, base + DW_SSI_DR);
}

uint32_t dw_ssi_read_data(addr_t base)
{
    uint32_t val;
    val = readl(base + DW_SSI_DR);
    return val;
}

void dw_ssi_write_txftl(addr_t base, uint32_t val)
{
    writel(val, base + DW_SSI_TXFTLR);
}

uint32_t dw_ssi_read_txftl(addr_t base)
{
    uint32_t val;
    val = readl(base + DW_SSI_TXFTLR);
    return val;
}

uint32_t dw_ssi_read_txfl(addr_t base)
{
    uint32_t val;
    val = readl(base + DW_SSI_TXFLR);
    return val;
}

uint32_t dw_ssi_read_rxfl(addr_t base)
{
    uint32_t val;
    val = readl(base + DW_SSI_RXFLR);
    return val;
}

/* Disable IRQ bits */
void dw_ssi_mask_irq(addr_t base, uint32_t mask)
{
    uint32_t new_mask;
    new_mask = readl(base + DW_SSI_IMR) & ~mask;
    writel(new_mask, base + DW_SSI_IMR);
}

/* Enable IRQ bits */
void dw_ssi_umask_irq(addr_t base, uint32_t mask)
{
    uint32_t new_mask;
    new_mask = readl(base + DW_SSI_IMR) | mask;
    writel(new_mask, base + DW_SSI_IMR);
}

/* read IRQ status */
u16 dw_ssi_irq_status(addr_t base)
{
    u16 irq_status = 0;
    irq_status = readl(base + DW_SSI_ISR);
    return irq_status;
}

/* auto clear irq after read */
void dw_ssi_clear_irq(addr_t base)
{
    readl(base + DW_SSI_ICR);
}

void dw_ssi_set_opmode(addr_t base, uint32_t opmode)
{
    return;
}

void dw_ssi_set_clk(addr_t base, uint32_t div)
{
    writel(div, base + DW_SSI_BAUDR);
}

void dw_ssi_set_cs(addr_t base, bool enable, uint32_t slave_num)
{
    if (enable)
        writel(BIT(slave_num), base + DW_SSI_SER);
    else
        writel(0, base + DW_SSI_SER);
}

void dw_ssi_enable(addr_t base, bool enable)
{
    uint32_t val = 0x0;
    val = (enable) ? 0x1 : 0x0;
    writel(val, base + DW_SSI_SSIENR);
}

void dw_ssi_set_dmac(addr_t base, uint32_t value)
{
    writel(value, base + DW_SSI_DMACR);
}

void dw_ssi_set_dmarxdl(addr_t base, uint32_t value)
{
    writel(value, base + DW_SSI_DMARDLR);
}

void dw_ssi_set_dmatxdl(addr_t base, uint32_t value)
{
    writel(value, base + DW_SSI_DMATDLR);
}
