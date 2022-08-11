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
#include "sys/types.h"

#define I2S_SC_TX_DEPTH 128
#define I2S_SC_RX_DEPTH 128
#define I2S_SC_INSTANCE_COUNT  8
#define I2S_SC_ERR_OVERRUN      1
#define I2S_SC_ERR_UNDERRUN     2

static inline uint32_t i2s_sc_sample_rate_calc(uint32_t fclk,
        uint32_t fsample, unsigned char chn_num, uint8_t chn_width)
{
    return ((2 * fclk + fsample * chn_num * chn_width) /
            (2 * fsample * chn_num * chn_width));
}

const static uint8_t ChnWidthTable[] = {8, 12, 16, 18, 20, 24, 28, 32};

void i2s_sc_reg_cur_setting(i2s_sc_config_info *dev)
{
    uint32_t reg_value = 0;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

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

static void i2s_sc_config_standard(i2s_sc_regs_t *base, uint8_t standard)
{
    if (standard == I2S_STD_PHILLIPS) {
        base->ctrl.sck_polar = 1;    /* config sck polar updated on falling edge */
        base->ctrl.ws_polar = 0;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 1;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB */
    }
    else if (standard == I2S_STD_LEFT_JUSTIFIED) {
        base->ctrl.sck_polar = 1;    /* config sck polar updated on falling edge */
        base->ctrl.ws_polar = 1;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 0;    /* config ws singal delay */
        base->ctrl.data_align = 0;    /* config data align:MSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else if (standard == I2S_STD_RIGHT_JUSTIFIED) {
        base->ctrl.sck_polar = 1;    /* config sck polar updated on falling edge */
        base->ctrl.ws_polar = 1;    /* config ws polar */
        base->ctrl.ws_mode = 1;    /* config word select mode */
        base->ctrl.data_ws_del = 0;    /* config ws singal delay */
        base->ctrl.data_align = 1;    /* config data align:LSB */
        base->ctrl.data_order = 0;    /* config data order:fisrt send MSB  */
    }
    else {
        dprintf(INFO, "unknown i2s sc stardard mode\n");
    }
}

static void i2s_sc_config_chnmode(i2s_sc_regs_t *base,
                                  i2s_sc_init_t cfg_info, unsigned char *chn_num)
{
    if (cfg_info.chn_mode == I2S_SC_CHN_MONO) {
        base->ctrl.audio_mode = 1;
        base->ctrl.mono_mode = 0;
        *chn_num = 2;
    }
    else if (cfg_info.chn_mode == I2S_SC_CHN_STEREO) {
        base->ctrl.audio_mode = 0;
        base->ctrl.mono_mode = 0;
        *chn_num = 2;
    }
    else {
        base->ctrl.ws_mode = cfg_info.ws_mode;
        *chn_num = __builtin_popcount((uint32_t)cfg_info.tdm_tx_chn_en);
        base->tdm_ctrl.tdm_en = 1;
        base->tdm_ctrl.chn_no = *chn_num - 1;
        base->tdm_ctrl.ch_en = cfg_info.tdm_tx_chn_en;
    }
}

static void i2s_sc_master_tx_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;  /* FIFO reset  */
    base->ctrl.ms_cfg = 1;    /* config master mode  */
    base->ctrl.dir_cfg = 1;    /* config direction of transmission:Tx  */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    /* config total data width  */
    base->ctrl.chn_width = dev->cfg_info.chn_width;
    /* config almost empty fifo level:tx fifo level */
    base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4) - 1;
    /* config almost full fifo level:defualt value */
    base->fifo_afull.threshold = I2S_SC_TX_DEPTH - 1;
    /* config resolution,valid number of data bit */
    base->sres.resolution = dev->cfg_info.tx_sample_resolution;
    base->srate.srate = i2s_sc_sample_rate_calc(dev->clock,
                        dev->cfg_info.audio_freq,
                        chn_num,
                        ChnWidthTable[dev->cfg_info.chn_width]);    /* config sample rate */

    dprintf(INFO, "i2s sc%d master tx cfg done\n", (dev->bus + 1));
}

