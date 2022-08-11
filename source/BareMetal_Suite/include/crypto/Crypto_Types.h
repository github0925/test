/**
 * @file  Crypto_Types.h
 * @brief Semidrive G9 hardware layer Crypto Driver.
 */

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef CRYPTO_TYPES_H
#define CRYPTO_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Platform_Types.h"

typedef enum {
    ALG_HASH_MD5,
    ALG_HASH_SHA1,
    ALG_HASH_SHA224,
    ALG_HASH_SHA256,
    ALG_HASH_SHA384,
    ALG_HASH_SHA512,
    ALG_HASH_SM3,
    ALG_HASH_MAX,
} crypto_alg_hash_e;

typedef enum {
    ALG_PK_INVALID  =   0U,
    ALG_PK_RSA      =   1U,
    ALG_PK_RSA512   =   2U,
    ALG_PK_RSA1024  =   3U,
    ALG_PK_RSA2048  =   4U,
    ALG_PK_RSA3072  =   5U,
    ALG_PK_RSA4096  =   6U,
    ALG_PK_ECDSA    =   0x10U,
    ALG_PK_ECDSA_P192 = 0x11U,
    ALG_PK_ECDSA_P256 = 0x12U,
    ALG_PK_ECDSA_P384 = 0x13U,
    ALG_PK_ECDSA_P521 = 0x14U,
} crypto_alg_pk_e;

typedef enum {
    AES_128,
    AES_192,
    AES_256,

} cipher_type_e;

typedef enum {
    KEY_RSA1024  =   3U,
    KEY_RSA2048  =   4U,
    KEY_RSA3072  =   5U,
    KEY_RSA4096  =   6U,

    KEY_ECDSA_P192 = 0x11U,
    KEY_ECDSA_P256 = 0x12U,
    KEY_ECDSA_P384 = 0x13U,
    KEY_ECDSA_P521 = 0x14U,

    EKY_EDDSA_ED25519 = 0x15,

    KEY_AES_128_ECB = 0x20,
    KEY_AES_128_CBC,
    KEY_AES_128_CFB,
    KEY_AES_128_OFB,
    KEY_AES_128_CTR,
    KEY_AES_128_END,

    KEY_AES_192_ECB = 0x30,
    KEY_AES_192_CBC,
    KEY_AES_192_CFB,
    KEY_AES_192_OFB,
    KEY_AES_192_CTR,
    KEY_AES_192_END,

    KEY_AES_256_ECB = 0x40,
    KEY_AES_256_CBC,
    KEY_AES_256_CFB,
    KEY_AES_256_OFB,
    KEY_AES_256_CTR,
    KEY_AES_256_END,

    KEY_HMAC_SHA1 = 0x50,
    KEY_HMAC_SHA256,
    KEY_HMAC_SHA384,
    KEY_HMAC_SHA512,
    KEY_HMAC_END,

    KEY_CMAC_128 = 0x60,
    KEY_CMAC_192,
    KEY_CMAC_256,
    KEY_CMAC_END,

} key_type_e;

typedef enum {
    MODE_ECB = 0,
    MODE_CBC = 1,
    MODE_CTR = 2,
    MODE_CFB = 3,
    MODE_OFB = 4,
    MODE_CCM = 5,
    MODE_GCM = 6,
    MODE_XTS = 7,
    MODE_CMAC = 8,
    MODE_MAX,
} cipher_mode_e;

typedef enum {
    MAC_CMAC,
    MAC_HMAC,
} mac_type_e;

typedef enum {
    CIPHER_ENC,
    CIPHER_DEC,
} cipher_op_e;

typedef enum {
    NOINIT = 1,
    READY,
    BUSY,
    FAILED,
} engine_status_e;

#define REQ_SIZE(n) (8 + (n) * sizeof(uint32))
#define GET_REQ_PARA_NUM(req)    ((req->sz - 8) / sizeof(uint32))

typedef enum {
    CRYPTO_SUCCESS = 0x00,
    CRYPTO_FAILED = 0xFF,
    CRYPTO_INVALID_PARAs = 0x11,
    CRYPTO_INVALID_API = 0x22,
    CRYPTO_NOT_SUPPORT = 0x33,
    CRYPTO_INVALID_ALGO = 0x44,
    CRYPTO_ECDSA_SIGN_INNER_ERROR = 0x55,
    CRYPTO_ECDSA_VERIFY_INNER_ERROR = 0x66,
    CRYPTO_CONTEXT_ID_EXISTED = 0x77,
    CRYPTO_CONTEXT_ID_NOT_EXISTED = 0x88,
    CRYPTO_CONTEXT_POOL_FULLED = 0x99,
    CRYPTO_BUFFER_SMALL = 0xAA,
    CRYPTO_UNALIGNED_DATA = 0xBB
} crypto_status_t;

typedef enum {
    CIPHER_KEY_INVALID = 0U,
    CIPHER_KEY_AES128 = 1U,
    CIPHER_KEY_AES256 = 2U,
} crypto_alg_cipher_e;

