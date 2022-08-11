/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
*sfety domain spi1\spi2\spi3\spi4
*/
#include <lib/console.h>
#include <assert.h>
#include <bits.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <lib/slt_module_test.h>
#include "spi_hal_master.h"
#include "spi_hal_master_internal.h"
#include <lib/slt_module_test.h>

typedef enum {

    SPI1 = RES_SPI_SPI1,
    SPI2 = RES_SPI_SPI2,
    SPI3 = RES_SPI_SPI3,
    SPI4 = RES_SPI_SPI4,
    SPI_MAX = 4,
} SPI_e;

typedef struct {
#define BUF_LEN 1
    uint8_t  channel;
    uint32_t reg;
    spidev_t dev;
    void *txbuf;
    void *rxbuf;
    void *handle;
} spi_ip_t;

typedef struct {
    mutex_t  mutex;
    spi_ip_t *spi_ip;
} spi_context_t;

static bool spi_loopback = true;

static void data_writer(spi_instance_t *instance)
{
    uint32_t tx_left, tx_room, rxtx_gap, max;
    uint32_t n_bytes = instance->spidev.bits_per_word / 8;
    spictrl_t *spictrl = instance->spictrl;
    addr_t base = spictrl->base;
    tx_left = (instance->spi_buffer.tx_end - instance->spi_buffer.tx_buf) /
              n_bytes;
    tx_room = spictrl->fifo_len - spictrl->spiDrvApiTable->spi_read_txfl(base);
    /*
     * Another concern is about the tx/rx mismatch, we though to use (dws->fifo_len - rxflr - txflr) as
     * one maximum value for tx, but it doesn't cover the data which is out of tx/rx fifo and inside the
     * shift registers. So a control from sw point of view is taken.
     */
    rxtx_gap =
        ((instance->spi_buffer.rx_end - instance->spi_buffer.rx_buf)
         - (instance->spi_buffer.tx_end - instance->spi_buffer.tx_buf)
        ) / n_bytes;
    max = MIN(MIN(tx_left, tx_room), (spictrl->fifo_len - rxtx_gap));
    uint16_t txw = 0;

    while (max--) {
        /* Set the tx word if the transfer's original "tx" is not null */
        if (instance->spi_buffer.tx_end - instance->spi_buffer.len) {
            if (n_bytes == 1)
                txw = *(u8 *)(instance->spi_buffer.tx_buf);
            else
                txw = *(u16 *)(instance->spi_buffer.tx_buf);
        }

        spictrl->spiDrvApiTable->spi_write_data(base, txw);
        instance->spi_buffer.tx_buf += n_bytes;
    }
}

static void data_reader(spi_instance_t *instance)
{
    uint32_t n_bytes = instance->spidev.bits_per_word / 8;
    spictrl_t *spictrl = instance->spictrl;
    addr_t base = spictrl->base;
    uint32_t rx_left = (instance->spi_buffer.rx_end -
                        instance->spi_buffer.rx_buf) / n_bytes;
    uint32_t max = MIN(rx_left, spictrl->spiDrvApiTable->spi_read_rxfl(base));
    uint16_t rxw;

    while (max--) {
        rxw = spictrl->spiDrvApiTable->spi_read_data(base);

        /* Care rx only if the transfer's original "rx" is not null */
        if (instance->spi_buffer.rx_end - instance->spi_buffer.len) {
            if (n_bytes == 1)
                *(u8 *)(instance->spi_buffer.rx_buf) = rxw;
            else
                *(u16 *)(instance->spi_buffer.rx_buf) = rxw;
        }

        instance->spi_buffer.rx_buf += n_bytes;
    }
}

