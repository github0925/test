/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/
#ifndef __CRYPTOENGINE_REG_FLD_DEF_H__
#define __CRYPTOENGINE_REG_FLD_DEF_H__

#include <atb_crypto.h>

typedef enum {
    MD5,
    SHA1,
    SHA224,
    SHA256,
    SHA384,
    SHA512,
    SM3,
} ce_hash_alg_bit_e;

typedef enum {
    EXT_MEMORY,         /* external memory */
    CE_SRAM_PUB,        /* CE sram public area */
    CE_SRAM_SEC,         /* CE sram secure area */
} hash_src_mem_type_e;

typedef enum {
    DMA_MEM_EXT,
    DMA_MEM_SRAM_PA = 1,   /* Secrue RAM public area */
    DMA_MEM_PKEMEM = 2,    /* PKE Operand memories */
    DMA_MEM_SRAM_SA = 3,   /* Secure RAM secure area */
    DMA_MEM_KEY = 4,       /* Input Key */
} ce_dma_mem_type_e;

typedef enum {
    CURVE_P256 = 1,
    CURVE_P384,
    CURVE_P512,
    CURVE_P192,
    CURVE_25519,
} ce_pke_curve_e;

typedef enum {
    MOD_NOP = 0,
    MOD_ADD = 1,
    MOD_SUB = 2,
    MOD_MUL_ODD = 3,
    MOD_REDUCT_ODD = 4,
    MOD_DIV_ODD = 5,
    MOD_INV_ODD = 6,
    MOD_SQRT_GFP = 7,   /* square root */
    MULTIPLE = 8,
    MOD_INV_EVEN_GFP = 9,
    MOD_REDUCT_ODD_GFP = 10,
    CLR_MEM = 15,
    MOD_EXP = 16,

    RSA_PRIV_GEN = 0x11,
    RSA_CRT_KPRAR_GEN = 0x12,
    RSA_CRT_DECRYPT = 0x13,
    RSA_ENCRYPT = 0x14,
    RSA_DECRYPT = 0x15,
    RSA_SIGN_GEN = 0x16,
    RSA_SIGN_VERIFY = 0x17,

    SM2_SIGN = 0x2D,
    SM2_VERIFY = 0x2E,
    SM2_KEYEX = 0x2F,

    ECDSA_SIGN_GEN = 0x30,
    ECDSA_SIGN_VERIFY = 0x31,

    EDDSA_POINT_MULTIPLICATION = 0x3b,
    EDDSA_SIGN_GEN  = 0x3c,
    EDDSA_SIGN_VERIFY = 0x3d
} ce_pke_op_e;

typedef enum {
    RSA_MODULUS_SLOT = 0,
    RSA_CIPHER_SLOT = 4,
    RSA_PLAIN_SLOT = 5,
    RSA_PRIVK_SLOT = 6,
    RSA_PUBK_SLOT = 8,
    RSA_SIGN_SLOT = 0x0B,
    RSA_HASH_SLOT = 0x0C,
} ce_rsa_op_slot_e;

typedef enum {
    ECDSA_p_SLOT = 0,
    ECDSA_n_SLOT = 1,
    ECDSA_xg_SLOT = 2,
    ECDSA_yg_SLOT = 3,
    ECDSA_a_SLOT = 4,
    ECDSA_b_SLOT = 5,
    ECDSA_d_SLOT = 6,
    ECDSA_k_SLOT = 7,
    ECDSA_x_SLOT = 8,
    ECDSA_y_SLOT = 9,
    ECDSA_r_SLOT = 10,
    ECDSA_s_SLOT = 11,
    ECDSA_hash_SLOT = 12,
} ce_ecdsa_op_slot_e;

typedef enum {
    EdDSA_P_SLOT = 0,
    EdDSA_L_SLOT = 1,
    EdDSA_Bx_SLOT = 2,
    EdDSA_By_SLOT = 3,
    EdDSA_d_SLOT = 4,
    EdDSA_I_SLOT = 5,
    EdDSA_Klsb_SLOT = 6,
    EdDSA_Kmsb_SLOT = 7,
    EdDSA_Rlsb_SLOT = 8,    //Ax
    EdDSA_Rmsb_SLOT = 9,    //Ay
    EdDSA_Rx_SLOT = 0xa,    //S
    EdDSA_Ry_SLOT = 0xb,
} ce_eddsa_op_slot_e;

#define PKE_MEM_SLOT_SZ     (0x200U)
#define PKE_MEM_SLOT_ADDR(slot) (PKE_MEM_SLOT_SZ * (slot))

#define SEL_CURVE_NO_ACC    0
#define SEL_CURVE_P256      1
#define SEL_CURVE_P384      2
#define SEL_CURVE_P521      3
#define SEL_CURVE_P192      4
#define SEL_CURVE_25519     5

static inline U32 ce_get_selcurve_v(U32 prime_sz)
{
    U32 v = ( prime_sz == 24 ?  SEL_CURVE_P192 :
              prime_sz == 32 ?  SEL_CURVE_P256 :
              prime_sz == 48 ?  SEL_CURVE_P384 :
              prime_sz == 66 ?  SEL_CURVE_P521 :
              SEL_CURVE_NO_ACC );
    return v;
}

typedef enum {
    CE_ECB = 0,
    CE_CBC = 1,
    CE_CTR = 2,
    CE_CFB = 3,
    CE_OFB = 4,
    CE_CCM = 5,
    CE_GCM = 6,
    CE_XTS = 7,
    CE_CMAC = 8,
    CE_AES_MODE_MAX,
} ce_aes_mode_e;

typedef enum {
    CE_KEY_SZ_128 = 0,
    CE_KEY_SZ_256 = 1,
    CE_KEY_SZ_192 = 2,
} ce_key_sz_e;

typedef enum {
    CE_OP_ENC = 0,
    CE_OP_DEC = 1,
} ce_aes_op_e;

typedef enum {
    CE_KEYTYPE_SMEM_PUB,
    CE_KEYTYPE_SMEM_PRIV,
    CE_KEYTYPE_KEYPORT,
} ce_keytype_e;

#endif  /* __CRYPTOENGINE_REG_FLD_DEF_H__ */
