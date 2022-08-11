//*****************************************************************************
//
// spi_hal.c - Driver for the spi master hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "spi_hal_master.h"
#include "spi_hal_master_internal.h"

static bool spictrl_inited = false;

spin_lock_t spi_spinlock = SPIN_LOCK_INITIAL_VALUE;

static spi_instance_t g_spiInstance[MAX_SPI_DEVICE_NUM] = {0};
static spictrl_t spictrl_info[SPI_RES_NUM] = {0};

spi_instance_t *t_spiInstance = g_spiInstance; //for spi native test only
spictrl_t *t_spictrl = spictrl_info; //for spi native test only
int spiloop_mode = 0; //for spi native test only

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

static int hal_spi_transfer(spi_instance_t *instance, spi_transfer_t *msg)
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

    if (spiloop_mode == 1)
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
        } while (instance->spi_buffer.rx_end > instance->spi_buffer.rx_buf);

        spictrl->spiDrvApiTable->spi_enable(base, false);
        spictrl->spiDrvApiTable->spi_set_cs(base, false,
                                            instance->spidev.slave_num);
        status = 0;
    }

    spictrl->instance = NULL;
    mutex_release(&spictrl->spi_mutex);
    return status;
}

int hal_spi_parallel_rw(void *handle, void *tbuf, void *rbuf, uint32_t len)
{
    spi_instance_t *instance = (spi_instance_t *)handle;
    spi_transfer_t msg = {
        .tx_buf = tbuf,
        .rx_buf = rbuf,
        .len    = len,
    };
    return hal_spi_transfer(instance, &msg);
}

int hal_spi_write(void *handle, void *buf, uint32_t len)
{
    spi_instance_t *instance = (spi_instance_t *)handle;
    spi_transfer_t msg = {
        .tx_buf = buf,
        .len    = len,
    };
    return hal_spi_transfer(instance, &msg);
}

int hal_spi_read(void *handle, void *buf, uint32_t len)
{
    spi_instance_t *instance = (spi_instance_t *)handle;
    spi_transfer_t msg = {
        .rx_buf = buf,
        .len    = len,
    };
    return hal_spi_transfer(instance, &msg);
}

static void spi_reset_chip(spictrl_t *spictrl)
{
    spictrl->spiDrvApiTable->spi_enable(spictrl->base, false);
    spictrl->spiDrvApiTable->spi_mask_irq(spictrl->base, 0xff);
    spictrl->spiDrvApiTable->spi_enable(spictrl->base, true);
}

static enum handler_return spi_int_handler(void *arg)
{
    spictrl_t *spictrl = (spictrl_t *)arg;
    addr_t base = spictrl->base;
    spi_instance_t *instance = spictrl->instance;
    u16 irq_status = spictrl->spiDrvApiTable->spi_irq_status(base);

    if (!(irq_status & 0x3f)) {
        instance->status = -1;
        event_signal(&spictrl->t_completed, false);
        return INT_RESCHEDULE;
    }

    /* Error handling */
    if (irq_status & (SPI_INT_TXOI | SPI_INT_RXOI | SPI_INT_RXUI)) {
        spictrl->spiDrvApiTable->spi_clear_irq(base);
        spi_reset_chip(spictrl);
        instance->status = -1;
        event_signal(&spictrl->t_completed, false);
        return INT_RESCHEDULE;
    }

    data_reader(instance);

    if (instance->spi_buffer.rx_end == instance->spi_buffer.rx_buf) {
        spictrl->spiDrvApiTable->spi_mask_irq(base, SPI_INT_TXEI);
        instance->status = 0;
        event_signal(&spictrl->t_completed, false);
        return INT_RESCHEDULE;
    }

    if (irq_status & SPI_INT_TXEI) {
        spictrl->spiDrvApiTable->spi_mask_irq(base, SPI_INT_TXEI);
        data_writer(instance);
        /* Enable TX irq always, it will be disabled when RX finished */
        spictrl->spiDrvApiTable->spi_umask_irq(base, SPI_INT_TXEI);
    }

    return INT_NO_RESCHEDULE;
}

static uint32_t spi_hw_init(spictrl_t *spictrl)
{
    uint32_t fifo;
    spi_reset_chip(spictrl);

    for (fifo = 1; fifo < 256; fifo++) {
        spictrl->spiDrvApiTable->spi_write_txftl(spictrl->base, fifo);

        if (fifo != spictrl->spiDrvApiTable->spi_read_txftl(spictrl->base))
            break;
    }

    spictrl->spiDrvApiTable->spi_write_txftl(spictrl->base, 0);
    return (fifo == 1) ? 0 : fifo;
}

