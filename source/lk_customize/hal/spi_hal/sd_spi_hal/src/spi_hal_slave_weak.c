//*****************************************************************************
//
// spi_hal_slave_weak.c - Driver for the spi hal slave Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <string.h>
#include <debug.h>
#include <stdlib.h>

#include "spi_hal_slave.h"

void hal_spi_slave_prepare(void *handle)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return;
}
void hal_spi_slave_wait_event(void *handle)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return;
}
void hal_spi_slave_init_buf(uint8_t **client_rbuf, uint8_t **client_tbuf)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return;
}

bool hal_spi_slave_creat_handle(void **handle, uint32_t spi_res_id)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return false;
}

bool hal_spi_slave_release_handle(void *handle)
{
    dprintf(ALWAYS, "%s weak function\n", __func__);
    return false;
}

