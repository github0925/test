/*
* dw_dmac1.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* dw dma controller c file
*
* Revision History:
* -----------------
* 0.1, 3/29/2019 yishao init version
* 0.2, 4/8/2019 yishao  add dmac mux part.
* 0.3, 4/16/2019 yishao fixed enable and disable channel issue.
* 0.4, 4/18/2019 yishao add irq handler functions .
*/
#ifndef __DW_DMAC1_C
#define __DW_DMAC1_C
#include "dw_dma1.h"
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <err.h>
#include <lib/reg.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define APB_BASE_START 0x30000000u
#define APB_BASE_END 0x31ffffffu
#define DW_DMAC_DEBUG (SPEW + 2)
#define REG_BIT_SET(a, b) ((a) |= (1 << (b)))
#define REG_BIT_CLEAR(a, b) ((a) &= ~(1 << (b)))
#define REG_BIT_FLIP(a, b) ((a) ^= (1 << (b)))
#define REG_BIT_CHECK(a, b) ((a) & (1 << (b)))

#define dbg_print(a)                                                             \
    {                                                                            \
        unsigned long target_addr = (unsigned long)(&(a));                       \
        dprintf(DW_DMAC_DEBUG, "%s:r(%p,0x%x)\n", #a, &(a), readl(target_addr)); \
    }

#define dbg_info_print(a, s)                                                             \
    {                                                                                    \
        unsigned long target_addr = (unsigned long)(&(a));                               \
        dprintf(DW_DMAC_DEBUG, "[%s]:%s:r(%p,0x%x)\n", s, #a, &(a), readl(target_addr)); \
    }

#define dbg_printq(a)                                                              \
    {                                                                              \
        unsigned long target_addr = (unsigned long)(&(a));                         \
        dprintf(DW_DMAC_DEBUG, "%s:r(%p,0x%llx)\n", #a, &(a), readq(target_addr)); \
    }
#define dbg_print_field(a)                                    \
    {                                                         \
        dprintf(DW_DMAC_DEBUG, "%s:r(0x%llx)\n", #a, (u64)a); \
    }
typedef struct
{
    dw_dmac1_t *dmac;
    dmac1_mux_t *dmac_mux;
    dmac1_irq_cbk_t irq_callback[DW_DMAC1_CHANNEL_NUMB];
    bool initialized; /*  This flag must be true, and initialized by platform. */

} dw_dmac1_context;

static dw_dmac1_context g_dmac1[DW_DMAC1_MAX_NUMB] = {0};

/* This function is used to set all of mux registers to 0xff */
void dw_init_dmac1_mux_reg(int instance)
{
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    g_dmac1[instance].dmac_mux->en_reg.en = 0;
    for (int i = 0; i < DW_DMAC1_CHANNEL_NUMB; i++)
    {
        g_dmac1[instance].dmac_mux->ch[i].wr_hdsk = 0xFF;
        g_dmac1[instance].dmac_mux->ch[i].rd_hdsk = 0xFF;
    }
    g_dmac1[instance].dmac_mux->en_reg.en = 1;
}
/* init and set dmac base address. */
void dw_init_dmac1(int instance, void *dmac_base, void *dmac_mux_base)
{

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);

    g_dmac1[instance].dmac = (dw_dmac1_t *)(dmac_base);
    g_dmac1[instance].dmac_mux = (dmac1_mux_t *)(dmac_mux_base);

    if (0 != dmac_mux_base)
    {
        dw_init_dmac1_mux_reg(instance);
    }

    for (int i = 0; i < DW_DMAC1_CHANNEL_NUMB; i++)
    {
        g_dmac1[instance].irq_callback[i].dmac_irq_blk_tr_done_handle = 0;
        g_dmac1[instance].irq_callback[i].dmac_irq_dma_tr_done_handle = 0;
        g_dmac1[instance].irq_callback[i].dmac_irq_dst_tr_comp_handle = 0;
        g_dmac1[instance].irq_callback[i].dmac_irq_src_tr_comp_handle = 0;
        g_dmac1[instance].irq_callback[i].dmac_irq_err_handle = 0;
    }
    g_dmac1[instance].initialized = true;
}

/* dmac common register configure. */
bool dw_dmac1_reg_cfg(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    dbg_print_field(instance);
    dbg_print_field(cfg.dmac_int_en);
    /* enable dma channel */
    if (0 == cfg.dmac_int_en)
    {
        g_dmac1[instance].dmac->dmac_cfg.dmac_en = 1;
        g_dmac1[instance].dmac->dmac_cfg.int_en = 0;
    }
    else
    {
        /* printf("dw_int_en = 1\n"); */
        g_dmac1[instance].dmac->dmac_cfg.dmac_en = 1;
        g_dmac1[instance].dmac->dmac_cfg.int_en = 1;
    }
    return true;
}

void dmac1_channel8_openscr_448(void)
{
    /* open scr 448 bit; */
    /* printf(".....beginning to open scr 448 bit...\n"); */
    vaddr_t rw = (vaddr_t)_ioaddr(APB_SCR_SAF_BASE + ((0x38) << 10));
    vaddr_t rdata = 0;
    rdata = readl(rw);
    rdata |= 0x1;
    writel(rdata, rw);
    /* printf("****finished open scr 448 bit***\n"); */
}
/* Will not use this channel  */
void dmac1_channel8_closescr_448(void)
{
    /* close scr 448 bit; */
    /* printf(".....beginning to close scr 448 bit...\n"); */

    vaddr_t rw = (vaddr_t)_ioaddr(APB_SCR_SAF_BASE + ((0x38) << 10));
    vaddr_t rdata = 0;
    rdata = readl(rw);
    rdata &= ~0x1;
    writel(rdata, rw);
    /* printf("****finished close scr 448 bit***\n"); */
}
/* convert peri addr to dma side */
/* dmac channel configure. */
bool dw_dmac1_ch_reg_cfg(const dmac1_coeff_t cfg)
{
    if (cfg.chan_id == 7)
        dmac1_channel8_openscr_448();
    /***************************************declare***********************************************/
    u32 src_or_dst;
    u64 llp = 0;
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;

    addr_t dma_src;
    addr_t dma_dst;

    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);

    /***************************************initial***********************************************/

    src_or_dst = ((cfg.dma_transfer_type == 2) ? 1 : 0); //attention : if src/dst is memory ,handshake number is not care ,right?
    llp = cfg.llp;

    dprintf(DW_DMAC_DEBUG, "dw_dmac1_ch_reg_cfg() \n");

    dma_src = cfg.src_addr;
    dma_dst = cfg.dst_addr;

    /* 1 sar */
    g_dmac1[instance].dmac->ch[chan].sar_l = dma_src & 0xFFFFFFFF;
#if ARCH_ARM64
    g_dmac1[instance].dmac->ch[chan].sar_h = (dma_src >> 32) & 0xFFFFFFFF;
#else
    g_dmac1[instance].dmac->ch[chan].sar_h = 0;
#endif
    /* 2 dst */
    g_dmac1[instance].dmac->ch[chan].dar_l = dma_dst & 0xFFFFFFFF;
#if ARCH_ARM64
    g_dmac1[instance].dmac->ch[chan].dar_h = (dma_dst >> 32) & 0xFFFFFFFF;
#else
    g_dmac1[instance].dmac->ch[chan].dar_h = 0;
#endif

    dbg_print(g_dmac1[instance].dmac->ch[chan].sar_l);
    dbg_print(g_dmac1[instance].dmac->ch[chan].sar_h);

    dbg_print(g_dmac1[instance].dmac->ch[chan].dar_l);
    dbg_print(g_dmac1[instance].dmac->ch[chan].dar_h);
    /* 3.chx_block_ts
     */
    dbg_print(cfg.block_transfer_size);
    g_dmac1[instance].dmac->ch[chan].blk_ts.vl = cfg.block_transfer_size - 1;

    /* check block size */
    dbg_print(g_dmac1[instance].dmac->ch[chan].blk_ts.vl);
    ASSERT(g_dmac1[instance].dmac->ch[chan].blk_ts.vl == (cfg.block_transfer_size - 1));

    /* dbg_print(g_dmac1[instance].dmac->ch[chan].blk_ts.vl); */
    /* 4.chx_ctl */
    g_dmac1[instance].dmac->ch[chan].ctl.vh = 0;
    g_dmac1[instance].dmac->ch[chan].ctl.vl = 0;
    g_dmac1[instance].dmac->ch[chan].ctl.sinc = cfg.sinc;
    g_dmac1[instance].dmac->ch[chan].ctl.dinc = cfg.dinc;
    g_dmac1[instance].dmac->ch[chan].ctl.src_tr_width = cfg.src_tr_width;
    g_dmac1[instance].dmac->ch[chan].ctl.dst_tr_width = cfg.dst_tr_width;
    g_dmac1[instance].dmac->ch[chan].ctl.arlen_en = cfg.arlen_en;
    g_dmac1[instance].dmac->ch[chan].ctl.arlen = cfg.arlen;
    g_dmac1[instance].dmac->ch[chan].ctl.awlen_en = cfg.awlen_en;
    g_dmac1[instance].dmac->ch[chan].ctl.awlen = cfg.awlen;
    g_dmac1[instance].dmac->ch[chan].ctl.src_msize = cfg.src_msize;
    g_dmac1[instance].dmac->ch[chan].ctl.dst_msize = cfg.dst_msize;
    /* dbg_print(g_dmac1[instance].dmac->ch[chan].ctl.v); */
    u64 dmac_cfg = (((u64)g_dmac1[instance].dmac->dmac_cfg.vh << 32) & 0xFFFFFFFF) + g_dmac1[instance].dmac->dmac_cfg.vl;
    u64 dmac_ch_ctl = (((u64)g_dmac1[instance].dmac->ch[chan].ctl.vh << 32) & 0xFFFFFFFF) + g_dmac1[instance].dmac->ch[chan].ctl.vl;
    dbg_print(dmac_cfg);
    dbg_print(dmac_ch_ctl);

    /* 5.chx_cfg */
    g_dmac1[instance].dmac->ch[chan].cfg.vl = 0;
    g_dmac1[instance].dmac->ch[chan].cfg.vh = 0;
    g_dmac1[instance].dmac->ch[chan].cfg.src_multblk_type = cfg.src_transfer_type;
    g_dmac1[instance].dmac->ch[chan].cfg.dst_multblk_type = cfg.dst_transfer_type;

    g_dmac1[instance].dmac->ch[chan].cfg.src_per = chan * 2; //Assigns a hardware handshaking interface (0 - DMAX_NUM_HS_IF-1)
    g_dmac1[instance].dmac->ch[chan].cfg.dst_per = chan * 2 + 1;

    g_dmac1[instance].dmac->ch[chan].cfg.tt_fc = cfg.dma_transfer_type;
    g_dmac1[instance].dmac->ch[chan].cfg.ch_pri = cfg.channel_pri;
    g_dmac1[instance].dmac->ch[chan].cfg.lock_ch = 0;
    g_dmac1[instance].dmac->ch[chan].cfg.lock_ch_l = 2;
    g_dmac1[instance].dmac->ch[chan].cfg.src_osr_lmt = 15; //Outstanding numb max to 16
    g_dmac1[instance].dmac->ch[chan].cfg.dst_osr_lmt = 15;
    if (src_or_dst)
    {
        g_dmac1[instance].dmac->ch[chan].cfg.hs_sel_src = cfg.hs_sel_src;
    }
    else
    {
        g_dmac1[instance].dmac->ch[chan].cfg.hs_sel_dst = cfg.hs_sel_dst;
    }
    dbg_print(g_dmac1[instance].dmac->ch[chan].cfg.vl);
    dbg_print(g_dmac1[instance].dmac->ch[chan].cfg.vh);

    if (0 != llp)
    {
        /*TODO: need fixed panic in z1.
        * g_dmac1[instance].dmac->ch[chan].llp.loc = ((llp >> 6) & 0xFFFFFFFFFFFFFFFF);
        */
        g_dmac1[instance].dmac->ch[chan].ctl.shadow_lli_valid = 1;
        g_dmac1[instance].dmac->ch[chan].llp.vl = (llp)&0xFFFFFFFF;
        g_dmac1[instance].dmac->ch[chan].llp.vh = (llp >> 32) & 0xFFFFFFFF;
        dbg_print(g_dmac1[instance].dmac->ch[chan].llp.vl);
        dbg_print(g_dmac1[instance].dmac->ch[chan].llp.vh);
    }
    g_dmac1[instance].dmac->ch[chan].axi_qos_h = 0;
    g_dmac1[instance].dmac->ch[chan].axi_qos_l = 0;
    /* dbg_print(g_dmac1[instance].dmac->ch[chan].axi_qos); */
    u64 axi_qos = (((u64)g_dmac1[instance].dmac->ch[chan].axi_qos_h << 32) & 0xFFFFFFFF) + g_dmac1[instance].dmac->ch[chan].axi_qos_l;
    dbg_print(axi_qos);

    /* set irq handle function. */
    if (NULL != cfg.irq_callback.dmac_irq_blk_tr_done_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.block_tr_done = 1;
        g_dmac1[instance].dmac->ch[chan].ctl.ioc_blk_tr = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_blk_tr_done_handle = cfg.irq_callback.dmac_irq_blk_tr_done_handle;
    }
    else
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.block_tr_done = 0;
    }

    if (NULL != cfg.irq_callback.dmac_irq_dma_tr_done_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.dma_tr_done = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_dma_tr_done_handle = cfg.irq_callback.dmac_irq_dma_tr_done_handle;
    }
    else
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.dma_tr_done = 0;
    }

    if (NULL != cfg.irq_callback.dmac_irq_dst_tr_comp_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.dst_tr_comp = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_dst_tr_comp_handle = cfg.irq_callback.dmac_irq_dst_tr_comp_handle;
    }
    else
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.dst_tr_comp = 0;
    }

    if (NULL != cfg.irq_callback.dmac_irq_src_tr_comp_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.src_tr_comp = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_src_tr_comp_handle = cfg.irq_callback.dmac_irq_src_tr_comp_handle;
    }
    else
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.src_tr_comp = 0;
    }
    if (NULL != cfg.irq_callback.dmac_irq_err_handle)
    {
        g_dmac1[instance].irq_callback[chan].dmac_irq_err_handle = cfg.irq_callback.dmac_irq_err_handle;
    }

    if (NULL != cfg.irq_callback.dmac_irq_ch_lock_clr_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.ch_lock_cleared = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_ch_lock_clr_handle = cfg.irq_callback.dmac_irq_ch_lock_clr_handle;
    }

    if (NULL != cfg.irq_callback.dmac_irq_ch_src_suspended_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.ch_src_suspended = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_ch_src_suspended_handle = cfg.irq_callback.dmac_irq_ch_src_suspended_handle;
    }

    if (NULL != cfg.irq_callback.dmac_irq_ch_suspended_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.ch_suspended = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_ch_suspended_handle = cfg.irq_callback.dmac_irq_ch_suspended_handle;
    }

    if (NULL != cfg.irq_callback.dmac_irq_ch_disabled_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.ch_disabled = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_ch_disabled_handle = cfg.irq_callback.dmac_irq_ch_disabled_handle;
    }

    if (NULL != cfg.irq_callback.dmac_irq_ch_aborted_handle)
    {
        g_dmac1[instance].dmac->ch[chan].int_stat_en.ch_aborted = 1;
        g_dmac1[instance].irq_callback[chan].dmac_irq_ch_aborted_handle = cfg.irq_callback.dmac_irq_ch_aborted_handle;
    }
    /* handle over all irq for driver. open all of irq. */
    if (NULL != cfg.irq_callback.dmac_irq_evt_handle)
    {

        /* enable all of irq. */
        g_dmac1[instance].dmac->ch[chan].int_stat_en.vl = cfg.dmac_int_mask;
        if (1 == g_dmac1[instance].dmac->ch[chan].int_stat_en.block_tr_done)
        {
            g_dmac1[instance].dmac->ch[chan].ctl.ioc_blk_tr = 1;
        }
        else
        {
            g_dmac1[instance].dmac->ch[chan].ctl.ioc_blk_tr = 0;
        }
        g_dmac1[instance].irq_callback[chan].dmac_irq_evt_handle = cfg.irq_callback.dmac_irq_evt_handle;
        g_dmac1[instance].irq_callback[chan].context = cfg.irq_callback.context;
        dprintf(DW_DMAC_DEBUG, "dmac1 enable dmac_irq_evt_handle. %d_%d %p %p \n", instance, chan, g_dmac1[instance].irq_callback[chan].dmac_irq_evt_handle, g_dmac1[instance].irq_callback[chan].context);
    }

    /* get source hand shaking number. */
    u64 sar = g_dmac1[instance].dmac->ch[chan].sar_h;
    sar = sar << 32;
    sar = sar + g_dmac1[instance].dmac->ch[chan].sar_l;

    /* TODO: decide what mem base there should be */
    if ((sar >= APB_BASE_START && sar <= APB_BASE_END) || (sar >= OSPI1_BASE && sar <= (OSPI1_BASE + 0x03FFFFFF)) || (sar >= OSPI2_BASE && sar <= (OSPI2_BASE + 0x03FFFFFF)))
    {
        int rd_hs_id = get_dma1_hs_id(sar, DMA1_MUX_RD);
        dprintf(SPEW, "sar(0x%llx)-(%d) \n", sar, rd_hs_id);
        if (DMA1_MUX_ERR_NO > rd_hs_id)
        {
            dw_dmac1_mux_cfg(cfg, rd_hs_id, DMA1_MUX_RD);
        }
        else
        {
            dprintf(CRITICAL, "Func:<%s>-line<%d> \n", __FUNCTION__, __LINE__);
        }
    }
    /* get destination hand shaking number. */
    u64 dar = g_dmac1[instance].dmac->ch[chan].dar_h;
    dar = (dar << 32);
    dar = dar + g_dmac1[instance].dmac->ch[chan].dar_l;

    //temporary modified for safety test -- if (DDR_MEMORY_BASE > dar) {
    /* TODO: decide what mem base there should be */
    if ((dar >= APB_BASE_START && dar <= APB_BASE_END) || (dar >= OSPI1_BASE && dar <= (OSPI1_BASE + 0x03FFFFFF)) || (dar >= OSPI2_BASE && dar <= (OSPI2_BASE + 0x03FFFFFF)))
    {
        int wr_hs_id = get_dma1_hs_id(dar, DMA1_MUX_WR);
        dprintf(SPEW, "dar(0x%llx)-(%d) \n", dar, wr_hs_id);
        if (DMA1_MUX_ERR_NO > wr_hs_id)
        {
            dw_dmac1_mux_cfg(cfg, wr_hs_id, DMA1_MUX_WR);
        }
        else
        {
            dprintf(CRITICAL, "Func:<%s>-line<%d> \n", __FUNCTION__, __LINE__);
        }
    }

    if (cfg.chan_id == 7)
        dmac1_channel8_closescr_448();
    return true;
}

