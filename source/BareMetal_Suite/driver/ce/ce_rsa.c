/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <arch.h>
#include <atb_crypto.h>
#include "cryptoengine_reg.h"
#include "cryptoengine_reg_field_def.h"
#include "ce_dma.h"
#include "ce_wrapper.h"

/* it is caller's duty to align/pad message to key size.*/

#if defined(CFG_CE_API_RSA_ENCRYPT)
U32 ce_rsa_encrypt(void *self,
                   U32 m, U32 m_sz,
                   U32 c, U32 *c_sz,
                   const rsa_key_t *key)
{
    if ((0 == m) || (0 == c)
        || (NULL == key) || (NULL == self)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return -1;
    }

    //crypto_rsa_dump_key(key);
    U32 n_sz = key->n_sz;

    if ((m_sz != n_sz) || (*c_sz < n_sz)) {
        DBG("%s: Opps, invalid para.\n", __FUNCTION__);
        return -2;
    }

    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    DBG("%s: base = 0x%x\n", __FUNCTION__, (U32)base);

    ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
    /* Note: big endian (Swap byte) not work since of an HW issue here.
     * the vce logic only 'load' register value (actually a copy per VCE
     * channel) when 'go' asserted. So far, it is impossile to set this bit,
     * load operand to pke mem, then go */
    ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif

    /* load n */
    ce_pkemem_write(base, key->n, PKE_MEM_SLOT_ADDR(RSA_MODULUS_SLOT), n_sz);
    /* load m */
    ce_pkemem_write(base, m, PKE_MEM_SLOT_ADDR(RSA_PLAIN_SLOT), n_sz);
    /* load e */
    ce_pkemem_write(base, key->e,
                    PKE_MEM_SLOT_ADDR(RSA_PUBK_SLOT), n_sz);

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(n_sz - 1)
                             | FV_CE0_PK_COMMANDREG_PK_TYPE(RSA_ENCRYPT);

    ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

    U32 v = ce->ce0_pk_statusreg;

    if ( v & (0xFFFu << 4)) {   /* Error flags */
        DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
            ((v >> 4) & 0xFFFu), (v & 0xFu));
        return -4;
    }

    /* read crypted text */
    ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(RSA_CIPHER_SLOT), c, n_sz);

    *c_sz = n_sz;

    return 0;
}
#endif /* CFG_CE_API_RSA_ENCRYPT */

#if defined(CFG_CE_API_RSA_DECRYPT)
U32 ce_rsa_decrypt(void *self,
                   U8 *m, U32 *m_sz,
                   const U8 *c, U32 c_sz,
                   const rsa_key_t *key)
{
    if ((NULL == m) || (NULL == c)
        || (NULL == key) || (NULL == self)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return -1;
    }

    //crypto_rsa_dump_key(key);
    U32 n_sz = key->n_sz;

    if ((*m_sz < n_sz) || (c_sz != n_sz)) {
        DBG("%s: Opps, invalid para.\n", __FUNCTION__);
        return -2;
    }

    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
    ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif

    ce_pkemem_write(base, key->n, PKE_MEM_SLOT_ADDR(RSA_MODULUS_SLOT), n_sz);
    ce_pkemem_write(base, c, PKE_MEM_SLOT_ADDR(RSA_CIPHER_SLOT), n_sz);
    ce_pkemem_write(base, key->d, PKE_MEM_SLOT_ADDR(RSA_PRIVK_SLOT), n_sz);

    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFF;

    /* refer spec 6.2.5.2 */
    ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(n_sz - 1)
                             | FV_CE0_PK_COMMANDREG_PK_TYPE(RSA_DECRYPT);

    ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

    U32 v = ce->ce0_pk_statusreg;

    if ( v & (0xFFFu << 4)) {   /* Error flags */
        DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
            ((v >> 4) & 0xFFFu), (v & 0xFu));
        return -4;
    }

    DBG("pke done, status reg is 0x%x\n", ce->ce0_pk_statusreg);
    ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(RSA_PLAIN_SLOT), m, n_sz);

    *m_sz = n_sz;

    return 0;
}
#endif  /* CFG_CE_API_RSA_DECRYPT */

