/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <debug.h>
#include "res.h"
#include "chip_res.h"
#include "spi_hal_test.h"
#include "dw_ssi_spi_uapi.h"

static spitest_res_t spitest_info[MAX_SPITEST_CTRL_NUM] = {
    {RES_SPI_SPI1, 0, false},
    {RES_SPI_SPI2, 0, false},
    {RES_SPI_SPI3, 0, false},
    {RES_SPI_SPI4, 0, false},
    {RES_SPI_SPI5, 0, false},
    {RES_SPI_SPI6, 0, false},
    {RES_SPI_SPI7, 0, false},
    {RES_SPI_SPI8, 0, false},
};

void spitest_dump_reg(addr_t base)
{
    dw_ssi_dump_regs(base);
}

void spitest_writedr(addr_t base, uint32_t val)
{
    dw_ssi_write_data(base, val);
}

uint32_t spitest_readdr(addr_t base)
{
    return dw_ssi_read_data(base);
}

void spitest_enable(addr_t base, bool enable)
{
    dw_ssi_enable(base, enable);
}

void spitest_set_clk(addr_t base, uint32_t div)
{
    dw_ssi_set_clk(base, div);
}

void spitest_set_cs(addr_t base, bool enable, uint32_t slave_num)
{
    dw_ssi_set_cs(base, enable, slave_num);
}

void spitest_mask_irq(addr_t base, uint32_t mask)
{
    dw_ssi_mask_irq(base, mask);
}

void spitest_umask_irq(addr_t base, uint32_t mask)
{
    dw_ssi_umask_irq(base, mask);
}

uint16_t spitest_get_irq_stat(addr_t base)
{
    return dw_ssi_irq_status(base);
}

void spitest_clear_irq(addr_t base)
{
    dw_ssi_clear_irq(base);
}

void spitest_get_res(spitest_res_t **spitest_info_p)
{
    for (int i = 0; i < MAX_SPITEST_CTRL_NUM; i++) {
        addr_t paddr;
        int32_t bus_idx;

        if (!res_get_info_by_id(spitest_info[i].id, &paddr, &bus_idx)) {
            spitest_info[i].base = paddr;
            spitest_info[i].valid = true;
        }
    }
	*spitest_info_p = spitest_info;
}
