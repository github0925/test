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

/* key A(Sk, Pk), message M , base point B*/
/*
1. Use SHA-512 to hash:
        h = SHA-512(Sk)
2. Compute Secret scalar:
        s = lower_half(h)
3. Compute pseudo-random value:
        r = SHA-512(upper-half(h) || M)
4. Point multiplication in ed25519
        P(x,y) = r·B
*/

/* it is caller's duty to align/pad message to key size.*/
extern U8 is_pke_in_ecc;
extern U32 current_curve_sz_t;
extern uint8_t common_buf[COMMON_BUF_SIZE];

//#define EDDSA_DEBUG

#if defined(EDDSA_DEBUG)
static void dump_ce_eddsa_regs(void *self)
{
    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    DBG("ce->ce0_pk_commandreg = 0x%x\n", ce->ce0_pk_commandreg);
    DBG("ce->ce0_pke_ctrl = 0x%x\n", ce->ce0_pke_ctrl);
    DBG("ce->ce0_pk_statusreg = 0x%x\n", ce->ce0_pk_statusreg);
}
#endif

/*only for ed25519, ed25519ph/ctx not support.*/
/**
 * @brief ed25519 signature generate
 * @param[in] msg.
 * @param[in] msg_sz.
 * @param[out] rs.(R||S)
 * @param[in/out] rs_sz.
 * @param[in] curve.
 * @param[in] key.
 * @return 0/non-0
 *  @retval 0    successful
 *  @retval non-0  failed
*/
U32 ce_eddsa_sign(void *self,
                  const U8 *msg, U32 msg_sz,
                  U32 rs, U32 *rs_sz,
                  const edc_curve_t *curve,
                  const eddsa_key_t *key)
{
    U32 ret = 0;
    U32 cnt = 0UL;

    /** @warn: use bss.(tcm) buffer */
    U32 *sz = (U32 *)&common_buf[160];     /*size:32, just a size dustbin*/
    *sz = 64;

    U8 *hash = &common_buf[192];    /*64*/
    U8 *buf = &common_buf[256];     /*64*/

    ce_pke_op_e op;
    U32 sel;

    if ((0 == rs) || (NULL == key)
        || (NULL == curve) || (NULL == self)) {
        DBG("%s: Opps, null pointer.\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;
    }

    U32 p_sz = key->prime_sz;    /*32byte for ed25519, 57byte for ed448*/

    if ((*rs_sz < (p_sz * 2))
        || (p_sz != curve->prime_sz)) {
        DBG("%s: Opps, invalid para\n", __FUNCTION__);
        return CRYPTO_INVALID_PARAs;
    }

    uintptr_t base = Mcu_GetModuleBase(((crypto_eng_t *)self)->m);
    cryptoengine_t *ce = (cryptoengine_t *) base;

    do {
        cnt++;
        ce->ce0_pk_commandreg = 0;
#if defined(CFG_CE_PKE_BIGENDIAN)
        /* Note: big endian (Swap byte) not work since of an HW issue here.
        * the vce logic only 'load' register value (actually a copy per VCE
        * channel) when 'go' asserted. So far, it is impossile to set this bit,
        * load operand to pke mem, then go */
        ce->ce0_pk_commandreg |= BM_CE0_PK_COMMANDREG_PK_SWAP;
#endif
        if ((current_curve_sz_t != curve->prime_sz + 1)     /*prime_sz + 1" to identify ed25519 curve*/
            || (is_pke_in_ecc == 0x00u)) {

            current_curve_sz_t = curve->prime_sz + 1;
            /* load P */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->p), PKE_MEM_SLOT_ADDR(EdDSA_P_SLOT), p_sz);
            /* load L */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->l), PKE_MEM_SLOT_ADDR(EdDSA_L_SLOT), p_sz);
            /* load Bx */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->bx), PKE_MEM_SLOT_ADDR(EdDSA_Bx_SLOT), p_sz);
            /* load By */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->by), PKE_MEM_SLOT_ADDR(EdDSA_By_SLOT), p_sz);
            /* load d */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->d), PKE_MEM_SLOT_ADDR(EdDSA_d_SLOT), p_sz);
        }
        cnt++;

        ret = crypto_hash(ALG_HASH_SHA512, (U32)(uintptr_t)(key->Sk), key->prime_sz, (U32 )(uintptr_t)hash, sz);
        if (ret != 0) {
            break;
        }
        cnt++;
