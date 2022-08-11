/*
 * dw_dma1.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * dma driver c file
 * TBD
 *
 * Revision History:
 * -----------------
 * 0.1, 10/22/2019 yishao init version
 */
#include <kernel/thread.h>
#include <lib/cbuf.h>
#include <lib/reg.h>
#include <platform.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>

#include "dw_dma1.h"
#include "res.h"
#include <__regs_base.h>
#define list_head list_node
#define list_del(x) list_delete(x)
#define INIT_LIST_HEAD(x) list_initialize(x)
#define DW_DMA1_DEBUG (SPEW + 2)

/**
 * list_first_entry - get the first element from a list
 * @ptr:        the list head to take the element from.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member)                                    \
    containerof((ptr)->next, type, member)

#define list_for_each_entry(entry, type, list, member)                               \
    list_for_every_entry(list, entry, type, member)

#define list_for_each_entry_safe(entry, type, temp_entry, list, member)              \
    list_for_every_entry_safe(list, entry, temp_entry, type, member)

static spin_lock_t lock = SPIN_LOCK_INITIAL_VALUE;
static struct list_node *dma1_chan_list = NULL;
static int dw_dmac1_inst_pos = 0;
static dw_dma1_chan_t dma1_channels[DW_DMAC1_MAX_NUMB][DW_DMAC1_CHANNEL_NUMB];

static inline struct dw_dma1_chan *to_dw_dma1_chan(struct dma_chan *chan)
{
    return containerof(chan, struct dw_dma1_chan, chan);
}
static u32 bytes2block(u32 blk_size, size_t bytes, unsigned int width,
                       size_t *len)
{

    u32 block;
    if ((bytes >> width) > blk_size) {
        block = blk_size;
        *len = block << width;
    } else {
        block = bytes >> width;
        *len = bytes;
    }

    return block;
}
/*
 */
static struct dma_desc *dw_get_desc(struct dw_dma1_chan *dwc,
                                    struct lli_list *lli, unsigned long flags)
{
    struct dma_desc *desc_ptr = NULL;
    // TODO: [Notice]Need add free code later !!!!
    desc_ptr = calloc(1, sizeof(dma_desc_t));
    if (!desc_ptr)
        return NULL;
    desc_ptr->chan = &dwc->chan;
    desc_ptr->lli_list_ptr = lli;
    desc_ptr->flags = flags;
    dwc->allocated_desc++;
    return desc_ptr;
}

static void dw_free_lli_list(struct lli_list *list)
{
    struct lli_list *curr = NULL;
    struct lli_list *first = list;
    struct lli_list *next = NULL;
    list_for_each_entry_safe(curr, lli_list_t, next, &first->dw_list, lli_node)
    {
        list_del(&curr->lli_node);
        free((void *)curr);
    }
}

struct lli_list *make_dw_dmac1_lli(void)
{
    // TODO: [Notice]Need add free code later !!!!
    lli_list_t *lli_ptr;
    static int tmp_id = 0;
    lli_ptr = memalign(64, sizeof(lli_list_t));
    if (!lli_ptr)
        return NULL;
    INIT_LIST_HEAD(&lli_ptr->dw_list);

    lli_ptr->phy_addr = (u64)p2ap((addr_t)_paddr((&lli_ptr->lli_item)));
    dprintf(DW_DMA1_DEBUG, "llp  0x%llx \n", lli_ptr->phy_addr);

    lli_ptr->id = tmp_id++;
    return lli_ptr;
}

static void dw_dma1_remove_desc(struct dma_desc *desc)
{

    if (NULL == desc)
        return;
    if (NULL != desc->lli_list_ptr) {
        dw_free_lli_list(desc->lli_list_ptr);
    }
    free(desc);
    desc = 0;
}

bool dw_dma1_chan_terminate(struct dma_chan *chan)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    bool result;
    spin_lock_saved_state_t state;
    spin_lock_irqsave(&chan->lock, state);
    chan->chan_status = DMA_COMP;
    result = dw_dmac1_ch_disable(dwc->dmac_cfg);
    spin_unlock_irqrestore(&chan->lock, state);
    return result;
}

void dw_dma1_chan_handle_err(struct dma_chan *chan)
{
    spin_lock_saved_state_t state;
    dw_dma1_chan_t *dwc = to_dw_dma1_chan(chan);
    spin_lock_irqsave(&chan->lock, state);
    // Disable dma channel
    dprintf(CRITICAL, "Handle error %p \n", dwc);

    if (chan->chan_status != DMA_COMP) {
        dw_dmac1_ch_disable(dwc->dmac_cfg);
        chan->chan_status = DMA_ERR;
    }
    spin_unlock_irqrestore(&chan->lock, state);
}

void dw_dma1_chan_tr_done(struct dma_chan *chan)
{
    spin_lock_saved_state_t state;
    spin_lock_irqsave(&chan->lock, state);
    chan->chan_status = DMA_COMP;
    spin_unlock_irqrestore(&chan->lock, state);
}

/* Next is interface functions
 * ----------------------------------------------------------*/

