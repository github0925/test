/*
 * dma.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * dma driver head file.
 * TBD
 *
 * Revision History:
 * -----------------
 * 0.1, 10/22/2019 yishao init version
 */
#ifndef __DW_DMA_H
#define __DW_DMA_H
#include "dma_cap.h"
#include "dw_dmac.h"
#include <kernel/spinlock.h>
/**
 * struct dw_dma_chan - internal dma channel structure
 */
typedef struct dw_dma_chan {

    /* protected by chan.lock. */
    struct dma_chan chan;
    struct dma_dev_cfg dev_cfg;
    enum dma_tr_direction direction;
    enum dma_transaction_type tr_type;
    u8 priority;
    u8 memset_val;
    unsigned int allocated_desc; /* desc already allocated. */
    unsigned int allocated_channel;
    /* hardware configuration */
    unsigned int blk_size;
    unsigned int len;
    dmac_coeff_t dmac_cfg;

} dw_dma_chan_t;

void dw_dma_reg(dma_instance_t *instance, struct list_node *channels);
/*unregister dma channels*/
void dw_dma_unreg(void);
bool dw_dma_chan_req(struct dma_chan *chan);
void dw_dma_dev_config(struct dma_chan *chan, struct dma_dev_cfg *cfg);
struct dma_desc *dw_prep_dma_memcpy(struct dma_chan *chan, void *dest,
                                    void *src, size_t len, unsigned long flags);
struct dma_desc *dw_prep_dma_memset(struct dma_chan *chan, u8 val,
                                    void *buf_addr, size_t count,
                                    unsigned long flags);
struct dma_desc *dw_prep_dma_dev(struct dma_chan *chan, void *buf_addr,
                                 size_t buf_len, unsigned long flags);
struct dma_desc *dw_prep_dma_cyclic(struct dma_chan *chan, void *buf_addr,
                                    size_t buf_len, size_t period_len,
                                    unsigned long flags);
void dw_dma_submit(struct dma_desc *desc);

enum dma_status dw_dma_sync_wait(struct dma_desc *desc, int timeout);

enum handler_return dw_dma_interrupt_handle(void *arg);
bool dw_dma_terminate(struct dma_desc *desc);
void dw_dma_free_desc(struct dma_desc *desc);
#endif