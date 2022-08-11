//*****************************************************************************
//
// sc.c
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
#include <lib/reg.h>
#include <irq_v.h>
#include <debug.h>
#include <string.h>
#include <__regs_base.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include <stdbool.h>
#include "i2s_cadence_sc.h"
#include "cdns_i2s_sc_regs.h"
#include "sd_audio.h"
#include "sys/types.h"

#define I2S_SC_TX_DEPTH 128
#define I2S_SC_RX_DEPTH 128

struct irq_handle_info {
    pcm_params_t pcm;
    struct dev_controller_info dev;
};

const int i2s_sc_clock_table[] = {400000000, 400000000, 98303999, 98303999, 98303999, 98303999, 98303999, 98303999};
const int i2s_sc_chan_width_table[] = {7, 0, 2, 4, 5, 7,};
const int i2s_sc_resolution_table[] = {31, 7, 15, 19, 23, 31,};
const static uint8_t ChnWidthTable[] = {8, 12, 16, 18, 20, 24, 28, 32};
static struct irq_handle_info irq_args[8];

static inline uint32_t i2s_sc_sample_rate_calc(uint32_t fclk,
        uint32_t fsample, unsigned char chn_num, uint8_t chn_width)
{
    return ((2 * fclk + fsample * chn_num * chn_width) /
            (2 * fsample * chn_num * chn_width));
}

void sdrv_i2s_sc_reg_cur_setting(struct dev_controller_info *dev)
{
    uint32_t reg_value = 0;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->addr;

    printf("***************I2S SC%d:0x%p**************************\n",
           dev->bus + 1, (void *)base);

    reg_value = base->ctrl.v;
    printf("i2s sc I2S_CTL(0x00):0x%x\n", reg_value);

    reg_value = base->ctrl_fdx.v;
    printf("i2s sc I2S_CTL_FDX(0x04):0x%x\n", reg_value);

    reg_value = base->sres.v;
    printf("i2s sc I2S_SRES(0x08):0x%x\n", reg_value);

    reg_value = base->sres_fdr.v;
    printf("i2s sc I2S_SRES_FDR(0x0C):0x%x\n", reg_value);

    reg_value = base->srate.v;
    printf("i2s sc I2S_SRATE(0x10):0x%x\n", reg_value);

    reg_value = base->stat.v;
    printf("i2s sc I2S_STAT(0x14):0x%x\n", reg_value);

    reg_value = base->fifo_level.v;
    printf("i2s sc FIFO_LEVEL(0x18):0x%x\n", reg_value);

    reg_value = base->fifo_aempty.v;
    printf("i2s sc FIFO_AEMPTY(0x1C):0x%x\n", reg_value);

    reg_value = base->fifo_afull.v;
    printf("i2s sc FIFO_AFULL(0x20):0x%x\n", reg_value);

    reg_value = base->fifo_level_fdr.v;
    printf("i2s sc FIFO_LEVEL_FDR(0x24):0x%x\n", reg_value);

    reg_value = base->fifo_aempty_fdr.v;
    printf("i2s sc FIFO_AEMPTY_FDR(0x28):0x%x\n", reg_value);

    reg_value = base->fifo_afull_fdr.v;
    printf("i2s sc FIFO_AEFULL_FDR(0x2C):0x%x\n", reg_value);

    reg_value = base->tdm_ctrl.v;
    printf("i2s sc TDM_CTL(0x30):0x%x\n", reg_value);

    reg_value = base->tdm_fd_dir.v;
    printf("i2s sc TDM_FD_DIR(0x34):0x%x\n", reg_value);

    printf("*****************************************\n");
}

static void sdrv_i2s_sc_dma_default_int(struct dev_controller_info dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;

    base->ctrl_fdx.ri2s_mask = 1;
    base->ctrl_fdx.rfifo_empty_mask = 0;
    base->ctrl_fdx.rfifo_aempty_mask = 0;
    base->ctrl_fdx.rfifo_full_mask = 0;
    base->ctrl_fdx.rfifo_afull_mask = 0;

    base->ctrl.i2s_mask = 1;
    base->ctrl.fifo_empty_mask = 0;
    base->ctrl.fifo_aempty_mask = 0;
    base->ctrl.fifo_full_mask = 0;
    base->ctrl.fifo_afull_mask = 0;
}