U32 ce_rsa_sign(void *self,
                const U8 *hash, U32 hash_sz,
                U32 sig, U32 *sig_sz,
                const rsa_key_t *key)
{
    U32 cnt = 0UL;
    if ((NULL == hash) || (0 == sig) || (NULL == key) || (NULL == self)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return -1;
    }

    U32 n_sz = key->n_sz;
    cnt++;
    invalidate_cache_range((void *)sig_sz, 4);
    if ((*sig_sz < n_sz) || (hash_sz != n_sz)) {
        DBG("%s: Opps, invalid para\n", __FUNCTION__);
        return -2;
    }

    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;
    cnt++;

    ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
    /* Note: big endian (Swap byte) not work since of an HW issue here.
     * the vce logic only 'load' register value (actually a copy per VCE
     * channel) when 'go' asserted. So far, it is impossile to set this bit,
     * load operand to pke mem, then go */
    ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif
    ce_pkemem_write(base, (U32)(uintptr_t)key->n, PKE_MEM_SLOT_ADDR(RSA_MODULUS_SLOT), n_sz);
    ce_pkemem_write(base, (U32)(uintptr_t)key->d, PKE_MEM_SLOT_ADDR(RSA_PRIVK_SLOT), n_sz);
    ce_pkemem_write(base, (U32)(uintptr_t)hash, PKE_MEM_SLOT_ADDR(RSA_HASH_SLOT), n_sz);
    cnt++;
    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFF;

    /* refer spec 6.2.5.2 */
    ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(n_sz - 1)
                             | FV_CE0_PK_COMMANDREG_PK_TYPE(RSA_SIGN_GEN);

    ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));
    cnt++;
    U32 v = ce->ce0_pk_statusreg;

    if ( v & (0xFFFu << 4)) {   /* Error flags */
        DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
            ((v >> 4) & 0xFFFu), (v & 0xFu));
        return -4;
    }
    cnt++;
    DBG("pke done, status reg is 0x%x\n", ce->ce0_pk_statusreg);
    ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(RSA_SIGN_SLOT), sig, n_sz);

    if (cnt != 5) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    *sig_sz = n_sz;

    clean_invalidate_cache_range((void *)sig_sz, 4);
    return 0;
}

U32 ce_rsa_verify(void *self,
                  const U8 *hash, U32 hash_sz,
                  U32 sig, U32 sig_sz,
                  const rsa_key_t *key)
{
    U32 res = -1;
    U32 cnt = 0;

    do {
        if ((NULL == hash) || (0 == sig) || (NULL == key) || (NULL == self)) {
            WARN("%s: Opps, null pointer.\n", __FUNCTION__);
            break;
        }

        cnt++;
        U32 n_sz = key->n_sz;

        if ((sig_sz != n_sz) || (hash_sz != n_sz)) {
            WARN("%s: Opps, invalid para\n", __FUNCTION__);
            res = -2;
            break;
        }

        cnt++;

        arch_clean_cache_range(hash, hash_sz);
        arch_clean_cache_range((void*)sig, sig_sz);
        arch_clean_cache_range(key->n, key->n_sz);
        arch_clean_cache_range(key->e, key->n_sz);

        uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
        cryptoengine_t *ce = (cryptoengine_t *) base;

        ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
        /* Note: big endian (Swap byte) not work since of an HW issue here.
         * the vce logic only 'load' register value (actually a copy per VCE
         * channel) when 'go' asserted. So far, it is impossile to
         * 'set this bit, load operand to pke mem, then go' */
        ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif
        ce_pkemem_write(base, (U32)(uintptr_t)key->n, PKE_MEM_SLOT_ADDR(RSA_MODULUS_SLOT), n_sz);
        ce_pkemem_write(base, (U32)(uintptr_t)key->e, PKE_MEM_SLOT_ADDR(RSA_PUBK_SLOT), n_sz);
        cnt++;
        ce_pkemem_write(base, (U32)(uintptr_t)hash, PKE_MEM_SLOT_ADDR(RSA_HASH_SLOT), n_sz);
        ce_pkemem_write(base, (U32)sig, PKE_MEM_SLOT_ADDR(RSA_SIGN_SLOT), n_sz);

        /* clear all int status bits */
        ce->ce0_intclr = 0xFFFFFFFF;
        cnt++;

        /* refer spec 6.2.5.2 */
        ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(n_sz - 1)
                                 | FV_CE0_PK_COMMANDREG_PK_TYPE(RSA_SIGN_VERIFY);

        ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

        while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
               || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

        U32 v = ce->ce0_pk_statusreg;

        if ( v & (0xFFFu << 4)) {   /* Error flags */
            WARN("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
                ((v >> 4) & 0xFFFu), (v & 0xFu));
            res = -3;
            break;
        }

        if (!((0 == (ce->ce0_pk_statusreg & (0xFFFu << 4))) && (4 == cnt))) {
            return -1;
        }
        res = 0;
    } while (0);

    return res;
}
