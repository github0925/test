/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: sample code for spi slave client
 *
 */

#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <app.h>

#include "spi_hal_slave.h"
#include "chip_res.h"

typedef struct __spiclient_t {
    void *spislave_handle;
    uint8_t *rbuf;
    uint8_t *tbuf;
} spiclient_t;
spiclient_t spiclient;

void spislave_client_handle_data(spiclient_t *spiclient_p)
{
    static uint8_t data = 0;
    data++;
    uint8_t client_rbuf[SPISLAVE_BUF_SIZE] = {0};
    uint8_t client_tbuf[SPISLAVE_BUF_SIZE] = {0};
    memcpy(client_rbuf, spiclient_p->rbuf, SPISLAVE_BUF_SIZE);

    for (int i = 0; i < SPISLAVE_BUF_SIZE; i++)
        dprintf(ALWAYS, "spislave client rbuf[%d]=%u\n", i, client_rbuf[i]);

    for (int i = 0; i < SPISLAVE_BUF_SIZE / 2; i++)
        client_tbuf[i] = data;

    memcpy(spiclient_p->tbuf, client_tbuf, SPISLAVE_BUF_SIZE);
}

void spislave_client_entry(const struct app_descriptor *app, void *args)
{
    hal_spi_slave_creat_handle(&spiclient.spislave_handle, RES_SPI_SPI5);
    hal_spi_slave_init_buf(&(spiclient.rbuf), &(spiclient.tbuf));

    while (1) {
        spislave_client_handle_data(&spiclient);
        hal_spi_slave_prepared(spiclient.spislave_handle);
        hal_spi_slave_wait_event(spiclient.spislave_handle);
    }
}

APP_START(spislave_client_example)
.entry = (app_entry)spislave_client_entry,
APP_END