int32_t hal_spi_init(void *handle, spidev_t *info)
{
    spi_instance_t *instance = (spi_instance_t *)handle;
    spictrl_t *spictrl = instance->spictrl;

    if (info->slave_num > spictrl->max_slave_num || info->slave_num < 0) {
        dprintf(ALWAYS, "%s invalid slave num %d\n", __func__, info->slave_num);
        return -1;
    }
    else {
        instance->spidev.slave_num = info->slave_num;
    }

    if (!info->bits_per_word
            || !(spictrl->bpw_mask & BIT(info->bits_per_word - 1))) {
        dprintf(ALWAYS, "%s invalid bit per word %d\n", __func__,
                info->bits_per_word);
        return -1;
    }
    else {
        instance->spidev.bits_per_word = info->bits_per_word;
    }

    if (info->speed_hz > spictrl->max_speed_hz) {
        instance->spidev.speed_hz = spictrl->max_speed_hz;
    }
    else {
        instance->spidev.speed_hz = info->speed_hz;
    }

    instance->clk_div =
        (DIV_ROUND_UP(spictrl->max_speed_hz,
                      instance->spidev.speed_hz) + 1) & 0xfffe;
    instance->spidev.bit_mode = info->bit_mode;
    instance->spidev.poll_mode = info->poll_mode;
    dprintf(ALWAYS, "%s end\n", __func__);
    return 0;
}

static spi_instance_t *hal_spi_get_instance(uint32_t spi_res_id)
{
    addr_t paddr;
    int32_t bus_idx;
    int32_t res_idx;
    int32_t ins_idx;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&spi_spinlock, states);

    if (!spictrl_inited) {
        for (int i = 0, j = 0; i < MAX_SPI_CTRL_NUM; i++) {
            if (!res_get_info_by_id(spictrl_res[i].id, &paddr, &bus_idx)
                    && spictrl_res[i].spi_opmode == SPI_CTRL_MASTER) {
                spictrl_info[j].id = spictrl_res[i].id;
                spictrl_info[j].irq = spictrl_res[i].irq;
                spictrl_info[j].spi_opmode = spictrl_res[i].spi_opmode;
                spictrl_info[j].max_speed_hz = MAX_SPI_FREQ;
                spictrl_info[j].max_slave_num = MAX_SPI_SLAVE_NUM;
                spictrl_info[j].bpw_mask = SPI_VALID_MASK;
                spictrl_info[j].base = p2v(paddr);
                spictrl_info[j].bus_index = bus_idx - 1;
                hal_spi_get_drv_api_interface(&spictrl_info[j].spiDrvApiTable);
                spictrl_info[j].fifo_len = spi_hw_init(&spictrl_info[j]);
                mutex_init(&spictrl_info[j].spi_mutex);
                event_init(&(spictrl_info[j].t_completed), false, EVENT_FLAG_AUTOUNSIGNAL);
                j++;
            }
        }

        spictrl_inited = true;
    }

    for (res_idx = 0; res_idx < SPI_RES_NUM; res_idx++) {
        if (spi_res_id == spictrl_info[res_idx].id)
            break;
    }

    if (res_idx == SPI_RES_NUM) {
        dprintf(ALWAYS, "spi get instance fail,spi_res_id = 0x%x\n", spi_res_id);
        spin_unlock_irqrestore(&spi_spinlock, states);
        return NULL;
    }

    if (spictrl_info[res_idx].spi_opmode == SPI_CTRL_SLAVE) {
        dprintf(ALWAYS, "Warning: spi controller %d is slave mode\n", bus_idx - 1);
        spin_unlock_irqrestore(&spi_spinlock, states);
        return NULL;
    }

    if (!spictrl_info[res_idx].inited) {
        register_int_handler(spictrl_info[res_idx].irq, &spi_int_handler,
                             (void *)(&spictrl_info[res_idx]));
        unmask_interrupt(spictrl_info[res_idx].irq);
        spictrl_info[res_idx].inited = true;
    }

    for (ins_idx = 0; ins_idx < MAX_SPI_DEVICE_NUM; ins_idx++) {
        if (g_spiInstance[ins_idx].occupied == true) {
            continue;
        }
        else {
            g_spiInstance[ins_idx].spictrl = &spictrl_info[res_idx];
            g_spiInstance[ins_idx].occupied = true;
            spin_unlock_irqrestore(&spi_spinlock, states);
            return &g_spiInstance[ins_idx];
        }
    }

    dprintf(ALWAYS, "spi get instance fail for full\n");
    spin_unlock_irqrestore(&spi_spinlock, states);
    return NULL;
}

static void hal_spi_release_instance(spi_instance_t *instance)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&spi_spinlock, states);

    if (instance->occupied == true)
        instance->occupied = false;

    spin_unlock_irqrestore(&spi_spinlock, states);
    return;
}

bool hal_spi_creat_handle(void **handle, uint32_t spi_res_id)
{
    spi_instance_t  *instance = NULL;
    SPI_HAL_ASSERT_PARAMETER(handle);
    instance = hal_spi_get_instance(spi_res_id);

    if (instance == NULL) {
        dprintf(ALWAYS, "%s instance null\n", __func__);
        return false;
    }

    *handle = instance;
    return true;
}

bool hal_spi_release_handle(void *handle)
{
    spi_instance_t  *instance = NULL;
    SPI_HAL_ASSERT_PARAMETER(handle);
    instance = (spi_instance_t *)handle;
    hal_spi_release_instance(instance);
    return true;
}
