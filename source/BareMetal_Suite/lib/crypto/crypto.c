/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <atb_crypto.h>
#include <bn.h>
#include <soc.h>
#include <arch.h>
#include <stddef.h>

#if defined(ATB_SIGNER)
#define iDBG(fmt, args...)     printf(fmt, ##args)
#else
#define iDBG(fmt, args...)     DBG(fmt, ##args)
#endif

//#define CRYPTO_WRAPPER_DEBUG

#if defined(CORE_host)
uint8_t common_buf[COMMON_BUF_SIZE];
#else
uint8_t common_buf[COMMON_BUF_SIZE] __attribute__((aligned(CACHE_LINE)));
#endif
extern crypto_eng_t *g_crypto_eng_list[];

U8 is_pke_in_ecc = 0x0u;
crypto_eng_t *crypto;

static U32 current_curve_sz = 0;

static U8 crypto_buf[528];
static rsa_key_t rsa_key;
static ecdsa_key_t ec_key;
static ecc_curve_t curve_buf;

static uint8_t huk[32]  = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

U32 crypto_get_hash_size(U8 alg)
{
    return (ALG_HASH_SHA256 == alg) ? 32 :
           (ALG_HASH_SHA1 == alg) ? 20 :
           (ALG_HASH_SHA224 == alg) ? 28 :
           (ALG_HASH_SHA384 == alg) ? 48 :
           (ALG_HASH_SHA512 == alg) ? 64 : 0;
}

U32 crypto_get_sig_sz(U8 alg)
{
    U32 sz = 0;

    if (ALG_PK_RSA512 == alg) {
        sz = 64;
    } else if (ALG_PK_RSA1024 == alg ) {
        sz = 128;
    } else if (ALG_PK_RSA2048 == alg) {
        sz = 256;
    } else if (ALG_PK_RSA3072 == alg) {
        sz = 384;
    } else if (ALG_PK_RSA4096 == alg) {
        sz = 512;
    } else if (ALG_PK_ECDSA_P256 == alg) {
        sz = 32 * 2;
    } else if (ALG_PK_ECDSA_P384 == alg) {
        sz = 48 * 2;
    } else if (ALG_PK_ECDSA_P521 == alg) {
        sz = 66 * 2;
    }

    return sz;
}

U32 crypto_get_rsa_pk_type(U32 n_sz)
{
    U8 type = ALG_PK_INVALID;

    if (64 == n_sz) {
        type = ALG_PK_RSA512;
    } else if (128 == n_sz) {
        type = ALG_PK_RSA1024;
    } else if (256 == n_sz) {
        type = ALG_PK_RSA2048;
    } else if (384 == n_sz) {
        type = ALG_PK_RSA3072;
    } else if (512 == n_sz) {
        type = ALG_PK_RSA4096;
    }

    return type;
}

U32 crypto_get_ecdsa_pk_type(U32 prime_sz)
{
    U8 type = ALG_PK_INVALID;

    if (24 == prime_sz) {
        type = ALG_PK_ECDSA_P192;
    } else if (32 == prime_sz) {
        type = ALG_PK_ECDSA_P256;
    } else if (48 == prime_sz) {
        type = ALG_PK_ECDSA_P384;
    } else if (66 == prime_sz) {
        type = ALG_PK_ECDSA_P521;
    }

    return type;
}

crypto_eng_t *crypto_find_eng(crypto_eng_type_e type)
{
    crypto_eng_t *p = NULL;

    /*
     * For ANY, the first one be picked up. For SW, the first one which is
     * 'SW' will be picked-up. For HW, the first 'HW' eng will be picked-up if
     * exist. If no luck (say, no HW eng all been disabled) the the first 'SW'
     * eng will be picked-up.
     */
    for (unsigned int i = 0; i < 2; i++) {
        for (crypto_eng_t **eng = &g_crypto_eng_list[0]; NULL != *eng; eng++) {
            U32 t = (*eng)->attr;

            if ((FV_CRYPTO_ATTR_TYPE(t) == type) || (CRYPTO_ENG_ANY == type)) {
                p = *eng;

                if (NULL != (*eng)->ops.is_disabled) {
                    if ((*eng)->ops.is_disabled(*eng)) {
                        p = NULL;
                    }
                }

                if (NULL != p) {
                    break;
                }
            }
        }

        if ((CRYPTO_ENG_HW == type)
            && (NULL == p)) {
            type = CRYPTO_ENG_SW;
        } else {
            break;
        }
    }

    if (NULL != p) {
        DBG("%s be selected as crypto engine.\n", p->name);
    }

    return p;
}