#if defined(EDDSA_DEBUG)
        DBG("s key :%d\n", key->prime_sz);
        DBG_ARRAY_DUMP((uint8_t *)key->Sk, key->prime_sz);
        DBG("p key :%d\n", key->prime_sz);
        DBG_ARRAY_DUMP((uint8_t *)key->Pk, key->prime_sz);
        DBG("msg :%d\n", msg_sz);
        DBG_ARRAY_DUMP((uint8_t *)msg, msg_sz);
        DBG("h :%d\n", *sz);
        DBG_ARRAY_DUMP((uint8_t *)hash, *sz);
#endif
        ret = crypto_hash_start(0xffff0001, ALG_HASH_SHA512);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32 )(uintptr_t)(&hash[32]), 32);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32 )(uintptr_t)(msg), msg_sz);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_finish(0xffff0001, (U32 )(uintptr_t)(buf), sz); /*get R*/
        if (ret != 0) {
            break;
        }
        cnt++;

        /* load Rlsb */
        ce_pkemem_write(base, (U32)(uintptr_t)(&buf[0]), PKE_MEM_SLOT_ADDR(EdDSA_Rlsb_SLOT), p_sz);
        /* load Rmsb */
        ce_pkemem_write(base, (U32)(uintptr_t)(&buf[32]), PKE_MEM_SLOT_ADDR(EdDSA_Rmsb_SLOT), p_sz);
        cnt++;

        op = EDDSA_POINT_MULTIPLICATION;
        sel = 5;
        /* clear all int status bits */
        ce->ce0_intclr = 0xFFFFFFFFu;

        ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(p_sz - 1)
                                | FV_CE0_PK_COMMANDREG_PK_SELCURVE(sel)
                                | FV_CE0_PK_COMMANDREG_PK_TYPE(op);

        ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

        while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
            || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));
        cnt++;
        U32 v = ce->ce0_pk_statusreg;

        if ( v & (0xFFFu << 4)) {   /* Error flags */
            DBG("Error Flag1 in PK_StatusReg is 0x%x, FailAddr=%d\n",
                ((v >> 4) & 0xFFFu), (v & 0xFu));
            return CRYPTO_ECDSA_SIGN_INNER_ERROR;
        }

        /* read point r·B */
        ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(EdDSA_Rx_SLOT), (U32 )(uintptr_t)(buf), p_sz);
        ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(EdDSA_Ry_SLOT), (U32 )(uintptr_t)(&buf[32]), p_sz);

        buf[32] |= (buf[0] << 7) & 0x80;
        mini_memcpy_s((void *)rs, &buf[32], p_sz);
        cnt++;
        ret = crypto_hash_start(0xffff0001, ALG_HASH_SHA512);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32 )(uintptr_t)(&buf[32]), p_sz);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32 )(uintptr_t)(key->Pk), p_sz);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32 )(uintptr_t)(msg), msg_sz);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_finish(0xffff0001, (U32 )(uintptr_t)(buf), sz); /*get k*/
        if (ret != 0) {
            break;
        }
        cnt++;
        /* load Rlsb */
        ce_pkemem_write(base, (U32)(uintptr_t)(&buf[0]), PKE_MEM_SLOT_ADDR(EdDSA_Klsb_SLOT), p_sz);
        /* load Rmsb */
        ce_pkemem_write(base, (U32)(uintptr_t)(&buf[32]), PKE_MEM_SLOT_ADDR(EdDSA_Kmsb_SLOT), p_sz);

        hash[0] &=  0xf8;
        hash[31] &= 0x7f;
        hash[31] |= 0x40;
        /* load s */
        ce_pkemem_write(base, (U32)(uintptr_t)(hash), PKE_MEM_SLOT_ADDR(EdDSA_Ry_SLOT), 32);
        cnt++;
        op = EDDSA_SIGN_GEN;
        sel = 5;
        /* clear all int status bits */
        ce->ce0_intclr = 0xFFFFFFFFu;
        ce->ce0_pk_commandreg = 0;

        ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(p_sz - 1)
                                | FV_CE0_PK_COMMANDREG_PK_SELCURVE(sel)
                                | FV_CE0_PK_COMMANDREG_PK_TYPE(op);

        ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

        while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
            || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

        v = ce->ce0_pk_statusreg;
        cnt++;
        if ( v & (0xFFFu << 4)) {   /* Error flags */
            DBG("Error Flag2 in PK_StatusReg is 0x%x, FailAddr=%d\n",
                ((v >> 4) & 0xFFFu), (v & 0xFu));
            return CRYPTO_ECDSA_SIGN_INNER_ERROR;
        }
        if (cnt != 10) {
            FLOW_ERROR_AND_RESET
            return -1;
        }

        ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(EdDSA_Rx_SLOT), (U32)(uintptr_t)(rs + 32), 32);

    } while (0);

    return 0;
}

