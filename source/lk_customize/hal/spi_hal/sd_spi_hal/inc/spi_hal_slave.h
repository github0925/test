//*****************************************************************************
//
// spi_hal_slave.h - Driver for the spi controller slave mode hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPI_HAL_SLAVE_H__
#define __SPI_HAL_SLAVE_H__
#ifdef __cplusplus
extern "C"
{
#endif


#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define SPISLAVE_BUF_SIZE 512

void hal_spi_slave_prepared(void *handle);
void hal_spi_slave_wait_event(void *handle);
void hal_spi_slave_init_buf(uint8_t **client_rbuf, uint8_t **client_tbuf);
bool hal_spi_slave_creat_handle(void **handle, uint32_t spi_res_id);
bool hal_spi_slave_release_handle(void *handle);

#ifdef __cplusplus
}
#endif
#endif
