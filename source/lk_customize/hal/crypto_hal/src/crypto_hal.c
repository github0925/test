//*****************************************************************************
//
// crypto_hal.c - the crypto hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <res.h>
#include <crypto_hal.h>
#include <trace.h>
#include <lk/init.h>

#define LOCAL_TRACE 0 //close local trace 1->0

#define VCE_NUM_MAX 9 //all ce on platform

#define VCE_NUM_SUPPORT_PKA_MAX 5 //all ce support pka,
#define RES_CE_MEM_CE1_VCE1 0x4010001 //used only in safety domain

//global instance
//static crypto_instance_t g_cryptoinstance[CE_MEM_RES_NUM] = {0};
static crypto_instance_t g_cryptoinstance[VCE_NUM_MAX] = {0};
/*
static domain_res_t g_crypto_all_resource = {
	.res_category = "secure_ce_mem",
	.res_info[0].res_glb_idx = RES_CE_MEM_CE2_VCE1,
	.res_info[0].res_describe = "",
	.res_info[0].phy_addr = 0x520000,
	.res_info[0].addr_range = 0x2000,
    .res_info[1].res_glb_idx = RES_CE_MEM_CE2_VCE2,
    .res_info[1].res_describe = "vce2",
    .res_info[1].phy_addr = 0x522000,
    .res_info[1].addr_range = 0x2000,
    .res_info[2].res_glb_idx = RES_CE_MEM_CE2_VCE3,
    .res_info[2].res_describe = "vce3",
    .res_info[2].phy_addr = 0x524000,
    .res_info[2].addr_range = 0x2000,
    .res_info[3].res_glb_idx = RES_CE_MEM_CE2_VCE4,
    .res_info[3].res_describe = "vce4",
    .res_info[3].phy_addr = 0x526000,
    .res_info[3].addr_range = 0x2000,
	.res_info[4].res_glb_idx = RES_CE_MEM_CE2_VCE5,
	.res_info[4].res_describe = "",
	.res_info[4].phy_addr = 0x528000,
	.res_info[4].addr_range = 0x2000,
	.res_info[5].res_glb_idx = RES_CE_MEM_CE2_VCE6,
	.res_info[5].res_describe = "",
	.res_info[5].phy_addr = 0x52A000,
	.res_info[5].addr_range = 0x2000,
	.res_info[6].res_glb_idx = RES_CE_MEM_CE2_VCE7,
	.res_info[6].res_describe = "",
	.res_info[6].phy_addr = 0x52C000,
	.res_info[6].addr_range = 0x2000,
	.res_info[7].res_glb_idx = RES_CE_MEM_CE2_VCE8,
	.res_info[7].res_describe = "",
	.res_info[7].phy_addr = 0x52E000,
	.res_info[7].addr_range = 0x2000,
	.res_num = CE_MEM_RES_NUM,
};
*/
//resource info define in domain_res.h g_ce_mem_res
//#define g_crypto_resource g_ce_mem_res

//ce resource index map to vce id
static uint32_t residx2ceid[VCE_NUM_MAX][2] = {
    {0, RES_CE_MEM_CE2_VCE1},
    {1, RES_CE_MEM_CE2_VCE2},
    {2, RES_CE_MEM_CE2_VCE3},
    {3, RES_CE_MEM_CE2_VCE4},
    {4, RES_CE_MEM_CE2_VCE5},
    {5, RES_CE_MEM_CE2_VCE6},
    {6, RES_CE_MEM_CE2_VCE7},
    {7, RES_CE_MEM_CE2_VCE8},
    {0, RES_CE_MEM_CE1},
};

//support pka ce index
static uint32_t support_pka_residx[VCE_NUM_SUPPORT_PKA_MAX] = {
    RES_CE_MEM_CE2_VCE1,
    RES_CE_MEM_CE2_VCE2,
    RES_CE_MEM_CE2_VCE3,
    RES_CE_MEM_CE2_VCE4,
    RES_CE_MEM_CE1,
};

