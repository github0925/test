/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <string.h>

#include <ce_reg.h>
#include <sx_dma.h>
#include <sx_errors.h>
#include <sx_pke_conf.h>
#include <trace.h>
#include <sram_conf.h>

#define LOCAL_TRACE 0 //close local trace 1->0

void cryptodma_config(uint32_t vce_id, block_t dst, block_t src, uint32_t length)
{
    uint32_t value;
    uint32_t transfer_len = dst.len < length ? dst.len : length;

    if (EXT_MEM == src.addr_type) {
        writel(addr_switch_to_ce(vce_id, src.addr_type, (_paddr((void*)src.addr))), _ioaddr((REG_DMA_SRC_ADDR_CE_(vce_id))));
    }
    else {
        writel((addr_t)src.addr, _ioaddr((REG_DMA_SRC_ADDR_CE_(vce_id))));
    }

    //TODO: compatible 64bit address
    value = reg_value(0, 0, CE_DMA_SRC_ADDR_H_SHIFT, CE_DMA_SRC_ADDR_H_MASK);
    value = reg_value(switch_addr_type(src.addr_type), value, CE_DMA_SRC_TYPE_SHIFT, CE_DMA_SRC_TYPE_MASK);
    writel(value, _ioaddr((REG_DMA_SRC_ADDR_H_CE_(vce_id))));

    LTRACEF("value = %x reg= 0x%x\n", value, REG_DMA_SRC_ADDR_H_CE_(vce_id));

    if (EXT_MEM == dst.addr_type) {
        writel(addr_switch_to_ce(vce_id, dst.addr_type, (_paddr((void*)dst.addr))),  _ioaddr(REG_DMA_DST_ADDR_CE_(vce_id)));
    }
    else {
        writel((addr_t)dst.addr,  _ioaddr(REG_DMA_DST_ADDR_CE_(vce_id)));
    }

    //TODO: compatible 64bit address
    value = reg_value(0, 0, CE_DMA_DST_ADDR_H_SHIFT, CE_DMA_DST_ADDR_H_MASK);
    value = reg_value(switch_addr_type(dst.addr_type), value, CE_DMA_DST_TYPE_SHIFT, CE_DMA_DST_TYPE_MASK);
    writel(value,  _ioaddr(REG_DMA_DST_ADDR_H_CE_(vce_id)));

    LTRACEF("cryptodma_config transfer_len: %d\n", transfer_len);
    writel(transfer_len, _ioaddr(REG_DMA_LEN_CE_(vce_id)));
}

void cryptodma_start(uint32_t vce_id)
{
    uint32_t value, read_value;

    read_value = readl(_ioaddr(REG_DMA_CTRL_CE_(vce_id)));

#if WAIT_PK_WITH_REGISTER_POLLING // polling
    value = 0x0;
#else  //wait interrupt
    value = 0x1;
#endif

    value = reg_value(value, read_value, CE_DMA_DONEINTEN_SHIFT, CE_DMA_DONEINTEN_MASK);
    value = reg_value(0x1, value, CE_DMA_GO_SHIFT, CE_DMA_GO_MASK);
    writel(value, _ioaddr(REG_DMA_CTRL_CE_(vce_id)));
}

void dma_wait_irq_fct(uint32_t vce_id)
{
    event_wait(&g_ce_signal[vce_id]);
    LTRACEF("wait end in dma\n");
}

void cryptodma_wait(uint32_t vce_id)
{
    // Wait until DMA is done
#if WAIT_PK_WITH_REGISTER_POLLING // polling
    int i = 0;

    while (readl(_ioaddr(REG_STAT_CE_(vce_id))) & 0x1) {
        if (5 == i % 30) {
            LTRACEF("DMA is busy.\n");
        }

        i++;
    }

#else  //wait interrupt
    dma_wait_irq_fct(vce_id);
    LTRACEF("wait interrupt in dma end\n");
#endif
}

uint32_t cryptodma_check_bus_error(uint32_t vce_id)
{
    uint32_t err = readl(_ioaddr(REG_ERRSTAT_CE_(vce_id))) & 0x70000;

    if (err) {
        LTRACEF("CRYPTODMA bus error: 0x%x\n", err);
        cryptodma_err_clear(vce_id);
        return err;
    }
    else {
        return CRYPTOLIB_SUCCESS;
    }
}

/**
 * @brief Check cryptodma fifo status
 * @param vce_id      vce index
 * @return CRYPTOLIB_DMA_ERR if bus error occured, CRYPTOLIB_SUCCESS otherwise
 */
