//*****************************************************************************
//
// spdif.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <spdif.h>
#include <spdif_ctrl_reg.h>
#include <math.h>
#include <irq.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <string.h>
#include <stdio.h>
#include <__regs_base.h>
#include <sys/types.h>
#include <kernel/event.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <lib/reg.h>

#define SPDIF_INT_STATUS_REG_BITS 0xffc00000
#define SPDIF_CLK 58980000
#define CAL_SPDIF_SAMPLERATE_CODE(SAMPLE_RATE) (round((1.0 * SPDIF_CLK) / (128 * SAMPLE_RATE) - 1))
static enum handler_return spdif_irq_handle(void *config);

void spdif_show_current_cfg(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;
    printf("spdif_ctrl 0x00: 0x%04x\n", base->ctrl.v);
    printf("tsamplerate:%d\nsfr_en:%d\nspdif_en:%d\nfifo_en:%d\n",
           base->ctrl.tsamplerate,
           base->ctrl.sfr_en,
           base->ctrl.spdif_en,
           base->ctrl.fifo_en);
    printf("clk_en:%d\ntr_mode:%d\nparity_check:%d\nparity_gen:%d\n",
           base->ctrl.clk_en,
           base->ctrl.tr_mode,
           base->ctrl.parity_check,
           base->ctrl.parity_gen);
    printf("validity_check:%d\nchannel_mode:%d\nduplicate:%d\nsetpreambb:%d\n",
           base->ctrl.validity_check,
           base->ctrl.channel_mode,
           base->ctrl.duplicate,
           base->ctrl.setpreambb);
    printf("use_fifo_if:%d\nparity_mask:%d\nunderr_mask:%d\novrerr_mask:%d\n",
           base->ctrl.use_fifo_if,
           base->ctrl.parity_mask,
           base->ctrl.underr_mask,
           base->ctrl.ovrerr_mask);
    printf("empty_mask:%d\naempty_mask:%d\nfull_mask:%d\nafull_mask:%d\n",
           base->ctrl.empty_mask,
           base->ctrl.aempty_mask,
           base->ctrl.full_mask,
           base->ctrl.afull_mask);
    printf("syncerr_mask:%d\nlock_mask:%d\nbegin_mask:%d\nintreq_mask:%d\n",
           base->ctrl.syncerr_mask,
           base->ctrl.lock_mask,
           base->ctrl.begin_mask,
           base->ctrl.intreq_mask);

    printf("int_reg    0x04: 0x%04x\n", base->interrput.v);
    printf("preamble delay:%d\n", base->interrput.preambledel);

    printf("fifo_ctrl  0x08: 0x%04x\n", base->fifo_ctrl.v);
    printf("aempty_threshold:%d\nafull_threshold:%d\n",
           base->fifo_ctrl.aempty_threshold,
           base->fifo_ctrl.afull_threshold);

    printf("fifo_stat  0x0c: 0x%04x\n", base->fifo_status.v);
}

bool spdif_int_register(spdif_top_cfg_t *cfg)
{
    cfg->is_added = 1;
    register_int_handler(cfg->interrupt_num, &spdif_irq_handle, (void *)cfg);
    // enable interrupt
    unmask_interrupt(cfg->interrupt_num);

    return true;
}

