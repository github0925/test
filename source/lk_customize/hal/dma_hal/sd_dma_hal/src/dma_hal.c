//*****************************************************************************
//
// dma_hal.c - Driver for the dma hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "dma_hal.h"
#include "dma_cap.h"
#if SAF_SYSTEM_CFG == 1
#include "dw_dma1.h"
#else
#include "dw_dma.h"
#endif
#include "irq.h"
#include "res.h"
#include <assert.h>
#include <kernel/thread.h>
#include <platform.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <string.h>
#include <sys/types.h>
#include <trace.h>
/* Debug setting */

#define DMA_HAL_DEBUG (SPEW + 2)
#define dbg_print(a)                                                           \
    {                                                                          \
        unsigned long target_addr = (unsigned long)(&(a));                     \
        dprintf(DMA_HAL_DEBUG, "%s:r(%p,0x%x)\n", #a, &(a),                    \
                readl(target_addr));                                           \
    }
// dbg_print with message
#define dbg_info_print(a, s)                                                   \
    {                                                                          \
        unsigned long target_addr = (unsigned long)(&(a));                     \
        dprintf(DMA_HAL_DEBUG, "[%s]:%s:r(%p,0x%x)\n", s, #a, &(a),            \
                readl(target_addr));                                           \
    }
// print long long
#define dbg_printq(a)                                                          \
    {                                                                          \
        unsigned long target_addr = (unsigned long)(&(a));                     \
        dprintf(DMA_HAL_DEBUG, "%s:r(%p,0x%llx)\n", #a, &(a),                  \
                readq(target_addr));                                           \
    }