void dw_dma1_reg(dma_instance_t *instance, struct list_node *channels)
{
    /*config base address with instance_numb*/
    dma1_chan_list = channels;
    dprintf(DW_DMA1_DEBUG, "%s: %d (%p) (%p) (%p)(%d)\n", __FUNCTION__,
            __LINE__, (void *)instance,
            (void *)(addr_t)instance->dma_cfg.phy_addr,
            (void *)(addr_t)instance->dma_cfg.mux_phy_addr, instance->inst_id);

    dw_init_dmac1(dw_dmac1_inst_pos,
                  (void *)_ioaddr(instance->dma_cfg.phy_addr),
                  (void *)_ioaddr(instance->dma_cfg.mux_phy_addr));
    register_int_handler(instance->dma_cfg.irq_numb, dw_dma1_interrupt_handle,
                         (void *)(addr_t)(instance->inst_id));
    unmask_interrupt(instance->dma_cfg.irq_numb);
    /*Regster functions here */

    instance->controllerTable.dma_chan_req = dw_dma1_chan_req;
    instance->controllerTable.dma_dev_config = dw_dma1_dev_config;
    instance->controllerTable.prep_dma_memcpy = dw_prep_dma1_memcpy;
    instance->controllerTable.prep_dma_memset = dw_prep_dma1_memset;
    instance->controllerTable.prep_dma_dev = dw_prep_dma1_dev;
    instance->controllerTable.prep_dma_cyclic = dw_prep_dma1_cyclic;
    instance->controllerTable.dma_submit = dw_dma1_submit;
    instance->controllerTable.dma_sync_wait = dw_dma1_sync_wait;
    instance->controllerTable.dma_terminate = dw_dma1_terminate;
    instance->controllerTable.dma_free_desc = dw_dma1_free_desc;
    dprintf(DW_DMA1_DEBUG, "%s: %d (%p) \n", __FUNCTION__, __LINE__,
            (void *)instance->controllerTable.dma_chan_req);
    /*register virtual channels to dma hal channel list*/
    for (int i = 0; i < DW_DMAC1_CHANNEL_NUMB; i++) {
        dw_dma1_chan_t *cptr = &dma1_channels[dw_dmac1_inst_pos][i];

        /* Get instance id  */
        cptr->chan.inst_id = instance->inst_id;
        cptr->chan.chan_id = i;
        cptr->chan.dmac_id = dw_dmac1_inst_pos;
        cptr->chan.chan_cap = DMA_MEM_CAP | DMA_PERI_CAP1;
        /*All of id */
        cptr->chan.vchan_id = list_length(channels);

        cptr->chan.chan_status = DMA_COMP;
        cptr->chan.lock = SPIN_LOCK_INITIAL_VALUE;
        cptr->priority = 7;
        cptr->blk_size = DW_DMAC1_MAX_BLK_SIZE;
        cptr->allocated_desc = 0;
        cptr->allocated_channel = 0;
        cptr->dmac_cfg.dmac_index = cptr->chan.dmac_id; /*0*/
        cptr->dmac_cfg.chan_id = i;                     /*0 ~ 6*/
        list_add_tail(channels, &cptr->chan.node);
        /* FIXME: should remove print in register process  */
        dprintf(DW_DMA1_DEBUG,
                "dw_dmac1 instance(%d) dmacid(%d) chan(0x%x) vchan(0x%x)  "
                "dmac_cfg ch(0x%x) dmac_index (0x%x)!\n",
                cptr->chan.inst_id, cptr->chan.dmac_id, cptr->chan.chan_id,
                cptr->chan.vchan_id, cptr->dmac_cfg.chan_id,
                cptr->dmac_cfg.dmac_index);
    }
    dw_dmac1_inst_pos++;
    dprintf(DW_DMA1_DEBUG, "%s: %d (%p) \n", __FUNCTION__, __LINE__,
            (void *)instance);
}

/* Remove all of dma1 channel and clear dmac instance pos.  */
void dw_dma1_unreg(void)
{
    dw_dmac1_inst_pos = 0;
    /* virtual chan list should be removed in hal layer. */
    dma1_chan_list = NULL;
}

/* Request a channel with capabilities */
bool dw_dma1_chan_req(struct dma_chan *chan)
{
    dw_dma1_chan_t *dwc = to_dw_dma1_chan(chan);
    if (0 == dwc->allocated_channel) {
        dwc->allocated_channel = 1;
        return true;
    } else {
        return false;
    }
}

void dw_dma1_dev_config(struct dma_chan *chan, struct dma_dev_cfg *cfg)
{
    spin_lock_saved_state_t state;
    spin_lock_irqsave(&chan->lock, state);
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    memcpy(&dwc->dev_cfg, cfg, sizeof(*cfg));
    spin_unlock_irqrestore(&chan->lock, state);
}