static int slt_spi_transfer(spi_instance_t *instance, spi_transfer_t *msg)
{
    uint32_t ctrl0 = 0;
    uint32_t txlevel = 0;
    uint32_t imask = 0;
    int32_t status = 0;
    spictrl_t *spictrl = instance->spictrl;
    addr_t base = spictrl->base;
    mutex_acquire(&spictrl->spi_mutex);

    if (msg->len % (instance->spidev.bits_per_word / 8)) {
        dprintf(ALWAYS, "%s,err: len%%[bits_per_word/8] not 0\n", __func__);
        mutex_release(&spictrl->spi_mutex);
        return -1;
    }

    spictrl->instance = instance;
    instance->spi_buffer.rx_buf = msg->rx_buf;
    instance->spi_buffer.rx_end = msg->rx_buf + msg->len;
    instance->spi_buffer.tx_buf = msg->tx_buf;
    instance->spi_buffer.tx_end = msg->tx_buf + msg->len;
    instance->spi_buffer.len = msg->len;
    spictrl->spiDrvApiTable->spi_enable(base, false);
    spictrl->spiDrvApiTable->spi_set_cs(base, true,
                                        instance->spidev.slave_num);
    spictrl->spiDrvApiTable->spi_set_clk(base, instance->clk_div);
    ctrl0 = (instance->spidev.bits_per_word - 1) << SPI_DFS32_OFFSET
            | (SPI_FRF_SPI << SPI_FRF_OFFSET)
            | (instance->spidev.bit_mode << SPI_MODE_OFFSET)
            | (SPI_TMOD_TR << SPI_TMOD_OFFSET);

    if (spi_loopback == true)
        ctrl0 = ctrl0 | (1 << SPI_SRL_OFFSET);

    spictrl->spiDrvApiTable->spi_write_ctrl0(base, ctrl0);
    spictrl->spiDrvApiTable->spi_mask_irq(base, 0xff);

    if (!instance->spidev.poll_mode) {
        txlevel = MIN(spictrl->fifo_len / 2,
                      instance->spi_buffer.len / (instance->spidev.bits_per_word / 8));
        spictrl->spiDrvApiTable->spi_write_txftl(base, txlevel);
        /* Set the interrupt mask */
        imask |= SPI_INT_TXEI | SPI_INT_TXOI | SPI_INT_RXUI | SPI_INT_RXOI;
        spictrl->spiDrvApiTable->spi_umask_irq(base, imask);
        spictrl->spiDrvApiTable->spi_enable(base, true);
        event_wait(&spictrl->t_completed);
        status = spictrl->instance->status;
        spictrl->spiDrvApiTable->spi_enable(base, false);
        spictrl->spiDrvApiTable->spi_set_cs(base, false,
                                            instance->spidev.slave_num);
    }
    else {
        spictrl->spiDrvApiTable->spi_enable(base, true);

        do {
            data_writer(instance);
            data_reader(instance);
            mb();
        }
        while (instance->spi_buffer.rx_end > instance->spi_buffer.rx_buf);

        spictrl->spiDrvApiTable->spi_enable(base, false);
        spictrl->spiDrvApiTable->spi_set_cs(base, false,
                                            instance->spidev.slave_num);
        status = 0;
    }

    spictrl->instance = NULL;
    mutex_release(&spictrl->spi_mutex);
    return status;
}

int slt_spi_parallel_rw(void *handle, void *tbuf, void *rbuf, uint32_t len)
{
    spi_instance_t *instance = (spi_instance_t *)handle;
    spi_transfer_t msg = {
        .tx_buf = tbuf,
        .rx_buf = rbuf,
        .len    = len,
    };
    return slt_spi_transfer(instance, &msg);
}

static void *slt_spi_malloc(void)
{
    void *sbuf = malloc(BUF_LEN);
    return sbuf;
}

static int slt_spi_checkout_result(spi_context_t *spi_context)
{
    int ret = -1;
    uint8_t tx = *((uint8_t *)spi_context->spi_ip->txbuf);
    uint8_t rx = *((uint8_t *)spi_context->spi_ip->rxbuf);
    uint8_t ch = spi_context->spi_ip->channel;

    mutex_acquire(&spi_context->mutex);

    if (tx == rx) {
        printf("spi%d pass\n", ch);
    }
    else {
        printf("spi%d fail\n", ch);
        goto out;
    }

    ret = 0;

out:
    mutex_release(&spi_context->mutex);
    return ret;
}