#define dbg_print_field(a)                                                     \
    {                                                                          \
        dprintf(DMA_HAL_DEBUG, "%s:r(0x%llx)\n", #a, (u64)a);                  \
    }

/*Next structure don't need be export to header files.*/
/*dma controller global index to really number */
typedef struct _dma_glb_idx_to_id {
    uint32_t res_glb_idx;
    uint32_t dma_really_num;
    uint32_t irq_numb;
    uint32_t mux_res_glb_idx;
} dma_glb_idx_to_id;

/*   DMA resource information.  */

typedef struct {
    uint32_t res_glb_idx;
    char res_describe[100];
    paddr_t phy_addr;
    uint32_t addr_range;
} dma_res_info_t;

typedef struct {
    uint32_t version;
    char res_category[20];
    uint8_t res_num;
    /* FIXME: it based on resource define. axi2ahb_sec_res_def */
    dma_res_info_t res_info[DEFAULT_DMA_MAX_NUM];
} dma_src_t;

/*   DMA MUX resource information.  */
typedef struct {
    uint32_t res_glb_idx;
    char res_describe[100];
    paddr_t phy_addr;
    uint32_t addr_range;
} dma_mux_res_info_t;

typedef struct {
    uint32_t version;
    char res_category[20];
    uint8_t res_num;
    /* FIXME: it based on resource define. axi2ahb_sec_res_def */
    dma_mux_res_info_t res_info[DEFAULT_DMA_MAX_NUM];
} dma_mux_src_t;

/*watchdog global instance*/
static dma_instance_t g_dmaInstance[DEFAULT_DMA_MAX_NUM] = {0};
static spin_lock_t dma_spin_lock = SPIN_LOCK_INITIAL_VALUE;
static bool dma_hal_initialized = false;

/*here to declare virtual channel chan_list*/
static struct list_node chan_list = LIST_INITIAL_VALUE(chan_list);
static struct list_node dmac_list = LIST_INITIAL_VALUE(dmac_list);

/*  */

static const dma_glb_idx_to_id g_dma_glb_idx_to_id[DEFAULT_DMA_MAX_NUM] = {
    {RES_DMA_DMA1, 0x1, DMA1_INTR_NUM, RES_DMA_MUX_DMA_MUX1},
    {RES_DMA_DMA2, 0x2, DMA2_INTR_NUM, RES_DMA_MUX_DMA_MUX2},
    {RES_DMA_DMA3, 0x3, DMA3_INTR_NUM, RES_DMA_MUX_DMA_MUX3},
    {RES_DMA_DMA4, 0x4, DMA4_INTR_NUM, RES_DMA_MUX_DMA_MUX4},
    {RES_DMA_DMA5, 0x5, DMA5_INTR_NUM, RES_DMA_MUX_DMA_MUX5},
    {RES_DMA_DMA6, 0x6, DMA6_INTR_NUM, RES_DMA_MUX_DMA_MUX6},
    {RES_DMA_DMA7, 0x7, DMA7_INTR_NUM, RES_DMA_MUX_DMA_MUX7},
    {RES_DMA_DMA8, 0x8, DMA8_INTR_NUM, RES_DMA_MUX_DMA_MUX8},
};

/*dma driver interface*/

/*--------------------next is interface functions
 * ---------------------------------*/

/*here to declare virtual channel chan_list*/

/**
 *  dma_init_early: A dma init function for dma controller hardware
 *  This function should be placed in platform init stage. Here to register
 * dmac. In this function, dma hal alloc resource by table.
 */
void hal_dma_init(void)
{

    /*  init dmac, set instance, unmask interrupt here. */
    /*  Add all avaliable dma controller */
    u32 i = 0;
    u32 j = 0;
    s32 ret = 0;
    int32_t ctrl_id = 0;
    if (true == dma_hal_initialized) {
        dprintf(DMA_HAL_DEBUG, "Deinit dma hal before init.\n");
        hal_dma_deinit();
    }
    /*If reginster instance success, add i */
    for (i = 0; i < DEFAULT_DMA_MAX_NUM; i++) {
        ret = res_get_info_by_id(g_dma_glb_idx_to_id[i].res_glb_idx,
                                 &g_dmaInstance[j].dma_cfg.phy_addr, &ctrl_id);
        if (ret < 0) {
            dprintf(DMA_HAL_DEBUG, "DMAC %d 0x%lx\n", ret,
                    g_dmaInstance[j].dma_cfg.phy_addr);
            continue;
        }

        ret = res_get_info_by_id(g_dma_glb_idx_to_id[i].mux_res_glb_idx,
                                 &g_dmaInstance[j].dma_cfg.mux_phy_addr,
                                 &ctrl_id);

        if (ret < 0) {
            dprintf(DMA_HAL_DEBUG, "DMAC %d \n", ret);
            continue;
        }

        g_dmaInstance[j].inst_id = j;
        g_dmaInstance[j].dma_inited = true;
        g_dmaInstance[j].dma_enabled = true;
        g_dmaInstance[j].dma_cfg.irq_numb = g_dma_glb_idx_to_id[i].irq_numb;

        if (1 == g_dma_glb_idx_to_id[i].dma_really_num) {
            dprintf(DMA_HAL_DEBUG, "DMAC 1\n");
            /*register dma1 */
#if SAF_SYSTEM_CFG == 1
            dw_dma1_reg(&g_dmaInstance[j], &chan_list);
#else
        } else {
            dprintf(DMA_HAL_DEBUG, "DMAC 2~8\n");
            /*register dma 2~8 */
            dw_dma_reg(&g_dmaInstance[j], &chan_list);
#endif
        }
        j++;
    }

    /*  Add virtual channel here */
    dma_hal_initialized = true;

    return;
}

/**
     *  hal_dma_deinit: A dma deinit function for all of dma controllers
   hardware

     */
void hal_dma_deinit(void)
{

#if SAF_SYSTEM_CFG == 1
    dw_dma1_unreg();
#else
    dw_dma_unreg();
#endif
    /*Remove virtual channel list at end. */
    list_clear_node(&chan_list);
    dma_hal_initialized = false;
}

static u32 ch_type_to_cap(enum dma_chan_tr_type ch_type)
{
    if (DMA_MEM == ch_type) {
        return DMA_MEM_CAP;
    } else if ((ch_type >= DMA_PERI_CAN1) && (ch_type <= DMA_PERI_UART8)) {
        return DMA_PERI_CAP1;
    } else if ((ch_type >= DMA_PERI_CAN5) && (ch_type <= DMA_PERI_CAN20)) {
        return DMA_PERI_CAP2;
    } else {
        return 0;
    }
}
/**
 *  Allocate a DMA slave channel
 * return a available dma_chan
 */
struct dma_chan *hal_dma_chan_req(enum dma_chan_tr_type ch_type)
{
    dma_chan_t *cptr = NULL;
    spin_lock_saved_state_t state;
    spin_lock_irqsave(&dma_spin_lock, state);
    u32 cap = ch_type_to_cap(ch_type);
    list_for_every_entry(&chan_list, cptr, dma_chan_t, node)
    {
        /* Filter channel by channel capabilities  */
        if (0 != (cap & cptr->chan_cap)) {
            if (g_dmaInstance[cptr->inst_id].controllerTable.dma_chan_req(
                    cptr)) {
                spin_unlock_irqrestore(&dma_spin_lock, state);

                dprintf(DMA_HAL_DEBUG,
                        "REQ Chan instance(%d) chan(0x%x) vchan(0x%x)!\n",
                        cptr->inst_id, cptr->chan_id, cptr->vchan_id);

                return cptr;
            }
        }
    }
    spin_unlock_irqrestore(&dma_spin_lock, state);
    return NULL;
}

/**
 *  Allocate a DMA slave channel with id number
 * return a available dma_chan
 * It's a debug function for testing
 */
struct dma_chan *hal_dma_chan_req_with_ch(u32 id)
{
    dma_chan_t *cptr = NULL;
    spin_lock_saved_state_t state;
    spin_lock_irqsave(&dma_spin_lock, state);
    list_for_every_entry(&chan_list, cptr, dma_chan_t, node)
    {
        if (id == cptr->vchan_id) {

            if (g_dmaInstance[cptr->inst_id].controllerTable.dma_chan_req(
                    cptr)) {
                spin_unlock_irqrestore(&dma_spin_lock, state);

                dprintf(DMA_HAL_DEBUG,
                        "REQ Chan instance(%d) chan(0x%x) vchan(0x%x)!\n",
                        cptr->inst_id, cptr->chan_id, cptr->vchan_id);

                return cptr;
            }
        }
    }
    spin_unlock_irqrestore(&dma_spin_lock, state);
    return NULL;
}

/*
    dma_dev_config: Set device and controller specific parameters
    @dma_chan the channel of dma_chan_req result
    @dma_dev_cfg  dma device configuration
    //return 0 is success.
    */
void hal_dma_dev_config(struct dma_chan *chan, struct dma_dev_cfg *cfg)
{

    if (g_dmaInstance[chan->inst_id].controllerTable.dma_dev_config) {
        g_dmaInstance[chan->inst_id].controllerTable.dma_dev_config(chan, cfg);
    } else {
        dprintf(CRITICAL, "dma_dev_config failed!\n");
    }
}

/*
    dw_prep_dma_memcpy: Get a descriptor for memory to memory transaction
    @dest the destination of memory copy
    @src  the source of memory copy
    @len  the length of memory copy
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_memcpy(struct dma_chan *chan, void *dest,
                                     void *src, size_t len, unsigned long flags)
{
    if (g_dmaInstance[chan->inst_id].controllerTable.prep_dma_memcpy) {
        return g_dmaInstance[chan->inst_id].controllerTable.prep_dma_memcpy(
            chan, dest, src, len, flags);
    } else {
        dprintf(CRITICAL, "prep_dma_memcpy failed!\n");
    }
    return NULL;
}

/*
    dw_prep_dma_memset: Get a descriptor for memset function.
    @val the value of memset unsigned char
    @buf_addr  the target of memory set
    @count  the number of memory set
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_memset(struct dma_chan *chan, u8 val,
                                     void *buf_addr, size_t count,
                                     unsigned long flags)
{
    if (g_dmaInstance[chan->inst_id].controllerTable.prep_dma_memset) {
        return g_dmaInstance[chan->inst_id].controllerTable.prep_dma_memset(
            chan, val, buf_addr, count, flags);
    } else {
        dprintf(CRITICAL, "prep_dma_memset failed!\n");
    }
    return NULL;
}

/*
    dw_prep_dma_dev: Get a descriptor for memory to peripheral transaction
    @buf_addr the destination of memory
    @buf_len  the length of transfer
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_dev(struct dma_chan *chan, void *buf_addr,
                                  size_t buf_len, unsigned long flags)
{
    if (g_dmaInstance[chan->inst_id].controllerTable.prep_dma_dev) {
        return g_dmaInstance[chan->inst_id].controllerTable.prep_dma_dev(
            chan, buf_addr, buf_len, flags);
    } else {
        dprintf(CRITICAL, "prep_dma_dev failed!\n");
        return NULL;
    }
}

/*
    dw_prep_dma_cyclic: Get a descriptor for memory to peripheral transaction in
   cyclic mode
    @buf_addr the destination of memory
    @buf_len  the length of transfer
    @period_len the length of period, buf_len must be a multiple of period_len.
    @flags Control flags
    return a dma_desc
    */
struct dma_desc *hal_prep_dma_cyclic(struct dma_chan *chan, void *buf_addr,
                                     size_t buf_len, size_t period_len,
                                     unsigned long flags)
{
    if (g_dmaInstance[chan->inst_id].controllerTable.prep_dma_cyclic) {
        return g_dmaInstance[chan->inst_id].controllerTable.prep_dma_cyclic(
            chan, buf_addr, buf_len, period_len, flags);
    } else {
        dprintf(CRITICAL, "prep_dma_cyclic failed!\n");
        return NULL;
    }
}

/*
    dma_submit: Submit a transfer
    @desc the descriptor of a dma transfer
    */
void hal_dma_submit(struct dma_desc *desc)
{
    if (g_dmaInstance[desc->chan->inst_id].controllerTable.dma_submit) {
        g_dmaInstance[desc->chan->inst_id].controllerTable.dma_submit(
            desc);
    } else {
        dprintf(CRITICAL, "dma_submit failed!\n");
    }
}

/*
    dma_sync_wait: Wait for dma transfer result.
    @desc the descriptor of a dma transfer
    @timeout ms if timeout this function will return a error status.
    */
enum dma_status hal_dma_sync_wait(struct dma_desc *desc, int timeout)
{
    if (g_dmaInstance[desc->chan->inst_id].controllerTable.dma_sync_wait) {
        return g_dmaInstance[desc->chan->inst_id].controllerTable.dma_sync_wait(
            desc, timeout);
    } else {
        dprintf(CRITICAL, "dma_submit failed!\n");
        return DMA_ERR;
    }
}

/*
    dma_terminate: terminate a dma channel.
    @chan dma transfer channel.
    */
bool hal_dma_terminate(struct dma_desc *desc)
{
    if (g_dmaInstance[desc->chan->inst_id].controllerTable.dma_terminate) {
        return g_dmaInstance[desc->chan->inst_id].controllerTable.dma_terminate(
            desc);
    } else {
        dprintf(CRITICAL, "dma_terminate failed!\n");
        return false;
    }
}
/*
    dma_free_desc: free a dma descriptor.
    Must free desc when you don't use it.
    @desc dma transfer descriptor.
    */
void hal_dma_free_desc(struct dma_desc *desc)
{
    if (g_dmaInstance[desc->chan->inst_id].controllerTable.dma_free_desc) {
        g_dmaInstance[desc->chan->inst_id].controllerTable.dma_free_desc(desc);
    } else {
        dprintf(CRITICAL, "dma_free_desc failed!\n");
    }
}