struct dma_desc *dw_prep_dma1_memcpy(struct dma_chan *chan, void *dest,
                                     void *src, size_t len, unsigned long flags)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    size_t tr_count = 0;
    dmac1_coeff_t dmac_cfg = {0};
    spin_lock_saved_state_t state;

    struct lli_list *curr = NULL;
    struct lli_list *first = NULL;
    struct lli_list *prev = NULL;
    if (!len) {
        dprintf(CRITICAL, "%s: bad len  (= 0)\n", __func__);
        return NULL;
    }
    dwc->tr_type = DMA_MEMCPY;
    /* set dmac instance */
    dmac_cfg.dmac_index = chan->dmac_id; /* 0 */
    dmac_cfg.chan_id = chan->chan_id;    /* 0~7 */
    /* set src and dst. */

    dmac_cfg.src_addr = p2ap((addr_t)_paddr(src));
    dmac_cfg.dst_addr = p2ap((addr_t)_paddr(dest));

    //
    dmac_cfg.src_transfer_type = DMA1_TRANSFER_TYPE_LLI;
    dmac_cfg.dst_transfer_type = DMA1_TRANSFER_TYPE_LLI;
    dmac_cfg.src_tr_width =
        __builtin_ctz(((addr_t)src | (addr_t)dest | len | 4));
    dmac_cfg.dst_tr_width = dmac_cfg.src_tr_width;

    dprintf(DW_DMA1_DEBUG, "debug (0x%llx)(0x%llx)(0x%x)(0x%x)!\n",
            dmac_cfg.src_addr, dmac_cfg.dst_addr, (u32)len,
            dmac_cfg.src_tr_width);
    /* Destination Address Increment 0:Increment 1: No Change */
    dmac_cfg.sinc = DMA1_TRANSFER_ADDR_INCREMENT;
    /* Source Address Increment 0:Increment 1: No Change */
    dmac_cfg.dinc = DMA1_TRANSFER_ADDR_INCREMENT;

    dmac_cfg.src_msize = DMA_BURST_TR_16ITEMS; /*32 data item*/
    dmac_cfg.dst_msize = DMA_BURST_TR_16ITEMS;

    dmac_cfg.dma_transfer_type = DMA_MEM2MEM;
    dmac_cfg.channel_pri = dwc->priority;
    dmac_cfg.arlen = 3;
    dmac_cfg.awlen = 3;
    dmac_cfg.arlen_en = 1;
    dmac_cfg.awlen_en = 1;

    dmac_cfg.hs_sel_src = 0;
    dmac_cfg.hs_sel_dst = 0;

    dmac_cfg.block_transfer_size =
        bytes2block(dwc->blk_size, len, dmac_cfg.src_tr_width, &tr_count);

    dmac_cfg.dmac_int_en = 0;
    dmac_cfg.llp = 0;

    dprintf(DW_DMA1_DEBUG,
            "block_transfer_size(%d) src_tr_width(0x%d) tr_cnt(%d)!\n",
            dmac_cfg.block_transfer_size, dmac_cfg.src_tr_width, (int)tr_count);

    for (size_t offset = 0; offset < len; offset += tr_count) {
        dmac1_ch_ctl_t ctl = {0};

        ctl.sinc = dmac_cfg.sinc;
        ctl.dinc = dmac_cfg.dinc;
        ctl.src_tr_width = dmac_cfg.src_tr_width;
        ctl.dst_tr_width = dmac_cfg.dst_tr_width;
        ctl.src_msize = dmac_cfg.src_msize;
        ctl.dst_msize = dmac_cfg.dst_msize;
        ctl.arlen_en = dmac_cfg.arlen_en;
        ctl.arlen = dmac_cfg.arlen;
        ctl.awlen_en = dmac_cfg.awlen_en;
        ctl.awlen = dmac_cfg.awlen;
        /* interrupt 0 disable */
        ctl.ioc_blk_tr = 1;
        /* not last shadow. */
        ctl.shadow_lli_valid = 1;
        curr = make_dw_dmac1_lli();
        if (!curr)
            goto err_desc;

        curr->lli_item.sar = dmac_cfg.src_addr + offset;
        curr->lli_item.dsr = dmac_cfg.dst_addr + offset;

        curr->lli_item.block_size =
            bytes2block(dwc->blk_size, len - offset, dmac_cfg.src_tr_width,
                        &tr_count) -
            1;

        ctl.shadow_lli_last = 0;

        curr->lli_item.ctl = ctl.vl;
        curr->lli_item.ctl = curr->lli_item.ctl | ((u64)ctl.vh << 32);

        dprintf(DW_DMA1_DEBUG, "lli item: %d \n", curr->id);
        /* first block */
        if (!first) {
            first = curr;
            list_add_tail(&first->dw_list, &curr->lli_node);
        } else {
            prev->lli_item.llp = curr->phy_addr;
            dprintf(DW_DMA1_DEBUG, "lli: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
                    prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
                    prev->lli_item.block_size, prev->lli_item.ctl);
            list_add_tail(&first->dw_list, &curr->lli_node);
        }
        prev = curr;
    }
    if (prev == NULL) {
        dprintf(CRITICAL, "%s: lli_list is NULL \n", __func__);
        return NULL;
    }
    /* Set to  last lli . */
    prev->lli_item.ctl |= DW_DMAC1_LLI_LAST_OFFSET;

    dprintf(DW_DMA1_DEBUG, "lli last: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
            prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
            prev->lli_item.block_size, prev->lli_item.ctl);

    dmac_cfg.llp = first->phy_addr;
    prev->lli_item.llp = 0;
    dprintf(DW_DMA1_DEBUG, "llp 0x%llx \n", dmac_cfg.llp);
    dprintf(DW_DMA1_DEBUG, "Last len: %d \n",
            (int)list_length(&prev->lli_node));

    list_for_each_entry(curr, lli_list_t, &first->dw_list, lli_node)
    {
        dprintf(DW_DMA1_DEBUG, "list: %d \n", curr->id);
        arch_clean_cache_range((addr_t)&curr->lli_item, sizeof(lli_list_t));
    }

    /* Handle interrupt flag */
    if (DMA_INTERRUPT & flags) {
        /* Enable interrupt */
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        mask.ch_src_suspended = 1;
        mask.ch_suspended = 1;
        mask.ch_disabled = 1;
        mask.ch_aborted = 1;
        dmac_cfg.dmac_int_mask = mask.vl;
    } else {
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        dmac_cfg.dmac_int_mask = mask.vl;
    }

    spin_lock_irqsave(&dwc->chan.lock, state);
    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));
    dwc->len = len;
    spin_unlock_irqrestore(&dwc->chan.lock, state);
    return dw_get_desc(dwc, first, flags);
err_desc:
    /* remove previous allocated desc */
    dw_free_lli_list(first);
    return NULL;
}

struct dma_desc *dw_prep_dma1_memset(struct dma_chan *chan, u8 val,
                                     void *buf_addr, size_t count,
                                     unsigned long flags)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    size_t tr_count = 0;
    dmac1_coeff_t dmac_cfg = {0};
    spin_lock_saved_state_t state;

    struct lli_list *curr = NULL;
    struct lli_list *first = NULL;
    struct lli_list *prev = NULL;
    if (!count) {
        dprintf(CRITICAL, "%s: bad count  (= 0)\n", __func__);
        return NULL;
    }
    /* set dmac instance */
    dmac_cfg.dmac_index = chan->dmac_id; /* 0 */
    dmac_cfg.chan_id = chan->chan_id;    /* 0~7 */

    /* set src and dst. */
    dwc->memset_val = val;
    dwc->tr_type = DMA_MEMSET;