/* config dmac */
bool dw_dmac1_cfg(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    if (false == g_dmac1[instance].initialized)
    {
        dprintf(ALWAYS, "Index(%d) addr(%p) uninitialized!\n", instance, g_dmac1[instance].dmac);
        return false;
    }
    dprintf(DW_DMAC_DEBUG, "dmac_%d addr(%p)\n", instance, g_dmac1[instance].dmac);
    dw_dmac1_reg_cfg(cfg);
    dw_dmac1_ch_reg_cfg(cfg);
    return true;
}
/* enable channel */
bool dw_dmac1_ch_enable(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    u32 reg_val = g_dmac1[instance].dmac->dmac_ch_en.vl;
    //dprintf(INFO, "before dmac_ch_en_t:0x%x\n", g_dmac1[instance].dmac->dmac_ch_en.vl);
    dbg_info_print(g_dmac1[instance].dmac->dmac_ch_en.vl, "Before Enable")
        //because there is write protect so don't care previous value.
        reg_val = ((1 << (chan)) + (1 << (chan + 8)));
    g_dmac1[instance].dmac->dmac_ch_en.vl = reg_val;
    dbg_info_print(g_dmac1[instance].dmac->dmac_ch_en.vl, "After Enable")

        return true;
}
/* disable channel */
bool dw_dmac1_ch_disable(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;
    u32 count = 0;
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    u32 reg_val = g_dmac1[instance].dmac->dmac_ch_en.vl;
    dbg_info_print(g_dmac1[instance].dmac->dmac_ch_en.vl, "Before Disable");
    /* because there is write protect so don't care previous value. */
    reg_val = ((0 << (chan)) + (1 << (chan + 8)));

    dprintf(DW_DMAC_DEBUG, "disable reg_val:0x%x\n", reg_val);
    g_dmac1[instance].dmac->dmac_ch_en.vl = reg_val;
    dbg_info_print(g_dmac1[instance].dmac->dmac_ch_en.vl, "After Disable");

    uint32_t tr_width, msize,max_exchange_count;
    uint32_t mux_number = 0;
    DMR1_MUX_DIRECT mux_dir = 0;
    tr_width = 1 << cfg.dst_tr_width;
    if(cfg.dst_msize == 0)
    {
        msize = 1;
    }
    else
    {
        msize = 2 << cfg.dst_msize;
    }
    max_exchange_count = 128 * 4 / (tr_width * msize);

    if (cfg.dma_transfer_type == DMA1_TRANSFER_DIR_MEM2PER_DMAC)
    {
        mux_dir = DMA1_MUX_WR;
        mux_number = g_dmac1[instance].dmac_mux->ch[chan].wr_hdsk;

        while((max_exchange_count-- > 0) && dw_dmac1_is_ch_enabled(cfg)) {
        dw_dmac1_mux_cfg(cfg, DMA1_MUX_EXTRA_PORT, mux_dir);
        dw_dmac1_mux_cfg(cfg, mux_number, mux_dir);
        }
    }

    while(( count < 100 ) && dw_dmac1_is_ch_enabled(cfg)) {
        spin(100);
        count ++;
    }

    ASSERT(!dw_dmac1_is_ch_enabled(cfg));
    return true;
}

