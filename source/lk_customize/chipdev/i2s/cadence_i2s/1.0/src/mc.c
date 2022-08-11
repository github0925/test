//*****************************************************************************
//
// mc.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <platform/interrupts.h>
#include <platform/debug.h>
#include <bits.h>
#include <assert.h>
#include <err.h>
#include <reg.h>
#include <lib/reg.h>
#include <irq_v.h>
#include <debug.h>
#include <string.h>
#include "i2s_cadence_mc.h"
#include "cdns_i2s_mc_regs.h"

#define I2S_MC_TX_DEPTH 512
#define I2S_MC_RX_DEPTH 512
#define I2S_MC_DATA_WIDTH 32
#define I2S_MC_INSTANCE_COUNT 2

static inline uint32_t i2s_mc_sample_rate_calc(uint32_t fclk,
        uint32_t fsample, uint8_t chn_width)
{
    return ((fclk + fsample * chn_width) / (2 * fsample * chn_width));
}

void i2s_mc_reg_cur_setting(i2s_mc_config_info *dev)
{
    uint32_t reg_value = 0;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    printf("***************I2S MC%d:0x%p**************************\n",
           dev->bus + 1, (void *)base);

    reg_value = base->ctrl.v;
    printf("i2s mc ctrl(0x00):0x%x\n", reg_value);

    reg_value = base->intr_stat.v;
    printf("i2s mc intr_stat(0x04):0x%x\n", reg_value);

    reg_value = base->srr.v;
    printf("i2s mc srr(0x08):0x%x\n", reg_value);

    reg_value = base->cid_ctrl.v;
    printf("i2s mc cid_ctrl(0x0C):0x%x\n", reg_value);

    reg_value = base->tfifo_stat.v;
    printf("i2s mc tfifo_stat(0x10):0x%x\n", reg_value);

    reg_value = base->rfifo_stat.v;
    printf("i2s mc rfifo_stat(0x14):0x%x\n", reg_value);

    reg_value = base->rfifo_ctrl.v;
    printf("i2s mc rfifo_ctrl(0x18):0x%x\n", reg_value);

    reg_value = base->tfifo_ctrl.v;
    printf("i2s mc tfifo_ctrl(0x1C):0x%x\n", reg_value);

    reg_value = base->dev_conf.v;
    printf("i2s mc dev_conf(0x20):0x%x\n", reg_value);

    reg_value = base->poll_stat.v;
    printf("i2s mc poll_stat(0x24):0x%x\n", reg_value);

    printf("*****************************************\n");
}