//rsa drive default method
const struct rsa_meth_st RSA_default_method = {
    {
        0 /* references */,
        1 /* is_static */,
    },
    NULL /* app_data */,

    rsa_signature_generation_blk /* sign */,
    rsa_signature_verification_blk /* verify */,

    rsa_encrypt_blk /* encrypt (defaults to rsa_default_encrypt) */,
    rsa_decrypt_blk /* decrypt (defaults to rsa_default_decrypt) */,

    rsa_private_key_generation_blk /* keygen (defaults to rsa_default_keygen) */,
    rsa_crt_key_generation_blk /* multi_prime_keygen (defaults to rsa_default_multi_prime_keygen) */,
    rsa_key_generation_blk /* keygen_ex, it will generate prime in function */,
};

//aes drive default method
const struct aes_meth_st AES_default_method = {
    aes_blk /* cipher */,
};

//hash drive default method
const struct hash_meth_st HASH_default_method = {
    hash_update_blk /* update */,
    hash_finish_blk /* finish */,
    hash_blk /* hash */,
    hmac_blk /* hmac */,
};

//rng drive default method
const struct rng_meth_st RNG_default_method = {
    rng_get_trng /* trng */,
    rng_get_prng /* prng */,
};

//ecc drive default method
const struct ec_meth_st EC_default_method = {
    ecc_generate_keypair /* gen key */,
    ecc_generate_private_key /* gen pri key */,
    ecc_generate_public_key,
    sm2_generate_signature,
    sm2_verify_signature,
    sm2_key_exchange,
    sm2_generate_digest,
    ecdsa_generate_signature,
    ecdsa_verify_signature,
};

//dsa drive default method
const struct dsa_meth_st DSA_default_method = {
    dsa_generate_private_key /* gen pri key */,
    dsa_generate_public_key,
    dsa_generate_signature,
    dsa_verify_signature,
};

int hal_crypto_residx2ceid(uint32_t res_idx)
{
    int i = 0;
    int ret = -1;

    for (i = 0; i < VCE_NUM_MAX; i++) {
        if (residx2ceid[i][1] == res_idx) {
            ret = residx2ceid[i][0];
            break;
        }
    }

    return ret;
}

int hal_crypto_support_pka(uint32_t res_idx)
{
    int i = 0;
    int ret = -1;

    for (i = 0; i < VCE_NUM_SUPPORT_PKA_MAX; i++) {

        if (res_idx == support_pka_residx[i]) {
            ret = 1;
            break;
        }
    }

    return ret;
}
//*****************************************************************************
//
//! .hal_crypto_get_instance
//!
//! @param crypto_idx input resource_id define in chipcfg/domain_res.h
//!
//! This function get  instance hand.
//! @return hanle crypto_instance_t
//
//*****************************************************************************
static crypto_instance_t* hal_crypto_get_instance(uint32_t crypto_idx)
{
    int vce_id = -1;
    int i;
    int ret = 0;
    int is_support_pka = 0;
    paddr_t phy_addr = 0;
    int32_t index = 0;

    ret = res_get_info_by_id(crypto_idx,&phy_addr,&index);

    LTRACEF("ret = %d\n", ret);

    if(ret == -1){
        //can not find res in current domain res
        return NULL;
    }

    vce_id = hal_crypto_residx2ceid(crypto_idx);

    LTRACEF("vce_id = %d\n", vce_id);

    if (vce_id == -1) {
        return NULL;
    }
    i = vce_id;

    if (g_cryptoinstance[i].is_created == 0) {

        g_cryptoinstance[i].vce_in_used = 1;
        g_cryptoinstance[i].vce_id = vce_id;
        //vceid == 0/1/2/3 can support pka , should set rsa ec dsa method
        is_support_pka = hal_crypto_support_pka(crypto_idx);
        if(is_support_pka == 1){
            g_cryptoinstance[i].rsa_method = (RSA_METHOD*)&RSA_default_method;
            g_cryptoinstance[i].ec_method = (EC_METHOD*)&EC_default_method;
            g_cryptoinstance[i].dsa_method = (DSA_METHOD*)&DSA_default_method;
        }
        g_cryptoinstance[i].aes_method = (AES_METHOD*)&AES_default_method;
        g_cryptoinstance[i].hash_method = (HASH_METHOD*)&HASH_default_method;
        g_cryptoinstance[i].rng_method = (RNG_METHOD*)&RNG_default_method;

        ce_init(vce_id);
        g_cryptoinstance[i].is_created = 1;
        return &g_cryptoinstance[i];

    }
    else {

        if (g_cryptoinstance[i].vce_in_used == 0) {
            g_cryptoinstance[i].vce_in_used = 1;
            return &g_cryptoinstance[i];
        }

    }
/*
    for (i = 0; i < g_crypto_resource.res_num; i++) {

        if (g_crypto_resource.res_info[i].res_glb_idx == crypto_idx) {

            if (g_cryptoinstance[i].is_created == 0) {

                g_cryptoinstance[i].vce_in_used = 1;
                g_cryptoinstance[i].vce_id = vce_id;
                //vceid == 0/1/2/3 can support pka , should set rsa ec dsa method
                g_cryptoinstance[i].rsa_method = (RSA_METHOD*)&RSA_default_method;
                g_cryptoinstance[i].aes_method = (AES_METHOD*)&AES_default_method;
                g_cryptoinstance[i].hash_method = (HASH_METHOD*)&HASH_default_method;
                g_cryptoinstance[i].rng_method = (RNG_METHOD*)&RNG_default_method;
                g_cryptoinstance[i].ec_method = (EC_METHOD*)&EC_default_method;
                g_cryptoinstance[i].dsa_method = (DSA_METHOD*)&DSA_default_method;
                return &g_cryptoinstance[i];

            }
            else {

                if (g_cryptoinstance[i].vce_in_used == 0) {
                    g_cryptoinstance[i].vce_in_used = 1;
                    return &g_cryptoinstance[i];
                }

            }

            break;
        }
    }
*/
    LTRACEF("return NULL\n");
    return NULL;
}
//*****************************************************************************
//
//! hal_crypto_release_instance.
//!
//! @param cryptoinstance crypto_instance_t
//!
//! This function release instance handle.
//! @return void
//
//*****************************************************************************
static void hal_crypto_release_instance(crypto_instance_t* cryptoinstance)
{
    if (cryptoinstance != NULL) {
        cryptoinstance->vce_in_used = 0;
    }
}