static void sdrv_i2s_sc_int_init(struct dev_controller_info dev,
                                 pcm_params_t pcm_info)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;
    u32 dir_mode, transfer_func_mode;

    sdrv_i2s_sc_dma_default_int(dev);

    dir_mode = pcm_info.mode & SD_AUDIO_DIR_MODE_ENABLE;
    transfer_func_mode = pcm_info.mode & SD_AUDIO_TRANSFER_MEDIA_MODE_ENABLE;

    if ((dir_mode == SD_AUDIO_DIR_MODE_TRANSMIT ||
            dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)
            && transfer_func_mode == SD_AUDIO_TRANSFER_WITH_CPU) {
        base->ctrl.fifo_aempty_mask = 1;
    }

    if ((dir_mode == SD_AUDIO_DIR_MODE_RECEIVE ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE)
            && transfer_func_mode == SD_AUDIO_TRANSFER_WITH_CPU) {
        base->ctrl.fifo_afull_mask = 1;
    }
    else if ((dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX
              || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)
             && transfer_func_mode == SD_AUDIO_TRANSFER_WITH_CPU) {
        base->ctrl_fdx.rfifo_afull_mask = 1;
    }

}

static bool sdrv_i2s_sc_receive_intmode(struct irq_handle_info
                                        *handle_info)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)handle_info->dev.addr;
    pcm_params_t *pcm_info = &handle_info->pcm;
    struct dev_controller_info dev = handle_info->dev;

    uint32_t sample = 0;
    uint32_t num = 0;
    uint8_t valid_bytes =
        (i2s_sc_resolution_table[pcm_info->resolution] + 1) / 8;
    u32 dir_mode = pcm_info->mode & SD_AUDIO_DIR_MODE_ENABLE;

    if ((dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX)
            || (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)) {
        num = base->fifo_level_fdr.level;
    }
    else {
        num = base->fifo_level.level;
    }

    while (num > 0 && pcm_info->xctrl.rx_len > 0) {
        sample = readl(handle_info->dev.addr + I2S_SC_FIFO_OFFSET);
        memcpy(pcm_info->xctrl.dst_addr, &sample, valid_bytes);
        num--;
        pcm_info->xctrl.dst_addr += valid_bytes;

        if (pcm_info->xctrl.rx_len >= valid_bytes)
            pcm_info->xctrl.rx_len -= valid_bytes;
        else
            pcm_info->xctrl.rx_len = 0;
    }

    if (pcm_info->xctrl.rx_len == 0) {
        dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev.bus + 1);
        event_signal(&(pcm_info->xctrl.rx_comp), false);
    }

    return true;
}

