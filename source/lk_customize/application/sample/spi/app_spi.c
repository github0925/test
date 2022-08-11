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

static int spi_to_normal(int argc, const cmd_args *argv)
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
    uint8_t buf1 = 0xff;

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

    printf("%s, int mode buf1 = %u\n", __func__, buf1);
    /* bits_per_word = 8,poll_mode = 1 */
    spidev.bits_per_word = 8;
    spidev.poll_mode = 1;
    buf1 = 0xf0;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        printf("%s, init fail\n", __func__);
        return -1;
    }

    hal_spi_write(spi_handle, (void *)(&buf1),  sizeof(buf1));
    hal_spi_read(spi_handle, (void *)(&buf1),  sizeof(buf1));
    printf(" %s, poll mode buf1 = %u\n", __func__, buf1);
    /* bits_per_word = 16,poll_mode = 0 */
    spidev.bits_per_word = 16;
    spidev.poll_mode = 0;
    uint16_t buf2 = 0xffff;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        printf("%s, init fail\n", __func__);
        return -1;
    }

    if (hal_spi_write(spi_handle, (void *)(&buf2),  sizeof(buf2)) != 0) {
        printf("%s, write fail\n", __func__);
        return -1;
    }

    if (hal_spi_read(spi_handle, (void *)(&buf2),  sizeof(buf2)) != 0) {
        printf("%s, read fail\n", __func__);
        return -1;
    }

    printf("%s, int mode buf2 = %u\n", __func__, buf2);
    /* bits_per_word = 16,poll_mode = 1 */
    spidev.bits_per_word = 16;
    spidev.poll_mode = 1;
    buf2 = 0xf0f0;

    if (hal_spi_init(spi_handle, &spidev) != 0) {
        printf("%s, init fail\n", __func__);
        return -1;
    }

    hal_spi_write(spi_handle, (void *)(&buf2),  sizeof(buf2));
    hal_spi_read(spi_handle, (void *)(&buf2),  sizeof(buf2));
    printf("%s, poll mode buf2 = %u\n", __func__, buf2);
    /*
     *bits_per_word = 16,poll_mode = 1,check err condition
     *len should times of [bits_per_word / 8]
    */
    uint8_t buf3[3] = {0xff, 0xff, 0xff};

    if (hal_spi_write(spi_handle, (void *)(&buf3),
                      sizeof(buf3) / sizeof(buf3[0])) != 0) {
        printf("%s, check wrong write\n", __func__);
    }
    else {
        printf("%s, check wrong write fail\n", __func__);
        return -1;
    }

    printf("%s, all function ok\n", __func__);
    hal_spi_release_handle(spi_handle);
    return 0;
}

/*
* below code belong to cmdline "spi_to_special"
*/
typedef struct __spiclient_dev_t {
    void *spi_handle;
    void *dio_handle;
    event_t client_event;
    spidev_t spidev;
    uint32_t irq;
    uint8_t rbuf[512];
    uint8_t tbuf[512];
} spiclient_dev_t;
spiclient_dev_t spiclient_dev;

static enum handler_return spiclient_int_handler(void *arg)
{
    /*
    * TODO
    * 1. check gpio irq be triggered
    * 2. clear irq
    * 3. send event_signal
    */
    return INT_RESCHEDULE;
}

static int spiclient_read_data(spiclient_dev_t *spiclient_dev_p)
{
    event_wait(&spiclient_dev_p->client_event);

    if (hal_spi_read(spiclient_dev_p->spi_handle,
                     (void *)spiclient_dev_p->rbuf, 512) != 0) {
        printf("%s, read fail\n", __func__);
        return -1;
    }

    for (int i = 0; i < 512; i++)
        printf("%s, rbuf[%d]=%u\n", __func__, i, spiclient_dev_p->rbuf[i]);

    return 0;
}

static int spiclient_write_data(spiclient_dev_t *spiclient_dev_p)
{
    static uint8_t data = 0;
    data++;
    event_wait(&spiclient_dev_p->client_event);

    for (int i = 0; i < 256; i++)
        spiclient_dev_p->tbuf[i] = data;

    if (hal_spi_write(spiclient_dev_p->spi_handle,
                      (void *)spiclient_dev_p->tbuf, 512) != 0) {
        printf("%s, write fail\n", __func__);
        return -1;
    }

    return 0;
}

static int spi_to_special(int argc, const cmd_args *argv)
{
    spiclient_dev.spidev.slave_num = 0;
    spiclient_dev.spidev.speed_hz = 10000000;
    spiclient_dev.spidev.bits_per_word = 8;
    spiclient_dev.spidev.bit_mode = SPI_MODE_0;
    spiclient_dev.spidev.poll_mode = 0;
    spiclient_dev.irq = GPIO2_GPIO_INT2_NUM;

    if (!hal_spi_creat_handle(&spiclient_dev.spi_handle, RES_SPI_SPI5)) {
        printf(" %s, get spi handle fail\n", __func__);
        return -1;
    }

    if (hal_spi_init(spiclient_dev.spi_handle, &spiclient_dev.spidev) != 0) {
        printf("%s, init fail\n", __func__);
        return -1;
    }

    if (!hal_dio_creat_handle(&spiclient_dev.dio_handle, RES_GPIO_GPIO2)) {
        printf(" %s, get dio handle fail\n", __func__);
        hal_spi_release_handle(spiclient_dev.spi_handle);
        return -1;
    }

    event_init(&spiclient_dev.client_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    register_int_handler(spiclient_dev.irq,
                         &spiclient_int_handler, (void *)(&spiclient_dev));
    unmask_interrupt(spiclient_dev.irq);

    /*
    * TODO
    * enable gpio irq edge mode
    */

    while (1) {
        spiclient_write_data(&spiclient_dev);
        spiclient_read_data(&spiclient_dev);
    }

    hal_spi_release_handle(spiclient_dev.spi_handle);
    hal_dio_release_handle(spiclient_dev.dio_handle);
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("spi_to_normal",
                                    "spi controller to normal slave",
                                    (console_cmd)&spi_to_normal)
STATIC_COMMAND("spi_to_special", "spi controller to controller slave",
               (console_cmd)&spi_to_special)
STATIC_COMMAND_END(spimaster_client_example);
#endif

APP_START(spimaster_client_example)
.flags = 0,
APP_END
