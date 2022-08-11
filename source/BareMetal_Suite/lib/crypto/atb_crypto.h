/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <common_hdr.h>
#include <soc_def.h>
#include <Crypto_Types.h>

typedef struct {
    uint32 context_id;     /* cpu_id(16bit)||job_id(16bit) */
    uint64 total_len;
    uint8 residual_data[128];
    uint16 context_pos;
    uint8 algo;
} ce_hash_context;


typedef struct {
    uint8 algo_mode;
    uint8 key_len;
    uint8 operation;       /*de/encrypt */
    uint8 padding_mode;
    uint32 context_id;     /* cpu_id(16bit)||job_id(16bit) */
    uint8 residual_data[16];
    uint16 context_pos;
    uint16 key_pos;

} ce_cipher_context;

#define HASH_JOB_LIST_NUMBER    (10)
#define CIPHER_JOB_LIST_NUMBER  (10)

#define EMPTY_CONTEXT_ID    (0xffffffffUL)
#define SEC_MEM_MAX_SIZE    (4096)
#define TOTAL_SLOT_NUMBER       (SEC_MEM_MAX_SIZE/32)
#define MAX_SA_SLOT_NUMBER  (TOTAL_SLOT_NUMBER - 6)

#define SA_SLOT_AES_INNER_KEY0      (MAX_SA_SLOT_NUMBER)
#define SA_SLOT_AES_INNER_KEY2      (MAX_SA_SLOT_NUMBER + 3)
#define SA_SLOT_AES_INNER_IV        (MAX_SA_SLOT_NUMBER + 1)
#define SA_SLOT_AES_INNER_CONTEXT   (MAX_SA_SLOT_NUMBER + 2)
#define SA_SLOT_HASH_INNER_KEY      (MAX_SA_SLOT_NUMBER + 4)
#define SA_SLOT_HASH_INNER_IV       (MAX_SA_SLOT_NUMBER + 5)

extern void arch_enable_interrupt(void);
extern void arch_disable_interrupt(void);

uint8 context_buffer_status[MAX_SA_SLOT_NUMBER];

static inline uint16 get_sa_slot(uint8 num)
{
    uint16 i;
    if (num > 1)
        return (0xffff);
    for (i = 0; i < MAX_SA_SLOT_NUMBER - 1; i++) {
        if ((0 == context_buffer_status[i]) &&
            (0 == context_buffer_status[i + num])) {
            context_buffer_status[i] = 0xfU;
            context_buffer_status[i + num] = 0xfU;
            return i;
        }
    }
    if (i >= MAX_SA_SLOT_NUMBER) {
        return (0xffff);
    }
    return i;
}

static inline uint16 relese_sa_slot(uint16 index, uint8 num)
{
    if (num > 1)
        return (0xffff);
    if ((index + num) >= MAX_SA_SLOT_NUMBER) {
        return (0xffff);
    }
    context_buffer_status[index] = 0x00U;
    context_buffer_status[index + num] = 0x00U;

    return 0x0U;
}

static inline BOOL crypto_is_pk_rsa(U8 alg)
{
    return ((alg >= ALG_PK_RSA512) && (alg <= ALG_PK_RSA4096));
}

static inline BOOL crypto_is_pk_ecdsa(U8 alg)
{
    return ((alg >= ALG_PK_ECDSA_P192) && (alg <= ALG_PK_ECDSA_P521));
}

enum {
    KEY_RSA,
    KEY_EDDSA,
    KEY_ECDSA,
    KEY_SYM,
} ks_type_e;

static inline uint32_t get_ks_sz_by_type(uint32_t type)
{
    return (KEY_RSA == type ? sizeof(ks_rsa_key_t) :
            KEY_ECDSA  == type ? sizeof(ks_ecdsa_key_t) :
            KEY_EDDSA  == type ? sizeof(ks_eddsa_key_t) :
            KEY_SYM == type ? sizeof(ks_sym_key_t) :
            0);
}

static inline bool is_valid_key_type(uint32_t type)
{
    return ((type >= KEY_RSA1024) && (type <= KEY_RSA4096))
           || ((type >= KEY_ECDSA_P192) && (type <= KEY_ECDSA_P521))
           || ((type >= KEY_AES_128_ECB) && (type < KEY_AES_128_END))
           || ((type >= KEY_AES_192_ECB) && (type < KEY_AES_192_END))
           || ((type >= KEY_AES_256_ECB) && (type < KEY_AES_256_END))
           || ((type >= KEY_HMAC_SHA1) && (type < KEY_HMAC_END))
           || ((type >= KEY_CMAC_128) && (type < KEY_CMAC_END)) ;
}