void crypto_rsa_pk_dump(const rsa_pk_t *key)
{
    if (NULL != key) {
        iDBG("n_sz=%d\n", key->n_sz);

        iDBG("Modulus(n) dump as: \n");
        DBG_ARRAY_DUMP(key->n, key->n_sz);

        iDBG("Exponent(e) dump as: \n");
        DBG_ARRAY_DUMP(key->e, key->n_sz);
    }
}

void crypto_rsa_dump_key(const rsa_key_t *key)
{
    if (NULL != key) {
        iDBG("dump rsa key as:\n");
        crypto_rsa_pk_dump((const rsa_pk_t *)key);

        iDBG("PrivK(d) dump as: \n");
        DBG_ARRAY_DUMP(key->d, key->n_sz);
    }
}

void crypto_ecdsa_pk_dump(const ecdsa_pk_t *key)
{
    iDBG("prime_sz=%d\n", key->prime_sz);

    iDBG("x dump as: \n");
    DBG_ARRAY_DUMP(key->x, key->prime_sz);

    iDBG("y dump as: \n");
    DBG_ARRAY_DUMP(key->y, key->prime_sz);
}

void crypto_ecdsa_dump_key(const ecdsa_key_t *key)
{
    if (NULL != key) {
        iDBG("dump ecdsa key as:\n");
        crypto_ecdsa_pk_dump((const ecdsa_pk_t *) key);

        iDBG("PrivK(d) dump as: \n");
        DBG_ARRAY_DUMP(key->d, key->prime_sz);

        iDBG("k dump as: \n");
        DBG_ARRAY_DUMP(key->k, key->prime_sz);
    }
}

void crypto_dump_ecc_curve(const ecc_curve_t *c)
{
    if (NULL != c) {
        iDBG("p dump as:\n");
        DBG_ARRAY_DUMP(c->p, c->prime_sz);
        iDBG("n dump as:\n");
        DBG_ARRAY_DUMP(c->n, c->prime_sz);
        iDBG("xg dump as:\n");
        DBG_ARRAY_DUMP(c->xg, c->prime_sz);
        iDBG("yg dump as:\n");
        DBG_ARRAY_DUMP(c->yg, c->prime_sz);
        iDBG("a dump as:\n");
        DBG_ARRAY_DUMP(c->a, c->prime_sz);
        iDBG("b dump as:\n");
        DBG_ARRAY_DUMP(c->b, c->prime_sz);
    }
}

U32 crypto_init(void)
{
    U32 res = -1;

    do {
        crypto = crypto_find_eng(CRYPTO_ENG_HW);

        if (NULL == crypto) {
            break;
        }

        if (NULL == crypto->ops.init) {
            res = -2;
            break;
        } else if (0 != crypto->ops.init(crypto)) {
            res = -3;
            break;
        }

        res = 0;
    } while (0);

    return res;
}

U32 crypto_deinit(void)
{
    U32 res = -1;

    do {
        if (NULL == crypto) {
            break;
        }

        if ( NULL != crypto->ops.deinit) {
            res = crypto->ops.deinit(crypto);

            if (0 != res) {
                break;
            }
        }

        res = 0;
    } while (0);

    return res;
}

U32 crypto_rsa_sign(U8 alg,
                    U32 msg, U32 msg_sz,
                    U32 sig, U32 *sig_sz,
                    U32 key)
{
    U32 res = -1;
    /** @warn: use bss.(tcm) buffer */
    U32 prefix_len = 0U;
    U32 *h_sz = (U32 *)&common_buf[160];
    U8 *hash = &common_buf[192];
    *h_sz = 512;

    do {
        if ((NULL == crypto)
            || (NULL == crypto->ops.hash)
            || (NULL == crypto->ops.rsa_sign)
            || (0 == key)
            || (0 == msg)
            || (0 == sig)) {
            res = -1;
            break;
        }
        rsa_key_t *ikey = &rsa_key;

        mini_memcpy_s(ikey, (uint8_t *)(uintptr_t)key, sizeof(rsa_key_t));
        U32 n_sz = ikey->n_sz;

        if (0 != crypto_hash(alg, (U32)msg, msg_sz, (U32)(uintptr_t)hash, h_sz)) {
            res = -3;
            break;
        }

        if (*h_sz < n_sz) {
            prefix_len = add_hashinfo(crypto_buf, n_sz, hash, *h_sz, alg);
            if (0 != pkcs1_v1p5_pad(crypto_buf, n_sz, prefix_len)) {
                res = -4;
                break;
            }
        } else {
            mini_memcpy_s(crypto_buf, hash, n_sz);
        }

        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            mini_mem_rvs_s(ikey->n, n_sz);
            mini_mem_rvs_s(ikey->d, n_sz);
            mini_mem_rvs_s(crypto_buf, n_sz);
        }
        res = crypto->ops.rsa_sign(crypto,
                                   crypto_buf, n_sz,
                                   sig, sig_sz, ikey);
        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            if (*sig_sz > 512) {
                res = -1;
            } else {
                invalidate_cache_range((void *)(uintptr_t)sig, *sig_sz);
                mini_mem_rvs_s((void *)(uintptr_t)sig, *sig_sz);
                clean_invalidate_cache_range((void *)(uintptr_t)sig, *sig_sz);
            }
        }
    } while (0);

    return res;
}

