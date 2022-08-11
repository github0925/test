//*****************************************************************************
//
// spi_hal_slave.c - Driver for the spi controller slave mode hal Module.
//
// Copyright (c) 2020-2030 SemiDrive Incorporated. All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "spi_hal_slave.h"
#include "spi_hal_slave_internal.h"

spin_lock_t spislave_spinlock = SPIN_LOCK_INITIAL_VALUE;
static bool spislave_inited = false;
static spictrl_slave_t spictrl_slave[SPI_RES_NUM] = {0};

static uint8_t spislave_rbuf[ROUNDUP(SPISLAVE_BUF_SIZE, CACHE_LINE)]
__attribute__((aligned(CACHE_LINE))) = {0};
static uint8_t spislave_tbuf[ROUNDUP(SPISLAVE_BUF_SIZE, CACHE_LINE)]
__attribute__((aligned(CACHE_LINE))) = {0};

static enum handler_return spislave_int_handler(void *arg)
{
    spictrl_slave_t *spictrl = (spictrl_slave_t *)arg;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&spislave_spinlock, states);
    u16 irq_status = spictrl->spiDrvApiTable->spi_irq_status(spictrl->base);
    dprintf(ALWAYS, "%s irq enter 0x%x\n", __func__, irq_status);
    spictrl->spiDrvApiTable->spi_mask_irq(spictrl->base, 0xff);
    spin_unlock_irqrestore(&spislave_spinlock, states);
    return INT_RESCHEDULE;
}

static void spi_reset_chip(spictrl_slave_t *spictrl)
{
    spictrl->spiDrvApiTable->spi_enable(spictrl->base, false);
    spictrl->spiDrvApiTable->spi_mask_irq(spictrl->base, 0xff);
    spictrl->spiDrvApiTable->spi_enable(spictrl->base, true);
}

static uint32_t spi_hw_init(spictrl_slave_t *spictrl)
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

extern const domain_res_t g_gpio_res;
void hal_spi_slave_prepared(void *handle)
{
    spictrl_slave_t *spictrl = (spictrl_slave_t *)handle;
    void *dio_handle;
    hal_dma_terminate(spictrl->desc_rx);
    hal_dma_terminate(spictrl->desc_tx);
    hal_dma_submit(spictrl->desc_rx);
    hal_dma_submit(spictrl->desc_tx);
    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    ASSERT(dio_handle);
    hal_dio_write_channel(dio_handle, PortConf_PIN_OSPI2_DATA7, 1);
    spin(2);
    hal_dio_write_channel(dio_handle, PortConf_PIN_OSPI2_DATA7, 0);
    hal_dio_release_handle(dio_handle);
}

void hal_spi_slave_wait_event(void *handle)
{
    spictrl_slave_t *spictrl = (spictrl_slave_t *)handle;
    event_wait(&spictrl->t_completed);
}

void hal_spi_slave_init_buf(uint8_t **client_rbuf, uint8_t **client_tbuf)
{
    *client_rbuf = spislave_rbuf;
    *client_tbuf = spislave_tbuf;
}

void spislave_rx_irq_handle(enum dma_status status, u32 param,
                            void *context)
{
    spictrl_slave_t *spictrl = (spictrl_slave_t *)context;
    event_signal(&spictrl->t_completed, false);
    dprintf(ALWAYS, "%s status %d\n", __func__, status);
}

void spislave_tx_irq_handle(enum dma_status status, u32 param,
                            void *context)
{
    dprintf(ALWAYS, "%s status %d\n", __func__, status);
}