static inline uint32_t get_ks_type(uint32_t key_type)
{
    uint32_t ks_t = 0;

    if ((key_type >= KEY_RSA1024) && (key_type <= KEY_RSA4096)) {
        ks_t = KEY_RSA;
    } else if ((key_type >= KEY_ECDSA_P192) && (key_type <= KEY_ECDSA_P521)) {
        ks_t = KEY_ECDSA;
    } else if (key_type == EKY_EDDSA_ED25519) {
        ks_t = KEY_EDDSA;
    }else {
        ks_t = KEY_SYM;
    }

    return ks_t;
}

typedef struct {
    U32 (*init) (void *self);
    U32 (*deinit) (void *self);
    U32 (*hash) (void *self, U8 alg, U32 , U32, U32 , U32 *h_sz);
    U32 (*hash_write_back) (void *self, ce_hash_context *context);
    U32 (*hash_update) (void *self, ce_hash_context *context, uintptr_t msg, U32 msg_sz);
    U32 (*hash_finish) (void *self, ce_hash_context *context, uintptr_t hash, U32 *hash_sz);
    U32 (*rsa_encrypt) (void *self,
                        U32 m, U32 m_sz,
                        U32 c, U32 *c_sz,
                        const rsa_key_t *);
    U32 (*rsa_decrypt) (void *self,
                        U32 m, U32 *m_sz,
                        U32 c, U32 c_sz,
                        const rsa_key_t *);
    U32 (*rsa_sign) (void *self,
                     const U8 *dgst, U32 d_sz,
                     U32 sig, U32 *sig_sz,
                     const rsa_key_t *);
    U32 (*rsa_verify) (void *self,
                       const U8 *dgst, U32 d_sz,
                       U32 sig, U32 sig_sz,
                       const rsa_key_t *);
    U32 (*ecdsa_sign) (void *self,
                       const U8 *dgst, U32 d_sz,
                       U32 sig, U32 *sig_sz,
                       const ecc_curve_t *,
                       const ecdsa_key_t *);
    U32 (*ecdsa_verify) (void *self,
                         const U8 *dgst, U32 d_sz,
                         U32 sig, U32 sig_sz,
                         const ecc_curve_t *,
                         const ecdsa_key_t *);
    U32 (*eddsa_sign) (void *self,
                  const U8 *msg, U32 msg_sz,
                  U32 rs, U32 *rs_sz,
                  const edc_curve_t *curve,
                  const eddsa_key_t *key);
    U32 (*eddsa_verify) (void *self,
                    const U8 *msg, U32 msg_sz,
                    U32 rs, U32 rs_sz,
                    const edc_curve_t *curve,
                    const eddsa_key_t *key);
    BOOL (*is_disabled) (void *self);
    U32 (*get_rnd) (void *self, U32 rnd, U32 len);
    int32_t (*cipher_enc) (void *self,
                           cipher_type_e type, cipher_mode_e mode,
                           const uint8_t *key, uint32_t key_sz,
                           uintptr_t iv, uint32_t iv_sz,
                           uintptr_t i_buf, int i_sz,
                           uintptr_t o_buf, int *o_sz);
    U32 (*cipher_start) (void *self, ce_cipher_context *context,
                          const U8 * key, U32 key_sz, U32 iv, U32 iv_sz);
    U32 (*cipher_update) (void *self, ce_cipher_context *context,
                          U32 msg, U32 msg_sz, U32 cipher, U32 *c_sz);
    U32 (*cipher_finish) (void *self, ce_cipher_context *context,
                          U32 cipher, U32 *c_sz);
    int32_t (*cipher_dec) (void *self,
                           cipher_type_e type, cipher_mode_e mode,
                           const uint8_t *key, uint32_t key_sz,
                           uint32_t iv, uint32_t iv_sz,
                           uint32_t i_buf, int i_sz,
                           uint32_t o_buf, int *o_sz);
    int32_t (*aead_enc)(void *self, cipher_type_e type, cipher_mode_e mode,
                        const uint8_t *key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t aad_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz);
    int32_t (*aead_dec)(void *self, cipher_type_e type, cipher_mode_e mode,
                        const uint8_t *key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t aad_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz);
    int (*cmac) (void *self,
                 cipher_type_e type, const uint8_t *key,
                 uint32_t msg, size_t msg_sz,
                 uint32_t mac, size_t *mac_sz);
    int (*hmac) (void *self, crypto_alg_hash_e type,
                 const uint8_t *key, uint32_t key_sz,
                 uint32_t msg, size_t msg_sz,
                 uint32_t mac, size_t *mac_sz);
    int32_t (*cipher_inner)(void *self, cipher_op_e op,
                            cipher_type_e type, cipher_mode_e mode,
                            bool use_key_port, const uint8_t *key, uint32_t key_sz,
                            uint32_t iv, uint32_t iv_sz,
                            uint32_t i_buf, int i_sz,
                            uint32_t o_buf, int *o_sz);
    U32 (*hash_inner)(void *self, U8 alg, bool use_keyport,
                      const U8 *key, uint32_t key_sz,
                      U32 in, U32 ilen,
                      U32 out, U32 *olen);
    U32 (*dma_copy)(void *self, U32 from, U32 to_addr, U32 len);
    U8 (*get_pke_ops_type)(void *self);
} crypto_ops_t;