bool sdrv_i2s_sc_transmit_intmode(struct irq_handle_info *handle_info)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)handle_info->dev.addr;
    pcm_params_t *pcm_info = &handle_info->pcm;
    struct dev_controller_info dev = handle_info->dev;

    uint32_t sample = 0;
    uint32_t resolution = pcm_info->resolution;
    uint32_t num = I2S_SC_TX_DEPTH - base->fifo_level.v;

    if (resolution == SD_AUDIO_SAMPLE_WIDTH_8BITS) {
        while (num > 0 && pcm_info->xctrl.tx_len > 0) {
            sample = (uint32_t)pcm_info->xctrl.src_addr[0];
            writel(sample, dev.addr + I2S_SC_FIFO_OFFSET);
            num--;
            pcm_info->xctrl.src_addr++;
            pcm_info->xctrl.tx_len--;
        }

        if (pcm_info->xctrl.tx_len == 0) {
            dprintf(INFO, "i2s sc%d transfer(transmit) done\n", dev.bus + 1);
            event_signal(&(pcm_info->xctrl.tx_comp), false);
        }

        return true;
    }
    else if (resolution == SD_AUDIO_SAMPLE_WIDTH_16BITS) {
        while (num > 0 && pcm_info->xctrl.tx_len > 0) {
            if (pcm_info->xctrl.tx_len >= 2) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8);
                pcm_info->xctrl.src_addr += 2;
                pcm_info->xctrl.tx_len -= 2;
            }
            else {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0];
                pcm_info->xctrl.src_addr++;
                pcm_info->xctrl.tx_len -= 1;
            }

            writel(sample, dev.addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (pcm_info->xctrl.tx_len == 0) {
            dprintf(INFO, "i2s sc%d transfer(transmit) done\n", dev.bus + 1);
            event_signal(&(pcm_info->xctrl.tx_comp), false);
        }

        return true;
    }
    else if (resolution == SD_AUDIO_SAMPLE_WIDTH_24BITS) {
        while (num > 0 && pcm_info->xctrl.tx_len > 0) {
            if (pcm_info->xctrl.tx_len >= 3) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8) | ((uint32_t)pcm_info->xctrl.src_addr[2]
                                 << 16);
                pcm_info->xctrl.src_addr += 3;
                pcm_info->xctrl.tx_len -= 3;
            }
            else if (pcm_info->xctrl.tx_len == 2) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8);
                pcm_info->xctrl.src_addr += 2;
                pcm_info->xctrl.tx_len -= 2;
            }
            else {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0];
                pcm_info->xctrl.src_addr++;
                pcm_info->xctrl.tx_len -= 1;
            }

            writel(sample, dev.addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (pcm_info->xctrl.tx_len == 0) {
            dprintf(INFO, "i2s sc%d transfer(transmit) done\n", dev.bus + 1);
            event_signal(&(pcm_info->xctrl.tx_comp), false);
        }

        return true;
    }
    else if (resolution == SD_AUDIO_SAMPLE_WIDTH_32BITS) {
        while (num > 0 && pcm_info->xctrl.tx_len > 0) {
            if (pcm_info->xctrl.tx_len >= 4) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8) | ((uint32_t)pcm_info->xctrl.src_addr[2]
                                 << 16) | ((uint32_t)pcm_info->xctrl.src_addr[3] << 24);
                pcm_info->xctrl.src_addr += 4;
                pcm_info->xctrl.tx_len -= 4;
            }
            else if (pcm_info->xctrl.tx_len == 3) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8) | ((uint32_t)pcm_info->xctrl.src_addr[2]
                                 << 16);
                pcm_info->xctrl.src_addr += 3;
                pcm_info->xctrl.tx_len -= 3;
            }
            else if (pcm_info->xctrl.tx_len == 2) {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0] | ((uint32_t)
                         pcm_info->xctrl.src_addr[1] << 8);
                pcm_info->xctrl.src_addr += 2;
                pcm_info->xctrl.tx_len -= 2;
            }
            else {
                sample = (uint32_t)pcm_info->xctrl.src_addr[0];
                pcm_info->xctrl.src_addr++;;
                pcm_info->xctrl.tx_len -= 1;
            }

            writel(sample, dev.addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (pcm_info->xctrl.tx_len == 0) {
            dprintf(INFO, "i2s sc%d transfer(transmit) done\n", dev.bus + 1);
            event_signal(&(pcm_info->xctrl.tx_comp), false);
        }

        return true;
    }

    return false;
}

static enum handler_return sdrv_i2s_sc_irq_handle(void *handle_info)
{
    struct irq_handle_info *handle = (struct irq_handle_info *)handle_info;
    struct dev_controller_info *dev = &handle->dev;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->addr;

    if (base->stat.tdata_underrun && base->ctrl.i2s_mask) {
        base->stat.tdata_underrun = 0;

        dprintf(INFO, "i2s sc%d:underrun\n", (dev->bus + 1));
    }

    if (base->stat.rdata_overrun && (base->ctrl.i2s_mask
                                     || base->ctrl_fdx.ri2s_mask)) {
        base->stat.rdata_overrun = 0;

        dprintf(INFO, "i2s sc%d:overrun\n", (dev->bus + 1));
    }

    if (base->stat.fifo_empty && base->ctrl.fifo_empty_mask) {
        base->stat.fifo_empty = 0;
    }

    if (base->stat.fifo_aempty && base->ctrl.fifo_aempty_mask) {
        base->stat.fifo_aempty = 0;
        sdrv_i2s_sc_transmit_intmode(handle_info);
    }

    if (base->stat.fifo_full && base->ctrl.fifo_full_mask) {
        base->stat.fifo_full = 0;
    }