#if ARCH_ARM64
    dmac_cfg.src_addr = p2ap((addr_t)_paddr(&dwc->memset_val));
#else
    dmac_cfg.src_addr = p2ap((addr_t)_paddr(&dwc->memset_val));
#endif
    dmac_cfg.dst_addr = p2ap((addr_t)_paddr(buf_addr));

    dmac_cfg.src_transfer_type = DMA1_TRANSFER_TYPE_CONTINUOUS;
    dmac_cfg.dst_transfer_type = DMA1_TRANSFER_TYPE_LLI;
    dmac_cfg.src_tr_width = DMA1_TRANSFER_WIDTH_8BITS;
    dmac_cfg.dst_tr_width = dmac_cfg.src_tr_width;

    dprintf(DW_DMA1_DEBUG, "debug (0x%llx)(0x%llx)(0x%x)(0x%x)!\n",
            dmac_cfg.src_addr, dmac_cfg.dst_addr, (u32)count,
            dmac_cfg.src_tr_width);
    /* Destination Address Increment 0:Increment 1: No Change */
    dmac_cfg.sinc = DMA1_TRANSFER_ADDR_NOCHANGE;
    /* Source Address Increment 0:Increment 1: No Change */
    dmac_cfg.dinc = DMA1_TRANSFER_ADDR_INCREMENT;

    dmac_cfg.src_msize = DMA_BURST_TR_1ITEM; /*32 data item*/
    dmac_cfg.dst_msize = DMA_BURST_TR_16ITEMS;

    dmac_cfg.dma_transfer_type = DMA_MEM2MEM;
    dmac_cfg.channel_pri = dwc->priority;
    dmac_cfg.arlen = 0;
    dmac_cfg.awlen = 0;
    dmac_cfg.arlen_en = 1;
    dmac_cfg.awlen_en = 1;

    dmac_cfg.hs_sel_src = 0;
    dmac_cfg.hs_sel_dst = 0;

    dmac_cfg.block_transfer_size =
        bytes2block(dwc->blk_size, count, dmac_cfg.src_tr_width, &tr_count);

    dmac_cfg.dmac_int_en = 0;
    dmac_cfg.llp = 0;

    dprintf(DW_DMA1_DEBUG,
            "block_transfer_size(%d) src_tr_width(0x%d) tr_cnt(%d)!\n",
            dmac_cfg.block_transfer_size, dmac_cfg.src_tr_width, (int)tr_count);

    for (size_t offset = 0; offset < count; offset += tr_count) {
        dmac1_ch_ctl_t ctl = {0};

        ctl.sinc = dmac_cfg.sinc;
        ctl.dinc = dmac_cfg.dinc;
        ctl.src_tr_width = dmac_cfg.src_tr_width;
        ctl.dst_tr_width = dmac_cfg.dst_tr_width;
        ctl.src_msize = dmac_cfg.src_msize;
        ctl.dst_msize = dmac_cfg.dst_msize;
        ctl.arlen_en = dmac_cfg.arlen_en;
        ctl.arlen = dmac_cfg.arlen;
        ctl.awlen_en = dmac_cfg.awlen_en;
        ctl.awlen = dmac_cfg.awlen;
        ctl.ioc_blk_tr = 1; /* interrupt 0 disable */
        /* not last shadow. */
        ctl.shadow_lli_valid = 1; //
        curr = make_dw_dmac1_lli();
        if (!curr)
            goto err_desc;

        curr->lli_item.sar = dmac_cfg.src_addr;
        /*dmac_cfg_dst_addr is buf */
        curr->lli_item.dsr = dmac_cfg.dst_addr + offset;

        curr->lli_item.block_size =
            bytes2block(dwc->blk_size, count - offset, dmac_cfg.src_tr_width,
                        &tr_count) -
            1;

        ctl.shadow_lli_last = 0;

        curr->lli_item.ctl = ctl.vl;
        curr->lli_item.ctl = curr->lli_item.ctl | ((u64)ctl.vh << 32);

        dprintf(DW_DMA1_DEBUG, "lli item: %d \n", curr->id);

        if (!first) {

            first = curr;
            list_add_tail(&first->dw_list, &curr->lli_node);
        } else {
            prev->lli_item.llp = curr->phy_addr;
            dprintf(DW_DMA1_DEBUG, "lli: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
                    prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
                    prev->lli_item.block_size, prev->lli_item.ctl);
            list_add_tail(&first->dw_list, &curr->lli_node);
        }

        prev = curr;
    }
    if (prev == NULL) {
        dprintf(CRITICAL, "%s: lli_list is NULL \n", __func__);
        return NULL;
    }
    /* Set to  last lli . */
    prev->lli_item.ctl |= DW_DMAC1_LLI_LAST_OFFSET;

    dprintf(DW_DMA1_DEBUG, "lli last: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
            prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
            prev->lli_item.block_size, prev->lli_item.ctl);

    dmac_cfg.llp = first->phy_addr;
    prev->lli_item.llp = 0;
    dprintf(DW_DMA1_DEBUG, "llp 0x%llx \n", dmac_cfg.llp);
    dprintf(DW_DMA1_DEBUG, "Last len: %d \n",
            (int)list_length(&prev->lli_node));

    list_for_each_entry(curr, lli_list_t, &first->dw_list, lli_node)
    {
        arch_clean_cache_range((addr_t)&curr->lli_item, sizeof(lli_list_t));
    }

    /* Handle interrupt flag */
    if (DMA_INTERRUPT & flags) {
        /* Enable interrupt */
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        mask.ch_src_suspended = 1;
        mask.ch_suspended = 1;
        mask.ch_disabled = 1;
        mask.ch_aborted = 1;
        dmac_cfg.dmac_int_mask = mask.vl;
    } else {
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        dmac_cfg.dmac_int_mask = mask.vl;
    }

    spin_lock_irqsave(&dwc->chan.lock, state);
    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));
    /* The width is 8 s */
    dwc->len = count << dmac_cfg.src_tr_width;
    spin_unlock_irqrestore(&dwc->chan.lock, state);
    return dw_get_desc(dwc, first, flags);