typedef enum {
    CRYPTO_ENG_ANY = 0,
    CRYPTO_ENG_SW = 1,
    CRYPTO_ENG_HW = 2,
} crypto_eng_type_e;

#define FV_CRYPTO_ATTR_TYPE(v)   ((v) & 0x03u)
#define BM_CRYPTO_ATTR_LE        (0x01u << 31)

#define EC_KEY_PRIME_SZ(x)       ((x) & 0xffu)
#define ECC_SM2_CURVE            (0x01u << 24u)

typedef struct {
    char *name;
    U32 attr;   /*bit31 : 0 - big endian, 1 - LE;
                  bit1-0: 01 - SW, 10 - HW, other - invalid*/
    module_e m;
    crypto_ops_t ops;
} crypto_eng_t;

static inline U32 get_hash_sz(U8 alg)
{
    if ( ALG_HASH_SHA256 == alg || ALG_HASH_SM3 == alg) {
        return 32;
    } else if (ALG_HASH_SHA512 == alg) {
        return 64;
    } else if (ALG_HASH_SHA1 == alg) {
        return 20;
    } else if (ALG_HASH_SHA384 == alg) {
        return 48;
    } else if (ALG_HASH_SHA224 == alg) {
        return 28;
    } else {
        return 0;
    }
}

static inline U32 get_hash_block_sz(U8 alg)
{
    if ( ALG_HASH_SM3 == alg) {
        return 64;
    } else if (alg >= ALG_HASH_SHA384) {
         return 128;
    } else {
        return 64;
    }
}

void crypto_rsa_dump_key(const rsa_key_t *key);
void crypto_ecdsa_dump_key(const ecdsa_key_t *key);
const edc_curve_t *crypto_get_edc_curve(void);
const ecc_curve_t *crypto_get_ecc_curve(U32 prime_sz);
const ecc_curve_t *crypto_get_sm2_curve(uint32_t prime_sz);
bool is_sm2_curve(const ecc_curve_t *curve);
U32 crypto_get_sig_sz(U8 alg);
U32 crypto_get_rsa_pk_type(U32 n_sz);
U32 crypto_get_ecdsa_pk_type(U32 prime_sz);
void crypto_rsa_pk_dump(const rsa_pk_t *key);
void crypto_ecdsa_pk_dump(const ecdsa_pk_t *key);

U32 crypto_hash_init( void );
U32 crypto_cipher_init( void );
crypto_eng_t *crypto_find_eng(crypto_eng_type_e type);
U32 crypto_init(void);
U32 crypto_deinit(void);
U32 crypto_hash(U8 alg, U32 in, U32 ilen, U32 out, U32 *olen);
U32 crypto_hash_start(U32 context_id, U8 algo);
U32 crypto_hash_update(U32 context_id, U32 msg, U32 msg_sz);
U32 crypto_hash_finish(U32 context_id, U32 hash, U32 *hash_sz);
U32 crypto_ecdsa_sign(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 rs, U32 *rs_sz,
                      U32 key);
U32 crypto_ecdsa_verify(U8 alg,
                        U32 msg, U32 msg_sz,
                        U32 rs, U32 rs_sz,
                        U32 key);
U32 crypto_eddsa_sign(U32 msg, U32 msg_sz,
                      U32 rs, U32 *rs_sz,
                      U32 key);
U32 crypto_eddsa_verify(U32 msg, U32 msg_sz,
                      U32 rs, U32 rs_sz,
                      U32 key);