//*****************************************************************************
//
//! hal_crypto_creat_handle.
//!
//! @param handle output crypto handle,
//! @param crypto_idx input resource_id define in chipcfg...domain_res.h
//!
//! This function get ce handle.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_creat_handle(void** handle, uint32_t crypto_idx)
{
    crypto_instance_t*  cryptoinstance = NULL;

    cryptoinstance = hal_crypto_get_instance(crypto_idx);

    if (cryptoinstance == NULL) {
        return CRYPTO_HAL_ERROR;
    }

    *handle = cryptoinstance;
    return CRYPTO_HAL_OK;
}

//*****************************************************************************
//
//! hal_crypto_delete_handle.
//!
//! @param handle input
//!
//! This function delete  instance hand.
//!
//! @return crypto_hal_status_t
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_delete_handle(void* handle)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_HAL_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;
        l_cryptoinstance->vce_in_used = 0;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_crypto_init.
//!
//! This function is for init ce
//!
//! @return  status
//
//*****************************************************************************
void hal_crypto_init(uint level)
{

    ce_globle_init();

    return ;
}

//*****************************************************************************
//
//! hal_crypto_deinit.
//!
//! @param handle input
//!
//! This function is for  deinit.
//!
//! @return crypto_hal_status_t status
//
//*****************************************************************************
crypto_hal_status_t hal_crypto_deinit(void* handle)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    crypto_instance_t* l_cryptoinstance = NULL;

    if (handle == NULL) {
        ret = CRYPTO_HAL_ERROR;
    }
    else {
        l_cryptoinstance = (crypto_instance_t*)handle;

        /*do ce deinit here*/

        l_cryptoinstance->is_init = 0;
    }

    return ret;
}

//*****************************************************************************
//
//! hal_trng_init.
//!
//! @param handle input, ce handle
//! @param crypto_cfg input, ce config for init
//! This function is for init trng
//!
//! @return  status
//
//*****************************************************************************
crypto_hal_status_t hal_trng_init(void* handle, crypto_config_t* crypto_cfg)
{
    crypto_hal_status_t ret = CRYPTO_HAL_OK;

    ret = trng_init(0);

    return ret;
}

LK_INIT_HOOK(cryto_init, hal_crypto_init, LK_INIT_LEVEL_TARGET);