U32 crypto_rsa_verify(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 sig, U32 sig_sz,
                      U32 key)
{
    U32 res = -1;
    U32 prefix_len = 0U;
    /** @warn: use bss.(tcm) buffer */
    U32 *h_sz = (U32 *)&common_buf[160];
    U8 *hash = &common_buf[192];
    *h_sz = 512;

    do {
        if ((NULL == crypto)
            || (NULL == crypto->ops.rsa_verify)
            || (0 == key)
            || (0 == msg)
            || (0 == sig)) {
            res = -1;
            break;
        }

        rsa_key_t *ikey = &rsa_key;

        mini_memcpy_s(ikey, (uint8_t *)(uintptr_t)key, sizeof(rsa_key_t));

        U32 n_sz = ikey->n_sz;

        if (sig_sz < n_sz) {
            res = -2;
            break;
        }

        if (0 != (res = crypto_hash(alg, (U32)msg, msg_sz, (U32)(uintptr_t)hash, h_sz))) {
            INFO("%s %d res:%u\n", __FUNCTION__, __LINE__, res);
            res = -3;
            break;
        }

        if (*h_sz < n_sz) {
            prefix_len = add_hashinfo(crypto_buf, n_sz, hash, *h_sz, alg);
            if (0 != pkcs1_v1p5_pad(crypto_buf, n_sz, prefix_len)) {
                res = -4;
                break;
            }
        } else {
            mini_memcpy_s(crypto_buf, hash, n_sz);
        }

        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            mini_mem_rvs_s(ikey->n, n_sz);
            mini_mem_rvs_s(ikey->e, n_sz);
            mini_mem_rvs_s(crypto_buf, n_sz);
            mini_mem_rvs_s((void *)(uintptr_t)sig, sig_sz);
        }

        res = crypto->ops.rsa_verify(crypto,
                                     crypto_buf, n_sz,
                                     (U32)(uintptr_t)sig, sig_sz, ikey
                                    );
    } while (0);

    return res;
}

#define REVERSE_EDC_CURVE(edc, p_sz) \
    do {\
        mini_mem_rvs_s(edc->p, p_sz);  \
        mini_mem_rvs_s(edc->l, p_sz);  \
        mini_mem_rvs_s(edc->bx, p_sz); \
        mini_mem_rvs_s(edc->by, p_sz); \
        mini_mem_rvs_s(edc->d, p_sz);  \
        mini_mem_rvs_s(edc->I, p_sz);  \
    } while(0)

#define REVERSE_ECC_CURVE(ec, p_sz) \
    do {\
        mini_mem_rvs_s(ec->p, p_sz);  \
        mini_mem_rvs_s(ec->n, p_sz);  \
        mini_mem_rvs_s(ec->xg, p_sz); \
        mini_mem_rvs_s(ec->yg, p_sz); \
        mini_mem_rvs_s(ec->a, p_sz);  \
        mini_mem_rvs_s(ec->b, p_sz);  \
    } while(0)

#define REVERSE_ECC_PUBKEY(key, p_sz)  \
    do {\
        mini_mem_rvs_s(key->x, p_sz);\
        mini_mem_rvs_s(key->y, p_sz);\
    } while(0)

#define REVERSE_ECC_PRIVKEY(key, p_sz)  \
    do {\
        mini_mem_rvs_s(key->d, p_sz);\
        mini_mem_rvs_s(key->k, p_sz);\
    } while(0)