/* Is dw dmac channel enabled. */
bool dw_dmac1_is_ch_enabled(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    u32 ch_enabled = g_dmac1[instance].dmac->dmac_ch_en.vl & (1 << (chan));
    dprintf(DW_DMAC_DEBUG, "dmac_ch_%d:0x%x\n", chan, ch_enabled);
    return ((ch_enabled == 0) ? false : true);
}
/* Clear */
void dw_dmac1_clear_chan_stat(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;
    /* Clear channel call back functions. */
    memset(&g_dmac1[instance].irq_callback[chan], 0, sizeof(dmac1_irq_cbk_t));
}
/* wait dma done */
bool dw_dmac1_tr_done(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;
    u32 auto_en = 1;
    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    u32 rdata = 0;
    switch (chan)
    {
    case 0:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch1_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 1:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch2_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 2:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch3_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 3:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch4_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 4:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch5_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 5:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch6_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 6:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch7_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;
    case 7:
        rdata = ((auto_en != 0) ? g_dmac1[instance].dmac->dmac_ch_en.ch8_en : g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);
        break;

    default:
        break;
    }

    if (auto_en != 0)
    {
        if ((rdata & 0x1) == 0)
        {
            return 1;
        }
        else
            return 0;
    }
    else
    {
        if ((rdata & 0x1) == 1)
        {
            return 1;
        }
        else
            return 0;
    }
    return true;
}

