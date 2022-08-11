/*
* app_spi.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: sample code for spi master client
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <lib/console.h>
#include "spi_hal_master.h"
#include "res.h"
#include "chip_res.h"
#include "hal_dio.h"
#include "hal_port.h"
#include <kernel/event.h>
#include "irq.h"

static int spi_demo_func(int argc, const cmd_args *argv)
{
    int ret = 0;
    void *spi_handle = NULL;
    spidev_t spidev;

    if (!hal_spi_creat_handle(&spi_handle, RES_SPI_SPI5)) {
        printf(" %s, get spi handle fail\n", __func__);
        return -1;
    }

    spidev.slave_num = 0;
    spidev.speed_hz = 10000000;
    spidev.bit_mode = SPI_MODE_0;
    /* bits_per_word = 8,poll_mode = 0 */
    spidev.bits_per_word = 8;
    spidev.poll_mode = 0;
    uint8_t buf1 = 0xaa;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        printf("%s, init fail\n", __func__);
        return -1;
    }

    if (hal_spi_write(spi_handle, (void *)(&buf1),  sizeof(buf1)) != 0) {
        printf("%s, write fail\n", __func__);
        return -1;
    }

    if (hal_spi_read(spi_handle, (void *)(&buf1),  sizeof(buf1)) != 0) {
        printf("%s, read fail\n", __func__);
        return -1;
    }

    hal_spi_release_handle(spi_handle);
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("spi_demo",
                                    "spi_demo",
                                    (console_cmd)&spi_demo_func)
STATIC_COMMAND_END(spi_demo);
#endif