static U32 ecdsa_sign_inner(U8 alg, U32 msg, U32 msg_sz,
                            U32 rs, U32 *rs_sz,
                            const ecc_curve_t *curve,
                            const ecdsa_key_t *key)
{
    U32 res = -1;
    /** @warn: use bss.(tcm) buffer */
    U32 *h_sz = (U32 *)&common_buf[160];
    U8 *hash = &common_buf[192];
    *h_sz = 512;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.ecdsa_sign)
            || (NULL == key) || (0 == msg) || (0 == rs)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }
        U32 p_sz = key->prime_sz;

        invalidate_cache_range((void *)rs_sz, 4);
        if (*rs_sz < 2 * p_sz) {
            DBG("error occur in ecdsa_sign_inner\n");
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        /** TODO: calulate ZA for SM2 signature */
        res = crypto_hash(alg, (U32)msg, msg_sz, (U32)(uintptr_t)hash, h_sz);
        if (0 != res) {
            break;
        }
#if defined(CFG_CRYPTO_PAD_DGST_2_ORDER)
        if (*h_sz < p_sz) {
            mini_memcpy_s(crypto_buf + p_sz - *h_sz, hash, *h_sz);
            mini_memclr_s(crypto_buf, p_sz - *h_sz);
        } else {
            mini_memcpy_s(crypto_buf, hash, p_sz);
        }

        *h_sz = p_sz;
#else
        mini_memcpy_s(crypto_buf, hash, *h_sz);
#endif

        ecdsa_key_t *ikey = &ec_key;
        mini_memcpy_s(ikey, key, sizeof(ecdsa_key_t));
        mini_memcpy_s(&curve_buf, curve, sizeof(ecc_curve_t));
        ecc_curve_t *p_curve = &curve_buf;
        do {
            crypto_get_rnd((U32)(uintptr_t)(ikey->k), p_sz);
            if (p_sz * 8 > 521) {
                ikey->k[0] = ikey->k[0] & 0x01U;
            }
        } while ((bn_is_zero(ikey->k, p_sz) || (bn_cmp(ikey->k, p_curve->p, p_sz) >= 0)));
        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            //DBG("%s: crypto eng in LE, to reverse things...\n", __FUNCTION__);
            is_pke_in_ecc = crypto->ops.get_pke_ops_type(crypto);
            if ((current_curve_sz != p_sz)
                || (is_pke_in_ecc == 0x00u)) {
                current_curve_sz = p_sz;
                REVERSE_ECC_CURVE(p_curve, p_sz);
            }
            REVERSE_ECC_PRIVKEY(ikey, p_sz);
            mini_mem_rvs_s(crypto_buf, p_sz);
        }

        //crypto_dump_ecc_curve(p_curve);

        res = crypto->ops.ecdsa_sign(crypto,
                                     crypto_buf, *h_sz,
                                     rs, rs_sz, p_curve, ikey);

        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            //DBG("%s: crypto eng in LE, to reverse signature to BE...\n",__FUNCTION__);
            if (*rs_sz > 132) {
                res = CRYPTO_ECDSA_SIGN_INNER_ERROR;
            } else {
                mini_mem_rvs_s((void *)(uintptr_t)rs, p_sz);
                mini_mem_rvs_s((void *)(uintptr_t)(rs + p_sz), p_sz);
                clean_invalidate_cache_range((void *)(uintptr_t)rs, *rs_sz);
            }
        }
    } while (0);

    return res;
}

U32 crypto_ecdsa_sign(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 rs, U32 *rs_sz,
                      U32 key)
{
    ecdsa_key_t * key_addr;

    if (0 == key) {
        DBG("Oops, error para\n");
        return CRYPTO_INVALID_PARAs;
    }

    key_addr = (ecdsa_key_t *)(uintptr_t)key;

    const ecc_curve_t *curve = crypto_get_ecc_curve(key_addr->prime_sz);
    if (NULL == curve) {
        DBG("Oops, error curve\n");
        return CRYPTO_INVALID_PARAs;
    }

    return ecdsa_sign_inner(alg, msg, msg_sz, rs, rs_sz, curve, key_addr);
}

static U32 ecdsa_verify_inner(
    U8 alg,
    U32 msg, U32 msg_sz,
    U32 rs, U32 rs_sz,
    const ecc_curve_t *curve,
    const ecdsa_key_t *key
)
{
    U32 res = -1;
    /** @warn: use bss.(tcm) buffer */
    U32 *h_sz = (U32 *)&common_buf[160];
    U8 *hash = &common_buf[192];
    U8 *i_rs = &common_buf[192 + 512];  /*66x2*/

    *h_sz = 512;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.ecdsa_verify)
            || (NULL == key) || (0 == msg) || (0 == rs)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        U32 p_sz = key->prime_sz;

        /* openssl rs is bigger than 2*p_sz, since some tag embedded */
        if (rs_sz < 2 * p_sz) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }

        res = crypto_hash(alg, msg, msg_sz, (U32)(uintptr_t)hash, h_sz);
        if (0 != res) {
            break;
        }

#if defined(CFG_CRYPTO_PAD_DGST_2_ORDER)

        if (*h_sz < p_sz) {
            mini_memcpy_s(crypto_buf + p_sz - *h_sz, hash, *h_sz);
            mini_memclr_s(crypto_buf, p_sz - *h_sz);
        } else {
            mini_memcpy_s(crypto_buf, hash, p_sz);
        }

        *h_sz = p_sz;
#else
        mini_memcpy_s(crypto_buf, hash, *h_sz);