    if (base->stat.fifo_afull && base->ctrl.fifo_afull_mask) {
        base->stat.fifo_afull = 0;
        sdrv_i2s_sc_receive_intmode(handle_info);
    }

    if (base->stat.rfifo_empty && base->ctrl_fdx.rfifo_empty_mask) {
        base->stat.rfifo_empty = 0;
    }

    if (base->stat.rfifo_aempty && base->ctrl_fdx.rfifo_aempty_mask) {
        base->stat.rfifo_aempty = 0;
    }

    if (base->stat.rfifo_full && base->ctrl_fdx.rfifo_full_mask) {
        base->stat.rfifo_full = 0;
        sdrv_i2s_sc_receive_intmode(handle_info);
    }

    if (base->stat.rfifo_afull && base->ctrl_fdx.rfifo_afull_mask) {
        base->stat.rfifo_afull = 0;

    }

    return INT_NO_RESCHEDULE;
}

/**
* @brief start up i2s.
*
* register interrupt and device controller register reset
* and some common controller settings.
*
* @param dev dev basic info
* @param pcm_info pcm info
*
* @return \b true
*
*/
bool sdrv_i2s_sc_startup(struct dev_controller_info dev,
                         pcm_params_t pcm_info)
{
    u32 transfer_func_mode, dir_mode, ms_mode;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;

    irq_args[dev.bus].dev = dev;
    irq_args[dev.bus].pcm = pcm_info;

    // register interrput and unmask interrupt;
    register_int_handler(dev.irq_num, &sdrv_i2s_sc_irq_handle,
                         &irq_args[dev.bus]);
    unmask_interrupt(dev.irq_num);

    // disable i2s;
    base->ctrl.i2s_en = 0;
    // sfr reset;
    base->ctrl.sfr_rst = 0;
    // fifo reset;
    base->ctrl.fifo_rst = 0;

    ms_mode = pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE;
    dir_mode = pcm_info.mode & SD_AUDIO_DIR_MODE_ENABLE;

    // set fifo threshold;
    if (dir_mode == SD_AUDIO_DIR_MODE_TRANSMIT ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT) {
        if (ms_mode == SD_AUDIO_TRANSFER_CODEC_SLAVE) {
            base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4) - 1;
        }
        else {
            base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4);
        }

        base->fifo_afull.threshold = (I2S_SC_TX_DEPTH - 1);
    }

    if (dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX) {
        if (ms_mode == SD_AUDIO_TRANSFER_CODEC_MASTER) {
            base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4) - 1;
        }
        else {
            base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4);
        }

        base->fifo_afull.threshold = (I2S_SC_TX_DEPTH - 1);
    }

    if (dir_mode == SD_AUDIO_DIR_MODE_RECEIVE ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE) {
        base->fifo_aempty.threshold = 0;
        base->fifo_afull.threshold = (I2S_SC_RX_DEPTH * 3 / 4);
    }

    // set tdm;
    if (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX) {
        base->tdm_ctrl.tdm_en = 1;
    }

    // set fdr;
    if (dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX ||
            dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX) {
        // reset full duplex fifo;
        base->ctrl_fdx.fifo_rst = 0;
        // enable full duplex fuction;
        base->ctrl_fdx.full_duplex = 1;

        // set full duplex rx fifo threshold;
        base->fifo_aempty_fdr.threshold = 0;
        base->fifo_afull_fdr.threshold = (I2S_SC_RX_DEPTH * 3 / 4);
    }

    // set interrupt;
    transfer_func_mode = pcm_info.mode & SD_AUDIO_TRANSFER_MEDIA_MODE_ENABLE;

    if (transfer_func_mode == SD_AUDIO_TRANSFER_WITH_DMA) {
        // do dma interrupt setting.
        sdrv_i2s_sc_dma_default_int(dev);
    }
    else if (transfer_func_mode == SD_AUDIO_TRANSFER_WITH_CPU) {
        // do cpu interrupt setting.
        sdrv_i2s_sc_int_init(dev, pcm_info);
    }
    else {
        dprintf(INFO,
                "func<%s>: i2s sc transfer mode(int/dma) not init or unknown arg.\n",
                __func__);
    }

    return true;
}

