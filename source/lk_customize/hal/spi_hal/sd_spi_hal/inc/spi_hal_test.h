/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __SPI_HAL_TEST_H__
#define __SPI_HAL_TEST_H__
#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_SPITEST_CTRL_NUM   8

typedef struct spitest_res {
    const uint32_t id;
    addr_t base;
    bool valid;
} spitest_res_t;

void spitest_dump_reg(addr_t base);
void spitest_writedr(addr_t base, uint32_t val);
uint32_t spitest_readdr(addr_t base);
void spitest_enable(addr_t base, bool enable);
void spitest_set_clk(addr_t base, uint32_t div);
void spitest_set_cs(addr_t base, bool enable, uint32_t slave_num);
void spitest_mask_irq(addr_t base, uint32_t mask);
void spitest_umask_irq(addr_t base, uint32_t mask);
uint16_t spitest_get_irq_stat(addr_t base);
void spitest_clear_irq(addr_t base);
void spitest_get_res(spitest_res_t **spitest_info);


#ifdef __cplusplus
}
#endif
#endif