#endif

        ecdsa_key_t *ikey = &ec_key;
        mini_memcpy_s(ikey, key, sizeof(ecdsa_key_t));
        mini_memcpy_s(&curve_buf, curve, sizeof(ecc_curve_t));
        ecc_curve_t *p_curve = &curve_buf;

        mini_memcpy_s((void *)i_rs, (void *)(uintptr_t)rs, rs_sz);

        if (crypto->attr & BM_CRYPTO_ATTR_LE) {
            is_pke_in_ecc = crypto->ops.get_pke_ops_type(crypto);
            if ((current_curve_sz != p_sz)
                || (is_pke_in_ecc == 0x00u)) {
                current_curve_sz = p_sz;
                REVERSE_ECC_CURVE(p_curve, p_sz);
            }
            REVERSE_ECC_PUBKEY(ikey, p_sz);
            mini_mem_rvs_s(crypto_buf, p_sz);
            mini_mem_rvs_s(i_rs, p_sz);
            mini_mem_rvs_s(i_rs + p_sz, p_sz);
        }

        res = crypto->ops.ecdsa_verify(crypto,
                                       crypto_buf, *h_sz,
                                       (uintptr_t)i_rs, rs_sz, p_curve, ikey);
    } while (0);

    return res;
}

U32 crypto_ecdsa_verify(
    U8 alg,
    U32 msg, U32 msg_sz,
    U32 rs, U32 rs_sz,
    U32 key
)
{
    ecdsa_key_t * key_addr;

    if (0 == key) {
        return CRYPTO_INVALID_PARAs;
    }
    key_addr = (ecdsa_key_t *)(uintptr_t)key;

    const ecc_curve_t *curve = crypto_get_ecc_curve(key_addr->prime_sz);

    if (NULL == curve) {
        return CRYPTO_INVALID_PARAs;
    }

    return ecdsa_verify_inner(alg, msg, msg_sz, rs, rs_sz, curve, key_addr);
}

U32 crypto_eddsa_sign(U32 msg, U32 msg_sz,
                      U32 rs, U32 *rs_sz,
                      U32 key)
{
    eddsa_key_t * key_addr;

    if ((0 == key) ||
        (NULL == crypto->ops.dma_copy)) {
        DBG("Oops, error para\n");
        return CRYPTO_INVALID_PARAs;
    }

    key_addr = (eddsa_key_t *)(uintptr_t)key;
    const edc_curve_t *curve = crypto_get_edc_curve();
    if (NULL == curve) {
        DBG("Oops, error curve\n");
        return CRYPTO_INVALID_PARAs;
    }

    eddsa_key_t *ikey = (eddsa_key_t *)&ec_key;    /*we reuse the ecdsa key slot for saving memory.*/
    edc_curve_t *p_curve = (edc_curve_t *)&curve_buf;    /*we reuse the ecc curve buffer for saving memory.*/
    mini_memcpy_s(ikey, key_addr, sizeof(eddsa_key_t));
    mini_memcpy_s(p_curve, curve, sizeof(edc_curve_t));

    REVERSE_EDC_CURVE(p_curve, 32);

    return crypto->ops.eddsa_sign(crypto, (U8 *)(uintptr_t)msg, msg_sz, rs, rs_sz, p_curve, ikey);
}

U32 crypto_eddsa_verify(U32 msg, U32 msg_sz,
                      U32 rs, U32 rs_sz,
                      U32 key)
{
    eddsa_key_t * key_addr;

    if ((0 == key) ||
        (NULL == crypto->ops.dma_copy)) {
        DBG("Oops, error para\n");
        return CRYPTO_INVALID_PARAs;
    }

    key_addr = (eddsa_key_t *)(uintptr_t)key;
    const edc_curve_t *curve = crypto_get_edc_curve();
    if (NULL == curve) {
        DBG("Oops, error curve\n");
        return CRYPTO_INVALID_PARAs;
    }

    eddsa_key_t *ikey = (eddsa_key_t *)&ec_key;    /*we reuse the ecdsa key slot for saving memory.*/
    edc_curve_t *p_curve = (edc_curve_t *)&curve_buf;    /*we reuse the ecc curve buffer for saving memory.*/
    mini_memcpy_s(ikey, key_addr, sizeof(eddsa_key_t));
    mini_memcpy_s(p_curve, curve, sizeof(edc_curve_t));

    REVERSE_EDC_CURVE(p_curve, 32);

    return crypto->ops.eddsa_verify(crypto, (U8 *)(uintptr_t)msg, msg_sz, rs, rs_sz, p_curve, ikey);
}

U32 crypto_get_rnd(U32 rnd, U32 size)
{
    U32 res = -1;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.get_rnd)
            || (0 == rnd)) {
            res = CRYPTO_INVALID_PARAs;
            break;
        }
        res = crypto->ops.get_rnd(crypto, rnd, size);

    } while (0);

    return res;
}

