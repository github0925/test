/********************************************************
 *          Copyright(c); 2018  Semidrive               *
 ********************************************************/

#ifndef __CRYPTOENGINE_H__
#define __CRYPTOENGINE_H__

#include <common_hdr.h>
#include <atb_crypto.h>
#include <soc.h>

#define Mcu_GetModuleBase(m) soc_get_module_base(m)

U32 ce_init(void *self);
U8 ce_get_pke_ops_type(void *self);
U32 ce_hrng_rd_rnd(void *self, U8 *rnd, U32 len);
U32 ce_hash(void *self, U8 alg, U32 in, U32 ilen, U32 out, U32 *olen);
U32 ce_hash_write_back(void *self, ce_hash_context *context);
U32 ce_hash_update(void *self, ce_hash_context *context, U32 msg, U32 msg_sz);
U32 ce_hash_finish(void *self, ce_hash_context *context, U32 hash, U32 *hash_sz_addr);

U32 ce_rsa_encrypt(void *self,
                   U32 m, U32 m_sz,
                   U32 c, U32 *c_sz,
                   const rsa_key_t *key);
U32 ce_rsa_decrypt(void *self,
                   U32 m, U32 *m_sz,
                   U32 c, U32 c_sz,
                   const rsa_key_t *key);
U32 ce_rsa_sign(void *self,
                const U8 *hash, U32 hash_sz,
                U32 sig, U32 *sig_sz,
                const rsa_key_t *key);
U32 ce_rsa_verify(void *self,
                  const U8 *hash, U32 hash_sz,
                  U32 sig, U32 sig_sz,
                  const rsa_key_t *key);
U32 ce_ecdsa_sign(void *self,
                  const U8 *hash, U32 hash_sz,
                  U32 rs, U32 *rs_sz,
                  const ecc_curve_t *curve,
                  const ecdsa_key_t *key);
U32 ce_ecdsa_verify(void *self,
                    const U8 *hash, U32 hash_sz,
                    U32 rs, U32 rs_sz,
                    const ecc_curve_t *curve,
                    const ecdsa_key_t *key);
U32 ce_eddsa_sign(void *self,
                  const U8 *msg, U32 msg_sz,
                  U32 rs, U32 *rs_sz,
                  const edc_curve_t *curve,
                  const eddsa_key_t *key);
U32 ce_eddsa_verify(void *self,
                    const U8 *msg, U32 msg_sz,
                    U32 rs, U32 rs_sz,
                    const edc_curve_t *curve,
                    const eddsa_key_t *key);
int32_t ce_cipher_enc(void *self,
                      cipher_type_e type, cipher_mode_e mode,
                      const uint8_t *key, uint32_t key_sz,
                      uintptr_t iv, uint32_t iv_sz,
                      uintptr_t i_buf, int i_sz,
                      uintptr_t o_buf, int *o_sz);
int32_t ce_cipher_dec(void *self,
                      cipher_type_e type, cipher_mode_e mode,
                      const uint8_t *key, uint32_t key_sz,
                      uint32_t iv, uint32_t iv_sz,
                      uint32_t i_buf, int i_sz,
                      uint32_t o_buf, int *o_sz);
U32 ce_cipher_start (void *self, ce_cipher_context *context,
                          const U8 *key, U32 key_sz, U32 iv, U32 iv_sz);
U32 ce_cipher_update (void *self, ce_cipher_context *context,
                      U32 i_buf, U32 i_sz, U32 o_buf, U32 *o_sz);
U32 ce_cipher_finish (void *self, ce_cipher_context *context,
                      U32 o_buf, U32 *o_sz);
int32_t ce_aead_enc(void *self, cipher_type_e type, cipher_mode_e mode,
                           const uint8_t *key, uint32_t key_sz,
                           uint32_t iv, uint32_t iv_sz,
                           uint32_t aad_sz, uint32_t plain_sz,
                           uint32_t i_buf, int i_sz,
                           uint32_t o_buf, int *o_sz);
int32_t ce_aead_dec(void *self, cipher_type_e type, cipher_mode_e mode,
                           const uint8_t *key, uint32_t key_sz,
                           uint32_t iv, uint32_t iv_sz,
                           uint32_t aad_sz, uint32_t plain_sz,
                           uint32_t i_buf, int i_sz,
                           uint32_t o_buf, int *o_sz
                       );
int ce_cmac(void *self, cipher_type_e type, const uint8_t *key,
            uint32_t msg, size_t msg_sz,
            uint32_t mac, size_t *mac_sz);
int ce_hmac(void *self, crypto_alg_hash_e type,
            const uint8_t *key, uint32_t key_sz,
            uint32_t msg, size_t msg_sz,
            uint32_t mac, size_t *mac_sz);
U32 ce_trng_rd_rnd(void *self, U32 rnd, U32 len);

int32_t ce_cipher_inner(void *self, cipher_op_e op,
                        cipher_type_e type, cipher_mode_e mode,
                        bool use_key_port, const uint8_t *key, uint32_t key_sz,
                        uint32_t iv, uint32_t iv_sz,
                        uint32_t i_buf, int i_sz,
                        uint32_t o_buf, int *o_sz);
U32 ce_hash_inner(void *self, U8 alg, bool use_keyport,
                  const U8 *key, uint32_t key_sz,
                  U32 in, U32 ilen,
                  U32 out, U32 *olen);
U32 ce_dma_mem_copy(void *self, U32 from, U32 to_addr, U32 len);
/* Keys, IV and CTX in secure area */
#define SA_SLOT_SZ      (0x20)
#define SA_SLOT_OFF(n)  ((n) * SA_SLOT_SZ)
#define SLOT_OFF(n)     ((n) * SA_SLOT_SZ)
#define VCE0_SA_BASE (CE_SMEM_BASE + 0x1000u)
#define SA_SLOT_ADDR(n) (VCE0_SA_BASE + SA_SLOT_OFF(n))
#define PA_SLOT_ADDR(n)    (CE_SMEM_BASE + SA_SLOT_OFF(n))

#define FIXED_SA_OFF            (0x800)
#define FIXED_SA_SLOT_ADDR(n)   (VCE0_SA_BASE + FIXED_SA_OFF + SA_SLOT_OFF(n))

#define FIXED_PA_HASH_DST       (PA_SLOT_ADDR(0))
#define FIXED_PA_HASH_DST_OFF   (0)
/* Start from here */
#define FIXED_SA_HASH_KEY       (0)
#define FIXED_SA_HASH_KEY_OFF   (SA_SLOT_OFF(127))      /*32B*/



#endif