/*only for ed25519, ed25519ph/ctx not support.*/
/**
 * @brief ed25519 signature verify
 * @param[in] msg.
 * @param[in] msg_sz.
 * @param[in] rs.(R||S)
 * @param[in] rs_sz.
 * @param[in] curve.
 * @param[in] key.
 * @return 0/non-0
 *  @retval 0    verify ok,
 *  @retval non-0  failed
*/
U32 ce_eddsa_verify(void *self,
                    const U8 *msg, U32 msg_sz,
                    U32 rs, U32 rs_sz,
                    const edc_curve_t *curve,
                    const eddsa_key_t *key)
{
    U32 ret = 0;
    U32 cnt = 0UL;

    /** @warn: use bss.(tcm) buffer */
    U32 *sz = (U32 *)&common_buf[160];  /*size:32, just a size dustbin*/
    *sz = 64;
    U8 *buf = &common_buf[192];  /*64*/

    U32 flag = 0;

    do {
        cnt++;
        if ( (0 == rs) || (NULL == key)
            || (NULL == curve) || (NULL == self)) {
            ret = CRYPTO_INVALID_PARAs;
            DBG("%s: Opps, null pointer.\n", __FUNCTION__);
            break;
        }

        U32 p_sz = key->prime_sz;

        if ((rs_sz != (p_sz * 2))
            || (p_sz != curve->prime_sz)) {
            ret = CRYPTO_INVALID_PARAs;
            DBG("%s: Opps, invalid para\n", __FUNCTION__);
            break;
        }
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
        if ((current_curve_sz_t != curve->prime_sz + 1)     /*prime_sz + 1" to identify ed25519 curve*/
            || (is_pke_in_ecc == 0x00u)) {
            current_curve_sz_t = curve->prime_sz + 1;
            /* load P */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->p), PKE_MEM_SLOT_ADDR(EdDSA_P_SLOT), p_sz);
            /* load L */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->l), PKE_MEM_SLOT_ADDR(EdDSA_L_SLOT), p_sz);
            /* load Bx */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->bx), PKE_MEM_SLOT_ADDR(EdDSA_Bx_SLOT), p_sz);
            /* load By */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->by), PKE_MEM_SLOT_ADDR(EdDSA_By_SLOT), p_sz);
            /* load d */
            ce_pkemem_write(base, (U32)(uintptr_t)(curve->d), PKE_MEM_SLOT_ADDR(EdDSA_d_SLOT), p_sz);
        }

        /* load I (only for ed25519)*/
        ce_pkemem_write(base, (U32)(uintptr_t)(curve->I), PKE_MEM_SLOT_ADDR(EdDSA_I_SLOT), p_sz);
        cnt++;
        ret = crypto_hash_start(0xffff0001, ALG_HASH_SHA512);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32)(uintptr_t)(rs), 32);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32)(uintptr_t)(key->Pk), 32);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_update(0xffff0001, (U32)(uintptr_t)(msg), msg_sz);
        if (ret != 0) {
            break;
        }
        ret = crypto_hash_finish(0xffff0001, (U32)(uintptr_t)(buf), sz); /*get k*/
        if (ret != 0) {
            break;
        }
        cnt++;