typedef struct {
    uint32 n_sz;   /* n, e, d must be in the same size */
    uint8 n[512];
    uint8 e[512];
    uint8 d[512];
} rsa_key_t;

/*
 * (r,s) Signature
 */
typedef struct {
    uint32 prime_sz;
    /* Public key Q(x, y) */
    uint8 x[68];
    uint8 y[68];
    /* Private key for signature generation */
    uint8 d[68];
    uint8 k[68];
} ecdsa_key_t;

typedef struct {
    uint32 prime_sz;
    /* Public key Q(x, y) */
    uint8 x[68];
    uint8 y[68];
} ecdsa_pk_t;

typedef struct {
    uint32 prime_sz;
    /* Public key Q(x, y) */
    uint8 Pk[32];
    uint8 Sk[32];
} eddsa_key_t;

typedef struct {
    uint32 n_sz;   /* n, e, d must be in the same size */
    uint8 n[512];
    uint8 e[512];
} rsa_pk_t;

typedef enum {
    EC_p192r1,
    EC_p256r1,
    EC_p384r1,
    EC_p521r1,
    EC_sm2p256v1,
    ED_25519
} ec_curve_e;

typedef struct {
    uint32 curve_id;
    uint32 prime_sz;
    uint8 p[68];
    uint8 n[68];
    uint8 xg[68];
    uint8 yg[68];
    uint8 a[68];
    uint8 b[68];
} ecc_curve_t;

typedef struct {
    uint32 curve_id;
    uint32 prime_sz;
    uint8 p[32];
    uint8 l[32];
    uint8 bx[32];
    uint8 by[32];
    uint8 d[32];
    uint8 I[32];
} edc_curve_t;

typedef struct {
    uint32 sz;     /* size of this struct */
    uint32 msg_sz; /* size of plain message */
#if defined(CFG_CRYPTO_CIPHER_PADDING)
    uint8 erek[32 + 16];  /* some cipher generate one more block */
#else
    uint8 erek[32];  /* some cipher generate one more block */
#endif
    uint8 cipher[0];  /* encrypted data (aligned with 16 bytes) + HMAC_sha256 */
} crypto_wrapper_t;

#define AES_CIPHER_TEXT_SZ(msg_sz)  ((((msg_sz) + 15) / 16)*16 + 16)
#define WRAPPER_BYTES(msg_sz)   \
    (sizeof(crypto_wrapper_t) + AES_CIPHER_TEXT_SZ(msg_sz) + 32)

/* bit mask to indicate owners, mapping to the CPU ID */
#define KEY_FLAG_OWNER(x)   ((x) & 0xffffu)
#define GFV_KEY_FLAG_OWNER(x)   ((x) & 0xffffu)
/* If set, indicating this key used for command authentication only */
#define KEY_FLAG_CAK            (0x01u << 16)
/* If not set, this key shall never be unwrapped into external memory */
#define KEY_FLAG_UNWRAP_ALLOWED (0x01u << 17)
/* If set, this key need to be authenticated before being used */
#define KEY_FLAG_CA_NEEDED      (0x01u << 18)
/* The CAK index to be used for command authentication */
#define KEY_FLAG_CAK_INDEX(x)   (((x) & 0x0fu) << 19)
#define GFV_KEY_FLAG_CAK_INDEX(x)   (((x) >> 19) & 0xfu)

typedef struct {
    uint16 sz;     /*size of this key: sizeof(hdr) + sizeof(key) */
    uint16 type;
    uint32 flag;
    uint32 id;
    uint32 tag;    /* fixed at : 0x304b4559:("0KEY")*/
    uint32 cak_id;
} ks_hdr_t;

#define KS_HDR_RSA(flag, type, id)  {sizeof(ks_rsa_key_t), type, flag, id, 0x304b4559}
#define KS_HDR_ECDSA(flag, type, id)  {sizeof(ks_ecdsa_key_t), type, flag, id, 0x304b4559}
#define KS_HDR_SYM(flag, type, id)  {sizeof(ks_sym_key_t), type, flag, id, 0x304b4559}

typedef struct {
    ks_hdr_t hdr;
    rsa_key_t key;
} ks_rsa_key_t;

typedef struct {
    ks_hdr_t hdr;
    ecdsa_key_t key;
} ks_ecdsa_key_t;

typedef struct {
    ks_hdr_t hdr;
    eddsa_key_t key;
} ks_eddsa_key_t;

typedef struct {
    uint32 sz;
    uint8 key[32];
} sym_key_t;

typedef struct {
    ks_hdr_t hdr;
    sym_key_t key;
} ks_sym_key_t;

typedef struct {
    ks_hdr_t hdr;
    sym_key_t key;
    uint64 rnd;
} ks_sym_key_ctn_t;

static inline boolean is_valid_cipher_mode(cipher_mode_e m)
{
    return (m < MODE_MAX);
}

static inline boolean is_valid_cipher_type(cipher_type_e type)
{
    return ((AES_128 == type) || (AES_192 == type) || (AES_256 == type));
}

static inline boolean is_valid_hash_alg(crypto_alg_hash_e alg)
{
    return (alg < ALG_HASH_MAX);
}

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_TYPES_H */
