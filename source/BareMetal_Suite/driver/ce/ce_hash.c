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
#include "ce_dma.h"

//#define HASH_DEBUG
//#define DEBUG_UPDATE_HASH

#if defined(HASH_DEBUG) && defined(DEBUG_ENABLE)
static void dump_hash_reg(void *self)
{
    cryptoengine_t *ce =
        (cryptoengine_t *)(Mcu_GetModuleBase(((crypto_eng_t *)self)->m));

    DBG("ce->ce0_hash_keyiv_addr = 0x%x\n", ce->ce0_hash_keyiv_addr);
    DBG("ce->ce0_hash_src_addr_h = 0x%x\n", ce->ce0_hash_src_addr_h);
    DBG("ce->ce0_hash_src_addr = 0x%x\n", ce->ce0_hash_src_addr);
    DBG("ce->ce0_hash_dst_addr = 0x%x\n", ce->ce0_hash_dst_addr);
    DBG("ce->ce0_hash_calc_len = 0x%x\n", ce->ce0_hash_calc_len);
    DBG("ce->ce0_hash_ctrl = 0x%x\n", ce->ce0_hash_ctrl);
    DBG("ce->ce0_hmac_key_ctrl = 0x%x\n", ce->ce0_hmac_key_ctrl);
}
#endif

static ce_hash_alg_bit_e hash_alg_map(crypto_alg_hash_e alg)
{
    return (ce_hash_alg_bit_e)alg;
}

