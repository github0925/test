/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include <atb_crypto.h>
#include "cryptoengine_reg.h"
#include "cryptoengine_reg_field_def.h"
#include "ce_wrapper.h"
#include "ce_dma.h"

//#define CE_CIPHER_DEBUG

static inline ce_aes_mode_e get_ce_aes_mode(cipher_mode_e mode)
{
    return (ce_aes_mode_e)mode;
}

static inline ce_key_sz_e get_ce_key_sz(cipher_type_e type)
{
    ce_key_sz_e v = CE_KEY_SZ_128;

    if (AES_192 == type) {
        v = CE_KEY_SZ_192;
    } else if (AES_256 == type) {
        v = CE_KEY_SZ_256;
    }

    return v;
}

#if defined(CE_CIPHER_DEBUG) && defined(DEBUG_ENABLE)
static void dump_ce_cipher_regs(void *self)
{
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    DBG("ce->ce0_cipher_dst_addr_h = 0x%x\n", ce->ce0_cipher_dst_addr_h);
    DBG("ce->ce0_cipher_dst_addr = 0x%x\n", ce->ce0_cipher_dst_addr);
    DBG("ce->ce0_cipher_src_addr_h = 0x%x\n", ce->ce0_cipher_src_addr_h);
    DBG("ce->ce0_cipher_src_addr = 0x%x\n", ce->ce0_cipher_src_addr);
    DBG("ce->ce0_cipher_key_addr = 0x%x\n", ce->ce0_cipher_key_addr);
    DBG("ce->ce0_cipher_iv_context_addr = 0x%x\n", ce->ce0_cipher_iv_context_addr);
    DBG("ce->ce0_cipher_ctrl = 0x%x\n", ce->ce0_cipher_ctrl);
    DBG("ce->ce0_cipher_payload_len = 0x%x\n", ce->ce0_cipher_payload_len);
    DBG("ce->ce0_cipher_header_len = 0x%x\n", ce->ce0_cipher_header_len);
}
#endif