/* print channel information */
bool dw_get_dmac1_status()
{
    dprintf(INFO, "Query current DMA controller status!\n");
    for (int i = 0; i < DW_DMAC1_MAX_NUMB; i++)
    {
        if (true == g_dmac1[i].initialized)
        {
            dprintf(INFO, "Index(%d) addr(%p) initialized!\n", i, g_dmac1[i].dmac);
        }
    }
    return true;
}

/* get src address */
addr_t dw_dmac1_get_transferred_src_addr(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    return (addr_t)g_dmac1[instance].dmac->ch[chan].sar_l;
}
/* get dst address */
addr_t dw_dmac1_get_transferred_dst_addr(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    return (addr_t)g_dmac1[instance].dmac->ch[chan].dar_l;
}
/* dma status */
bool dw_dmac1_ch_stat(dmac1_coeff_t cfg)
{
    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);

    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.sinc);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.dinc);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.src_tr_width);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.dst_tr_width);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.arlen_en);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.arlen);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.awlen_en);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.awlen);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.src_msize);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].ctl.dst_msize);

    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.src_multblk_type);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.dst_multblk_type);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.src_per);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.dst_per);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.tt_fc);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.ch_pri);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.lock_ch);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.lock_ch_l);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.src_osr_lmt);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.dst_osr_lmt);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.hs_sel_src);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].cfg.hs_sel_dst);

    dbg_print_field(g_dmac1[instance].dmac->dmac_ch_en.vl);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].int_stat_en.vl);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].int_stat.vl);
    dbg_print_field(g_dmac1[instance].dmac->ch[chan].int_stat.block_tr_done);

    return true;
}