static int slt_spi_init(void **spi_handle, spidev_t *spidev, void **txbuf,
                        void **rxbuf, uint32_t spi_reg)
{
    int ret = -1;

    if (!hal_spi_creat_handle(spi_handle, spi_reg)) {
        printf("get spi handle fail\n");
        goto out;
    }

    spidev->slave_num = 0;
    spidev->speed_hz = 10000000;
    spidev->bit_mode = SPI_MODE_0;
    /* bits_per_word = 8,poll_mode = 0 */
    spidev->bits_per_word = 8;
    spidev->poll_mode = 0;

    if ((*txbuf = slt_spi_malloc()) == NULL) {
        printf("slt_spi_malloc txbuf fail\n");
        goto out;
    }

    if ((*rxbuf = slt_spi_malloc()) == NULL) {
        printf("slt_spi_malloc rxbuf fail\n");
        goto out;
    }

    ret = 1;
out:
    return ret;
}

static int slt_spi_internal_ip_diagnose(spi_context_t *spi_context)
{
    int ret = -1;
    uint8_t tx = *((uint8_t *)spi_context->spi_ip->txbuf);
    uint8_t ch = spi_context->spi_ip->channel;

    mutex_acquire(&spi_context->mutex);
    ret = slt_spi_parallel_rw(spi_context->spi_ip->handle,
                              spi_context->spi_ip->txbuf, spi_context->spi_ip->rxbuf, BUF_LEN);
    dprintf(ALWAYS, "slt spi%d, tx = %d, rx = %d\n", ch, tx,
            *((uint8_t *)spi_context->spi_ip->rxbuf));
    mutex_release(&spi_context->mutex);
    return ret;
}

int TEST_SAFE_SS_9(uint times, uint timeout, char *result_string)
{
    int ret = -1;
    spi_context_t spi_context;
    spi_ip_t spi_ip[SPI_MAX] = {
        [0] = {
            .channel = 1,
            .reg = SPI1,
        },
        [1] = {
            .channel = 2,
            .reg = SPI2,
        },
        [2] = {
            .channel = 3,
            .reg = SPI3,
        },
        [3] = {
            .channel = 4,
            .reg = SPI4,
        }
    };

    mutex_init(&spi_context.mutex);

    for (uint8_t idx = 0; idx < SPI_MAX; idx++) {
        if (slt_spi_init(&spi_ip[idx].handle, &spi_ip[idx].dev, &spi_ip[idx].txbuf,
                         &spi_ip[idx].rxbuf, spi_ip[idx].reg) < 0) {
            dprintf(ALWAYS, "slt spi init fail\n");
            goto out;
        }

        if (hal_spi_init(spi_ip[idx].handle, &spi_ip[idx].dev) < 0) {
            dprintf(ALWAYS, "hal spi init fail\n");
            goto out;
        }

        *((uint8_t *)spi_ip[idx].txbuf) = spi_ip[idx].channel;
    }

    for (uint8_t idx = 0; idx < SPI_MAX; idx++) {
        spi_context.spi_ip = &spi_ip[idx];

        if (slt_spi_internal_ip_diagnose(&spi_context) < 0) {
            dprintf(ALWAYS, "slt_spi_internal_ip_diagnose fail\n");
            goto out;
        }
    }

    for (uint8_t idx = 0; idx < SPI_MAX; idx++) {
        spi_context.spi_ip = &spi_ip[idx];

        if (slt_spi_checkout_result(&spi_context) < 0 ) {
            dprintf(ALWAYS, "slt_spi_checkout_result fail\n");
            goto out;
        }
    }

    ret = 0;
out:

    for (uint8_t idx = 0; idx < SPI_MAX; idx++) {
        if (spi_ip[idx].txbuf) {
            free(spi_ip[idx].txbuf);
            spi_ip[idx].txbuf = NULL;
        }

        if (spi_ip[idx].rxbuf) {
            free(spi_ip[idx].rxbuf);
            spi_ip[idx].rxbuf = NULL;
        }
    }

    return ret;
}

SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_9, TEST_SAFE_SS_9,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);