err_desc:
    /* remove previous allocated desc */
    dw_free_lli_list(first);
    return NULL;
}

struct dma_desc *dw_prep_dma1_dev(struct dma_chan *chan, void *buf_addr,
                                  size_t buf_len, unsigned long flags)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    size_t tr_count = 0;
    dmac1_coeff_t dmac_cfg = {0};
    spin_lock_saved_state_t state;

    struct lli_list *curr = NULL;
    struct lli_list *first = NULL;
    struct lli_list *prev = NULL;
    if (!buf_len) {
        dprintf(CRITICAL, "%s: bad buffer length (= 0)\n", __func__);
        return NULL;
    }
    dwc->tr_type = DMA_SLAVE;
    /* set dmac instance */
    dmac_cfg.dmac_index = chan->dmac_id; /* 0 */
    dmac_cfg.chan_id = chan->chan_id;    /* 0~7 */
    /* set src and dst. */

    dmac_cfg.src_transfer_type = DMA1_TRANSFER_TYPE_LLI;
    dmac_cfg.dst_transfer_type = DMA1_TRANSFER_TYPE_LLI;

    dmac_cfg.channel_pri = dwc->priority;
    dmac_cfg.arlen = 3;
    dmac_cfg.awlen = 3;
    dmac_cfg.arlen_en = 1;
    dmac_cfg.awlen_en = 1;

    dmac_cfg.hs_sel_src = 0;

    dmac_cfg.dmac_int_en = 1;
    dmac_cfg.llp = 0;

    switch (dwc->dev_cfg.direction) {

    case DMA_MEM2DEV:

        dmac_cfg.src_addr = p2ap((addr_t)_paddr(buf_addr));
        dmac_cfg.dst_addr = p2ap(dwc->dev_cfg.dst_addr);

        dmac_cfg.src_tr_width = __builtin_ctz(((addr_t)buf_addr | buf_len | 4));
        dmac_cfg.dst_tr_width = __builtin_ctz(dwc->dev_cfg.dst_addr_width);
        dmac_cfg.src_msize = dwc->dev_cfg.src_maxburst;
        dmac_cfg.dst_msize = dwc->dev_cfg.dst_maxburst;
        /* Destination Address Increment 0:Increment 1: No Change */
        dmac_cfg.sinc = DMA1_TRANSFER_ADDR_INCREMENT;
        /* Source Address Increment 0:Increment 1: No Change */
        dmac_cfg.dinc = DMA1_TRANSFER_ADDR_NOCHANGE;
        dmac_cfg.dma_transfer_type = DMA_MEM2DEV;
        break;

    case DMA_DEV2MEM:
        dmac_cfg.src_addr = p2ap(dwc->dev_cfg.src_addr);
        dmac_cfg.dst_addr = p2ap((addr_t)_paddr(buf_addr));

        dmac_cfg.src_tr_width = __builtin_ctz(dwc->dev_cfg.src_addr_width);
        dmac_cfg.dst_tr_width = __builtin_ctz(((addr_t)buf_addr | buf_len | 4));
        dprintf(DW_DMA1_DEBUG,
                "dev_cfg.src_tr_width %d dmac_cfg.src_tr_width %d \n",
                (int)dwc->dev_cfg.src_addr_width, (int)dmac_cfg.src_tr_width);

        dmac_cfg.src_msize = dwc->dev_cfg.src_maxburst;
        dmac_cfg.dst_msize = dwc->dev_cfg.dst_maxburst;

        /* Source Address Increment 0:Increment 1: No Change */
        dmac_cfg.sinc = DMA1_TRANSFER_ADDR_NOCHANGE;
        /* Destination Address Increment 0:Increment 1: No Change */
        dmac_cfg.dinc = DMA1_TRANSFER_ADDR_INCREMENT;
        dmac_cfg.dma_transfer_type = DMA_DEV2MEM;
        break;
    default:
        return NULL;
    }
    dmac_cfg.block_transfer_size =
        bytes2block(dwc->blk_size, buf_len, dmac_cfg.src_tr_width, &tr_count);

    for (size_t offset = 0; offset < buf_len; offset += tr_count) {
        dmac1_ch_ctl_t ctl = {0};
        ctl.sinc = dmac_cfg.sinc;
        ctl.dinc = dmac_cfg.dinc;
        ctl.src_tr_width = dmac_cfg.src_tr_width;
        ctl.dst_tr_width = dmac_cfg.dst_tr_width;
        ctl.src_msize = dmac_cfg.src_msize;
        ctl.dst_msize = dmac_cfg.dst_msize;
        ctl.arlen_en = dmac_cfg.arlen_en;
        ctl.arlen = dmac_cfg.arlen;
        ctl.awlen_en = dmac_cfg.awlen_en;
        ctl.awlen = dmac_cfg.awlen;
        /* interrupt 0 disable */
        ctl.ioc_blk_tr = 1;
        /* not last shadow. */
        ctl.shadow_lli_valid = 1; //
        curr = make_dw_dmac1_lli();
        if (!curr)
            goto err_desc;

        if (DMA_MEM2DEV == dwc->dev_cfg.direction) {

            curr->lli_item.sar = dmac_cfg.src_addr + offset;
            curr->lli_item.dsr = dmac_cfg.dst_addr;
        } else {
            curr->lli_item.sar = dmac_cfg.src_addr;
            curr->lli_item.dsr = dmac_cfg.dst_addr + offset;
        }

        curr->lli_item.block_size =
            bytes2block(dwc->blk_size, buf_len - offset, dmac_cfg.src_tr_width,
                        &tr_count) -
            1;

        ctl.shadow_lli_last = 0;

        curr->lli_item.ctl = ctl.vl;
        curr->lli_item.ctl = curr->lli_item.ctl | ((u64)ctl.vh << 32);

        dprintf(DW_DMA1_DEBUG, "lli item: %d \n", curr->id);
        /* first block */
        if (!first) {
            first = curr;
            list_add_tail(&first->dw_list, &curr->lli_node);
        } else {
            prev->lli_item.llp = curr->phy_addr;
            dprintf(DW_DMA1_DEBUG, "lli: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
                    prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
                    prev->lli_item.block_size, prev->lli_item.ctl);
            list_add_tail(&first->dw_list, &curr->lli_node);
        }
        prev = curr;
    }
    if (prev == NULL) {
        dprintf(CRITICAL, "%s: lli_list is NULL \n", __func__);
        return NULL;
    }
    prev->lli_item.ctl |= DW_DMAC1_LLI_LAST_OFFSET;

    dprintf(DW_DMA1_DEBUG, "lli last: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
            prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
            prev->lli_item.block_size, prev->lli_item.ctl);

    dmac_cfg.llp = first->phy_addr;
    prev->lli_item.llp = 0;
    dprintf(DW_DMA1_DEBUG, "llp 0x%llx \n", dmac_cfg.llp);
    dprintf(DW_DMA1_DEBUG, "Last len: %d \n",
            (int)list_length(&prev->lli_node));

    list_for_each_entry(curr, lli_list_t, &first->dw_list, lli_node)
    {
        dprintf(DW_DMA1_DEBUG, "list: %d \n", curr->id);
        arch_clean_cache_range((addr_t)&curr->lli_item, sizeof(lli_list_t));
    }

    /* Handle interrupt flag */
    if (DMA_INTERRUPT & flags) {
        /* Enable interrupt */
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        mask.ch_src_suspended = 1;
        mask.ch_suspended = 1;
        mask.ch_disabled = 1;
        mask.ch_aborted = 1;
        dmac_cfg.dmac_int_mask = mask.vl;
    } else {
        /*TODO: Need enable for handle tr status
         This mode will not stop */
        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        dmac_cfg.dmac_int_mask = mask.vl;
    }
    spin_lock_irqsave(&chan->lock, state);
    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));
    dwc->len = buf_len;
    spin_unlock_irqrestore(&chan->lock, state);
    return dw_get_desc(dwc, first, flags);