/**
* @brief set transfer master/slave and interface mode.
*
* @param dev  basic dev info
* @param pcm_info pcm info
*
* @return \b true if succeed or \b false
*/
bool sdrv_i2s_sc_set_format(struct dev_controller_info dev,
                            pcm_params_t pcm_info)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;
    u32 ms_mode = pcm_info.mode & SD_AUDIO_TRANSFER_MODE_ENABLE;
    u32 interface_format = pcm_info.standard;

    // set master.
    if (ms_mode == SD_AUDIO_TRANSFER_CODEC_SLAVE) {
        base->ctrl.ms_cfg = 1;
    }
    else {
        base->ctrl.ms_cfg = 0;
    }

    // set i2s interface;
    if (interface_format == SD_AUDIO_I2S_STANDARD_PHILLIPS) {
        base->ctrl.sck_polar = 0;    /* config sck polar updated on rising edge */
        base->ctrl.ws_polar = 0;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 1;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB */
    }
    else if (interface_format == SD_AUDIO_I2S_LEFT_JUSTIFIED) {
        base->ctrl.sck_polar = 0;    /* config sck polar updated on rising edge */
        base->ctrl.ws_polar = 1;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 0;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else if (interface_format == SD_AUDIO_I2S_RIGHT_JUSTIFIED) {
        base->ctrl.sck_polar = 0;    /* config sck polar updated on rising edge */
        base->ctrl.ws_polar = 1;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 0;    /* config ws singal delay */
        base->ctrl.data_align = 1;    /* config data align:LSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else if (interface_format == SD_AUDIO_I2S_DSP_A) {
        base->ctrl.sck_polar = 0;    /* config sck polar updated on rising edge */
        base->ctrl.ws_polar = 0;    /* config ws polar */
        base->ctrl.ws_mode = 0;    /* config word select mode */
        base->ctrl.data_ws_del = 1;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else if (interface_format == SD_AUDIO_I2S_DSP_B) {
        base->ctrl.sck_polar = 0;    /* config sck polar updated on rising edge */
        base->ctrl.ws_polar = 0;    /* config ws polar */
        base->ctrl.ws_mode = 0;    /* config word select mode */
        base->ctrl.data_ws_del = 0;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else {
        dprintf(INFO, "i2s interface mode no init or unknown interface mode.\n");
        return false;
    }

    return true;
}

/**
* @brief set some hardware parameters.
*
* Such as channel number, channel width, slot width, sample rate.
*
* @param dev  basic dev info
* @param pcm_info pcm info
*
* @return \b true or \b false if failed.
*/
bool sdrv_i2s_sc_set_hw_parameters(struct dev_controller_info dev,
                                   pcm_params_t pcm_info)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;
    u32 tx_chan_num, rx_chan_num, chan_num = 0;
    u32 dir_mode = 0;

    // set slot width;
    base->ctrl.chn_width = i2s_sc_chan_width_table[pcm_info.slot_width];

    // set sample width;
    dir_mode = pcm_info.mode & SD_AUDIO_DIR_MODE_ENABLE;
    base->sres.resolution = i2s_sc_resolution_table[pcm_info.resolution];

    if (dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX
            || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX) {
        base->sres_fdr.resolution = i2s_sc_resolution_table[pcm_info.resolution];
    }

    // set chan num;
    tx_chan_num = pcm_info.mode & SD_AUDIO_CHANNEL_NUM_TX_ENABLE;
    rx_chan_num = pcm_info.mode & SD_AUDIO_CHANNEL_NUM_RX_ENABLE;

    if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_MONO
            && (dir_mode == SD_AUDIO_DIR_MODE_TRANSMIT
                || dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX)) ||
            (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_MONO
             && (dir_mode == SD_AUDIO_DIR_MODE_RECEIVE
                 || dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX))) {
        base->ctrl.audio_mode = 1;
        base->ctrl.mono_mode = 0;
        chan_num = 2;
    }
    else if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_STEREO
              && (dir_mode == SD_AUDIO_DIR_MODE_TRANSMIT
                  || dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX)) ||
             (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_STEREO
              && (dir_mode == SD_AUDIO_DIR_MODE_RECEIVE
                  || dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX))) {
        base->ctrl.audio_mode = 0;
        base->ctrl.mono_mode = 0;
        chan_num = 2;
    }
    else if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_4CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT)) ||
             (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_4CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE))) {
        chan_num = 4;
        base->tdm_ctrl.chn_no = chan_num - 1;
        base->tdm_ctrl.ch_en = 0xf;
    }
    else if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_8CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT)) ||
             (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_8CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE))) {
        chan_num = 8;
        base->tdm_ctrl.chn_no = chan_num - 1;
        base->tdm_ctrl.ch_en = 0xff;
    }
    else if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_4CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT
                  || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)) ||
             (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_4CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE
                  || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX))) {
        chan_num = 4;
        base->tdm_ctrl.chn_no = chan_num - 1;
        base->tdm_ctrl.ch_en = 0xf;
        base->tdm_fd_dir.ch_txen = 0xf;
        base->tdm_fd_dir.ch_rxen = 0xf;
    }
    else if ((tx_chan_num == SD_AUDIO_CHANNEL_NUM_TX_8CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_TRANSMIT
                  || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)) ||
             (rx_chan_num == SD_AUDIO_CHANNEL_NUM_RX_8CHANS
              && (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_RECEIVE
                  || dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX))) {
        chan_num = 8;
        base->tdm_ctrl.chn_no = chan_num - 1;
        base->tdm_ctrl.ch_en = 0xff;
        base->tdm_fd_dir.ch_txen = 0xff;
        base->tdm_fd_dir.ch_rxen = 0xff;
    }
    else {
        return false;
    }

    // set sample rate;
    base->srate.srate = i2s_sc_sample_rate_calc(i2s_sc_clock_table[dev.bus],
                        pcm_info.sample_rate,
                        chan_num,
                        ChnWidthTable[i2s_sc_chan_width_table[pcm_info.slot_width]]);

    return true;
}