int crypto_cmac(cipher_type_e type,
                uint32_t key, uint32_t key_sz,
                uint32_t msg, size_t msg_sz,
                uint32_t mac, size_t *mac_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.cmac)
            || (0 == key) || (0 == msg) || (NULL == mac_sz) || (0 == mac)
            || (key_sz > sizeof(sym_key_t))) {
            DBG("%s: Opps, invalid ptrs\n", __FUNCTION__);
            break;
        }

        if (!is_valid_cipher_type(type)) {
            res = -2;
            break;
        }
        key_addr = (uint8_t *)(uintptr_t)key;
        res = crypto->ops.cmac(crypto, type, key_addr, msg, msg_sz, mac, mac_sz);

    } while (0);

    return res;
}

int crypto_hmac(crypto_alg_hash_e type,
                uint32_t key, uint32_t key_sz,
                uint32_t msg, size_t msg_sz,
                uint32_t mac, size_t *mac_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hmac)
            || (0 == key) || (0 == msg) || (NULL == mac_sz) || (0 == mac)) {
            DBG("%s: Opps, invalid ptrs\n", __FUNCTION__);
            break;
        }

        if (!is_valid_hash_alg(type)) {
            res = -2;
            break;
        }

        key_addr = (uint8_t *)(uintptr_t)key;

        res = crypto->ops.hmac(crypto, type, key_addr, key_sz, msg, msg_sz, mac, mac_sz);

    } while (0);

    return res;
}

/*out_put =sz + plain_sz + erek + cipher + mac (72+n*block_size)
 * sz           4
 * plain_sz     4
 * erek = AES(rek, aes_256_ecb, huk)  32
 * cipher = AES(msg, aes_256_cbc, erek, iv) n*block_size(32)
 * mac = HMAC(sz | erek | cipher, huk)  32
 */
int crypto_wrap(uint32_t msg, uint32_t msg_sz,
                uint32_t wrp, uint32_t *wrp_sz)
{
    int32_t res = -1;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hmac)
            || (NULL == crypto->ops.dma_copy)
            || (NULL == crypto->ops.cipher_enc) || (0 == msg)
            || (NULL == wrp_sz) || (0 == wrp)) {
            DBG("%s: Opps, invalid ptrs\n", __FUNCTION__);
            break;
        }
        /** @warn: use bss.(tcm) buffer */
        uint8_t *rek = &common_buf[64];  /*32*/
        uint8_t *iv = &common_buf[64 + 32];  /*32*/
        uint32_t *erek_sz = (uint32_t *)&common_buf[64 + 32 + 32];  /*4*/
        for (int i = 0; i < 32; i++) {
            iv[i] = i;
        }

        if (crypto->ops.get_rnd != NULL) {
            crypto->ops.get_rnd(crypto, (uintptr_t)rek, 32);
        }

        uint32_t erek_offset = offsetof(crypto_wrapper_t, erek);
        uint32_t msg_sz_offset = offsetof(crypto_wrapper_t, msg_sz);
        uint32_t sz_offset = offsetof(crypto_wrapper_t, sz);
        *erek_sz = offsetof(crypto_wrapper_t, erek);

        invalidate_cache_range(wrp_sz, 4);
        invalidate_cache_range((void *)(uintptr_t)wrp, *wrp_sz);

        mini_memcpy_s((void *)(uintptr_t)(wrp + msg_sz_offset), (void *)&msg_sz, 4);
        invalidate_cache_range((void *)(uintptr_t)(wrp + msg_sz_offset), 4);

        if (NULL != crypto->ops.cipher_inner) {
            if (0 != crypto->ops.cipher_inner(crypto, CIPHER_ENC, AES_256, MODE_ECB,
                                              true, (const uint8_t *)0, 32,
                                              0, 0, (U32)(uintptr_t)rek, 32,
                                              (U32)(wrp + erek_offset), (int *)erek_sz)) {
                res = -2;
                break;
            }
        } else {
            if (0 != crypto->ops.cipher_enc(crypto, AES_256, MODE_ECB,
                                            huk, 32, 0, 0,
                                            (uintptr_t)rek, 32,
                                            (wrp + erek_offset), (int *)erek_sz)) {
                res = -2;
                break;
            }
        }

        if (*erek_sz > sizeof(((crypto_wrapper_t *)0)->erek)) {
            return -1;
        }

        uint32_t *c_sz = (uint32_t *)&common_buf[64 + 32 + 32];  /*4*/

        *c_sz = *wrp_sz - sizeof(crypto_wrapper_t);

        uint32_t cipher_offset = offsetof(crypto_wrapper_t, cipher);

        if (0 != crypto->ops.cipher_enc(crypto, AES_256, MODE_CBC,
                                        rek, 32,
                                        (uintptr_t)iv, 32,
                                        msg, msg_sz,
                                        (wrp + cipher_offset), (int *)c_sz)) {
            res = -3;
            break;
        }

        uint32_t t_c_sz = *c_sz;
        uint32_t *hmac_sz = (uint32_t *)&common_buf[64 + 32 + 32];  /*4*/

        *hmac_sz = *wrp_sz - sizeof(crypto_wrapper_t) - t_c_sz;

        uint32_t t_sz = sizeof(crypto_wrapper_t) + t_c_sz;
        uint32_t temp = t_sz + 32;

        mini_memcpy_s((void *)(uintptr_t)(wrp + sz_offset), (void *)&temp, 4);

        if (NULL != crypto->ops.hash_inner) {
            if (0 != crypto->ops.hash_inner(crypto, ALG_HASH_SHA256, true,
                                            (const uint8_t *)0, 32, wrp, t_sz,
                                            wrp + t_sz, (U32 *)hmac_sz)) {
                res = -4;
                break;
            }
        } else {
            if (0 != crypto->ops.hmac(crypto, ALG_HASH_SHA256, huk, 32, wrp,
                                      t_sz,  wrp + t_sz, (size_t *)hmac_sz)) {
                res = -4;
                break;
            }
        }

        if (*hmac_sz != 32) {
            return -1;
        }
        mini_memcpy_s((void *)wrp_sz, (void *)(uintptr_t)(wrp + sz_offset), 4);
        clean_invalidate_cache_range(wrp_sz, 4);
        res = 0;

    } while (0);

    return res;
}