static void i2s_mc_init(i2s_mc_config_info *dev)
{
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    /* Disable chn0~chn7 */
    base->ctrl.i2s_en = 0;
    /* sfr reset */
    base->ctrl.sfr_rst = 0;
    /* fifo reset */
    base->ctrl.tfifo_rst = 0;
    base->ctrl.rfifo_rst = 0;
    /* sync reset */
    base->ctrl.tsync_rst = 0;
    base->ctrl.rsync_rst = 0;

    /* config cho-ch7 direction */
    base->ctrl.tr_cfg = dev->cfg_info.chn_direction;

    if (dev->cfg_info.loop_back_test_en) {

        base->ctrl.loopback = 0xf;
        /* loop_back_0_1,0:tx,1:rx */
        /* loop_back_2_3,2:tx,3:rx */
        /* loop_back_4_5,4:tx,5:rx */
        /* loop_back_6_7,6:tx,7:rx */
        base->ctrl.tr_cfg = 0x55;

        if ((dev->cfg_info.tx_mode == I2S_MC_MOD_MASTER)
                && (dev->cfg_info.rx_mode == I2S_MC_MOD_SLAVE)) {
            base->ctrl.tsync_loopback = 0;
            base->ctrl.rsync_loopback = 1;
        }
        else if (dev->cfg_info.rx_mode == I2S_MC_MOD_MASTER
                 && dev->cfg_info.tx_mode == I2S_MC_MOD_SLAVE) {
            base->ctrl.tsync_loopback = 1;
            base->ctrl.rsync_loopback = 0;
        }

        dprintf(INFO, "i2s mc loop back config!\n");
    }
    else {
        /* No loopback */
        base->ctrl.loopback = 0;
        base->ctrl.tsync_loopback = 0;
        base->ctrl.rsync_loopback = 0;
    }

    /* config master/slave mode */
    if (dev->cfg_info.tx_mode == I2S_MC_MOD_MASTER) {
        base->ctrl.t_ms = 1;
        /* config sample rate,resolution */
        base->srr.tsrate = i2s_mc_sample_rate_calc(dev->clock,
                           dev->cfg_info.tx_audio_freq, I2S_MC_DATA_WIDTH);
        base->srr.tresolution = dev->cfg_info.tx_sample_resolution;

        /* Config standard */
        if (dev->cfg_info.tx_standard == I2S_STD_PHILLIPS)
            base->dev_conf.tran_dev_cfg = 0x08;
        else if (dev->cfg_info.tx_standard == I2S_STD_LEFT_JUSTIFIED)
            base->dev_conf.tran_dev_cfg = 0x1A;
        else if (dev->cfg_info.tx_standard == I2S_STD_RIGHT_JUSTIFIED)
            base->dev_conf.tran_dev_cfg = 0x12;
        else
            dprintf(INFO, "i2s mc transmitter:standard out of range");
    }
    else if (dev->cfg_info.tx_mode == I2S_MC_MOD_SLAVE) {
        base->ctrl.t_ms = 0;
        /* config resolution */
        base->srr.tresolution = dev->cfg_info.tx_sample_resolution;

        /* Config standard */
        if (dev->cfg_info.tx_standard == I2S_STD_PHILLIPS)
            base->dev_conf.tran_dev_cfg = 0x08;
        else if (dev->cfg_info.tx_standard == I2S_STD_LEFT_JUSTIFIED)
            base->dev_conf.tran_dev_cfg = 0x1A;
        else if (dev->cfg_info.tx_standard == I2S_STD_RIGHT_JUSTIFIED)
            base->dev_conf.tran_dev_cfg = 0x12;
        else
            dprintf(INFO, "i2s mc transmitter:standard out of range");
    }
    else
        dprintf(INFO, "i2s mc transmitter:no init\n");

    if (dev->cfg_info.rx_mode == I2S_MC_MOD_MASTER) {
        base->ctrl.r_ms = 1;
        /* config sample rate,resolution */
        base->srr.rsrate = i2s_mc_sample_rate_calc(dev->clock,
                           dev->cfg_info.rx_audio_freq, I2S_MC_DATA_WIDTH);
        base->srr.rresolution = dev->cfg_info.rx_sample_resolution;

        /* Config standard */
        if (dev->cfg_info.rx_standard == I2S_STD_PHILLIPS)
            base->dev_conf.rec_dev_cfg = 0x08;
        else if (dev->cfg_info.rx_standard == I2S_STD_LEFT_JUSTIFIED)
            base->dev_conf.rec_dev_cfg = 0x1A;
        else if (dev->cfg_info.rx_standard == I2S_STD_RIGHT_JUSTIFIED)
            base->dev_conf.rec_dev_cfg = 0x12;
        else
            dprintf(INFO, "i2s mc receiver:standard out of range");
    }
    else if (dev->cfg_info.rx_mode == I2S_MC_MOD_SLAVE) {
        base->ctrl.r_ms = 0;
        /* config resolution */
        base->srr.rresolution = dev->cfg_info.rx_sample_resolution;

        /* Config standard */
        if (dev->cfg_info.rx_standard == I2S_STD_PHILLIPS)
            base->dev_conf.rec_dev_cfg = 0x08;
        else if (dev->cfg_info.rx_standard == I2S_STD_LEFT_JUSTIFIED)
            base->dev_conf.rec_dev_cfg = 0x1A;
        else if (dev->cfg_info.rx_standard == I2S_STD_RIGHT_JUSTIFIED)
            base->dev_conf.rec_dev_cfg = 0x12;
        else
            dprintf(INFO, "i2s mc receiver:standard out of range");
    }
    else
        dprintf(INFO, "i2s mc receiver:no init\n");

    /* config FIFO threshold */
    base->tfifo_ctrl.taempty_threshold = I2S_MC_TX_DEPTH - 32;
    base->tfifo_ctrl.tafull_threshold = I2S_MC_TX_DEPTH - 1;
    base->rfifo_ctrl.raempty_threshold = 0;
    base->rfifo_ctrl.rafull_threshold = I2S_MC_RX_DEPTH - 32;

    dprintf(INFO, "i2s mc config complete!\n");
}

