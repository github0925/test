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

extern uint32_t ce_trng_init(void *self);
extern uint8_t context_buffer_status[MAX_SA_SLOT_NUMBER];

#if defined(CE_DEBUG)
static void ce_get_dump(void *base)
{
    cryptoengine_t *ce = (cryptoengine_t *)base;

    DBG("ce->ce0_stat = 0x%x\n", ce->ce0_stat);
    DBG("ce->ce0_errstat = 0x%x\n", ce->ce0_errstat);
    DBG("ce->ce0_pk_hwconfigreg = 0x%x\n", ce->ce0_pk_hwconfigreg);
}
#endif

/*if used in ssystem, it does not need config hardware.*/
#if !defined(CFG_SSYSTEM_LOAD)
static U32 ce_sram_cfg(void *base)
{
    cryptoengine_t *ce = (cryptoengine_t *)base;

    ce->sesram_size = FV_SESRAM_SIZE_CE0_RAM_SIZE(8)
                      | FV_SESRAM_SIZE_CE1_RAM_SIZE(8)
                      | FV_SESRAM_SIZE_CE2_RAM_SIZE(8)
                      | FV_SESRAM_SIZE_CE3_RAM_SIZE(8);
    ce->sesram_size_cont = FV_SESRAM_SIZE_CONT_CE4_RAM_SIZE(8)
                           | FV_SESRAM_SIZE_CONT_CE5_RAM_SIZE(8)
                           | FV_SESRAM_SIZE_CONT_CE6_RAM_SIZE(8)
                           | FV_SESRAM_SIZE_CONT_CE7_RAM_SIZE(8);
    ce->sesram_sasize = FV_SESRAM_SASIZE_CE0_RAM_SASIZE(4)
                        | FV_SESRAM_SASIZE_CE1_RAM_SASIZE(4)
                        | FV_SESRAM_SASIZE_CE2_RAM_SASIZE(4)
                        | FV_SESRAM_SASIZE_CE3_RAM_SASIZE(4);
    ce->sesram_sasize_cont = FV_SESRAM_SASIZE_CONT_CE4_RAM_SASIZE(4)
                             | FV_SESRAM_SASIZE_CONT_CE5_RAM_SASIZE(4)
                             | FV_SESRAM_SASIZE_CONT_CE6_RAM_SASIZE(4)
                             | FV_SESRAM_SASIZE_CONT_CE7_RAM_SASIZE(4);

    return 0;
}
#endif

static U32 ce_register_reset(void *base)
{
    cryptoengine_t *ce = (cryptoengine_t *)base;

    ce->ce0_inten = 0x00;

    return 0;
}

U32 ce_init(void *self)
{
    void *base = (void *)Mcu_GetModuleBase(((crypto_eng_t *)self)->m);

    for (int i = 0; i < MAX_SA_SLOT_NUMBER; i++) {
        context_buffer_status[i] = 0x0u;
    }

    /*hash and cipher contexts initlize*/
    crypto_hash_init();
    crypto_cipher_init();

    ce_register_reset(base);
    #if !defined(CFG_SSYSTEM_LOAD)
    ce_sram_cfg(base);
    #endif

    //ce_trng_init(self);

    return 0;
}

U32 ce_deinit(void *self)
{
    DBG("%s called. Nothing to do.\n", __FUNCTION__);

    return 0;
}

BOOL ce_is_disabled(void *self)
{
    /*
    module_e m = ((crypto_eng_t *)self)->m;

    return soc_is_ce_disabled(m);
    */
    return false;
}

#if !defined(CRYPTO_ENG)
#define CRYPTO_ENG  CRYPTO_ENG1
#endif

const crypto_eng_t eng_hw_ce = {
    "hw_ce",  /* name */
    BM_CRYPTO_ATTR_LE | CRYPTO_ENG_HW,      /* type */
    CRYPTO_ENG,        /* module */
    {
        .init = ce_init,
        .deinit = ce_deinit,
        .hash = ce_hash,
        .hash_write_back = ce_hash_write_back,
        .hash_update = ce_hash_update,
        .hash_finish = ce_hash_finish,
#if defined(CFG_CE_API_RSA_ENCRYPT)
        .rsa_encrypt = ce_rsa_encrypt,
#else
        .rsa_encrypt = NULL,
#endif
#if defined(CFG_CE_API_RSA_DECRYPT)
        .rsa_decrypt = ce_rsa_decrypt,
#else
        .rsa_decrypt = NULL,
#endif
#if 0
        .rsa_sign = ce_rsa_sign,
#endif
        .rsa_verify = ce_rsa_verify,
#if 0
        .ecdsa_sign = ce_ecdsa_sign,   /* ecdsa_sign */
#endif
        .ecdsa_verify = ce_ecdsa_verify,   /* ecdsa_verify */
#if 0
        .eddsa_sign = ce_eddsa_sign,
        .eddsa_verify = ce_eddsa_verify,
#endif
        .is_disabled = NULL,
#if 0
        .get_rnd = ce_trng_rd_rnd,
        .cipher_enc = ce_cipher_enc,
        .cipher_dec = ce_cipher_dec,
        .cipher_start = ce_cipher_start,
        .cipher_update = ce_cipher_update,
        .cipher_finish = ce_cipher_finish,
        .aead_enc = ce_aead_enc,
        .aead_dec = ce_aead_dec,
        .cmac = ce_cmac,
        .hmac = ce_hmac,
#endif
        .cipher_inner = ce_cipher_inner,
        .hash_inner = ce_hash_inner,
        .dma_copy = ce_dma_mem_copy,
        .get_pke_ops_type = ce_get_pke_ops_type,
    },
};
