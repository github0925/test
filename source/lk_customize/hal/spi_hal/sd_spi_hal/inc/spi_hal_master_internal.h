//*****************************************************************************
//
// spi_hal.h - Driver for the spi hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPI_HAL_M_INTERNAL_H__
#define __SPI_HAL_M_INTERNAL_H__
#ifdef __cplusplus
extern "C"
{
#endif


#include "spi_hal_common.h"

#undef BIT
#define BIT(nr) (1U << (nr))

#define MAX_SPI_DEVICE_NUM 8*2
#define MAX_SPI_FREQ       120000000
#define MAX_SPI_SLAVE_NUM  16

#define SPI_BPW_MASK(bits) BIT((bits) - 1)
#define SPI_VALID_MASK (SPI_BPW_MASK(8) | SPI_BPW_MASK(16))

typedef struct spi_instance spi_instance_t;
typedef struct spictrl spictrl_t;
typedef struct spi_transfer spi_transfer_t;

struct spi_transfer {
    void        *tx_buf;
    void        *tx_end;
    void        *rx_buf;
    void        *rx_end;
    uint32_t    len;
};

struct spi_instance {
    int32_t clk_div;
    bool occupied;
    int32_t status;
    spidev_t spidev;
    spictrl_t *spictrl;
    spi_transfer_t spi_buffer;
};

struct spictrl {
    uint32_t id;
    uint32_t irq;
    int32_t max_speed_hz;
    int32_t max_slave_num;
    int32_t bpw_mask;
    addr_t base;
    int32_t bus_index;
    uint32_t spi_opmode;
    uint32_t fifo_len;
    event_t t_completed;
    mutex_t spi_mutex;
    const spi_drv_api_interface_t *spiDrvApiTable;
    spi_instance_t *instance;
    bool inited;
};


#ifdef __cplusplus
}
#endif
#endif