#if defined(EDDSA_DEBUG)
        DBG("s key :%d\n", key->prime_sz);
        DBG_ARRAY_DUMP(key->Pk, key->prime_sz);
        DBG("msg :%d\n", msg_sz);
        DBG_ARRAY_DUMP(msg, msg_sz);
        DBG("k :%d\n", buf_length);
        DBG_ARRAY_DUMP(buf, buf_length);
#endif
        /* load Klsb*/
        ce_pkemem_write(base, (U32)(uintptr_t)(buf), PKE_MEM_SLOT_ADDR(EdDSA_Klsb_SLOT), 32);

        /* load Kmsb*/
        ce_pkemem_write(base, (U32)(uintptr_t)(&buf[32]), PKE_MEM_SLOT_ADDR(EdDSA_Kmsb_SLOT), 32);

        /*load Ay*/
        mini_memcpy_s(buf, key->Pk, 32);
        buf[31] = buf[31] & 0x7f;         /*Ay*/
        ce_pkemem_write(base, (U32)(uintptr_t)(buf), PKE_MEM_SLOT_ADDR(EdDSA_Rmsb_SLOT), p_sz);
        cnt++;

        /*load Ry*/
        mini_memcpy_s(buf, (void *)rs, 32);
        buf[31] = buf[31] & 0x7f;        /*Ry*/
        ce_pkemem_write(base, (U32)(uintptr_t)(buf), PKE_MEM_SLOT_ADDR(EdDSA_Ry_SLOT), p_sz);

        /* load S*/
        ce_pkemem_write(base, (U32)(uintptr_t)(uint8 *)(rs + 32), PKE_MEM_SLOT_ADDR(EdDSA_Rx_SLOT), p_sz);

        /* clear all int status bits */
        ce->ce0_intclr = 0xFFFFFFFFu;
        cnt++;

        U32 sel = 5;
        ce_pke_op_e op = EDDSA_SIGN_VERIFY;
        flag |= (key->Pk[31] & 0x80) ? BM_CE0_PK_COMMANDREG_PK_FLAGA : 0;
        flag |= (((uint8 *)rs)[31] & 0x80) ? BM_CE0_PK_COMMANDREG_PK_FLAGB : 0;

        ce->ce0_pk_commandreg |= FV_CE0_PK_COMMANDREG_PK_SIZE(p_sz - 1)
                                 | FV_CE0_PK_COMMANDREG_PK_SELCURVE(sel)
                                 | FV_CE0_PK_COMMANDREG_PK_TYPE(op)
                                 | flag;

        ce->ce0_pke_ctrl |= BM_CE0_PKE_CTRL_PKE_GO;

        while ((!(ce->ce0_intstat & BM_CE0_INTSTAT_PKE_DONE_INTSTAT))
               || (ce->ce0_stat & BM_CE0_PK_STATUSREG_PK_BUSY));

        U32 v = ce->ce0_pk_statusreg;
        cnt++;

        if ( v & (0xFFFu << 4)) {   /* Error flags */
            DBG("Error Flag in PK_StatusReg is 0x%x, FailAddr=%d\n",
                ((v >> 4) & 0xFFFu), (v & 0xFu));
            ret = CRYPTO_ECDSA_VERIFY_INNER_ERROR;
            break;
        }

        /*get the Ax,verify the sign*/
        //ce_pkemem_read(base, PKE_MEM_SLOT_ADDR(EdDSA_Rlsb_SLOT), (uintptr_t)(buf), 32);
        if (cnt != 8) {
            FLOW_ERROR_AND_RESET
            return -1;
        }
        ret = 0;
    } while (0);

    return ret;
}