static uint32_t cryptodma_check_fifo_empty(uint32_t vce_id)
{
    uint32_t dma_status = readl(_ioaddr(REG_DMA_CTRL_CE_(vce_id))) & 0x200;  //0x300

    if (dma_status) {
        LTRACEF("CRYPTODMA fifo error 0x%x\n", dma_status);
        return dma_status;
    }
    else {
        return CRYPTOLIB_SUCCESS;
    }
}

uint32_t cryptodma_check_status(uint32_t vce_id)
{
    return cryptodma_check_bus_error(vce_id) | cryptodma_check_fifo_empty(vce_id);
}

void cryptodma_err_clear(uint32_t vce_id)
{
    uint32_t value_state = reg_value(1, 0x0, CE_DMA_INTEGRITY_ERROR_INTCLR_SHIFT, CE_DMA_INTEGRITY_ERROR_INTCLR_MASK);
    writel(value_state, _ioaddr(REG_INTCLR_CE_(vce_id)));

    value_state = reg_value(0, 0x0, CE_DMA_INTEGRITY_ERROR_INTCLR_SHIFT, CE_DMA_INTEGRITY_ERROR_INTCLR_MASK);
    writel(value_state, _ioaddr(REG_INTCLR_CE_(vce_id)));
}

uint32_t memcpy_blk_common(uint32_t vce_id, block_t dst, block_t src, uint32_t length, bool cache_op)
{
    uint32_t status;

    if (!length) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    if (dst.len < length) {
        length = dst.len;
    }

    if (cache_op) {
        clean_cache_block(&src, vce_id);
        invalidate_cache_block(&dst, vce_id);
    }

    cryptodma_config(vce_id, dst, src, length);
    cryptodma_start(vce_id);
    cryptodma_wait(vce_id);

    status = cryptodma_check_status(vce_id);

    return status;
}

uint32_t memcpy_blk_cache(uint32_t vce_id, block_t dst, block_t src, uint32_t length, bool cache_op_src, bool cache_op_dst)
{
    uint32_t status;

    if (!length) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    if (dst.len < length) {
        length = dst.len;
    }

    if (cache_op_src) {
        clean_cache_block(&src, vce_id);
    }

    if (cache_op_dst) {
        invalidate_cache_block(&dst, vce_id);
    }

    cryptodma_config(vce_id, dst, src, length);
    cryptodma_start(vce_id);
    cryptodma_wait(vce_id);

    status = cryptodma_check_status(vce_id);

    return status;
}

uint32_t memcpy_blk(uint32_t vce_id, block_t dst, block_t src, uint32_t length)
{
    return memcpy_blk_common(vce_id, dst, src, length, true);
}

uint32_t point2CryptoRAM_rev(uint32_t vce_id, block_t src, uint32_t size, uint32_t offset)
{
    if (src.len < size * 2) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    uint32_t status = mem2CryptoRAM_rev(vce_id, src, size, offset, true);

    if (status) {
        return status;
    }

    return mem2CryptoRAM_rev(vce_id, block_t_convert(src.addr + size, src.len - size, src.addr_type), size, offset + 1, true);
}

uint32_t point2CryptoRAM(uint32_t vce_id, block_t src, uint32_t size, uint32_t offset)
{
    if (src.len < size * 2) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    uint32_t status = mem2CryptoRAM(vce_id, src, size, offset, true);

    if (status) {
        return status;
    }

    return mem2CryptoRAM(vce_id, block_t_convert(src.addr + size, src.len - size, src.addr_type), size, offset + 1, true);
}

uint32_t CryptoRAM2point_rev(uint32_t vce_id, block_t dst, uint32_t size, uint32_t offset)
{
    if (dst.len < size * 2) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    uint32_t status  = CryptoRAM2mem_rev(vce_id, dst, size, offset, true);

    if (status) {
        return status;
    }

    return CryptoRAM2mem_rev(vce_id, block_t_convert(dst.addr + size, dst.len - size, dst.addr_type), size, offset + 1, true);
}

uint32_t CryptoRAM2point(uint32_t vce_id, block_t dst, uint32_t size, uint32_t offset)
{
    if (dst.len < size * 2) {
        return CRYPTOLIB_INVALID_PARAM;
    }

    uint32_t status  = CryptoRAM2mem(vce_id, dst, size, offset, true);

    if (status) {
        return status;
    }

    return CryptoRAM2mem(vce_id, block_t_convert(dst.addr + size, dst.len - size, dst.addr_type), size, offset + 1, true);
}

