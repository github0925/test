//*****************************************************************************
//
// dw_ssi_spi_uapi.h - Driver for the spi chipdev Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************


#ifndef __DW_SSI_SPI_UAPI__
#define __DW_SSI_SPI_UAPI__

void dw_ssi_dump_regs(addr_t base);
void dw_ssi_write_ctrl0(addr_t base, uint32_t val);
void dw_ssi_write_data(addr_t base, uint32_t val);
uint32_t dw_ssi_read_data(addr_t base);
void dw_ssi_write_txftl(addr_t base, uint32_t val);
uint32_t dw_ssi_read_txftl(addr_t base);
uint32_t dw_ssi_read_txfl(addr_t base);
uint32_t dw_ssi_read_rxfl(addr_t base);
void dw_ssi_mask_irq(addr_t base, uint32_t mask);
void dw_ssi_umask_irq(addr_t base, uint32_t mask);
u16 dw_ssi_irq_status(addr_t base);
void dw_ssi_clear_irq(addr_t base);
void dw_ssi_set_opmode(addr_t base, uint32_t opmode);
void dw_ssi_set_clk(addr_t base, uint32_t div);
void dw_ssi_set_cs(addr_t base, bool enable, uint32_t slave_num);
void dw_ssi_enable(addr_t base, bool enable);
void dw_ssi_set_dmac(addr_t base, uint32_t value);
void dw_ssi_set_dmarxdl(addr_t base, uint32_t value);
void dw_ssi_set_dmatxdl(addr_t base, uint32_t value);

#endif
