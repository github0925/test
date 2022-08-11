//*****************************************************************************
//
// spi_hal_slave.h - Driver for the spi controller slave mode hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPI_HAL_S_INTERNAL_H__
#define __SPI_HAL_S_INTERNAL_H__
#ifdef __cplusplus
extern "C"
{
#endif


#include "spi_hal_common.h"
#include "hal_dio.h"
#include "hal_port.h"

#define SPI_SLAVE_BPW 8

#define SPI_CPHA        0x01
#define SPI_CPOL        0x02
#define SPI_MODE_0      (0|0)
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)

typedef struct __spictrl_slave {
    uint32_t id;
    uint32_t irq;
    paddr_t paddr;
    vaddr_t base;
    int32_t bus_index;
    uint32_t spi_opmode;
    uint32_t fifo_len;
    scr_signal_t spi_scr_signal;
    event_t t_completed;
    const spi_drv_api_interface_t *spiDrvApiTable;
    uint32_t spi_dma_res;
    struct dma_dev_cfg cfg_rx;
    struct dma_chan *rx_chan;
    struct dma_desc *desc_rx;
    struct dma_dev_cfg cfg_tx;
    struct dma_chan *tx_chan;
    struct dma_desc *desc_tx;
    bool inited;
} spictrl_slave_t;


#ifdef __cplusplus
}
#endif
#endif