static spictrl_slave_t *hal_spi_slave_get_instance(uint32_t spi_res_id)
{
    addr_t paddr;
    int32_t bus_idx;
    int32_t res_idx;
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&spislave_spinlock, states);

    if (!spislave_inited) {
        for (int i = 0, j = 0; i < MAX_SPI_CTRL_NUM; i++) {
            if (!res_get_info_by_id(spictrl_res[i].id, &paddr, &bus_idx)
                    && spictrl_res[i].spi_opmode == SPI_CTRL_SLAVE) {
                spictrl_slave[j].id = spictrl_res[i].id;
                spictrl_slave[j].irq = spictrl_res[i].irq;
                spictrl_slave[j].spi_opmode = spictrl_res[i].spi_opmode;
                spictrl_slave[j].spi_dma_res = spictrl_res[i].spi_dma_res;
                spictrl_slave[j].spi_scr_signal = spictrl_res[i].spi_scr_signal;
                spictrl_slave[j].paddr = paddr;
                spictrl_slave[j].base = p2v(paddr);
                spictrl_slave[j].bus_index = bus_idx - 1;
                hal_spi_get_drv_api_interface(&spictrl_slave[j].spiDrvApiTable);
                spictrl_slave[j].fifo_len = spi_hw_init(&spictrl_slave[j]);
                event_init(&(spictrl_slave[j].t_completed), false,
                           EVENT_FLAG_AUTOUNSIGNAL);
                j++;
            }
        }

        spislave_inited = true;
    }

    for (res_idx = 0; res_idx < SPI_RES_NUM; res_idx++) {
        if (spi_res_id == spictrl_slave[res_idx].id)
            break;
    }

    if (res_idx == SPI_RES_NUM) {
        dprintf(ALWAYS, "spislave get instance fail,spi_res_id = 0x%x\n",
                spi_res_id);
        spin_unlock_irqrestore(&spislave_spinlock, states);
        return NULL;
    }

    if (spictrl_slave[res_idx].spi_opmode == SPI_CTRL_MASTER) {
        dprintf(ALWAYS, "Warning: spislave controller %d is master mode\n",
                bus_idx - 1);
        spin_unlock_irqrestore(&spislave_spinlock, states);
        return NULL;
    }

    if (!spictrl_slave[res_idx].inited) {
        uint32_t ctrl0 = 0;
        uint32_t imask = 0;
        uint32_t dmaen = 0;
        scr_handle_t spi_scr;
        spictrl_slave_t *spictrl = &spictrl_slave[res_idx];
        spi_scr = hal_scr_create_handle(spictrl->spi_scr_signal);
        hal_scr_set(spi_scr, 1);
        hal_scr_delete_handle(spi_scr);
        ctrl0 = ((SPI_SLAVE_BPW - 1) << SPI_DFS32_OFFSET)
                | (SPI_FRF_SPI << SPI_FRF_OFFSET)
                | (SPI_MODE_0 << SPI_MODE_OFFSET)
                | (SPI_TMOD_TR << SPI_TMOD_OFFSET);
        imask = SPI_INT_TXOI | SPI_INT_RXUI;
        dmaen = SPI_DMA_RXEN | SPI_DMA_TXEN;
        spictrl->spiDrvApiTable->spi_enable(spictrl->base, false);
        spictrl->spiDrvApiTable->spi_write_ctrl0(spictrl->base, ctrl0);
        spictrl->spiDrvApiTable->spi_mask_irq(spictrl->base, 0xff);
        spictrl->spiDrvApiTable->spi_set_dmac(spictrl->base, dmaen);
        spictrl->spiDrvApiTable->spi_set_dmarxdl(spictrl->base, 3);
        spictrl->spiDrvApiTable->spi_set_dmatxdl(spictrl->base, 4);
        spictrl->spiDrvApiTable->spi_umask_irq(spictrl->base, imask);
        spictrl->spiDrvApiTable->spi_enable(spictrl->base, true);
        register_int_handler(spictrl->irq, &spislave_int_handler, (void *)spictrl);
        unmask_interrupt(spictrl->irq);
        spictrl->cfg_rx.direction = DMA_DEV2MEM;
        spictrl->cfg_rx.src_addr = spictrl->paddr + 0x60;
        spictrl->cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        spictrl->cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        spictrl->cfg_rx.src_maxburst = DMA_BURST_TR_4ITEMS;
        spictrl->cfg_rx.dst_maxburst = DMA_BURST_TR_32ITEMS;
        spictrl->rx_chan = hal_dma_chan_req(spictrl->spi_dma_res);
        hal_dma_dev_config(spictrl->rx_chan, &spictrl->cfg_rx);
        spictrl->desc_rx = hal_prep_dma_dev(spictrl->rx_chan, spislave_rbuf,
                                            SPISLAVE_BUF_SIZE, DMA_INTERRUPT);
        spictrl->desc_rx->dmac_irq_evt_handle = spislave_rx_irq_handle;
        spictrl->desc_rx->context = (void *)spictrl;
        spictrl->cfg_tx.direction = DMA_MEM2DEV;
        spictrl->cfg_tx.dst_addr = spictrl->paddr + 0x60;
        spictrl->cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        spictrl->cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        spictrl->cfg_tx.src_maxburst = DMA_BURST_TR_32ITEMS;
        spictrl->cfg_tx.dst_maxburst = DMA_BURST_TR_4ITEMS;
        spictrl->tx_chan = hal_dma_chan_req(spictrl->spi_dma_res);
        hal_dma_dev_config(spictrl->tx_chan, &spictrl->cfg_tx);
        spictrl->desc_tx = hal_prep_dma_dev(spictrl->tx_chan, spislave_tbuf,
                                            SPISLAVE_BUF_SIZE, DMA_INTERRUPT);
        spictrl->desc_tx->dmac_irq_evt_handle = spislave_tx_irq_handle;
        spictrl->desc_tx->context = (void *)spictrl;
        spictrl->inited = true;
    }

    spin_unlock_irqrestore(&spislave_spinlock, states);
    return &spictrl_slave[res_idx];
}

bool hal_spi_slave_creat_handle(void **handle, uint32_t spi_res_id)
{
    spictrl_slave_t  *instance = NULL;
    SPI_HAL_ASSERT_PARAMETER(handle);
    instance = hal_spi_slave_get_instance(spi_res_id);

    if (instance == NULL) {
        dprintf(ALWAYS, "%s instance null\n", __func__);
        return false;
    }

    *handle = instance;
    return true;
}

bool hal_spi_slave_release_handle(void *handle)
{
    SPI_HAL_ASSERT_PARAMETER(handle);
    handle = NULL;
    return true;
}