static void i2s_sc_master_rx_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;    /* FIFO reset  */
    base->ctrl.ms_cfg = 1;    /* config master mode  */
    base->ctrl.dir_cfg = 0;    /* config direction of transmission:Rx  */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    base->fifo_aempty.threshold = 0;
    base->fifo_afull.threshold = (I2S_SC_RX_DEPTH * 3 / 4);
    base->ctrl.chn_width = dev->cfg_info.chn_width;
    base->sres.resolution = dev->cfg_info.rx_sample_resolution;
    base->srate.srate = i2s_sc_sample_rate_calc(dev->clock,
                        dev->cfg_info.audio_freq,
                        chn_num,
                        ChnWidthTable[dev->cfg_info.chn_width]);    /* config sample rate */

    dprintf(INFO, "i2s sc%d master rx cfg done\n", (dev->bus + 1));
}

static void i2s_sc_slave_tx_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;  /* FIFO reset  */
    base->ctrl.ms_cfg = 0;    /* config master mode  */
    base->ctrl.dir_cfg = 1;    /* config direction of transmission:Tx  */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    base->ctrl.chn_width = dev->cfg_info.chn_width;
    base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4);
    base->fifo_afull.threshold = (I2S_SC_TX_DEPTH - 1);
    base->sres.resolution = dev->cfg_info.tx_sample_resolution;

    dprintf(INFO, "i2s sc%d slave tx cfg done\n", (dev->bus + 1));
}

static void i2s_sc_slave_rx_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;  /* FIFO reset  */
    base->ctrl.ms_cfg = 0;    /* config master mode  */
    base->ctrl.dir_cfg = 0;    /* config direction of transmission:Rx  */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    base->ctrl.chn_width = dev->cfg_info.chn_width;
    base->fifo_aempty.threshold = 0;
    base->fifo_afull.threshold = (I2S_SC_RX_DEPTH * 3 / 4);
    base->sres.resolution = dev->cfg_info.rx_sample_resolution;

    dprintf(INFO, "i2s sc%d slave rx cfg done\n", (dev->bus + 1));
}

static void i2s_sc_master_full_duplex_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;    /* tx FIFO reset  */
    base->ctrl_fdx.fifo_rst = 0;    /* rx FIFO reset */
    base->ctrl.ms_cfg = 1;    /* config master mode */
    base->ctrl_fdx.full_duplex = 1;   /* Enable full-duplex */
    base->ctrl_fdx.i2s_ftx_en = 1;    /* Enable tx sdo */
    base->ctrl_fdx.i2s_frx_en = 1;    /* Enable rx sdi */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4);
    base->fifo_afull.threshold = (I2S_SC_TX_DEPTH - 1);
    base->fifo_aempty_fdr.threshold = 0;
    base->fifo_afull_fdr.threshold = (I2S_SC_RX_DEPTH * 3 / 4);

    base->ctrl.chn_width = dev->cfg_info.chn_width;
    base->sres.resolution = dev->cfg_info.tx_sample_resolution;
    base->sres_fdr.resolution = dev->cfg_info.rx_sample_resolution;
    base->srate.srate = i2s_sc_sample_rate_calc(dev->clock,
                        dev->cfg_info.audio_freq,
                        chn_num,
                        ChnWidthTable[dev->cfg_info.chn_width]);    /* config sample rate */

    dprintf(INFO, "i2s sc%d master full-duplex cfg done\n", (dev->bus + 1));
}

static void i2s_sc_slave_full_duplex_config(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;
    unsigned char chn_num = 0;

    base->ctrl.i2s_en = 0;    /*  Disable i2s  */
    base->ctrl.sfr_rst = 0;    /* SFR reset  */
    base->ctrl.fifo_rst = 0;    /* tx FIFO reset  */
    base->ctrl_fdx.fifo_rst = 0;    /* rx FIFO reset */
    base->ctrl.ms_cfg = 0;    /* config master mode */
    base->ctrl_fdx.full_duplex = 1;   /* Enable full-duplex */
    base->ctrl_fdx.i2s_ftx_en = 1;    /* Enable tx sdo */
    base->ctrl_fdx.i2s_frx_en = 1;    /* Enable rx sdi */

    i2s_sc_config_standard(base, dev->cfg_info.standard);
    i2s_sc_config_chnmode(base, dev->cfg_info, &chn_num);

    base->fifo_aempty.threshold = (I2S_SC_TX_DEPTH * 3 / 4) - 1;
    base->fifo_afull.threshold = (I2S_SC_TX_DEPTH - 1);
    base->fifo_aempty_fdr.threshold = 0;
    base->fifo_afull_fdr.threshold = (I2S_SC_RX_DEPTH * 3 / 4);

    base->ctrl.chn_width = dev->cfg_info.chn_width;
    base->sres.resolution = dev->cfg_info.tx_sample_resolution;
    base->sres_fdr.resolution = dev->cfg_info.rx_sample_resolution;

    dprintf(INFO, "i2s sc%d master full-duplex cfg done\n", (dev->bus + 1));
}

