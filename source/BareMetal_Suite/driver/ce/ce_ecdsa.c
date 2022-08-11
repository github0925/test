/********************************************************
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

/*
 *  Q(xg, yg)   Public key (Q lies on EC)
 *  d           Private key
 *  (r,s)       Signature
 *  hash        hash value
 */
/*
 * Terms regarding ECC.
 * q            field size where q=2^m (for binary field, GF(2^m))
 * p            field size (for prime field, GF(p))
 * a, b         parameters defining the curve E.
 *                  y^2 + xy = x^3 + ax^2 + b, for GF(2^m)
 *                  y^2 mod p = (x^3 + ax + b) mod p, for GF(p)
 * G(xg, yg)    Generator Point on the EC
 * n            the order of point G
 * h            cofactor of the EC (usually 2/4 for binary field and 1
 *              for prim field)
 */

/* it is caller's duty to align/pad message to key size.*/
extern U8 is_pke_in_ecc;

U32 current_curve_sz_t = 0;

U32 ce_ecdsa_sign(void *self,
                  const U8 *hash, U32 hash_sz,
                  U32 rs, U32 *rs_sz,
                  const ecc_curve_t *curve,
                  const ecdsa_key_t *key)
{
    U32 cnt = 0UL;

    if ((NULL == hash) || (0 == rs) || (NULL == key)
        || (NULL == curve) || (NULL == self)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;
    }

    U32 p_sz = key->prime_sz;
    cnt++;

    if ((hash_sz != p_sz) || (*rs_sz < (p_sz * 2))
        || (p_sz != curve->prime_sz)) {
        DBG("%s: Opps, invalid para\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;

    }
    cnt++;
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    //crypto_ecdsa_dump_key(key);

    ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
    /* Note: big endian (Swap byte) not work since of an HW issue here.
     * the vce logic only 'load' register value (actually a copy per VCE
     * channel) when 'go' asserted. So far, it is impossile to set this bit,
     * load operand to pke mem, then go */
    ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif
    if ((current_curve_sz_t != curve->prime_sz)
        || (is_pke_in_ecc == 0x00u)) {
        current_curve_sz_t = curve->prime_sz;
        /* load p */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->p), PKE_MEM_SLOT_ADDR(ECDSA_p_SLOT), p_sz);
        /* load n */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->n), PKE_MEM_SLOT_ADDR(ECDSA_n_SLOT), p_sz);
        /* load xg */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->xg), PKE_MEM_SLOT_ADDR(ECDSA_xg_SLOT), p_sz);
        /* load yg */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->yg), PKE_MEM_SLOT_ADDR(ECDSA_yg_SLOT), p_sz);
        /* load a */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->a), PKE_MEM_SLOT_ADDR(ECDSA_a_SLOT), p_sz);
        /* load b */
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->b), PKE_MEM_SLOT_ADDR(ECDSA_b_SLOT), p_sz);
    }
    cnt++;

    /* load d */
    ce_pkemem_write(base, (U32)(uintptr_t)(key->d), PKE_MEM_SLOT_ADDR(ECDSA_d_SLOT), p_sz);
    /* load k */
    ce_pkemem_write(base, (U32)(uintptr_t)(key->k), PKE_MEM_SLOT_ADDR(ECDSA_k_SLOT), p_sz);
    /* load hash */
    ce_pkemem_write(base, (U32)(uintptr_t)(hash), PKE_MEM_SLOT_ADDR(ECDSA_hash_SLOT), p_sz);
    cnt++;
    /* clear all int status bits */
    ce->ce0_intclr = 0xFFFFFFFFu;

    U32 sel = ce_get_selcurve_v(p_sz);
    ce_pke_op_e op = ECDSA_SIGN_GEN;

    if (is_sm2_curve(curve)) {
        sel = SEL_CURVE_NO_ACC;
        op = SM2_SIGN;
    }
    cnt++;
    ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(p_sz - 1)
                             | FV_CE0_PK_COMMANDREG_PK_SELCURVE(sel)
                             | FV_CE0_PK_COMMANDREG_PK_TYPE(op);

    ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

    while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
           || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));
    cnt++;
    U32 v = ce->ce0_pk_statusreg;

    if ( v & (0xFFFu << 4)) {   /* Error flags */
        DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
            ((v >> 4) & 0xFFFu), (v & 0xFu));
        return CRYPTO_ECDSA_SIGN_INNER_ERROR;
    }

    /* read crypted text */
    ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(ECDSA_r_SLOT), rs, p_sz);
    ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(ECDSA_s_SLOT), rs + p_sz, p_sz);

    *rs_sz = 2 * p_sz;
    cnt++;
    clean_invalidate_cache_range((void *)rs_sz, 4);
    invalidate_cache_range((void *)rs, *rs_sz);

    if (cnt != 7) {
        FLOW_ERROR_AND_RESET
        return -1;
    }
    return 0;
}