static enum handler_return i2s_mc_receive_intmode(void *devarg)
{
    uint32_t sample = 0;
    i2s_mc_config_info *dev = (i2s_mc_config_info *)devarg;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;
    uint32_t num = base->rfifo_stat.v;
    uint8_t valid_bytes = (dev->cfg_info.rx_sample_resolution + 1) / 8;

    while ((num > 0) && (dev->cfg_info.rx_count > 0)) {
        sample = readl(dev->base_addr + I2S_MC_FIFO_OFFSET);
        memcpy(dev->cfg_info.prx_buffer, (uint8_t *)(&sample), valid_bytes);
        num--;
        dev->cfg_info.prx_buffer += valid_bytes;

        if ( dev->cfg_info.rx_count >= valid_bytes)
            dev->cfg_info.rx_count -= valid_bytes;
        else
            dev->cfg_info.rx_count = 0;

        if (dev->cfg_info.rx_count == 0) {
            dprintf(INFO, "i2s mc%d transfer(receive) done.\n", dev->bus + 1);
        }
    }

    return INT_NO_RESCHEDULE;
}

enum handler_return i2s_mc_transmit_intmode(void *devarg)
{
    uint32_t sample = 0;
    i2s_mc_config_info *dev = (i2s_mc_config_info *) devarg;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;
    uint32_t num = I2S_MC_TX_DEPTH - base->tfifo_stat.v;

    if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_8_BIT) {
        while ((num > 0) && (dev->cfg_info.tx_count > 0)) {
            sample = (uint32_t)dev->cfg_info.ptx_buffer[0];
            writel(sample, dev->base_addr + I2S_MC_FIFO_OFFSET);
            num--;
            dev->cfg_info.ptx_buffer++;
            dev->cfg_info.tx_count--;
        }

        return true;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_16_BIT) {
        while ((num > 0) && (dev->cfg_info.tx_count > 0)) {
            if (dev->cfg_info.tx_count >= 2) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8);
                dev->cfg_info.ptx_buffer += 2;
                dev->cfg_info.tx_count -= 2;
            }
            else {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0];
                dev->cfg_info.ptx_buffer++;
                dev->cfg_info.tx_count -= 1;
            }

            writel(sample, dev->base_addr + I2S_MC_FIFO_OFFSET);
            num--;
        }

        return true;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_24_BIT) {
        while ((num > 0) && (dev->cfg_info.tx_count > 0)) {
            if (dev->cfg_info.tx_count >= 3) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8) | ((uint32_t)dev->cfg_info.ptx_buffer[2]
                                 << 16);
                dev->cfg_info.ptx_buffer += 3;
                dev->cfg_info.tx_count -= 3;
            }
            else if (dev->cfg_info.tx_count == 2) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8);
                dev->cfg_info.ptx_buffer += 2;
                dev->cfg_info.tx_count -= 2;
            }
            else {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0];
                dev->cfg_info.ptx_buffer++;
                dev->cfg_info.tx_count -= 1;
            }

            writel(sample, dev->base_addr + I2S_MC_FIFO_OFFSET);
            num--;
        }

        return true;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_32_BIT) {
        while ((num > 0) && (dev->cfg_info.tx_count > 0)) {
            if (dev->cfg_info.tx_count >= 4) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8) | ((uint32_t)dev->cfg_info.ptx_buffer[2]
                                 << 16) | ((uint32_t)dev->cfg_info.ptx_buffer[3] << 24);
                dev->cfg_info.ptx_buffer += 4;
                dev->cfg_info.tx_count -= 4;
            }
            else if (dev->cfg_info.tx_count == 3) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8) | ((uint32_t)dev->cfg_info.ptx_buffer[2]
                                 << 16);
                dev->cfg_info.ptx_buffer += 3;
                dev->cfg_info.tx_count -= 3;
            }
            else if (dev->cfg_info.tx_count == 2) {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0] | ((uint32_t)
                         dev->cfg_info.ptx_buffer[1] << 8);
                dev->cfg_info.ptx_buffer += 2;
                dev->cfg_info.tx_count -= 2;
            }
            else {
                sample = (uint32_t)dev->cfg_info.ptx_buffer[0];
                dev->cfg_info.ptx_buffer++;;
                dev->cfg_info.tx_count -= 1;
            }

            writel(sample, dev->base_addr + I2S_MC_FIFO_OFFSET);
            num--;
        }

        return true;
    }

    return false;
}

