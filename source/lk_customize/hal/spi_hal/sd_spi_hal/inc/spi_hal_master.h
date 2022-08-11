//*****************************************************************************
//
// spi_hal_uapi.h - Driver for the spi hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPI_HAL_MASTER_H__
#define __SPI_HAL_MASTER_H__


#include <string.h>
#include <assert.h>
#include <stdlib.h>

typedef struct spidev {
    int32_t slave_num;
    int32_t speed_hz;
    int32_t bits_per_word;
    uint32_t bit_mode;
#define SPI_CPHA        0x01
#define SPI_CPOL        0x02
#define SPI_MODE_0      (0|0)
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)
    bool poll_mode;
} spidev_t;

int hal_spi_parallel_rw(void *handle, void *tbuf, void *rbuf,
                        uint32_t len);
int hal_spi_write(void *handle, void *buf, uint32_t len);
int hal_spi_read(void *handle, void *buf, uint32_t len);
int32_t hal_spi_init(void *handle, spidev_t *info);
bool hal_spi_creat_handle(void **handle, uint32_t spi_res_id);
bool hal_spi_release_handle(void *handle);


#endif