U32 crypto_rsa_sign(U8 alg,
                    U32 msg, U32 msg_sz,
                    U32 sig, U32 *sig_sz,
                    U32 key);
U32 crypto_rsa_verify(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 sig, U32 sig_sz,
                      U32 key);
U32 crypto_get_rnd(U32 rnd, U32 size);

U32 pkcs1_v1p5_unpad(const U8 *pstr, U32 pslen, /* pad string and its len */
                     U8 *str, U32 *slen);                    /* unpad string and its len*/
U32 add_hashinfo(U8 *pstr, U32 pslen,
                   const U8 *str, U32 slen, U8 alg);
U32 pkcs1_v1p5_pad(U8 *pstr, U32 pslen,        /* pad string and its len */
                   U32 slen);               /* unpad string and its len*/
#if defined(CORE_sec)
void ks_clear_all_key(void);
void soc_assert_security_violation(void);

#define FLOW_ERROR_AND_RESET    {\
    arch_disable_interrupt();\
    ks_clear_all_key();\
    soc_assert_security_violation();\
    arch_enable_interrupt();\
}
#else
#define FLOW_ERROR_AND_RESET
#endif
int str2bn(const char *str, unsigned char *bn, unsigned int *bn_sz);
int ccm_input_init(uint8_t *hdr, uint32_t M, uint32_t L, bool with_aad);
int ccm_input_setiv(uint8_t *hdr, const uint8_t *nonce, size_t nlen, size_t mlen);
void ccm_input_aad(const uint8_t *aad, size_t alen, uint8_t *out, size_t *olen);
uint32_t crypto_get_aead_hdr_sz(cipher_mode_e mode, const uint8_t *in, uint32_t i_sz,
                                uint32_t *hdr_sz);

int32_t crypto_cipher_dec(cipher_type_e type, cipher_mode_e mode,
                          uint32_t key, uint32_t key_sz,
                          uint32_t iv, uint32_t iv_sz,
                          uint32_t msg, int *msg_sz,
                          uint32_t cipher, int c_sz);
int32_t crypto_cipher_enc(cipher_type_e type, cipher_mode_e mode,
                          uintptr_t key, uint32_t key_sz,
                          uintptr_t iv, uint32_t iv_sz,
                          uintptr_t msg, int msg_sz,
                          uintptr_t cipher, int *c_sz);
U32 cipher_start (U32 context_id, cipher_type_e type, cipher_mode_e mode,
                 cipher_op_e ops, U32 key, U32 key_sz,
                 U32 iv, U32 iv_sz);
U32 cipher_update (U32 context_id, U32 in, U32 in_sz, U32 out, U32 *out_sz);
U32 cipher_finish (U32 context_id);
int32_t crypto_aead_enc(cipher_type_e type, cipher_mode_e mode,
                        uint32_t key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t aad_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz);
int32_t crypto_aead_dec(cipher_type_e type, cipher_mode_e mode,
                        uint32_t key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t aad_sz, uint32_t plain_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz
                       );
int crypto_cmac(cipher_type_e type,
                uint32_t key, uint32_t key_sz,
                uint32_t msg, size_t msg_sz,
                uint32_t mac, size_t *mac_sz);
int crypto_hmac(crypto_alg_hash_e type,
                uint32_t key, uint32_t key_sz,
                uint32_t msg, size_t msg_sz,
                uint32_t mac, size_t *mac_sz);
int crypto_wrap(const uint32_t msg, uint32_t msg_sz,
                uint32_t wrp, uint32_t *wrp_sz);
int crypto_unwrap(uint32_t msg, uint32_t *msg_sz,
                  uint32_t wrp, uint32_t wrp_sz);

uint8_t *ks_get_free_slot(uint32_t type);
int32_t ks_uninstall_key(uint32_t id);
uint8_t *ks_install_key(uint32_t id, uint32_t key);
uint8_t *ks_get_ks_by_id(uint32_t id);

U32 crypto_sm2_sign(U8 alg,
                    U32 msg, U32 msg_sz,
                    U32 rs, U32 *rs_sz,
                    U32 key);
U32 crypto_sm2_verify(U8 alg,
                      U32 msg, U32 msg_sz,
                      U32 rs, U32 rs_sz,
                      U32 key);

void dump_wrap(crypto_wrapper_t *wrp);

#define SYM_KEY_NUM     32
#define RSA_KEY_NUM     8
#define EDDSA_KEY_NUM   8
#define ECDSA_KEY_NUM   16

#define COMMON_BUF_SIZE 1024

#endif  /* __CRYPTO_H__ */