err_desc:
    /* Remove previous allocated desc */
    dw_free_lli_list(first);
    return NULL;
}

struct dma_desc *dw_prep_dma1_cyclic(struct dma_chan *chan, void *buf_addr,
                                     size_t buf_len, size_t period_len,
                                     unsigned long flags)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    size_t tr_count = 0;

    dmac1_coeff_t dmac_cfg = {0};
    spin_lock_saved_state_t state;

    struct lli_list *curr = NULL;
    struct lli_list *first = NULL;
    struct lli_list *prev = NULL;

    /* Check data len */
    if (!buf_len) {
        dprintf(CRITICAL, "%s: bad buffer length (= 0)\n", __func__);
        return NULL;
    }

    if (buf_len % period_len) {
        dprintf(
            CRITICAL,
            "%s: buffer_length (%zd) is not a multiple of period_len (%zd)\n",
            __func__, buf_len, period_len);
    }

    if ((period_len >> dmac_cfg.src_tr_width) > dwc->blk_size) {
        dprintf(CRITICAL, "%s: period_len (%zd) is bigger than blk_size (%d)\n",
                __func__, period_len, dwc->blk_size);
    }

    /* set dmac instance */
    dmac_cfg.dmac_index = chan->dmac_id; /* 0 */
    dmac_cfg.chan_id = chan->chan_id;    /* 0~7 */
    /* set src and dst. */
    dwc->tr_type = DMA_CYCLIC;

    dmac_cfg.src_transfer_type = DMA1_TRANSFER_TYPE_LLI;
    dmac_cfg.dst_transfer_type = DMA1_TRANSFER_TYPE_LLI;

    dmac_cfg.channel_pri = dwc->priority;
    dmac_cfg.arlen = 3;
    dmac_cfg.awlen = 3;
    dmac_cfg.arlen_en = 1;
    dmac_cfg.awlen_en = 1;

    dmac_cfg.hs_sel_src = 0;

    dmac_cfg.dmac_int_en = 1;
    dmac_cfg.llp = 0;

    switch (dwc->dev_cfg.direction) {
    case DMA_MEM2DEV:

        dmac_cfg.src_addr = p2ap((addr_t)_paddr(buf_addr));
        dmac_cfg.dst_addr = p2ap(dwc->dev_cfg.dst_addr);

        dmac_cfg.src_msize = dwc->dev_cfg.src_maxburst; /*32 data item*/
        dmac_cfg.dst_msize = dwc->dev_cfg.dst_maxburst;

        dmac_cfg.src_tr_width =
            __builtin_ctz(((addr_t)buf_addr | period_len | 4));
        dmac_cfg.dst_tr_width = __builtin_ctz(dwc->dev_cfg.dst_addr_width);
        /* Destination Address Increment 0:Increment 1: No Change */
        dmac_cfg.sinc = DMA1_TRANSFER_ADDR_INCREMENT;
        /* Source Address Increment 0:Increment 1: No Change */
        dmac_cfg.dinc = DMA1_TRANSFER_ADDR_NOCHANGE;
        dmac_cfg.dma_transfer_type = DMA_MEM2DEV;
        break;

    case DMA_DEV2MEM:
        dmac_cfg.src_addr = p2ap(dwc->dev_cfg.src_addr);
        dmac_cfg.dst_addr = p2ap((addr_t)_paddr(buf_addr));

        dmac_cfg.src_msize = dwc->dev_cfg.src_maxburst; /*32 data item*/
        dmac_cfg.dst_msize = dwc->dev_cfg.dst_maxburst;

        dmac_cfg.src_tr_width = __builtin_ctz(dwc->dev_cfg.src_addr_width);
        dmac_cfg.dst_tr_width =
            __builtin_ctz(((addr_t)buf_addr | period_len | 4));
        /* Source Address Increment 0:Increment 1: No Change */
        dmac_cfg.sinc = DMA1_TRANSFER_ADDR_NOCHANGE;
        /* Destination Address Increment 0:Increment 1: No Change */
        dmac_cfg.dinc = DMA1_TRANSFER_ADDR_INCREMENT;
        dmac_cfg.dma_transfer_type = DMA_DEV2MEM;
        break;
    default:
        return NULL;
    }
    dmac_cfg.block_transfer_size =
        bytes2block(dwc->blk_size, buf_len, dmac_cfg.src_tr_width, &tr_count);

    for (size_t offset = 0; offset < buf_len; offset += period_len) {
        dmac1_ch_ctl_t ctl = {0};
        ctl.sinc = dmac_cfg.sinc;
        ctl.dinc = dmac_cfg.dinc;
        ctl.src_tr_width = dmac_cfg.src_tr_width;
        ctl.dst_tr_width = dmac_cfg.dst_tr_width;
        ctl.src_msize = dmac_cfg.src_msize;
        ctl.dst_msize = dmac_cfg.dst_msize;
        ctl.arlen_en = dmac_cfg.arlen_en;
        ctl.arlen = dmac_cfg.arlen;
        ctl.awlen_en = dmac_cfg.awlen_en;
        ctl.awlen = dmac_cfg.awlen;
        ctl.ioc_blk_tr = 1; /* interrupt 0 disable */
        /* not last shadow. */
        ctl.shadow_lli_valid = 1; //
        curr = make_dw_dmac1_lli();
        if (!curr)
            goto err_desc;

        if (DMA_MEM2DEV == dwc->dev_cfg.direction) {

            curr->lli_item.sar = dmac_cfg.src_addr + offset;
            curr->lli_item.dsr = dmac_cfg.dst_addr;
        } else {
            curr->lli_item.sar = dmac_cfg.src_addr;
            curr->lli_item.dsr = dmac_cfg.dst_addr + offset;
        }

        curr->lli_item.block_size =
            bytes2block(dwc->blk_size, period_len, dmac_cfg.src_tr_width,
                        &tr_count) -
            1;
        if (tr_count < dwc->blk_size << dmac_cfg.src_tr_width) {
            dprintf(DW_DMA1_DEBUG, "llp last block! %d %d  0x%x 0x%x\n",
                    (int)tr_count, (int)(buf_len - offset),
                    curr->lli_item.block_size, dmac_cfg.src_tr_width);
        }

        curr->lli_item.ctl = ctl.vl;
        curr->lli_item.ctl = curr->lli_item.ctl | ((u64)ctl.vh << 32);

        dprintf(DW_DMA1_DEBUG, "lli item: %d \n", curr->id);
        /* first block */
        if (!first) {
            first = curr;
            list_add_tail(&first->dw_list, &curr->lli_node);
        } else {
            prev->lli_item.llp = curr->phy_addr;
            dprintf(DW_DMA1_DEBUG, "lli: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
                    prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
                    prev->lli_item.block_size, prev->lli_item.ctl);
            list_add_tail(&first->dw_list, &curr->lli_node);
        }
        prev = curr;
    }
    if (prev == NULL) {
        dprintf(CRITICAL, "%s: lli_list is NULL \n", __func__);
        return NULL;
    }
    /* last llp use first frames */
    prev->lli_item.llp = first->phy_addr;
    dprintf(DW_DMA1_DEBUG, "lli: 0x%llx 0x%llx 0x%llx 0x%x 0x%llx \n",
            prev->lli_item.llp, prev->lli_item.sar, prev->lli_item.dsr,
            prev->lli_item.block_size, prev->lli_item.ctl);

    dmac_cfg.llp = first->phy_addr;
    dprintf(DW_DMA1_DEBUG, "llp 0x%llx \n", dmac_cfg.llp);
    dprintf(DW_DMA1_DEBUG, "Last len: %d \n",
            (int)list_length(&prev->lli_node));

    list_for_each_entry(curr, lli_list_t, &first->dw_list, lli_node)
    {
        dprintf(DW_DMA1_DEBUG, "list: %d \n", curr->id);
        arch_clean_cache_range((addr_t)&curr->lli_item, sizeof(lli_list_t));
    }

    /* Handle interrupt flag */
    if (DMA_INTERRUPT & flags) {
        /* Enable interrupt */
        dmac_cfg.dmac_int_en = 1;

        dmac1_ch_int_stat_en_t mask = {0};
        mask.block_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        mask.ch_src_suspended = 1;
        mask.ch_suspended = 1;
        mask.ch_disabled = 1;
        mask.ch_aborted = 1;
        dmac_cfg.dmac_int_mask = mask.vl;
    } else {

        dmac_cfg.dmac_int_en = 1;
        dmac1_ch_int_stat_en_t mask = {0};
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA1_ALL_ERR;
        dmac_cfg.dmac_int_mask = mask.vl;
    }
    spin_lock_irqsave(&chan->lock, state);
    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));
    dwc->len = buf_len;
    spin_unlock_irqrestore(&chan->lock, state);
    return dw_get_desc(dwc, first, flags);