int crypto_unwrap(uint32_t msg, uint32_t *msg_sz,
        uint32_t wrp, uint32_t wrp_sz)
{
    int32_t res = -1;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.hmac)
            || (NULL == crypto->ops.dma_copy)
            || (NULL == crypto->ops.cipher_dec) || (0 == msg)
            || (NULL == msg_sz) || (0 == wrp)) {
            DBG("%s: Opps, invalid ptrs\n", __FUNCTION__);
            break;
        }

        uint8_t *iv = &common_buf[64];           /*32*/
        uint8_t *hmac = &common_buf[64 + 32];    /*32*/
        uint32_t *hmac_sz = (uint32_t *)&common_buf[64 + 32 + 32];  /*4*/
        for (int i = 0; i < 32; i++) {
            iv[i] = i;
        }

        *hmac_sz = 32;
        uint32_t c_sz = wrp_sz - 32 - sizeof(crypto_wrapper_t);//AES_CIPHER_TEXT_SZ(p_wrp->msg_sz);
        uint32_t t_sz = sizeof(crypto_wrapper_t) + c_sz;

        //DBG("%s: c_sz=%d, t_sz = %d, wrp_sz=%d\n", __FUNCTION__, c_sz, t_sz, wrp_sz);

        if (NULL != crypto->ops.hash_inner) {
            if (0 != crypto->ops.hash_inner(crypto, ALG_HASH_SHA256, true,
                                            (const uint8_t *)0, 32, wrp, t_sz,
                                            (U32)(uintptr_t)hmac, (U32 *)hmac_sz)) {
                res = -4;
                break;
            }
        } else {
            if (0 != crypto->ops.hmac(crypto, ALG_HASH_SHA256, huk, 32, wrp,
                                      t_sz, (U32)(uintptr_t)hmac, (size_t *)hmac_sz)) {
                res = -4;
                break;
            }
        }

        if (*hmac_sz != 32) {
            return -1;
        }
#if defined(CRYPTO_WRAPPER_DEBUG)
        DBG("%s: dump hmac(%d bytes) as:\n", __FUNCTION__, *hmac_sz);
        DBG_ARRAY_DUMP(hmac, *hmac_sz);
#endif
        //crypto->ops.dma_copy(crypto, wrp + t_sz, (U32)(uintptr_t)local_buf, hmac_sz);
        if (0 != mini_memcmp_s(hmac, (void *)(uintptr_t)(wrp + t_sz), *hmac_sz)) {
            res = -5;
            break;
        }

        uint8_t *rek = &common_buf[64 + 32];    /*64*/;
        uint32_t *rek_sz = (uint32_t *)&common_buf[64 + 32 + 64]; /*4*/
        *rek_sz = 64;

        uint32_t erek_offset = offsetof(crypto_wrapper_t, erek);
        uint32_t erek_size = sizeof(((crypto_wrapper_t *)0)->erek);

        if (NULL != crypto->ops.cipher_inner) {
            if (0 != crypto->ops.cipher_inner(crypto, CIPHER_DEC, AES_256, MODE_ECB,
                                              true, (const uint8_t *)0, 32,
                                              (U32)(uintptr_t)NULL, 0,
                                              (U32)(wrp + erek_offset), erek_size,
                                              (U32)(uintptr_t)rek, (int *)rek_sz)) {
                res = -10;
                break;
            }
        } else {
            if (0 != crypto->ops.cipher_dec(crypto, AES_256, MODE_ECB,
                                            huk, 32, (U32)(uintptr_t)NULL, 0,
                                            (U32)(wrp + erek_offset), erek_size,
                                            (U32)(uintptr_t)rek, (int *)rek_sz)) {
                res = -10;
                break;
            }
        }