int32_t ce_cipher_inner(void *self, cipher_op_e op,
                        cipher_type_e type, cipher_mode_e mode,
                        bool use_key_port, const uint8_t *key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz)
{
    uint32_t local_addr;
    uint32_t cnt = 0UL;

    if (((NULL == key) && (!use_key_port))
        || ((0 == iv) && (MODE_ECB != mode) && (MODE_CMAC != mode)
            && (MODE_CCM != mode) && (MODE_GCM != mode))
        || (0 == i_buf) ||  (0 == o_buf)
        || (NULL == self) || (NULL == o_sz)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    if (((MODE_XTS == mode) && (key_sz % 2))) {/** @TODO:i_sz % 16, check?*/
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    if ((!use_key_port) && (NULL != key)) {
        if (MODE_XTS == mode) {
            mini_memcpy_s((void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY0), key, key_sz / 2);
            mini_memcpy_s((void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY2), key + key_sz / 2, key_sz / 2);
            clean_invalidate_cache_range((const void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY0), key_sz / 2);
            clean_invalidate_cache_range((const void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY2), key_sz / 2);
        } else {
            mini_memcpy_s((void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY0), key, key_sz);
            clean_invalidate_cache_range((const void *)SA_SLOT_ADDR(SA_SLOT_AES_INNER_KEY0), key_sz);
        }
    }
    cnt++;
#if defined(CE_CIPHER_DEBUG)
    DBG("%s: %s\n", __FUNCTION__, CIPHER_ENC == op ? "Encryption" : "Decryption");
    DBG("%s: Dump key(%d bytes) as\n", __FUNCTION__, key_sz);
    DBG_ARRAY_DUMP(key, key_sz);
#endif

    if ((0 != iv) && (MODE_ECB != mode) && (MODE_CMAC != mode)) {
        uintptr_t iv_addr = SA_SLOT_OFF(SA_SLOT_AES_INNER_IV);    /* Use addr offset in CE DMA.*/
        ce_dma_copy(base, (U32)iv, (U32)iv_addr, iv_sz, DMA_MEM_EXT, DMA_MEM_SRAM_SA);
    }
    cnt++;
#if defined(CE_CIPHER_DEBUG)
    invalidate_cache_range((void *)iv, iv_sz);
    DBG("%s: Dump iv(%d bytes) as\n", __FUNCTION__, iv_sz);
    DBG_ARRAY_DUMP((uint8_t *)(uintptr_t)iv, iv_sz);

    invalidate_cache_range((void *)i_buf, i_sz);
    DBG("%s: Dump msg(%d bytes) as\n", __FUNCTION__, i_sz);
    DBG_ARRAY_DUMP((uint8_t *)(uintptr_t)i_buf, i_sz);
#endif
    local_addr = soc_to_dma_address(o_buf);
    ce->ce0_cipher_dst_addr_h = FV_CE0_CIPHER_DST_ADDR_H_CIPHER_DST_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_cipher_dst_addr = (uint32_t)local_addr;

    local_addr = soc_to_dma_address(i_buf);
    ce->ce0_cipher_src_addr_h = FV_CE0_CIPHER_SRC_ADDR_H_CIPHER_SRC_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_cipher_src_addr = (uint32_t)local_addr;
    cnt++;
    if (use_key_port) {
        /* key address shall be 16 bytes aligned for keyport */
        ce->ce0_cipher_key_addr = FV_CE0_CIPHER_KEY_ADDR_CIPHER_KEY_ADDR((uintptr_t)key);
    } else {
        ce->ce0_cipher_key_addr = FV_CE0_CIPHER_KEY_ADDR_CIPHER_KEY_ADDR(SA_SLOT_OFF(SA_SLOT_AES_INNER_KEY0));

        if (MODE_XTS == mode) {
            ce->ce0_cipher_key_addr |= FV_CE0_CIPHER_KEY_ADDR_CIPHER_KEY2_ADDR(SA_SLOT_OFF(SA_SLOT_AES_INNER_KEY2));
        }
    }
    cnt++;
    uint32_t hdr_sz = 0;
    uint8_t hdr_buf[24];
    if (MODE_CCM == mode) {
        mini_memcpy_s(hdr_buf, (void *)i_buf, 24);

        if ( 0 != crypto_get_aead_hdr_sz(MODE_CCM, (U8 *)(uintptr_t)hdr_buf, i_sz, &hdr_sz)) {
            return -4;
        }
    } else if (MODE_GCM == mode) {
        mini_memcpy_s(hdr_buf, (void *)(i_buf + i_sz - 16), 16);

        if ( 0 != crypto_get_aead_hdr_sz(MODE_GCM, (U8 *)(uintptr_t)hdr_buf, i_sz, &hdr_sz)) {
            return -4;
        }
    }
    cnt++;
    ce->ce0_cipher_header_len = hdr_sz;
    /* iv/ctx always in secure area */
    ce->ce0_cipher_iv_context_addr = FV_CE0_CIPHER_IV_CONTEXT_ADDR_CIPHER_IV_ADDR(SA_SLOT_OFF(SA_SLOT_AES_INNER_IV))
                                     | FV_CE0_CIPHER_IV_CONTEXT_ADDR_CIPHER_CONTEXT_ADDR(SA_SLOT_OFF(SA_SLOT_AES_INNER_CONTEXT));

    if (mode == MODE_GCM) {
        /* no need to pass I(A)/I(C) to ce */
        i_sz -= 16;
    }
    cnt++;
    ce->ce0_cipher_payload_len = i_sz - hdr_sz;

    uint32_t ctl = FV_CE0_CIPHER_CTRL_AESMODE(0x01u << get_ce_aes_mode(mode))
                   | FV_CE0_CIPHER_CTRL_CIPHER_KEYSIZE(get_ce_key_sz(type));

    if (use_key_port) {
        ctl |= FV_CE0_CIPHER_CTRL_CIPHER_KEYTYPE(CE_KEYTYPE_KEYPORT);
    } else {
        ctl |= FV_CE0_CIPHER_CTRL_CIPHER_KEYTYPE(CE_KEYTYPE_SMEM_PRIV)
               | FV_CE0_CIPHER_CTRL_CIPHER_KEY2TYPE(CE_KEYTYPE_SMEM_PRIV);
    }
    cnt++;
    if (CIPHER_DEC == op) {
        ctl |= BM_CE0_CIPHER_CTRL_CIPHERMODE | BM_CE0_CIPHER_CTRL_CIPHER_DECKEYCAL;
    }

    ce->ce0_cipher_ctrl = ctl;

#if defined(CE_CIPHER_DEBUG) && defined(DEBUG_ENABLE)
    dump_ce_cipher_regs(self);
#endif

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;
    cnt++;
    ce->ce0_cipher_ctrl |= BM_CE0_CIPHER_CTRL_CIPHER_GO;

    while (!(ce->ce0_intstat & BM_CE0_INTSTAT_CIPHER_DONE_INTSTAT));

    U32 v = ce->ce0_errstat;

    if (cnt != 10UL) {
        FLOW_ERROR_AND_RESET
        return -1;
    }

    if (0 != FV_CE0_ERRSTAT_CIPHER_ERR_STAT(v)) {   /* Error flags */
        DBG("cipher_err_stat = 0x%x\n", FV_CE0_ERRSTAT_CIPHER_ERR_STAT(v));
        return -4;
    }

#if defined(CFG_CE_CIPHER_PADDING)
#error "CIPHER_PADDING TODO"
#else

    if (((mode == MODE_CCM) && (CIPHER_ENC == op))
        || mode == MODE_GCM) {
        *o_sz = ROUNDUP(i_sz, 16) + 16;
    } else if (mode == MODE_CMAC){
        *o_sz = 16;
    } else {
        *o_sz = ROUNDUP(i_sz, 16);
    }

#endif
    clean_invalidate_cache_range((void *)o_sz, 4);
#if defined(CE_CIPHER_DEBUG)
    invalidate_cache_range((void *)o_buf, *o_sz);
    DBG("%s: dump output(%d bytes) as \n", __FUNCTION__, *o_sz);
    DBG_ARRAY_DUMP((uint8_t *)(uintptr_t)o_buf, *o_sz);
#endif

    return 0;
}

int32_t ce_cipher_enc(void *self,
                      cipher_type_e type, cipher_mode_e mode,
                      const uint8_t *key, uint32_t key_sz,
                      uintptr_t iv, uint32_t iv_sz,
                      uintptr_t i_buf, int i_sz,
                      uintptr_t o_buf, int *o_sz)
{
    return ce_cipher_inner(self, CIPHER_ENC, type, mode, false, key,
                           key_sz, iv, iv_sz, i_buf, i_sz, o_buf, o_sz);
}

int32_t ce_cipher_dec(void *self,
                      cipher_type_e type, cipher_mode_e mode,
                      const uint8_t *key, uint32_t key_sz,
                      uint32_t iv, uint32_t iv_sz,
                      uint32_t i_buf, int i_sz,
                      uint32_t o_buf, int *o_sz)
{
    return ce_cipher_inner(self, CIPHER_DEC, type, mode, false, key, key_sz,
                           iv, iv_sz, i_buf, i_sz, o_buf, o_sz);
}

int ce_cmac(void *self, cipher_type_e type, const uint8_t *key,
            uint32_t msg, size_t msg_sz,
            uint32_t mac, size_t *mac_sz)
{
    uint32_t key_sz = (type == AES_128 ? 16 :
                       type == AES_192 ? 24 :
                       type == AES_256 ? 32 :
                       0);

    return  ce_cipher_enc(self, type, MODE_CMAC,
                          key, key_sz,
                          0, 0,
                          msg, msg_sz,
                          mac, (int *)mac_sz);
}

int32_t ce_aead_enc(void *self, cipher_type_e type, cipher_mode_e mode,
                    const uint8_t *key, uint32_t key_sz,
                    uint32_t iv, uint32_t iv_sz,
                    uint32_t aad_sz, uint32_t plain_sz,
                    uint32_t i_buf, int i_sz,
                    uint32_t o_buf, int *o_sz)
{
    /* check input validity */

    int32_t res =  ce_cipher_inner(self, CIPHER_ENC, type, mode, false,
                                   key, key_sz, iv, iv_sz, i_buf, i_sz,
                                   o_buf, o_sz);
    /* output is consists of aad || ciphertext || tag */
    if (0 != res) {
        return -2;
    }

    return 0;
}

int32_t ce_aead_dec(void *self, cipher_type_e type, cipher_mode_e mode,
                    const uint8_t *key, uint32_t key_sz,
                    uint32_t iv, uint32_t iv_sz,
                    uint32_t aad_sz, uint32_t plain_sz,
                    uint32_t i_buf, int i_sz,
                    uint32_t o_buf, int *o_sz
                   )
{
    int32_t ret = -1;
    int true_input_sz;
    int32_t tag_size;

    uint8_t temp_buf_out[16];

    tag_size = i_sz - aad_sz - plain_sz - 16;
    /* The input buf 'i_buf' consists of Aad || CipherText || len(Aad)64bit || len(CipherText)64bit || Tag*/
    true_input_sz = i_sz - tag_size;

    do {
        if (aad_sz % 16 != 0 || plain_sz % 16 != 0) {
            break;
        }

        if (0 != ce_cipher_inner(self, CIPHER_DEC, type, mode, false, key, key_sz,
                                 iv, iv_sz, i_buf, true_input_sz, o_buf, o_sz)) {
            break;
        }

        /* The output buf 'o_buf' consists of Aad || PlainText || Tag*/
        mini_memcpy_s((void *)temp_buf_out, (void *)(o_buf + aad_sz + plain_sz), 16);

        if (mode == MODE_CCM) {
            uint8_t v_or = 0;

            for (int i = 0; i < 16; i++) {
                v_or |= temp_buf_out[i];
            }

            ret = 0u == v_or ? 0 : -2;
        } else if (mode == MODE_GCM) {
            uint8_t temp_buf_in[16];
            if (tag_size > 0) {
                mini_memcpy_s((void *)temp_buf_in, (void *)(i_buf + aad_sz + plain_sz + 16), tag_size);
            }

            if (0 != mini_memcmp_s(&temp_buf_out[0], &temp_buf_in[0],
                                   (size_t)(tag_size))) {
                /* aead verify error, will not return msg */
                *o_sz = 0x0u;
            }

            ret = 0;
        }
    } while (0);

    return ret;
}

U32 ce_cipher_start (void *self, ce_cipher_context *context,
                          const U8 *key, U32 key_sz, U32 iv, U32 iv_sz)
{
    uint32_t cnt = 0UL;
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);

    if ((NULL == context) || (NULL == key) || (iv_sz > 16)
        || ((MODE_ECB != context->algo_mode) && (0 == iv))) {
        DBG("%s: Opps, iv size %d\n", __FUNCTION__, iv_sz);
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    if ((MODE_XTS == context->algo_mode) && (key_sz / 2 != 16 ) && (key_sz % 2 != 0)) {
        /* XTS mode only support 128bit key.*/
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;
    if (MODE_XTS == context->algo_mode) {
        mini_memcpy_s((void *)SA_SLOT_ADDR(context->key_pos), key, key_sz / 2);
        mini_memcpy_s((void *)(SA_SLOT_ADDR(context->key_pos) + SA_SLOT_SZ/2), key + key_sz / 2, key_sz / 2);
        clean_invalidate_cache_range((const void *)SA_SLOT_ADDR(context->key_pos), key_sz / 2);
        clean_invalidate_cache_range((const void *)(SA_SLOT_ADDR(context->key_pos) + SA_SLOT_SZ/2), key_sz / 2);
    } else {
        mini_memcpy_s((void *)SA_SLOT_ADDR(context->key_pos), key, key_sz);
        clean_invalidate_cache_range((const void *)SA_SLOT_ADDR(context->key_pos), key_sz);
    }
    cnt++;
    if (MODE_ECB == context->algo_mode) {
        /* no need to set iv */
    } else if ((MODE_GCM == context->algo_mode) ||
                (MODE_CCM == context->algo_mode)) {
        /** TODO: set two IV(iv and nonce)*/
        uintptr_t addr = (U32)SA_SLOT_OFF(context->context_pos);
        ce_dma_copy(base, iv, addr, iv_sz, DMA_MEM_EXT, DMA_MEM_SRAM_SA);
    } else {
#if defined(CE_CIPHER_DEBUG)
        invalidate_cache_range((void *)iv, iv_sz);
        DBG("Iv is:\n");
        DBG_ARRAY_DUMP((uint8_t *)iv, iv_sz);
#endif
        uintptr_t addr = SA_SLOT_OFF(context->context_pos);
        ce_dma_copy(base, iv, addr, iv_sz, DMA_MEM_EXT, DMA_MEM_SRAM_SA);
    }
    cnt++;

    if (cnt != 4UL) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    return 0;
}

U32 ce_cipher_update (void *self, ce_cipher_context *context,
                      U32 i_buf, U32 i_sz, U32 o_buf, U32 *o_sz)
{
    uint32_t cnt = 0UL;

    if ((NULL == context) || (0 == i_buf) ||
        (0 == o_buf) || (NULL == o_sz)) {
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;

    if (i_sz % 16) {
        return CRYPTO_INVALID_PARAs;
    }
    cnt++;

    uint32_t local_addr;
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    local_addr = soc_to_dma_address(o_buf);
    ce->ce0_cipher_dst_addr_h = FV_CE0_CIPHER_DST_ADDR_H_CIPHER_DST_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_cipher_dst_addr = (uint32_t)local_addr;

    local_addr = soc_to_dma_address(i_buf);
    ce->ce0_cipher_src_addr_h = FV_CE0_CIPHER_SRC_ADDR_H_CIPHER_SRC_TYPE(EXT_MEMORY) | (uint8_t)(0);
    ce->ce0_cipher_src_addr = (uint32_t)local_addr;
    cnt++;

    /* Not use keyport. Key only in secure mem*/
    ce->ce0_cipher_key_addr = FV_CE0_CIPHER_KEY_ADDR_CIPHER_KEY_ADDR(SA_SLOT_OFF(context->key_pos));
    if (MODE_XTS == context->algo_mode) {
        /*For XTS mode ,there only support 128bit key.*/
        ce->ce0_cipher_key_addr |= FV_CE0_CIPHER_KEY_ADDR_CIPHER_KEY2_ADDR(SA_SLOT_OFF(context->key_pos) + SA_SLOT_SZ/2);
    }
    cnt++;

    uint32_t hdr_sz = 0;
    uint8_t hdr_buf[24];
    if (MODE_CCM == context->algo_mode) {
        mini_memcpy_s((void *)hdr_buf, (void *)i_buf, 24);
        if ( 0 != crypto_get_aead_hdr_sz(MODE_CCM, (U8*)(uintptr_t)hdr_buf, i_sz, &hdr_sz)) {
            return -4;
        }
    } else if (MODE_GCM == context->algo_mode) {
        mini_memcpy_s((void *)hdr_buf, (void *)(i_buf + i_sz - 16), 16);
        if ( 0 != crypto_get_aead_hdr_sz(MODE_GCM, (U8*)(uintptr_t)hdr_buf, i_sz, &hdr_sz)) {
            return -4;
        }
    }
    cnt++;

    ce->ce0_cipher_header_len = hdr_sz;
    /* iv/ctx always in secure area */
    ce->ce0_cipher_iv_context_addr = FV_CE0_CIPHER_IV_CONTEXT_ADDR_CIPHER_IV_ADDR(SA_SLOT_OFF(context->context_pos))
                                     | FV_CE0_CIPHER_IV_CONTEXT_ADDR_CIPHER_CONTEXT_ADDR(context->context_pos);

    if (MODE_GCM == context->algo_mode) {
        /* no need to pass I(A)/I(C) to ce */
        i_sz -= 16;
    }
    cnt++;

    ce->ce0_cipher_payload_len = i_sz - hdr_sz;

    uint32_t ctl = FV_CE0_CIPHER_CTRL_AESMODE(0x01u << get_ce_aes_mode(context->algo_mode))
                   | FV_CE0_CIPHER_CTRL_CIPHER_KEYSIZE(get_ce_key_sz(context->key_len));


    ctl |= FV_CE0_CIPHER_CTRL_CIPHER_KEYTYPE(CE_KEYTYPE_SMEM_PRIV)
           | FV_CE0_CIPHER_CTRL_CIPHER_KEY2TYPE(CE_KEYTYPE_SMEM_PRIV);
    if (CIPHER_DEC == context->operation) {
        ctl |= BM_CE0_CIPHER_CTRL_CIPHERMODE | BM_CE0_CIPHER_CTRL_CIPHER_DECKEYCAL;
    }
    //ctl |= BM_CE0_CIPHER_CTRL_CIPHER_SAVE | BM_CE0_CIPHER_CTRL_CIPHER_LOAD;
    ce->ce0_cipher_ctrl = ctl;

#if defined(CE_CIPHER_DEBUG) && defined(DEBUG_ENABLE)
    dump_ce_cipher_regs(self);
#endif

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;
    cnt++;
    ce->ce0_cipher_ctrl |= BM_CE0_CIPHER_CTRL_CIPHER_GO;

    while (!(ce->ce0_intstat & BM_CE0_INTSTAT_CIPHER_DONE_INTSTAT));

    U32 v = ce->ce0_errstat;

    if (0 != FV_CE0_ERRSTAT_CIPHER_ERR_STAT(v)) {   /* Error flags */
        DBG("cipher_err_stat = 0x%x\n", FV_CE0_ERRSTAT_CIPHER_ERR_STAT(v));
        return -4;
    }

#if defined(CFG_CE_CIPHER_PADDING)
#error "CIPHER_PADDING TODO"
#else
    cnt++;
    if (((context->algo_mode == MODE_CCM) && (CIPHER_ENC == context->operation))
        || context->algo_mode == MODE_GCM) {
        *o_sz = ROUNDUP(i_sz, 16) + 16;
    } else {
        *o_sz = ROUNDUP(i_sz, 16);
    }
    cnt++;
    if (cnt != 9) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
#endif
    clean_invalidate_cache_range((void *)o_sz, 4);
#if defined(CE_CIPHER_DEBUG)
    invalidate_cache_range((void *)o_buf, *o_sz);
    DBG("%s: dump output(%d bytes) as \n", __FUNCTION__, *o_sz);
    DBG_ARRAY_DUMP((uint8_t *)(uintptr_t)o_buf, *o_sz);
#endif

    return 0;
}

U32 ce_cipher_finish (void *self, ce_cipher_context *context,
                      U32 o_buf, U32 *o_sz)
{
    if ((NULL == context) ||
        (0 == o_buf) || (NULL == o_sz)) {
        return -1;
    }

    if (MODE_CCM == context->algo_mode) {

    } else if (MODE_GCM == context->algo_mode) {

    }

    /** TODO: this function may be used to caculate padding data.*/
    return 0;
}