bool dw_dmac1_mux_cfg(dmac1_coeff_t cfg, DMR1_REQ_PERI port, DMR1_MUX_DIRECT direct)
{

    u32 instance = cfg.dmac_index;
    u32 chan = cfg.chan_id;

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);
    ASSERT(chan < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(chan >= 0);
    if (DMA1_TRANSFER_DIR_MEM2MEM_DMAC == cfg.dma_transfer_type)
    {
        /* Don't need transfer in mem2mem. */
        return false;
    }

    /* disable MUX at first , should it will interrupt other transfer process? */
    g_dmac1[instance].dmac_mux->en_reg.en = 0;

    if (DMA1_MUX_WR == direct)
    {
        g_dmac1[instance].dmac_mux->ch[chan].wr_hdsk = port;
    }
    else
    {
        g_dmac1[instance].dmac_mux->ch[chan].rd_hdsk = port;
    }
    /* Re-enable dma mux. */
    g_dmac1[instance].dmac_mux->en_reg.en = 1;
    __asm__ volatile("nop");
    __asm__ volatile("nop");

    /* check result of mux op. */
    if (DMA1_MUX_WR == direct)
    {
        dprintf(DW_DMAC_DEBUG, "dma(%d) ch(%d) wr mux(%d)!\n", cfg.dmac_index, cfg.chan_id, g_dmac1[instance].dmac_mux->ch[chan].updated_wr_hdsk);
        return ((g_dmac1[instance].dmac_mux->ch[chan].updated_wr_hdsk == port) ? true : false);
    }
    else
    {
        dprintf(DW_DMAC_DEBUG, "dma(%d) ch(%d) rd mux(%d)!\n", cfg.dmac_index, cfg.chan_id, g_dmac1[instance].dmac_mux->ch[chan].updated_rd_hdsk);
        return ((g_dmac1[instance].dmac_mux->ch[chan].updated_rd_hdsk == port) ? true : false);
    }
    /* Check result of MUX. */
}