#if defined(CRYPTO_WRAPPER_DEBUG)
        DBG("%s: dump rek(%d bytes) as:\n", __FUNCTION__, rek_sz);
        DBG_ARRAY_DUMP(rek, rek_sz);
#endif

        uint32_t cipher_offset = offsetof(crypto_wrapper_t, cipher);
        if (0 != crypto->ops.cipher_dec(crypto, AES_256, MODE_CBC,
                                        rek, 32,
                                        (U32)(uintptr_t)iv, 32,
                                        (U32)(wrp + cipher_offset), c_sz,
                                        msg, (int *)msg_sz)) {
            res = -11;
            break;
        }

#if defined(CRYPTO_WRAPPER_DEBUG)
        DBG("%s: dump msg(%d bytes) as:\n", __FUNCTION__, *msg_sz);
        DBG_ARRAY_DUMP(msg, *msg_sz);
#endif

        res = 0;

    } while (0);

    return res;
}

U32 crypto_sm2_sign(U8 alg,
                    U32 msg, U32 msg_sz,
                    U32 rs, U32 *rs_sz,
                    U32 key)
{
    ecdsa_key_t * key_addr;

    if (0 == key) {
        return CRYPTO_INVALID_PARAs;
    }

    key_addr = (ecdsa_key_t *)(uintptr_t)key;

    const ecc_curve_t *curve = crypto_get_sm2_curve(key_addr->prime_sz);

    if (NULL == curve) {
        return CRYPTO_INVALID_PARAs;
    }

    return ecdsa_sign_inner(ALG_HASH_SM3, msg, msg_sz, rs, rs_sz, curve, key_addr);
}

U32 crypto_sm2_verify(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 rs, U32 rs_sz,
                      U32 key)
{
    ecdsa_key_t * key_addr;

    if (0 == key) {
        return CRYPTO_INVALID_PARAs;
    }

    key_addr = (ecdsa_key_t *)(uintptr_t)key;

    const ecc_curve_t *curve = crypto_get_sm2_curve(key_addr->prime_sz);

    if (NULL == curve) {
        return CRYPTO_INVALID_PARAs;
    }

    return ecdsa_verify_inner(ALG_HASH_SM3, msg, msg_sz, rs, rs_sz, curve, key_addr);
}

/*
 *  Note: It's caller's duty to pack header and aad into i_buf.
 *  The format of header/aad is listed here: https://tools.ietf.org/html/rfc3610
 */
int32_t crypto_aead_enc(cipher_type_e type, cipher_mode_e mode,
                        uint32_t key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t add_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.aead_enc)
            || (0 == key) || (0 == i_buf)
            || (0 == o_buf) || (NULL == o_sz)) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            break;
        }

        if (!is_valid_cipher_type(type) || !is_valid_cipher_mode(mode)) {
            res = -2;
            break;
        }

        key_addr = (uint8_t *)(uintptr_t)key;

        res = crypto->ops.aead_enc(crypto, type, mode, key_addr, key_sz, iv, iv_sz,
                    add_sz, plain_sz, i_buf, i_sz, o_buf, o_sz);

    } while (0);

    return res;
}

int32_t crypto_aead_dec(cipher_type_e type, cipher_mode_e mode,
                        uint32_t key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t aad_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz)
{
    int32_t res = -1;
    uint8_t * key_addr;

    do {
        if ((NULL == crypto) || (NULL == crypto->ops.aead_enc)
            || (0== key) || (0 == i_buf)
            || (0 == o_buf) || (NULL == o_sz)) {
            DBG("%s: Opps, NULL PTRs\n", __FUNCTION__);
            break;
        }

        if (!is_valid_cipher_type(type) || !is_valid_cipher_mode(mode)) {
            res = -2;
            break;
        }

        key_addr = (uint8_t *)(uintptr_t)key;

        res = crypto->ops.aead_dec(crypto, type, mode, key_addr, key_sz, iv, iv_sz,
                                   aad_sz, plain_sz, i_buf, i_sz, o_buf, o_sz);

    } while (0);

    return res;
}