err_desc:
    /* Remove previous allocated desc */
    dw_free_lli_list(first);
    return NULL;
}

void dw_dma1_submit(struct dma_desc *desc)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(desc->chan);
    spin_lock_saved_state_t state;
    dprintf(DW_DMA1_DEBUG, "src: 0x%llx dst: 0x%llx \n", dwc->dmac_cfg.src_addr,
            dwc->dmac_cfg.dst_addr);
    if (DMA1_TRANSFER_DIR_MEM2PER_DMAC == dwc->dmac_cfg.dma_transfer_type) {
        arch_clean_cache_range((addr_t)_ioaddr(ap2p(dwc->dmac_cfg.src_addr)),
                               dwc->len);
    } else if (DMA1_TRANSFER_DIR_PER2MEM_DMAC ==
               dwc->dmac_cfg.dma_transfer_type) {

        arch_invalidate_cache_range(
            (addr_t)_ioaddr(ap2p(dwc->dmac_cfg.dst_addr)), dwc->len);
    } else if (DMA1_TRANSFER_DIR_MEM2MEM_DMAC ==
               dwc->dmac_cfg.dma_transfer_type) {
        if (DMA_MEMSET == dwc->tr_type) {
            /* Memset only invalidate one cache size. */

            arch_clean_cache_range(
                (addr_t)_ioaddr(ap2p(dwc->dmac_cfg.src_addr)), 1);
        } else {

            arch_clean_cache_range(
                (addr_t)_ioaddr(ap2p(dwc->dmac_cfg.src_addr)), dwc->len);
        }
        arch_invalidate_cache_range(
            (addr_t)_ioaddr(ap2p(dwc->dmac_cfg.dst_addr)), dwc->len);
    }
    // TODO: this part will spe
    spin_lock_irqsave(&dwc->chan.lock, state);
    desc->chan->chan_status = DMA_IN_PROGRESS;

    if (1 == dwc->dmac_cfg.dmac_int_en) {
        dwc->dmac_cfg.irq_callback.dmac_irq_evt_handle =
            (void(*)(u32, u32, void*))desc->dmac_irq_evt_handle;
        dwc->dmac_cfg.irq_callback.context = desc->context;
    }

    dw_dmac1_cfg(dwc->dmac_cfg);
    dw_dmac1_ch_enable(dwc->dmac_cfg);
    spin_unlock_irqrestore(&dwc->chan.lock, state);
}