static void i2s_mc_dma_default_init(i2s_mc_config_info *dev)
{
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    base->cid_ctrl.i2s_mask = 0;
    base->cid_ctrl.tfifo_empty_mask = 0;
    base->cid_ctrl.tfifo_aempty_mask = 0;
    base->cid_ctrl.tfifo_full_mask = 0;
    base->cid_ctrl.tfifo_afull_mask = 0;

    base->cid_ctrl.rfifo_empty_mask = 0;
    base->cid_ctrl.rfifo_aempty_mask = 0;
    base->cid_ctrl.rfifo_full_mask = 0;
    base->cid_ctrl.rfifo_afull_mask = 0;

    base->cid_ctrl.intreq_mask = 1;
}

static void i2s_mc_int_init(i2s_mc_config_info *dev)
{
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    base->cid_ctrl.intreq_mask = 1;
    base->cid_ctrl.i2s_mask =  dev->cfg_info.chn_int_en;

    base->cid_ctrl.tfifo_empty_mask = !!dev->cfg_info.tfifo_empty_int_handle;
    base->cid_ctrl.tfifo_aempty_mask = !!dev->cfg_info.tfifo_aempty_int_handle;
    base->cid_ctrl.tfifo_full_mask = !!dev->cfg_info.tfifo_full_int_handle;
    base->cid_ctrl.tfifo_afull_mask = !!dev->cfg_info.tfifo_afull_int_handle;
    base->cid_ctrl.rfifo_empty_mask = !!dev->cfg_info.rfifo_empty_int_handle;
    base->cid_ctrl.rfifo_aempty_mask = !!dev->cfg_info.rfifo_aempty_int_handle;
    base->cid_ctrl.rfifo_full_mask = !!dev->cfg_info.rfifo_full_int_handle;
    base->cid_ctrl.rfifo_afull_mask = !!dev->cfg_info.rfifo_afull_int_handle;

    if (!dev->cfg_info.tfifo_aempty_int_handle) {
        if ((dev->cfg_info.tx_mode != I2S_MC_MOD_NO_INIT)
                && (dev->cfg_info.func_mode == I2S_FUNC_WITH_INT)) {
            dev->cfg_info.tfifo_aempty_int_handle = i2s_mc_transmit_intmode;
            base->cid_ctrl.tfifo_aempty_mask = 1;
        }
    }

    if (!dev->cfg_info.rfifo_afull_int_handle) {
        if ((dev->cfg_info.rx_mode != I2S_MC_MOD_NO_INIT)
                && (dev->cfg_info.func_mode == I2S_FUNC_WITH_INT)) {
            dev->cfg_info.rfifo_afull_int_handle = i2s_mc_receive_intmode;
            base->cid_ctrl.rfifo_afull_mask = 1;
        }
    }
}

static enum handler_return i2s_mc_irq_handle(void *handle)
{
    bool resched = false;
    i2s_mc_config_info *dev = handle;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    if (base->cid_ctrl.i2s_mask && base->intr_stat.tdata_underrun) {
        dprintf(INFO, "i2s mc%d underrun code:0x%x\n", dev->bus + 1,
                base->intr_stat.underrun_code);

        if (dev->cfg_info.underrun_int_handle != NULL)
            resched = dev->cfg_info.underrun_int_handle(dev,
                      base->intr_stat.underrun_code);

        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.tdata_underrun = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;
    }

    if (base->cid_ctrl.i2s_mask && base->intr_stat.rdata_overrun) {
        dprintf(INFO, "i2s mc%d overrun code:0x%x\n", dev->bus + 1,
                base->intr_stat.overrun_code);

        if (dev->cfg_info.overrun_int_handle != NULL)
            resched = dev->cfg_info.underrun_int_handle(dev,
                      base->intr_stat.overrun_code);

        base->intr_stat.rdata_overrun = 0;
    }