/**
* @brief trigger i2s pcm transfer start/stop playback/capture.
*
* @param dev a struct containing basic dev info
* @param cmd command to execute
*
* @return \b true if successful or \b false.
*/
bool sdrv_i2s_sc_trigger(struct dev_controller_info dev, int cmd)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;
    uint32_t cnt = 0;

    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START) {
        base->ctrl.i2s_en = 0;
        base->ctrl.dir_cfg = 1;
        base->ctrl_fdx.i2s_ftx_en = 1;
        base->ctrl.i2s_en = 1;
        base->ctrl.intreq = 1;

    }
    else if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_START) {
        base->ctrl.i2s_en = 0;
        base->ctrl.dir_cfg = 0;
        base->ctrl_fdx.i2s_frx_en = 1;
        base->ctrl.i2s_en = 1;
        base->ctrl.intreq = 1;
    }
    else if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP) {
        base->ctrl.intreq = 0;
        /* wait til tx fifo empty */
        while ((cnt++) < 50) {
            if (base->stat.fifo_empty) {
                break;
            }
            spin(20);
        }
        base->ctrl_fdx.i2s_ftx_en = 0;
        spin(30);
        base->stat.v = 0;
    }
    else if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP) {
        base->ctrl.intreq = 0;
        base->ctrl_fdx.i2s_frx_en = 0;
        /* read rx fifo empty */
        while ((cnt++) < I2S_SC_RX_DEPTH * 2) {
            readl(dev.addr + I2S_SC_FIFO_OFFSET);
            if (base->fifo_level_fdr.v == 0) {
                if(((base->ctrl_fdx.full_duplex == 1) && (base->stat.rfifo_empty))
                    || ((base->ctrl_fdx.full_duplex == 0) && (base->stat.fifo_empty))) {
                    break;
                }
            }
        }
        /* rst rfifo */
        base->ctrl_fdx.fifo_rst = 0;
    }
    else {
        dprintf(INFO, "func<%s>: unknown pcm trigger command.\n", __func__);
        return false;
    }

    return true;
}

/**
* @brief disable i2s, stop clk.
*
* @param dev a struct containing basic dev info
*
* @return \b true if successful or \b false.
*/
bool sdrv_i2s_sc_shutdown(struct dev_controller_info dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev.addr;

    base->ctrl.intreq = 0;
    base->ctrl.i2s_en = 0;

    return true;
}
