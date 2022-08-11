/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include <atb_crypto.h>
#include "cryptoengine_reg.h"
#include "cryptoengine_reg_field_def.h"
#include "ce_wrapper.h"

/*from sa to extern mem*/
U32 ce_pkemem_read(U32 base, U32 from_off, U32 to, U32 len)
{
    cryptoengine_t *ce = (cryptoengine_t *)(uintptr_t)base;
    uint32_t local_addr;

    if (0 == to) {
        return -1;
    }

    if (ce->ce0_stat & BM_CE0_DMA_CTRL_DMA_CMDFIFO_FULL) {
        DBG(" ce dma cmd fifo is full.\n");
        return -2;
    }

    ce->ce0_dma_src_addr = from_off;
    /* 0 - external memories; 1 - internal secure sram*/
    ce->ce0_dma_src_addr_h = FV_CE0_DMA_SRC_ADDR_H_DMA_SRC_TYPE(DMA_MEM_PKEMEM);

    local_addr = soc_to_dma_address(to);
    ce->ce0_dma_dst_addr_h = FV_CE0_DMA_DST_ADDR_H_DMA_DST_TYPE(DMA_MEM_EXT) | (uint8_t)(0);
    ce->ce0_dma_dst_addr = (uint32_t)local_addr;

    ce->ce0_dma_len = len;

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_dma_ctrl |= (BM_CE0_DMA_CTRL_DMA_DONEINTEN
                         | BM_CE0_DMA_CTRL_DMA_GO);

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_DMA_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_STAT_DMA_BUSY));

    invalidate_cache_range((void *)(uintptr_t)to, len);

    return 0;
}

/*from extern mem to sa*/
U32 ce_pkemem_write(U32 base, U32 from, U32 to_off, U32 len)
{
    cryptoengine_t *ce = (cryptoengine_t *)(uintptr_t)base;
    uint32_t local_addr;

    if (0 == from) {
        return -1;
    }

    if (ce->ce0_stat & BM_CE0_DMA_CTRL_DMA_CMDFIFO_FULL) {
        DBG(" ce dma cmd fifo is full.\n");
        return -2;
    }

    local_addr = soc_to_dma_address(from);
    ce->ce0_dma_src_addr_h = FV_CE0_DMA_SRC_ADDR_H_DMA_SRC_TYPE(DMA_MEM_EXT) | (uint8_t)(0);
    ce->ce0_dma_src_addr = (uint32_t)local_addr;

    ce->ce0_dma_dst_addr = to_off;
    /* 0 - external memories; 1 - internal secure sram*/
    ce->ce0_dma_dst_addr_h = FV_CE0_DMA_DST_ADDR_H_DMA_DST_TYPE(DMA_MEM_PKEMEM);

    ce->ce0_dma_len = len;

    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_dma_ctrl |= (BM_CE0_DMA_CTRL_DMA_DONEINTEN
                         | BM_CE0_DMA_CTRL_DMA_GO);

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_DMA_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_STAT_DMA_BUSY));

    return 0;
}

U32 ce_dma_copy(U32 base, U32 from, U32 to, U32 len, ce_dma_mem_type_e s_attr, ce_dma_mem_type_e d_attr)
{
    cryptoengine_t *ce = (cryptoengine_t *)(uintptr_t)base;
    uint32_t addr_64;

    if ((0 == to) && (DMA_MEM_SRAM_SA != d_attr) &&
        (DMA_MEM_SRAM_PA != d_attr)) {
        return -1;
    }
    if ((0 == from) && (DMA_MEM_SRAM_SA != s_attr) &&
        (DMA_MEM_SRAM_PA != s_attr)) {
        return -1;
    }

    if (ce->ce0_stat & BM_CE0_DMA_CTRL_DMA_CMDFIFO_FULL) {
        DBG(" ce dma cmd fifo is full.\n");
        return -2;
    }

    if (DMA_MEM_EXT == s_attr) {
        addr_64 = soc_to_dma_address(from);
    } else {
        addr_64 = from;
    }
    ce->ce0_dma_src_addr_h = FV_CE0_DMA_SRC_ADDR_H_DMA_SRC_TYPE(s_attr) | (uint8_t)(0);
    ce->ce0_dma_src_addr = (uint32_t)addr_64;

    if (DMA_MEM_EXT == d_attr) {
        addr_64 = soc_to_dma_address(to);
    } else {
        addr_64 = to;
    }
    ce->ce0_dma_dst_addr_h = FV_CE0_DMA_DST_ADDR_H_DMA_DST_TYPE(d_attr) | (uint8_t)(0);
    ce->ce0_dma_dst_addr = (uint32_t)addr_64;

    ce->ce0_dma_len = len;

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_dma_ctrl |= (BM_CE0_DMA_CTRL_DMA_DONEINTEN
                         | BM_CE0_DMA_CTRL_DMA_GO);
    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_DMA_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_STAT_DMA_BUSY));

    return 0;
}

/*from extern to extern(this is used for 64bit addr move originally.)*/
U32 ce_dma_mem_copy(void *self, U32 from, U32 to, U32 len)
{
    U32 base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    return ce_dma_copy(base, from, to, len, DMA_MEM_EXT, DMA_MEM_EXT);
}