static enum handler_return i2s_sc_receive_intmode(void *devarg)
{
    i2s_sc_config_info *dev = (i2s_sc_config_info *)devarg;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    uint32_t sample = 0;
    uint32_t num = 0;
    uint8_t valid_bytes = (dev->cfg_info.rx_sample_resolution + 1) / 8;

    if (dev->cfg_info.mode == I2S_SC_MOD_MASTER_FULL_DUPLEX
            || dev->cfg_info.mode == I2S_SC_MOD_SLAVE_FULL_DUPLEX) {
        num = base->fifo_level_fdr.level;
    }
    else {
        num = base->fifo_level.level;
    }

    while (num > 0 && dev->cfg_info.rx_count > 0) {
        sample = readl(dev->base_addr + I2S_SC_FIFO_OFFSET);
        memcpy(dev->cfg_info.prx_buffer, &sample, valid_bytes);
        num--;
        dev->cfg_info.prx_buffer += valid_bytes;

        if (dev->cfg_info.rx_count >= valid_bytes)
            dev->cfg_info.rx_count -= valid_bytes;
        else
            dev->cfg_info.rx_count = 0;
    }

    if (dev->cfg_info.rx_count == 0) {
        dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev->bus + 1);
        event_signal(&(dev->rx_comp), false);
    }

    return INT_NO_RESCHEDULE;
}

enum handler_return i2s_sc_transmit_intmode(void *devarg)
{
    i2s_sc_config_info *dev = (i2s_sc_config_info *) devarg;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    uint32_t sample = 0;
    uint32_t num = I2S_SC_TX_DEPTH - base->fifo_level.v;