enum dma_status dw_dma1_get_chan_status(struct dma_chan *chan)
{
    struct dw_dma1_chan *dwc = to_dw_dma1_chan(chan);
    // Don't change channel status here
    if (dw_dmac1_tr_done(dwc->dmac_cfg)) {
        return DMA_COMP;
    } else {
        return DMA_IN_PROGRESS;
    }
}

enum dma_status dw_dma1_sync_wait(struct dma_desc *desc, int timeout)
{
    dprintf(DW_DMA1_DEBUG, "%s: %d\n", __FUNCTION__, __LINE__);
    lk_time_t start = current_time();
    while ((start + timeout > current_time()) || (timeout < 0)) {

        if (DMA_COMP == dw_dma1_get_chan_status(desc->chan)) {

            dprintf(DW_DMA1_DEBUG, "%s: %d (%d)\n", __FUNCTION__, __LINE__,
                    DMA_COMP);
            return DMA_COMP;
        }
        thread_sleep(1);
    }
    dprintf(DW_DMA1_DEBUG, "Wait time out! %d ms  \n", timeout);
    desc->chan->chan_status = DMA_ERR;
    return DMA_ERR;
}

bool dw_dma1_terminate(struct dma_desc *desc)
{
    return dw_dma1_chan_terminate(desc->chan);
}

void dw_dma1_free_desc(struct dma_desc *desc)
{
    // release channel.
    dw_dma1_chan_t *dwc = to_dw_dma1_chan(desc->chan);
    dwc->allocated_desc = 0;
    if (0 == dwc->allocated_desc) {
        dwc->allocated_channel = 0;
    }
    // clear type
    dwc->tr_type = DMA_MEMCPY;
    dw_dmac1_clear_chan_stat(dwc->dmac_cfg);
    dw_dma1_terminate(desc);
    dw_dma1_remove_desc(desc);
}

enum handler_return dw_dma1_interrupt_handle(void *arg)
{
    unsigned int instance = (uintptr_t)arg;
    spin_lock_saved_state_t state;
    dma_chan_t *cptr = NULL;

    spin_lock_irqsave(&lock, state);

    list_for_every_entry(dma1_chan_list, cptr, dma_chan_t, node)
    {

        if (cptr->inst_id == instance) {
            dprintf(DW_DMA1_DEBUG, "IRQ status enter \n");

            int chan = cptr->chan_id;
            int dmac_inst = cptr->dmac_id;
            if (dw_dmac1_ch_has_int(dmac_inst, chan)) {
                /* get status */
                dmac1_ch_int_stat_t status =
                    dw_dmac1_get_ch_int_stat(dmac_inst, chan);
                /* clear status first */
                dw_dmac1_clr_int_stat(dmac_inst, chan, status);
                dprintf(DW_DMA1_DEBUG,
                        "IRQ status dw_dmac inst(%d) chan(0x%x) "
                        "status(0x%x)(0x%x)!\n",
                        dmac_inst, chan, status.vl, status.vh);

                /* Here handle dma functions. */
                /*  DON'T change the line for a irq handle bug. handle irq here.
                 */
                dw_dmac1_irq_handle(dmac_inst, chan, status);

                if (status.vl & DMA1_ALL_ERR) {
                    dw_dma1_chan_handle_err(cptr);
                } else if (status.vl & DMA1_DMA_TR_DONE) {

                    dw_dma1_chan_tr_done(cptr);
                } else if (status.vl & DMA1_ABORTED) {
                    /* DMA aborted. */
                } else if (status.vl & DMA1_DISABLED) {
                    /* DMA disable. */
                }
            }
        }
    }
    spin_unlock_irqrestore(&lock, state);
    return INT_NO_RESCHEDULE;
}