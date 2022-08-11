//*****************************************************************************
//
// spi_hal_common.h - Driver for the spi master and slave hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPI_HAL_COMMON__
#define __SPI_HAL_COMMON__
#ifdef __cplusplus
extern "C"
{
#endif


#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <lib/reg.h>
#include <stdlib.h>

#include "dma_hal.h"
#include "chip_res.h"
#include "irq.h"
#include "res.h"
#include "domain_res_cnt.h"
#include "scr_hal.h"
#include "dw_ssi_spi_uapi.h"

#define LOCAL_TRACE 1

/* Check the arguments. */
#define SPI_HAL_ASSERT_PARAMETER(handle)  \
    if(handle == NULL){ \
        LTRACEF("paramenter error handle:%p\n",handle);    \
        return false;   \
    }   \

#define MAX_SPI_CTRL_NUM   8

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET      0
#define SPI_DFS32_OFFSET    16

#define SPI_FRF_OFFSET      4
#define SPI_FRF_SPI         0x0
#define SPI_FRF_SSP         0x1
#define SPI_FRF_MICROWIRE   0x2
#define SPI_FRF_RESV        0x3

#define SPI_MODE_OFFSET     6
#define SPI_SCPH_OFFSET     6
#define SPI_SCOL_OFFSET     7

#define SPI_TMOD_OFFSET     8
#define SPI_TMOD_TR         0x0
#define SPI_TMOD_TO         0x1
#define SPI_TMOD_RO         0x2
#define SPI_TMOD_EPROMREAD  0x3

#define SPI_SRL_OFFSET      11

/* Bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI        (1 << 0)
#define SPI_INT_TXOI        (1 << 1)
#define SPI_INT_RXUI        (1 << 2)
#define SPI_INT_RXOI        (1 << 3)
#define SPI_INT_RXFI        (1 << 4)
#define SPI_INT_MSTI        (1 << 5)

/* DMA set interface*/
#define SPI_DMA_RXEN       0x1
#define SPI_DMA_TXEN       0x2

#define SPI_CTRL_MASTER 0
#define SPI_CTRL_SLAVE 1

#if SPI1_CTRL_SLAVE_MODE
#define SPI1_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI1_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI2_CTRL_SLAVE_MODE
#define SPI2_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI2_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI3_CTRL_SLAVE_MODE
#define SPI3_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI3_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI4_CTRL_SLAVE_MODE
#define SPI4_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI4_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI5_CTRL_SLAVE_MODE
#define SPI5_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI5_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI6_CTRL_SLAVE_MODE
#define SPI6_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI6_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI7_CTRL_SLAVE_MODE
#define SPI7_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI7_CTRL_MS SPI_CTRL_MASTER
#endif

#if SPI8_CTRL_SLAVE_MODE
#define SPI8_CTRL_MS SPI_CTRL_SLAVE
#else
#define SPI8_CTRL_MS SPI_CTRL_MASTER
#endif

typedef struct spi_res {
    uint32_t id;
    uint32_t irq;
    uint32_t spi_opmode;
    uint32_t spi_dma_res;
    scr_signal_t spi_scr_signal;
} spi_res_t;
static const spi_res_t spictrl_res[MAX_SPI_CTRL_NUM] = {
    {RES_SPI_SPI1, SPI1_O_SSI_INTR_NUM, SPI1_CTRL_MS, DMA_PERI_SPI1, SCR_SAFETY__RW__spi1_i_opmode},
    {RES_SPI_SPI2, SPI2_O_SSI_INTR_NUM, SPI2_CTRL_MS, DMA_PERI_SPI2, SCR_SAFETY__RW__spi2_i_opmode},
    {RES_SPI_SPI3, SPI3_O_SSI_INTR_NUM, SPI3_CTRL_MS, DMA_PERI_SPI3, SCR_SAFETY__RW__spi3_i_opmode},
    {RES_SPI_SPI4, SPI4_O_SSI_INTR_NUM, SPI4_CTRL_MS, DMA_PERI_SPI4, SCR_SAFETY__RW__spi4_i_opmode},
    {RES_SPI_SPI5, SPI5_O_SSI_INTR_NUM, SPI5_CTRL_MS, DMA_PERI_SPI5, SCR_SEC__RW__spi5_i_opmode},
    {RES_SPI_SPI6, SPI6_O_SSI_INTR_NUM, SPI6_CTRL_MS, DMA_PERI_SPI6, SCR_SEC__RW__spi6_i_opmode},
    {RES_SPI_SPI7, SPI7_O_SSI_INTR_NUM, SPI7_CTRL_MS, DMA_PERI_SPI7, SCR_SEC__RW__spi7_i_opmode},
    {RES_SPI_SPI8, SPI8_O_SSI_INTR_NUM, SPI8_CTRL_MS, DMA_PERI_SPI8, SCR_SEC__RW__spi8_i_opmode},
};

/* spi driver interface structure */
typedef struct spi_drv_api_interface {
    void (*spi_dump_regs)(addr_t base);
    void (*spi_write_ctrl0)(addr_t base, uint32_t val);
    void (*spi_write_data)(addr_t base, uint32_t val);
    uint32_t (*spi_read_data)(addr_t base);
    void (*spi_write_txftl)(addr_t base, uint32_t val);
    uint32_t (*spi_read_txftl)(addr_t base);
    uint32_t (*spi_read_txfl)(addr_t base);
    uint32_t (*spi_read_rxfl)(addr_t base);
    void (*spi_mask_irq)(addr_t base, uint32_t mask);
    void (*spi_umask_irq)(addr_t base, uint32_t mask);
    uint16_t (*spi_irq_status)(addr_t base);
    void (*spi_clear_irq)(addr_t base);
    void (*spi_set_opmode)(addr_t base, uint32_t opmode);
    void (*spi_set_clk)(addr_t base, uint32_t div);
    void (*spi_set_cs)(addr_t base, bool enable, uint32_t slave_num);
    void (*spi_enable)(addr_t base, bool enable);
    void (*spi_set_dmac)(addr_t base, uint32_t value);
    void (*spi_set_dmarxdl)(addr_t base, uint32_t value);
    void (*spi_set_dmatxdl)(addr_t base, uint32_t value);
} spi_drv_api_interface_t;

/* spi driver interface */
static const spi_drv_api_interface_t spiDrvApiInterface = {
    dw_ssi_dump_regs,
    dw_ssi_write_ctrl0,
    dw_ssi_write_data,
    dw_ssi_read_data,
    dw_ssi_write_txftl,
    dw_ssi_read_txftl,
    dw_ssi_read_txfl,
    dw_ssi_read_rxfl,
    dw_ssi_mask_irq,
    dw_ssi_umask_irq,
    dw_ssi_irq_status,
    dw_ssi_clear_irq,
    dw_ssi_set_opmode,
    dw_ssi_set_clk,
    dw_ssi_set_cs,
    dw_ssi_enable,
    dw_ssi_set_dmac,
    dw_ssi_set_dmarxdl,
    dw_ssi_set_dmatxdl,
};

static inline void hal_spi_get_drv_api_interface(const
        spi_drv_api_interface_t **spiDrvApiTable)
{
    *spiDrvApiTable = &spiDrvApiInterface;
}


#ifdef __cplusplus
}
#endif
#endif
