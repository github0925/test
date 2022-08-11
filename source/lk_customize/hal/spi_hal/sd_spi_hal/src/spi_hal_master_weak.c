//*****************************************************************************
//
// spi_hal.c - Driver for the spi hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <sys/types.h>
#include <string.h>
#include <platform.h>
#include <kernel/event.h>
#include <stdlib.h>

#include "spi_hal_master.h"

int hal_spi_parallel_rw(void *handle, void *tbuf, void *rbuf, uint32_t len)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return -1;
}

int hal_spi_write(void *handle, void *buf, uint32_t len)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return -1;
}

int hal_spi_read(void *handle, void *buf, uint32_t len)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return -1;
}

int32_t hal_spi_init(void *handle, spidev_t *info)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return -1;
}

bool hal_spi_creat_handle(void **handle, uint32_t spi_res_id)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return false;
}

bool hal_spi_release_handle(void *handle)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return false;
}