bool spdif_int_transmit(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;
    u32 wdata = 0;
    u32 num = SPDIF_FIFO_DEPTH - base->fifo_status.fifo_level;

    if (cfg->cfg_info.resolution == SPDIF_FORMAT_8BITS) {
        while (num > 0 && cfg->cfg_info.tx_count > 0) {
            wdata = (u32)(cfg->cfg_info.ptx_buffer[0] << 16);
            writel(wdata, cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
            num--;
            cfg->cfg_info.ptx_buffer++;
            cfg->cfg_info.tx_count--;
        }
    }
    else if (cfg->cfg_info.resolution == SPDIF_FORMAT_16BITS) {
        while ( num > 0 && cfg->cfg_info.tx_count > 0) {
            if (cfg->cfg_info.tx_count >= 2) {
                wdata = (u32)(cfg->cfg_info.ptx_buffer[0] << 8)
                        | ((u32)cfg->cfg_info.ptx_buffer[1] << 16);
                cfg->cfg_info.ptx_buffer += 2;
                cfg->cfg_info.tx_count -= 2;
            }
            else {
                wdata = (u32)(cfg->cfg_info.ptx_buffer[0] << 16);
                cfg->cfg_info.ptx_buffer++;
                cfg->cfg_info.tx_count--;
            }

            writel(wdata, cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
            num--;
        }
    }
    else if (cfg->cfg_info.resolution == SPDIF_FORMAT_20BITS
             || cfg->cfg_info.resolution == SPDIF_FORMAT_24BITS) {
        while ( num > 0 && cfg->cfg_info.tx_count > 0) {
            if (cfg->cfg_info.tx_count > 3) {
                wdata = (u32)(cfg->cfg_info.ptx_buffer[0]) |
                        ((u32)cfg->cfg_info.ptx_buffer[1] << 8) |
                        ((u32)cfg->cfg_info.ptx_buffer[2] << 16);
                cfg->cfg_info.ptx_buffer += 3;
                cfg->cfg_info.tx_count -= 3;
            }
            else if (cfg->cfg_info.tx_count >= 2) {
                wdata = (u32)(cfg->cfg_info.ptx_buffer[0] << 8) | (u32)(
                            cfg->cfg_info.ptx_buffer[1] << 16);
                cfg->cfg_info.ptx_buffer += 2;
                cfg->cfg_info.tx_count -= 2;
            }
            else {
                wdata = (u32)(cfg->cfg_info.ptx_buffer[0] << 16);
                cfg->cfg_info.ptx_buffer++;
                cfg->cfg_info.tx_count--;
            }

            writel(wdata, cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
            num--;
        }
    }

    if (cfg->cfg_info.tx_count == 0) {
        event_signal(&(cfg->tx_comp), false);
    }

    return true;
}

bool spdif_int_receive(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;
    u32 rdata = 0;
    u32 num = base->fifo_status.fifo_level;

    if (cfg->cfg_info.resolution == SPDIF_FORMAT_8BITS) {
        while (num > 0 && cfg->cfg_info.rx_count > 0) {
            rdata = (u32)(cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
            memcpy(cfg->cfg_info.prx_buffer, (char *)(&rdata) + 2, 1);
            num--;
            cfg->cfg_info.prx_buffer++;
            cfg->cfg_info.rx_count--;
        }
    }
    else if (cfg->cfg_info.resolution == SPDIF_FORMAT_16BITS) {
        while ((num > 0) && (cfg->cfg_info.rx_count > 0)) {
            if (cfg->cfg_info.rx_count >= 2) {
                rdata = readl(cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
                memcpy(cfg->cfg_info.prx_buffer, (char *)(&rdata) + 1, 2);
                cfg->cfg_info.prx_buffer += 2;
                cfg->cfg_info.rx_count -= 2;
            }
            else {
                cfg->cfg_info.rx_count = 0;
            }

            num--;
        }
    }
    else if (cfg->cfg_info.resolution == SPDIF_FORMAT_20BITS
             || cfg->cfg_info.resolution == SPDIF_FORMAT_24BITS) {
        while ((num > 0) && (cfg->cfg_info.rx_count > 0)) {
            if (cfg->cfg_info.rx_count > 3) {
                rdata = readl(cfg->base_addr + SPDIF_FIFO_ADDR_OFFSET);
                memcpy(cfg->cfg_info.prx_buffer, &rdata, 3);
                cfg->cfg_info.prx_buffer += 3;
                cfg->cfg_info.rx_count -= 3;
            }
            else {
                cfg->cfg_info.rx_count = 0;
            }

            num--;
        }
    }

    if (cfg->cfg_info.rx_count == 0) {
        dprintf(INFO, "spdif %d transfer(receive) done\n", cfg->bus + 1);
        event_signal(&(cfg->rx_comp), false);
    }

    return true;
}


static void spdif_interrupt_cfg(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    base->ctrl.underr_mask = 1;
    base->ctrl.ovrerr_mask = 1;
    base->ctrl.parity_mask = !!cfg->cfg_info.int_handle.parity_int_handle;
    base->ctrl.syncerr_mask = !!cfg->cfg_info.int_handle.syncerr_int_handle;
    base->ctrl.lock_mask = !!cfg->cfg_info.int_handle.lock_int_handle;
    base->ctrl.begin_mask = !!cfg->cfg_info.int_handle.begin_int_handle;

    if (cfg->cfg_info.transfer_mode == SPDIF_TR_WITH_DMA) {
        base->ctrl.empty_mask = 0;
        base->ctrl.aempty_mask = 0;
        base->ctrl.full_mask = 0;
        base->ctrl.afull_mask = 0;
    }
    else if (cfg->cfg_info.transfer_mode == SPDIF_TR_WITH_INT) {
        base->ctrl.empty_mask = !!cfg->cfg_info.int_handle.empty_int_handle;
        base->ctrl.aempty_mask = !!cfg->cfg_info.int_handle.aempty_int_handle;
        base->ctrl.full_mask = !!cfg->cfg_info.int_handle.full_int_handle;
        base->ctrl.afull_mask = !!cfg->cfg_info.int_handle.afull_int_handle;

        if (!cfg->cfg_info.int_handle.aempty_int_handle) {
            if ((cfg->cfg_info.tr_mode == SPDIF_TRANSMITTER)
                    && cfg->cfg_info.transfer_mode == SPDIF_TR_WITH_INT) {
                cfg->cfg_info.int_handle.aempty_int_handle = (void *)spdif_int_transmit;
                base->ctrl.aempty_mask = 1;
            }
        }

        if (!cfg->cfg_info.int_handle.afull_int_handle) {
            if ((cfg->cfg_info.tr_mode == SPDIF_RECEIVER)
                    && cfg->cfg_info.transfer_mode == SPDIF_TR_WITH_INT) {
                cfg->cfg_info.int_handle.afull_int_handle = (void *)spdif_int_receive;
                base->ctrl.afull_mask = 1;
            }
        }
    }
    else {
        dprintf(INFO, "spdif %d: unkown transfer mode.\n", cfg->bus + 1);
    }
}

bool spdif_init(spdif_top_cfg_t *cfg)
{

    spdif_int_register(cfg);
    event_init(&cfg->tx_comp, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&cfg->rx_comp, false, EVENT_FLAG_AUTOUNSIGNAL);

    return true;
}

// transmitter cfg
static void spdif_tx_config(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    base->ctrl.tr_mode = 1;
    base->ctrl.duplicate = cfg->cfg_info.duplicate;
    base->ctrl.parity_gen = cfg->cfg_info.parity;

    if (cfg->cfg_info.preamble_delay) {
        base->ctrl.setpreambb = 1;
        base->interrput.preambledel = cfg->cfg_info.preamble_delay - 1;
    }
    else {
        base->ctrl.setpreambb = 0;
        base->interrput.preambledel = 0;
    }

    // set tx threshold.
    if (cfg->cfg_info.aempty_threshold == SPDIF_FIFO_THRESHOLD_DEFAULT) {
        base->fifo_ctrl.aempty_threshold = SPDIF_FIFO_DEPTH * 3 / 4;
    }
    else if (cfg->cfg_info.aempty_threshold < SPDIF_FIFO_DEPTH) {
        base->fifo_ctrl.aempty_threshold = cfg->cfg_info.aempty_threshold;
    }
    else {
        dprintf(INFO, "spdif %d: wrong aempty_threshold.", cfg->bus + 1);
    }

    if (cfg->cfg_info.afull_threshold == SPDIF_FIFO_THRESHOLD_DEFAULT) {
        base->fifo_ctrl.afull_threshold = SPDIF_FIFO_DEPTH - 1;
    }
    else if (cfg->cfg_info.afull_threshold < SPDIF_FIFO_DEPTH) {
        base->fifo_ctrl.afull_threshold = cfg->cfg_info.afull_threshold;
    }
    else {
        dprintf(INFO, "spdif %d: wrong afull_threshold.", cfg->bus + 1);
    }
}

// receiver cfg
static void spdif_rx_config(spdif_top_cfg_t *cfg)
{

    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    base->ctrl.tr_mode = 0;
    base->ctrl.validity_check = cfg->cfg_info.validity;
    base->ctrl.parity_check = cfg->cfg_info.parity;

    // set rx threshold.
    if (cfg->cfg_info.aempty_threshold == SPDIF_FIFO_THRESHOLD_DEFAULT) {
        base->fifo_ctrl.aempty_threshold = 0;
    }
    else if (cfg->cfg_info.aempty_threshold < SPDIF_FIFO_DEPTH) {
        base->fifo_ctrl.aempty_threshold = cfg->cfg_info.aempty_threshold;
    }
    else {
        dprintf(INFO, "spdif %d: wrong aempty_threshold.\n", cfg->bus + 1);
    }

    if (cfg->cfg_info.afull_threshold == SPDIF_FIFO_THRESHOLD_DEFAULT) {
        base->fifo_ctrl.afull_threshold = SPDIF_FIFO_DEPTH / 2;
    }
    else if (cfg->cfg_info.afull_threshold < SPDIF_FIFO_DEPTH) {
        base->fifo_ctrl.afull_threshold = cfg->cfg_info.afull_threshold;
    }
    else {
        dprintf(INFO, "spdif %d: wrong afull_threshold.\n", cfg->bus + 1);
    }
}

// reset core and setup cfg, can perform recovery from power save mode as well.
bool spdif_config(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    // reset
    base->ctrl.sfr_en = 0;// reset sfr, auto set to 1.
    base->ctrl.spdif_en = 0;// write 1 to start spdif.
    base->ctrl.fifo_en = 0;// reset fifo, auto set to 1.
    base->ctrl.clk_en = 1;// power save mode.
    spin(100);
    base->ctrl.clk_en = 0;// enable clock.
    spin(100);

    if (cfg->cfg_info.tr_mode == SPDIF_TRANSMITTER) {
        spdif_tx_config(cfg);
    }
    else if (cfg->cfg_info.tr_mode == SPDIF_RECEIVER) {
        spdif_rx_config(cfg);
    }
    else {
        dprintf(INFO, "spdif %d: tx/rx mode not init.\n", cfg->bus + 1);
    }

    // common
    base->ctrl.tsamplerate = CAL_SPDIF_SAMPLERATE_CODE(
                                 cfg->cfg_info.tsample_rate);
    base->ctrl.use_fifo_if = cfg->cfg_info.use_fifo_if;

    if (cfg->cfg_info.ch_mode == SPDIF_STEREO) {
        base->ctrl.channel_mode = 0;
    }
    else if (cfg->cfg_info.ch_mode == SPDIF_MONO) {
        base->ctrl.channel_mode = 1;
    }
    else {
        dprintf(INFO, "spdif %d: channel mode not init.\n", cfg->bus + 1);
    }

    spdif_interrupt_cfg(cfg);

    return true;
}

bool spdif_start(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    base->ctrl.spdif_en = 1;
    base->ctrl.intreq_mask = 1;

    return true;
}

bool spdif_stop(spdif_top_cfg_t *cfg)
{
    spdif_reg *base = (spdif_reg *)cfg->base_addr;
    base->ctrl.spdif_en = 0;
    base->ctrl.intreq_mask = 0;

    event_destroy(&cfg->tx_comp);
    event_destroy(&cfg->rx_comp);

    return true;
}

// sleep after stop.
bool spdif_sleep(spdif_top_cfg_t *cfg)
{
    // dropped in power save mode.
    spdif_reg *base = (spdif_reg *)cfg->base_addr;

    if (base->ctrl.spdif_en == 0) {
        base->ctrl.clk_en = 1;
        spin(100);
        return true;
    }

    return false;
}

static enum handler_return spdif_irq_handle(void *config)
{
    spdif_top_cfg_t *cfg = (spdif_top_cfg_t *)config;
    bool resched = false;
    spdif_reg *base = (spdif_reg *)cfg->base_addr;
    // save and clear int status.
    spdif_fifo_stat_reg fifo_status = base->fifo_status;
    base->interrput.v &= (~SPDIF_INT_STATUS_REG_BITS);

    if (fifo_status.ovrerr_flag && base->ctrl.ovrerr_mask) {//1 overun
        if (cfg->cfg_info.int_handle.ovrerr_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.ovrerr_int_handle(cfg);

        dprintf(INFO, "spdif %d ovrerrun\n", cfg->bus + 1);
    }

    if (fifo_status.underr_flag && base->ctrl.underr_mask) {//2 underrun
        if (cfg->cfg_info.int_handle.underr_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.underr_int_handle(cfg);

        dprintf(INFO, "spdif %d underrun\n", cfg->bus + 1);
    }

    if (fifo_status.aempty_flag && base->ctrl.aempty_mask) {//3 aempty
        if (cfg->cfg_info.int_handle.aempty_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.aempty_int_handle(cfg);

        dprintf(INFO, "spdif %d aempty\n", cfg->bus + 1);
    }

    if (fifo_status.afull_flag && base->ctrl.afull_mask) {//4 afull
        if (cfg->cfg_info.int_handle.afull_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.afull_int_handle(cfg);

        dprintf(INFO, "spdif %d afull\n", cfg->bus + 1);
    }

    if (fifo_status.empty_flag && base->ctrl.empty_mask) {//5 empty
        if (cfg->cfg_info.int_handle.empty_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.empty_int_handle(cfg);

        dprintf(INFO, "spdif %d empty\n", cfg->bus + 1);
    }

    if (fifo_status.full_flag && base->ctrl.full_mask) {//6 full
        if (cfg->cfg_info.int_handle.full_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.full_int_handle(cfg);

        dprintf(INFO, "spdif %d full\n", cfg->bus + 1);
    }

    if (fifo_status.parity_flag && base->ctrl.parity_mask
            && base->ctrl.parity_check) {//7 parity
        if (cfg->cfg_info.int_handle.parity_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.parity_int_handle(cfg);

        dprintf(INFO, "spdif %d parity\n", cfg->bus + 1);
    }

    if (fifo_status.syncerr_flag && base->ctrl.syncerr_mask) {//8 syncerr
        if (cfg->cfg_info.int_handle.syncerr_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.syncerr_int_handle(cfg);

        dprintf(INFO, "spdif %d syncerr\n", cfg->bus + 1);
    }

    if (fifo_status.lock_flag && base->ctrl.lock_mask) {//9 lock
        if (cfg->cfg_info.int_handle.lock_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.lock_int_handle(cfg);

        dprintf(INFO, "spdif %d lock\n", cfg->bus + 1);
    }

    if (fifo_status.begin_flag && base->ctrl.begin_mask) {//10 begin
        if (cfg->cfg_info.int_handle.begin_int_handle != NULL)
            resched = cfg->cfg_info.int_handle.begin_int_handle(cfg);

        dprintf(INFO, "spdif %d begin\n", cfg->bus + 1);
    }

    return resched ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}