/* print dmac reset */
bool dw_dmac1_reset(int instance)
{

    printf("Reset dmac instance %d", instance);
    g_dmac1[instance].dmac->dmac_reset.reset = 1;
    dbg_print_field(g_dmac1[instance].dmac->dmac_reset.reset);
    return true;
}

/* enable irq */
bool dw_dmac1_enable_irq(int instance)
{
    g_dmac1[instance].dmac->dmac_cfg.int_en = 1;
    /* dbg_print_field(g_dmac1[instance].dmac->dmac_cfg.int_en); */
    return true;
}
/* disable irq */
bool dw_dmac1_disable_irq(int instance)
{
    g_dmac1[instance].dmac->dmac_cfg.int_en = 0;
    /* dbg_print_field(g_dmac1[instance].dmac->dmac_cfg.int_en); */
    return true;
}
/* here get int_stat chan from */
dmac1_ch_int_stat_t dw_dmac1_get_ch_int_stat(int instance, int chan)
{
    dmac1_ch_int_stat_t stat = {0};
    stat.vl = g_dmac1[instance].dmac->ch[chan].int_stat.vl;
    stat.vh = g_dmac1[instance].dmac->ch[chan].int_stat.vh;
    return stat;
}

bool dw_dmac1_ch_has_int(int instance, int chan)
{
    if ((g_dmac1[instance].dmac->dmac_int_stat.vl & (1 << chan)) != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* clear dw_dmac1_int_stat here chan */
void dw_dmac1_clr_int_stat(int instance, int chan, dmac1_ch_int_stat_t status)
{
    g_dmac1[instance].dmac->ch[chan].int_clr.vl = status.vl;
    g_dmac1[instance].dmac->ch[chan].int_clr.vh = status.vh;
}

enum handler_return dw_dmac1_irq_handle(int instance, int channel, dmac1_ch_int_stat_t status)
{

    bool resched = false;

    ASSERT(instance < DW_DMAC1_MAX_NUMB);
    ASSERT(instance >= 0);

    ASSERT(channel < DW_DMAC1_CHANNEL_NUMB);
    ASSERT(channel >= 0);

    if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_evt_handle)
    {
        dbg_info_print(status.vl, "irq handle");
        void *context = g_dmac1[instance].irq_callback[channel].context;
        if (0 != status.block_tr_done)
        {
            /* cyclic mode block tr done. next will clear this irq. */
            g_dmac1[instance].irq_callback[channel].dmac_irq_evt_handle(DMA_COMP, 0, context);
            /* g_dmac1[instance].dmac->ch[i].int_clr.block_tr_done = 1; */
        }
        if (1 == status.dma_tr_done)
        {

            g_dmac1[instance].irq_callback[channel].dmac_irq_evt_handle(DMA_COMP, 0, context);
        }
        if (0 != (status.vl & DMA1_ALL_ERR) || (1 == status.ch_suspended) || (1 == status.ch_aborted) || (1 == status.ch_disabled) || (1 == status.ch_aborted))
        {
            g_dmac1[instance].irq_callback[channel].dmac_irq_evt_handle(DMA_ERR, status.vl, context);
        }
        if ((1 == status.ch_suspended) || (1 == status.ch_src_suspended))
        {
            g_dmac1[instance].irq_callback[channel].dmac_irq_evt_handle(DMA_PAUSED, 0, context);
        }
        resched = INT_NO_RESCHEDULE;
    }
    if (1 == status.block_tr_done)
    {
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_blk_tr_done_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_blk_tr_done_handle();
        }
    }

    if (1 == status.dma_tr_done)
    {
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_dma_tr_done_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_dma_tr_done_handle();
        }
    }

    if (1 == status.src_tr_comp)
    {
        dbg_info_print(status.vl, "irq handle src_tr_comp");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_src_tr_comp_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_src_tr_comp_handle();
        }
    }

    if (1 == status.dst_tr_comp)
    {
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_dst_tr_comp_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_dst_tr_comp_handle();
        }
    }

    if (0 != (status.vl & DMA1_ALL_ERR))
    {
        dbg_info_print(status.vl, "irq handle error");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_err_handle)
        {
            u32 err = (status.vl & 0x3F7FE0);
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_err_handle(err);
        }
    }

    if (1 == status.ch_lock_cleared)
    {
        dbg_info_print(status.vl, "irq handle ch_lock_cleared");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_ch_lock_clr_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_ch_lock_clr_handle();
        }
    }

    if (1 == status.ch_src_suspended)
    {
        dbg_info_print(status.vl, "irq handle ch_src_suspended");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_ch_src_suspended_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_ch_src_suspended_handle();
        }
    }

    if (1 == status.ch_suspended)
    {
        dbg_info_print(status.vl, "irq handle ch_suspended");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_ch_suspended_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_ch_suspended_handle();
        }
    }

    if (1 == status.ch_disabled)
    {
        dbg_info_print(status.vl, "irq handle ch_disabled");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_ch_disabled_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_ch_disabled_handle();
        }
    }

    if (1 == status.ch_aborted)
    {
        dbg_info_print(status.vl, "irq handle ch_aborted");
        if (NULL != g_dmac1[instance].irq_callback[channel].dmac_irq_ch_aborted_handle)
        {
            resched = g_dmac1[instance].irq_callback[channel].dmac_irq_ch_aborted_handle();
        }
    }

    return resched ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}
#endif