    if (base->cid_ctrl.tfifo_empty_mask && base->intr_stat.tfifo_empty) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.tfifo_empty = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.tfifo_empty_int_handle != NULL )
            resched = dev->cfg_info.tfifo_empty_int_handle(dev);
    }

    if (base->cid_ctrl.tfifo_aempty_mask && base->intr_stat.tfifo_aempty) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.tfifo_aempty = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.tfifo_aempty_int_handle != NULL )
            resched = dev->cfg_info.tfifo_aempty_int_handle(dev);
    }

    if (base->cid_ctrl.tfifo_full_mask && base->intr_stat.tfifo_full) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.tfifo_full = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.tfifo_full_int_handle != NULL)
            resched = dev->cfg_info.tfifo_full_int_handle(dev);
    }

    if (base->cid_ctrl.tfifo_afull_mask && base->intr_stat.tfifo_afull) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.tfifo_afull = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.tfifo_afull_int_handle != NULL)
            resched = dev->cfg_info.tfifo_afull_int_handle(dev);
    }

    if (base->cid_ctrl.rfifo_empty_mask && base->intr_stat.rfifo_empty) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.rfifo_empty = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.rfifo_empty_int_handle != NULL )
            resched = dev->cfg_info.rfifo_empty_int_handle(dev);
    }

    if (base->cid_ctrl.rfifo_aempty_mask && base->intr_stat.rfifo_aempty) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.rfifo_aempty = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.rfifo_aempty_int_handle != NULL )
            resched = dev->cfg_info.rfifo_aempty_int_handle(dev);
    }

    if (base->cid_ctrl.rfifo_full_mask && base->intr_stat.rfifo_full) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.rfifo_full = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.rfifo_full_int_handle != NULL)
            resched = dev->cfg_info.rfifo_full_int_handle(dev);
    }

    if (base->cid_ctrl.rfifo_afull_mask && base->intr_stat.rfifo_afull) {
        i2s_mc_i2s_intr_stat_reg_t temp = base->intr_stat;
        temp.rfifo_afull = 0;
        temp.rdata_overrun = 1;
        base->intr_stat.v = temp.v;

        if (dev->cfg_info.rfifo_afull_int_handle != NULL)
            resched = dev->cfg_info.rfifo_afull_int_handle(dev);
    }

    return resched ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}

bool i2s_mc_register(i2s_mc_config_info *cfg)
{
    cfg->is_added = 1;
    register_int_handler(cfg->interrupt_num, &i2s_mc_irq_handle,
                         (void *)cfg);
    // enable interrupt
    unmask_interrupt(cfg->interrupt_num);

    return true;
}

bool i2s_mc_config(i2s_mc_init_t *cfg_info, void *handle)
{
    i2s_mc_config_info *dev = handle;

    if ((dev->bus < I2S_MC_INSTANCE_COUNT) && (dev->is_added)) {
        dev->cfg_info = *cfg_info;
        i2s_mc_init(dev);

        return true;
    }
    else
        dprintf(INFO, "i2s mc:instance num out of range\n");

    return false;
}

bool i2s_mc_start(void *handle)
{
    i2s_mc_config_info *dev = handle;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)dev->base_addr;

    if ((dev->bus < I2S_MC_INSTANCE_COUNT) && (dev->is_added)) {
        if (dev->cfg_info.loop_back_test_en)
            /* Enable channel */
            base->ctrl.i2s_en = 0xff;
        else
            /* Enable channel */
            base->ctrl.i2s_en = dev->cfg_info.chn_enable;

        /* sync */
        base->ctrl.rsync_rst = 1;
        base->ctrl.tsync_rst = 1;

        if (dev->cfg_info.func_mode == I2S_FUNC_WITH_INT) {
            i2s_mc_int_init(dev);
        }
        else {
            i2s_mc_dma_default_init(dev);
        }

        return true;
    }
    else
        dprintf(INFO, "i2s mc:instance num out of range\n");

    return false;
}

bool i2s_mc_stop(void *dev)
{
    i2s_mc_config_info *instance = dev;
    i2s_mc_regs_t *base = (i2s_mc_regs_t *)instance->base_addr;

    if ((instance->bus < I2S_MC_INSTANCE_COUNT) && (instance->is_added)) {
        /* Disable chn0~chn7 */
        base->ctrl.i2s_en = 0;
        base->cid_ctrl.intreq_mask = 0;
        return true;
    }
    else
        dprintf(INFO, "i2s mc:instance num out of range\n");

    return false;
}