uint32_t mem2CryptoRAM_rev(uint32_t vce_id, block_t src, uint32_t size, uint32_t offset, bool cache_op)
{
    uint32_t status;
    uint32_t i = 0;
    block_t dst;
    uint8_t* temp_buf;
    struct mem_node* mem_n;

    if (!src.len || !size || (size > RSA_MAX_SIZE)) {
        LTRACEF("mem2CryptoRAM_rev: Src:%d or size:%d is null (skip) !\n", src.len, size);
        return CRYPTOLIB_INVALID_PARAM;
    }

    if (src.len > size) {
        dprintf(INFO, "mem2CryptoRAM_rev: Src longer (=%d) than size (=%d) (cutting) !\n",
                src.len, size);
        src.len = size;
    }

    //buffer for bytes order reverse

    mem_n = ce_malloc(RSA_MAX_SIZE);

    if (mem_n != NULL) {
        temp_buf = mem_n->ptr;
    }
    else {
        return CRYPTOLIB_PK_N_NOTVALID;
    }

    //memset(temp_buf, 0, RSA_MAX_SIZE);

    //reverse byte order
    for (i = 0; i < size; i++) {
        temp_buf[i] = src.addr[size - i - 1];
    }

    dst.addr_type = PKE_INTERNAL;
    dst.len = size;

    dst.addr = (uint8_t*)(addr_t)BA414EP_ADDR_MEMLOC(offset - 1, 0);

    //clean_cache((addr_t)temp_buf, RSA_MAX_SIZE);

    status = memcpy_blk_common(vce_id, dst, block_t_convert(temp_buf, size, EXT_MEM), src.len, cache_op);

    ce_free(mem_n);

    if (status) {
        return status;
    }

    return status;
}

uint32_t CryptoRAM2mem_rev(uint32_t vce_id, block_t dst, uint32_t size, uint32_t offset, bool cache_op)
{
    block_t src;
    uint32_t status;
    uint8_t* temp_buf;
    struct mem_node* mem_n;
    //buffer for bytes order reverse

    src.addr = (uint8_t*)(addr_t)BA414EP_ADDR_MEMLOC(offset - 1, 0);

    src.len = size;
    src.addr_type = PKE_INTERNAL;

    mem_n = ce_malloc(RSA_MAX_SIZE);

    if (mem_n != NULL) {
        temp_buf = mem_n->ptr;
    }
    else {
        return CRYPTOLIB_PK_N_NOTVALID;
    }

    status = memcpy_blk_common(vce_id, block_t_convert(temp_buf, dst.len, EXT_MEM), src, size, cache_op);

    if (status) {
        LTRACEF("CryptoRAM2mem_rev: status=%x", status);
        ce_free(mem_n);
        return status;
    }

    //reverse byte order
    for (uint32_t i = 0; i < size; i++) {
        //LTRACEF("CryptoRAM2mem_rev: temp_buf %d = %x!\n",(size - i - 1),temp_buf[size - i - 1]);
        dst.addr[i] = temp_buf[size - i - 1];
    }

    ce_free(mem_n);
    return status;
}

uint32_t mem2CryptoRAM(uint32_t vce_id, block_t src, uint32_t size, uint32_t offset, bool cache_op)
{
    block_t dst;

    dst.addr = (uint8_t*)(addr_t)BA414EP_ADDR_MEMLOC(offset - 1, 0);

    dst.len = size;
    dst.addr_type = PKE_INTERNAL;
    return memcpy_blk_common(vce_id, dst, src, size, cache_op);
}

uint32_t CryptoRAM2mem(uint32_t vce_id, block_t dst, uint32_t size, uint32_t offset, bool cache_op)
{
    block_t src;

    src.addr = (uint8_t*)(addr_t)BA414EP_ADDR_MEMLOC(offset - 1, 0);
    src.len = size;
    src.addr_type = PKE_INTERNAL;
    return memcpy_blk_common(vce_id, dst, src, size, cache_op);
}

int memcmp_rev(uint8_t* src, const uint8_t* dst, uint32_t len)
{
    /*
    uint8_t temp_buf[RSA_MAX_SIZE];

    for (uint32_t i = 0; i < len; i++) {
        temp_buf[i] = src[len - i - 1];
    }

    return memcmp(temp_buf, dst, len);
    */
    const unsigned char* su1, *su2;
    int res = 0;

    for (su1 = src + len - 1, su2 = dst; 0 < len; --su1, ++su2, len--)
        if ((res = *su1 - *su2) != 0) {
            //LTRACEF("memcmp_rev: error at:%d byte, src=%d ,dst=%d ,src at %p!\n", len, *su1, *su2, src);
            break;
        }

    return res;
}