    if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_8_BIT) {
        while (num > 0 && dev->cfg_info.tx_count > 0) {
            sample = (uint32_t)dev->cfg_info.ptx_buffer[0];
            writel(sample, dev->base_addr + I2S_SC_FIFO_OFFSET);
            num--;
            dev->cfg_info.ptx_buffer++;
            dev->cfg_info.tx_count--;
        }

        if (dev->cfg_info.tx_count == 0) {
            dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev->bus + 1);
            event_signal(&(dev->tx_comp), false);
        }

        return INT_NO_RESCHEDULE;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_16_BIT) {
        while (num > 0 && dev->cfg_info.tx_count > 0) {
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

            writel(sample, dev->base_addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (dev->cfg_info.tx_count == 0) {
            dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev->bus + 1);
            event_signal(&(dev->tx_comp), false);
        }

        return true;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_24_BIT) {
        while (num > 0 && dev->cfg_info.tx_count > 0) {
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

            writel(sample, dev->base_addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (dev->cfg_info.tx_count == 0) {
            dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev->bus + 1);
            event_signal(&(dev->tx_comp), false);
        }

        return true;
    }
    else if (dev->cfg_info.tx_sample_resolution == I2S_SAMPLE_32_BIT) {
        while (num > 0 && dev->cfg_info.tx_count > 0) {
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

            writel(sample, dev->base_addr + I2S_SC_FIFO_OFFSET);
            num--;
        }

        if (dev->cfg_info.tx_count == 0) {
            dprintf(INFO, "i2s sc%d transfer(receive) done\n", dev->bus + 1);
            event_signal(&(dev->tx_comp), false);
        }

        return true;
    }

    return false;
}

static void i2s_sc_dma_default_int(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

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

    /* At the last Enable i2s interrupt main switch */
    base->ctrl.intreq = 1;
}

static void i2s_sc_int_init(i2s_sc_config_info *dev)
{
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    base->ctrl.i2s_mask = 1;
    base->ctrl.fifo_empty_mask = !!dev->cfg_info.fifo_empty_int_handle;
    base->ctrl.fifo_aempty_mask = !!dev->cfg_info.fifo_aempty_int_handle;
    base->ctrl.fifo_full_mask = !!dev->cfg_info.fifo_full_int_handle;
    base->ctrl.fifo_afull_mask = !!dev->cfg_info.fifo_afull_int_handle;
    base->ctrl_fdx.ri2s_mask = 1;
    base->ctrl_fdx.rfifo_empty_mask = !!dev->cfg_info.rfifo_empty_int_handle;
    base->ctrl_fdx.rfifo_full_mask = !!dev->cfg_info.rfifo_full_int_handle;
    base->ctrl_fdx.rfifo_aempty_mask = !!dev->cfg_info.rfifo_aempty_int_handle;
    base->ctrl_fdx.rfifo_afull_mask = !!dev->cfg_info.rfifo_afull_int_handle;

    if (!dev->cfg_info.fifo_aempty_int_handle) {
        if ((dev->cfg_info.mode == I2S_SC_MOD_MASTER_TX ||
                dev->cfg_info.mode == I2S_SC_MOD_SLAVE_TX ||
                dev->cfg_info.mode == I2S_SC_MOD_MASTER_FULL_DUPLEX ||
                dev->cfg_info.mode == I2S_SC_MOD_SLAVE_FULL_DUPLEX)
                && dev->cfg_info.func_mode == I2S_FUNC_WITH_INT) {
            dev->cfg_info.fifo_aempty_int_handle = i2s_sc_transmit_intmode;
            base->ctrl.fifo_aempty_mask = 1;
        }
    }

    if ((dev->cfg_info.mode == I2S_SC_MOD_MASTER_RX ||
            dev->cfg_info.mode == I2S_SC_MOD_SLAVE_RX)
            && dev->cfg_info.func_mode == I2S_FUNC_WITH_INT) {
        if (!dev->cfg_info.fifo_afull_int_handle) {
            dev->cfg_info.fifo_afull_int_handle = i2s_sc_receive_intmode;
            base->ctrl.fifo_afull_mask = 1;
        }
    }
    else if ((dev->cfg_info.mode == I2S_SC_MOD_MASTER_FULL_DUPLEX ||
              dev->cfg_info.mode == I2S_SC_MOD_SLAVE_FULL_DUPLEX)
             && dev->cfg_info.func_mode == I2S_FUNC_WITH_INT) {
        if (!dev->cfg_info.rfifo_afull_int_handle)
            dev->cfg_info.rfifo_afull_int_handle = i2s_sc_receive_intmode;

        base->ctrl_fdx.rfifo_afull_mask = 1;
    }

    /* At the last config i2s interrupt main switch */
    base->ctrl.intreq = 1;
}

static enum handler_return i2s_sc_irq_handle(void *handle)
{
    bool resched = false;
    i2s_sc_config_info *dev = handle;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    if (base->stat.tdata_underrun && base->ctrl.i2s_mask) {
        base->stat.tdata_underrun = 0;

        if (dev->cfg_info.exception_int_handle != NULL )
            resched = dev->cfg_info.exception_int_handle(dev,
                      I2S_SC_ERR_UNDERRUN);

        dprintf(INFO, "i2s sc%d:underrun\n", (dev->bus + 1));
    }

    if (base->stat.rdata_overrun && (base->ctrl.i2s_mask
                                     || base->ctrl_fdx.ri2s_mask)) {
        base->stat.rdata_overrun = 0;

        if (dev->cfg_info.exception_int_handle != NULL)
            resched = dev->cfg_info.exception_int_handle(dev,
                      I2S_SC_ERR_OVERRUN);

        dprintf(INFO, "i2s sc%d:overrun\n", (dev->bus + 1));
    }

    if (base->stat.fifo_empty && base->ctrl.fifo_empty_mask) {
        base->stat.fifo_empty = 0;

        if (dev->cfg_info.fifo_empty_int_handle != NULL)
            resched = dev->cfg_info.fifo_empty_int_handle(dev);
    }

    if (base->stat.fifo_aempty && base->ctrl.fifo_aempty_mask) {
        base->stat.fifo_aempty = 0;

        if (dev->cfg_info.fifo_aempty_int_handle != NULL)
            resched = dev->cfg_info.fifo_aempty_int_handle(dev);
    }

    if (base->stat.fifo_full && base->ctrl.fifo_full_mask) {
        base->stat.fifo_full = 0;

        if (dev->cfg_info.fifo_full_int_handle != NULL)
            resched = dev->cfg_info.fifo_full_int_handle(dev);
    }

    if (base->stat.fifo_afull && base->ctrl.fifo_afull_mask) {
        base->stat.fifo_afull = 0;

        if (dev->cfg_info.fifo_afull_int_handle != NULL)
            resched = dev->cfg_info.fifo_afull_int_handle(dev);
    }

    if (base->stat.rfifo_empty && base->ctrl_fdx.rfifo_empty_mask) {
        base->stat.rfifo_empty = 0;

        if (dev->cfg_info.rfifo_empty_int_handle != NULL)
            resched = dev->cfg_info.rfifo_empty_int_handle(dev);
    }

    if (base->stat.rfifo_aempty && base->ctrl_fdx.rfifo_aempty_mask) {
        base->stat.rfifo_aempty = 0;

        if (dev->cfg_info.rfifo_aempty_int_handle != NULL)
            resched = dev->cfg_info.rfifo_aempty_int_handle(dev);
    }

    if (base->stat.rfifo_full && base->ctrl_fdx.rfifo_full_mask) {
        base->stat.rfifo_full = 0;

        if (dev->cfg_info.rfifo_full_int_handle != NULL)
            resched = dev->cfg_info.rfifo_full_int_handle(dev);
    }

    if (base->stat.rfifo_afull && base->ctrl_fdx.rfifo_afull_mask) {
        base->stat.rfifo_afull = 0;

        if (dev->cfg_info.rfifo_afull_int_handle != NULL)
            resched = dev->cfg_info.rfifo_afull_int_handle(dev);
    }

    return resched ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}

bool i2s_sc_register(i2s_sc_config_info *cfg)
{
    cfg->is_added = 1;
    register_int_handler(cfg->interrupt_num, &i2s_sc_irq_handle,
                         (void *)cfg);
    // enable interrupt
    unmask_interrupt(cfg->interrupt_num);

    return true;
}

bool i2s_sc_config(i2s_sc_config_info *dev, i2s_sc_init_t *config)
{
    dev->cfg_info = *config;

    switch (dev->cfg_info.mode) {
        case I2S_SC_MOD_MASTER_TX:
            i2s_sc_master_tx_config(dev);
            break;

        case I2S_SC_MOD_MASTER_RX:
            i2s_sc_master_rx_config(dev);
            break;

        case I2S_SC_MOD_SLAVE_TX:
            i2s_sc_slave_tx_config(dev);
            break;

        case I2S_SC_MOD_SLAVE_RX:
            i2s_sc_slave_rx_config(dev);
            break;

        case I2S_SC_MOD_MASTER_FULL_DUPLEX:
            i2s_sc_master_full_duplex_config(dev);
            break;

        case I2S_SC_MOD_SLAVE_FULL_DUPLEX:
            i2s_sc_slave_full_duplex_config(dev);
            break;

        default:
            dprintf(INFO, "i2s sc:Unknown mode\n");
    }

    return true;
}

bool i2s_sc_start(void *handle)
{
    i2s_sc_config_info *dev = handle;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    if (dev->bus < I2S_SC_INSTANCE_COUNT && dev->is_added) {
        /* Firstly,Enable i2s,Then enable interrupt */
        base->ctrl.i2s_en = 1;

        if (dev->cfg_info.func_mode == I2S_FUNC_WITH_INT) {
            i2s_sc_int_init(dev);
        }
        else {
            i2s_sc_dma_default_int(dev);
        }

        return true;
    }
    else {
        dprintf(INFO, "i2s sc:instance num out of range\n");
    }

    return false;
}

bool i2s_sc_stop(void *handle)
{
    i2s_sc_config_info *dev = handle;
    i2s_sc_regs_t *base = (i2s_sc_regs_t *)dev->base_addr;

    if (dev->bus < I2S_SC_INSTANCE_COUNT && dev->is_added) {
        base->ctrl.i2s_en = 0;
        base->ctrl.intreq = 0;

        return true;
    }
    else {
        dprintf(INFO, "i2s sc:instance num out of range\n");
    }

    return false;
}