U32 ce_ecdsa_verify(void *self,
                    const U8 *hash, U32 hash_sz,
                    U32 rs, U32 rs_sz,
                    const ecc_curve_t *curve,
                    const ecdsa_key_t *key)
{
    U32 res = -1;
    U32 cnt = 0;

    do {
        if ((NULL == hash) || (0 == rs) || (NULL == key)
            || (NULL == curve) || (NULL == self)) {
            res = CRYPTO_INVALID_PARAs;
            DBG("%s: Opps, null pointer.\n", __FUNCTION__);
            break;
        }

        cnt++;
        U32 p_sz = key->prime_sz;

        if ((hash_sz != p_sz) || (rs_sz != (p_sz * 2))
            || (p_sz != curve->prime_sz)) {
            res = CRYPTO_INVALID_PARAs;
            DBG("%s: Opps, invalid para\n", __FUNCTION__);
            break;
        }

        arch_clean_cache_range((void*)hash, hash_sz);
        arch_clean_cache_range((void*)rs, rs_sz);
        arch_clean_cache_range((void*)key, sizeof(ecdsa_key_t));
        arch_clean_cache_range((void*)curve, sizeof(ecc_curve_t));

        cnt++;
        uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
        cryptoengine_t *ce = (cryptoengine_t *) base;

        ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
        /* Note: big endian (Swap byte) not work since of an HW issue here.
         * the vce logic only 'load' register value (actually a copy per VCE
         * channel) when 'go' asserted. So far, it is impossile to set this bit,
         * load operand to pke mem, then go */
        ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif
        cnt++;
        if ((current_curve_sz_t != curve->prime_sz)
            || (is_pke_in_ecc == 0x00u)) {
            current_curve_sz_t = curve->prime_sz;
            /* load p */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->p), PKE_MEM_SLOT_ADDR(ECDSA_p_SLOT), p_sz);
            /* load n */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->n), PKE_MEM_SLOT_ADDR(ECDSA_n_SLOT), p_sz);

            /* load xg */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->xg), PKE_MEM_SLOT_ADDR(ECDSA_xg_SLOT), p_sz);
            /* load yg */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->yg), PKE_MEM_SLOT_ADDR(ECDSA_yg_SLOT), p_sz);

            /* load a */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->a), PKE_MEM_SLOT_ADDR(ECDSA_a_SLOT), p_sz);
            /* load b */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->b), PKE_MEM_SLOT_ADDR(ECDSA_b_SLOT), p_sz);
        }
        cnt++;
        ce_pkemem_write(base, (U32)(uintptr_t)(key->x), PKE_MEM_SLOT_ADDR(ECDSA_x_SLOT), p_sz);
        ce_pkemem_write(base, (U32)(uintptr_t)(key->y), PKE_MEM_SLOT_ADDR(ECDSA_y_SLOT), p_sz);

        cnt++;
        /* load hash */
        /*hash come from tcm,No need to maintain cache */
        ce_pkemem_write(base, (U32)(uintptr_t)(hash), PKE_MEM_SLOT_ADDR(ECDSA_hash_SLOT), p_sz);
        /* load r */
        ce_pkemem_write(base, (U32)(rs), PKE_MEM_SLOT_ADDR(ECDSA_r_SLOT), p_sz);
        cnt++;
        /* load s */
        ce_pkemem_write(base, (U32)(rs + p_sz), PKE_MEM_SLOT_ADDR(ECDSA_s_SLOT), p_sz);
        /* load x */

        cnt++;
        /* load y */

        /* clear all int status bits */
        ce->ce0_intclr = 0xFFFFFFFFu;

        U32 sel = ce_get_selcurve_v(p_sz);
        ce_pke_op_e op = ECDSA_SIGN_VERIFY;

        if (is_sm2_curve(curve)) {
            sel = SEL_CURVE_NO_ACC;
            op = SM2_VERIFY;
        }

        ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(p_sz - 1)
                                 | FV_CE0_PK_COMMANDREG_PK_SELCURVE(sel)
                                 | FV_CE0_PK_COMMANDREG_PK_TYPE(op);

        ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

        while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
               || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

        cnt++;

        U32 v = ce->ce0_pk_statusreg;

        if ( v & (0xFFFu << 4)) {   /* Error flags */
            DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
                ((v >> 4) & 0xFFFu), (v & 0xFu));
            res = CRYPTO_ECDSA_VERIFY_INNER_ERROR;
            break;
        }

        if (!((0 == (ce->ce0_pk_statusreg & (0xFFFu << 4))) && (8 == cnt))) {
            return -1;
        }
        res = 0;
    } while (0);

    return res;
}