/* olen is a local core var */
U32 ce_hash_inner(void *self, U8 alg, bool use_keyport,
                  const U8 *key, uint32_t key_sz,
                  U32 in, U32 ilen,
                  U32 out, U32 *olen)
{
    U32 cnt = 0UL;
    cryptoengine_t *ce =
        (cryptoengine_t *)(Mcu_GetModuleBase(((crypto_eng_t *)self)->m));
    uint32_t local_addr;

    if ((NULL == ce) || (0 == in) || (0 == out)) {
        DBG("%s: Opps, invalid paras.\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    if (0 == get_hash_sz(alg)) {
        DBG("%s: Opps, unsupported hash alg %d\n", __FUNCTION__, alg);
        return CRYPTO_INVALID_ALGO;
    }
    cnt++;
    if (get_hash_sz(alg) > *olen) {
        DBG("%s: buffer not big enough\n", __FUNCTION__);
        return CRYPTO_BUFFER_SMALL;
    }
    cnt++;
    U32 hash_mode = (0x1u << hash_alg_map(alg));

    arch_clean_cache_range((void *)in, ilen);
    local_addr = soc_to_dma_address(in);
    ce->ce0_hash_src_addr_h = FV_CE0_HASH_SRC_ADDR_H_HASH_SRC_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_hash_src_addr = (uint32_t)local_addr;

    ce->ce0_hash_calc_len = ilen;

    ce->ce0_hash_dst_addr = FV_CE0_HASH_DST_ADDR_HASH_DST_TYPE(DMA_MEM_SRAM_PA)
                            | FV_CE0_HASH_DST_ADDR_HASH_DST_ADDR(SLOT_OFF(0));
    cnt++;
    if ((NULL != key) && !use_keyport) {
        /* key in sram_priv */
        uint32_t key_addr = SA_SLOT_ADDR(SA_SLOT_HASH_INNER_KEY);
        mini_memcpy_s((void *)key_addr, key, key_sz);
        clean_invalidate_cache_range((const void *)key_addr, key_sz);
        ce->ce0_hash_keyiv_addr = FV_CE0_HASH_KEYIV_ADDR_HASH_KEY_ADDR(SA_SLOT_OFF(SA_SLOT_HASH_INNER_KEY))
                                  | FV_CE0_HASH_KEYIV_ADDR_HASH_IV_ADDR(SA_SLOT_OFF(SA_SLOT_HASH_INNER_IV));
#if defined(HASH_DEBUG)
        DBG("Dump key(%d bytes) as:\n");
        DBG_ARRAY_DUMP(key, key_sz);
#endif
    } else if (use_keyport) {
        /* key address shall be 8 bytes aligned for key port */
        ce->ce0_hash_keyiv_addr = FV_CE0_HASH_KEYIV_ADDR_HASH_KEY_ADDR((uintptr_t)key)
                                  | FV_CE0_HASH_KEYIV_ADDR_HASH_IV_ADDR(SA_SLOT_OFF(SA_SLOT_HASH_INNER_IV));
    }
    cnt++;
    U32 hash_ctrl = FV_CE0_HASH_CTRL_HASH_MODE(hash_mode)
                    | BM_CE0_HASH_CTRL_HASH_PADDINGEN;

    if ((NULL != key) || use_keyport) {
        hash_ctrl |= BM_CE0_HASH_CTRL_HASH_HMAC_EN | BM_CE0_HASH_CTRL_HASH_KEY;
        ce->ce0_hmac_key_ctrl &= ~(FM_CE0_HMAC_KEY_CTRL_HMAC_KEY_TYPE
                                   | FM_CE0_HMAC_KEY_CTRL_HMAC_KEY_LEN);

        if (use_keyport) {
            ce->ce0_hmac_key_ctrl |= FV_CE0_HMAC_KEY_CTRL_HMAC_KEY_TYPE(CE_KEYTYPE_KEYPORT)
                                     | FV_CE0_HMAC_KEY_CTRL_HMAC_KEY_LEN(key_sz);
        } else {
            ce->ce0_hmac_key_ctrl |= FV_CE0_HMAC_KEY_CTRL_HMAC_KEY_TYPE(CE_KEYTYPE_SMEM_PRIV)
                                     | FV_CE0_HMAC_KEY_CTRL_HMAC_KEY_LEN(key_sz);
        }
    }
    cnt++;
    ce->ce0_hash_ctrl = hash_ctrl;

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_hash_ctrl |= BM_CE0_HASH_CTRL_HASH_GO;

#if defined(HASH_DEBUG) && defined(DEBUG_ENABLE)
    dump_hash_reg(self);
#endif

    while (!(ce->ce0_intstat & BM_CE0_INTSTAT_HASH_DONE_INTSTAT));
    cnt++;
    *olen = get_hash_sz(alg);

    if (cnt != 7) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    uint32_t dst_add = PA_SLOT_ADDR(0);
    ce_dma_copy((U32)ce, (U32)dst_add, out, *olen, DMA_MEM_SRAM_PA, DMA_MEM_EXT);
    invalidate_cache_range((void *)out, *olen);

#if defined(HASH_DEBUG)
    DBG("Dump hash (%d bytes) as:\n", *olen);
    DBG_ARRAY_DUMP((uint8_t *)(uintptr_t)dst_add, *olen);
#endif

    return 0;
}

U32 ce_hash(void *self, U8 alg, U32 in, U32 ilen, U32 out, U32 *olen)
{
    return ce_hash_inner(self, alg, false, NULL, 0, in, ilen, out, olen);
}

int ce_hmac(void *self, crypto_alg_hash_e type,
            const uint8_t *key, uint32_t key_sz,
            uint32_t msg, size_t msg_sz,
            uint32_t mac, size_t *mac_sz)
{
    DBG("%s: key=%p, key_sz=%d, msg_sz=%d, mac_sz=%d\n", \
        __FUNCTION__, key, key_sz, (uint32_t)msg_sz, (uint32_t)*mac_sz);
    return ce_hash_inner(self, type, false, key, key_sz, msg, msg_sz, mac, mac_sz);
}

U32 ce_hash_write_back(void *self, ce_hash_context *context)
{
    if (NULL == context) {
        return CRYPTO_INVALID_PARAs;
    }
    U32 hash_sz = get_hash_sz(context->algo);
    if (0 == hash_sz) {
        DBG("%s: Opps, unsupported hash alg %d\n", __FUNCTION__, context->algo);
        return CRYPTO_INVALID_ALGO;
    }

    uint32_t dst_addr = FIXED_PA_HASH_DST;
#ifdef DEBUG_UPDATE_HASH
    U8 temp[64];
    mini_memcpy_s(temp, (void *)dst_addr, hash_sz);
    DBG("Hash Write Back:%d,ID:%x\n", hash_sz, context->context_id);
    DBG_ARRAY_DUMP(temp, hash_sz);
#endif
    invalidate_cache_range((void *)dst_addr, hash_sz);
    mini_memcpy_s((void *)SA_SLOT_ADDR(context->context_pos), (void *)dst_addr, hash_sz);
    clean_invalidate_cache_range((void *)SA_SLOT_ADDR(context->context_pos), hash_sz);

    return 0;
}

U32 ce_hash_update(void *self, ce_hash_context *context, U32 msg, U32 msg_sz)
{
    U32 cnt = 0UL;
    cryptoengine_t *ce =
        (cryptoengine_t *)(Mcu_GetModuleBase(((crypto_eng_t *)self)->m));
    U32 local_addr;

    if (NULL == context || 0 == msg) {
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    U32 hash_sz = get_hash_sz(context->algo);
    U32 block_sz = get_hash_block_sz(context->algo);

    if (0 == hash_sz) {
        DBG("%s: Opps, unsupported hash alg %d\n", __FUNCTION__, context->algo);
        return CRYPTO_INVALID_ALGO;
    }
    cnt++;
    U32 hash_mode = (0x1u << hash_alg_map(context->algo));

    arch_clean_cache_range((void*)msg, msg_sz);
    local_addr = soc_to_dma_address(msg);
    ce->ce0_hash_src_addr_h = FV_CE0_HASH_SRC_ADDR_H_HASH_SRC_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_hash_src_addr = (uint32_t)local_addr;

    ce->ce0_hash_calc_len = msg_sz;
    cnt++;
    ce->ce0_hash_dst_addr = FV_CE0_HASH_DST_ADDR_HASH_DST_TYPE(DMA_MEM_SRAM_PA)
                            | FV_CE0_HASH_DST_ADDR_HASH_DST_ADDR(FIXED_PA_HASH_DST_OFF);

    U32 hash_ctrl = FV_CE0_HASH_CTRL_HASH_MODE(hash_mode);
    hash_ctrl |= BM_CE0_HASH_CTRL_HASH_SHAUPDATE;
    if (block_sz <= context->total_len) {
        hash_ctrl |= BM_CE0_HASH_CTRL_HASH_INIT | BM_CE0_HASH_CTRL_HASH_INIT_SECURE; /* load the iv */
    }
    cnt++;
    ce->ce0_hash_keyiv_addr = FV_CE0_HASH_KEYIV_ADDR_HASH_KEY_ADDR(FIXED_SA_HASH_KEY_OFF)
                                  | FV_CE0_HASH_KEYIV_ADDR_HASH_IV_ADDR(SA_SLOT_OFF(context->context_pos));

    ce->ce0_hash_ctrl = hash_ctrl;

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_hash_ctrl |= BM_CE0_HASH_CTRL_HASH_GO;

    while (!(ce->ce0_intstat & BM_CE0_INTSTAT_HASH_DONE_INTSTAT));
    cnt++;
    U8 *dst_addr = (U8 *)FIXED_PA_HASH_DST;
#ifdef DEBUG_UPDATE_HASH
    if (context->context_id == 2) {
        DBG("Msg start %x\n",(uint32_t)msg);
        DBG("Msg size %d\n", msg_sz);
    }
    U8 temp[64];
    mini_memcpy_s(temp, (void *)dst_addr, hash_sz);
    DBG("Hash Update:%d,ID:%x\n", hash_sz, context->context_id);
    DBG_ARRAY_DUMP(temp, hash_sz);
#endif

    if (cnt != 5) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    invalidate_cache_range((void *)dst_addr, hash_sz);
    mini_memcpy_s((void *)SA_SLOT_ADDR(context->context_pos), (U8 *)dst_addr, hash_sz);
    clean_invalidate_cache_range((void *)SA_SLOT_ADDR(context->context_pos), hash_sz);

    return 0;
}

U32 ce_hash_finish(void *self, ce_hash_context *context, U32 hash, U32 *hash_sz_addr)
{
    U32 cnt = 0UL;
    cryptoengine_t *ce =
        (cryptoengine_t *)(Mcu_GetModuleBase(((crypto_eng_t *)self)->m));
    U32 local_addr;

    if (NULL == context || 0 == hash) {
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;

    U32 hash_sz = get_hash_sz(context->algo);
    U32 block_sz = get_hash_block_sz(context->algo);
    if (0 == hash_sz) {
        DBG("%s: Opps, unsupported hash alg %d\n", __FUNCTION__, context->algo);
        return CRYPTO_INVALID_ALGO;
    }
    cnt++;

    U32 hash_mode = (0x1u << hash_alg_map(context->algo));
    clean_invalidate_cache_range((void *)(uintptr_t)context->residual_data, sizeof(context->residual_data));
    local_addr = soc_to_dma_address((U32)(uintptr_t)context->residual_data);
    ce->ce0_hash_src_addr_h = FV_CE0_HASH_SRC_ADDR_H_HASH_SRC_TYPE(EXT_MEMORY);
    ce->ce0_hash_src_addr = (uint32_t)local_addr;
    cnt++;

    ce->ce0_hash_calc_len = block_sz;

    ce->ce0_hash_dst_addr = FV_CE0_HASH_DST_ADDR_HASH_DST_TYPE(DMA_MEM_SRAM_PA)
                            | FV_CE0_HASH_DST_ADDR_HASH_DST_ADDR(FIXED_PA_HASH_DST_OFF);

    U32 hash_ctrl = FV_CE0_HASH_CTRL_HASH_MODE(hash_mode);
    cnt++;

    if (block_sz <= context->total_len) {
        hash_ctrl |= BM_CE0_HASH_CTRL_HASH_INIT | BM_CE0_HASH_CTRL_HASH_INIT_SECURE; /* load the iv */
    }
    ce->ce0_hash_keyiv_addr = FV_CE0_HASH_KEYIV_ADDR_HASH_KEY_ADDR(FIXED_SA_HASH_KEY_OFF)
                                  | FV_CE0_HASH_KEYIV_ADDR_HASH_IV_ADDR(SA_SLOT_OFF(context->context_pos));

    ce->ce0_hash_ctrl = hash_ctrl;
    cnt++;
    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_hash_ctrl |= BM_CE0_HASH_CTRL_HASH_GO;

    while (!(ce->ce0_intstat & BM_CE0_INTSTAT_HASH_DONE_INTSTAT));
    cnt++;

    U32 dst_addr = FIXED_PA_HASH_DST;
#ifdef DEBUG_UPDATE_HASH
    U8 temp[64];
    invalidate_cache_range((const void *)dst_addr, hash_sz);
    mini_memcpy_s(temp, (void *)dst_addr, hash_sz);
    DBG("Hash Finish:%d,ID:%x\n", hash_sz, context->context_id);
    DBG_ARRAY_DUMP(temp, hash_sz);
#endif
    if (cnt != 6) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    ce_dma_copy((U32)ce, dst_addr, hash, hash_sz, DMA_MEM_SRAM_PA, DMA_MEM_EXT);
    *hash_sz_addr = hash_sz;
    invalidate_cache_range((const void *)hash, *hash_sz_addr);

    return 0;
}
