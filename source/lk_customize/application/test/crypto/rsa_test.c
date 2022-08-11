/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <err.h>
#include <string.h>
#include <malloc.h>

#include <app.h>
#include <lib/console.h>
#include <platform.h>

#include <sd_rsa.h>
#include <trace.h>

#include "ce_test.h"
#include "rsa_data.h"
#include <sx_rsa.h>

#define LOCAL_TRACE 0 //close local trace 1->0

static uint64_t total_time = 0;
static int rsa_test_fail = 0;

int memcmp_reverse(uint8_t* src, const uint8_t* dst, uint32_t len)
{
    const unsigned char* su1, *su2;
    int res = 0;

    for (su1 = src + len - 1, su2 = dst; 0 < len; --su1, ++su2, len--)
        if ((res = *su1 - *su2) != 0) {
            LTRACEF("memcmp_reverse: error at:%d byte, src=%d ,dst=%d ,src at %p!\n", len, *su1, *su2, src);
            break;
        }

    return res;
}

uint32_t rsa_enc(uint32_t vce_id, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* n, uint8_t* pub_expo, uint8_t* msg, uint8_t* except_s)
{
    uint32_t res;
    rsa_pubkey_t rsa_pubkey_temp;
    uint8_t __attribute__((aligned(CACHE_LINE))) result32[RSA_4096_LEN];
    uint32_t result_len = keysize;
    uint64_t cur_time;
    void* crypto_handle;

    rsa_pubkey_temp.n = n;
    rsa_pubkey_temp.n_len = keysize;
    rsa_pubkey_temp.e = pub_expo;
    rsa_pubkey_temp.e_len = keysize;

    LTRACEF("rsa encryption enter \n");

    cur_time = current_time_hires();

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    res = hal_rsa_encrypt(crypto_handle, &rsa_pubkey_temp, msg, keysize, result32, result_len, SD_RSA_PADDING_NONE);
    hal_crypto_delete_handle(crypto_handle);

    total_time += current_time_hires() - cur_time;

    if (res) {
        LTRACEF("rsa_encrypt_blk for %d, result: %d\n\n", keysize, res);
        return res;
    }

    res = memcmp_reverse(/*(void *)(sram_base + RSA_SRAM_DST_OFFSET)*/result32, except_s, keysize);

    if (res) {
        LTRACEF("rsa_encrypt_blk compare result: %d\n\n", res);
        ce_printf_binary("rsa encrypt result", /*(void *)(sram_base + RSA_SRAM_DST_OFFSET)*/result32, keysize);
        ce_printf_binary("rsa encrypt excepted_result", except_s, keysize);
        rsa_test_fail++;
    }
    else {
        LTRACEF("rsa encryption pass \n");
    }

    return res;
}

uint32_t rsa_enc_trav(uint32_t vce_id)
{
    uint32_t ret;
    uint32_t status;

    LTRACEF("-----rsa enc DDR base test begin------\n");
    status = rsa_enc(vce_id, RSA_1024_LEN, HAL_EXT_MEM, n_1024, public_expo_1024, msg_1024, cipher_1024);
    status |= rsa_enc(vce_id, RSA_2048_LEN, HAL_EXT_MEM, n_2048, public_expo_2048, msg_2048, cipher_2048);
    status |= rsa_enc(vce_id, RSA_3072_LEN, HAL_EXT_MEM, n_3072, public_expo_3072, msg_3072, cipher_3072);
    status |= rsa_enc(vce_id, RSA_4096_LEN, HAL_EXT_MEM, n_4096, public_expo_4096, msg_4096, cipher_4096);

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}

uint32_t rsa_dec(uint32_t vce_id, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* n, uint8_t* priv_key, uint8_t* cipher, uint8_t* except_s)
{
    uint8_t __attribute__((aligned(CACHE_LINE))) result[RSA_4096_LEN];
    uint32_t result_len = 0;
    uint32_t res;
    uint64_t cur_time;
    rsa_keypair_t pri_key_temp;
    void* crypto_handle;

    LTRACEF("rsa decrypt enter \n");

    pri_key_temp.n = n;
    pri_key_temp.n_len = keysize;
    pri_key_temp.d = priv_key;
    pri_key_temp.d_len = keysize;

    cur_time = current_time_hires();
    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    res = hal_rsa_decrypt(crypto_handle, &pri_key_temp, result_len, result, keysize,
                      cipher, keysize, SD_RSA_PADDING_NONE);
    hal_crypto_delete_handle(crypto_handle);
    total_time += current_time_hires() - cur_time;

    if (res) {
        LTRACEF("rsa_decrypt_blk for %d, result: %d\n\n", keysize, res);
        return res;
    }

    res = memcmp_reverse(result, except_s, keysize);

    if (res) {
        LTRACEF("rsa_decrypt_blk compare result: %d\n\n", res);
        ce_printf_binary("rsa decrypt result", result, keysize);
        rsa_test_fail++;
    }
    else {
        LTRACEF("rsa decryption test pass \n");
    }

    return res;
}

uint32_t rsa_dec_trav(uint32_t vce_id)
{
    uint32_t ret;
    uint32_t status;

    LTRACEF("----rsa dec-DDR base test-enter-----\n");
    status = rsa_dec(vce_id, RSA_1024_LEN, HAL_EXT_MEM, n_1024, private_key_1024 + RSA_1024_LEN, cipher_1024, msg_1024);
    status |= rsa_dec(vce_id, RSA_2048_LEN, HAL_EXT_MEM, n_2048, private_key_2048, cipher_2048, msg_2048);
    status |= rsa_dec(vce_id, RSA_3072_LEN, HAL_EXT_MEM, n_3072, private_key_3072, cipher_3072, msg_3072);
    status |= rsa_dec(vce_id, RSA_4096_LEN, HAL_EXT_MEM, n_4096, private_key_4096, cipher_4096, msg_4096);

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}

uint8_t __attribute__((aligned(CACHE_LINE))) sig_gen_temp_buf[RSA_4096_LEN];
//signature generation
uint32_t sig_gen(uint32_t vce_id, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* n, uint8_t* priv_key, uint8_t* msg, uint8_t* except_s)
{
    uint32_t res;
    uint64_t cur_time;
    uint8_t __attribute__((aligned(CACHE_LINE))) temp_buf[RSA_4096_LEN];
    uint32_t temp_buf_len = 0;
    void* crypto_handle;
    rsa_keypair_t pri_key_temp;

    pri_key_temp.n = n;
    pri_key_temp.n_len = keysize;
    pri_key_temp.d = priv_key;
    pri_key_temp.d_len = keysize;

    memset(temp_buf, 0, RSA_4096_LEN);

    LTRACEF("rsa sig_gen enter\n");
    //ce_printf_binary("sig_gen temp_buf = ", temp_buf, keysize);

    cur_time = current_time_hires();

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    res = hal_rsa_sign(crypto_handle, 0, msg, keysize, sig_gen_temp_buf,
                   temp_buf_len, &pri_key_temp, SD_RSA_PADDING_NONE, 0);
    hal_crypto_delete_handle(crypto_handle);
    total_time += current_time_hires() - cur_time;

    res = memcmp_reverse(except_s, sig_gen_temp_buf, keysize);

    if (res) {
        LTRACEF("signature gen for %d result error, compare result: %d\n\n", keysize, res);
        //ce_printf_binary("signature gen result", (void *)(sram_base + RSA_SRAM_DST_OFFSET), keysize);
        ce_printf_binary("signature gen result", temp_buf, keysize);
        rsa_test_fail++;
    }
    else {
        LTRACEF("rsa signature gen pass \n");
    }

    return res;
}

uint32_t rsa_sig_gen_trav(uint32_t vce_id)
{
    uint32_t ret;
    uint32_t status;

    LTRACEF("----rsa sig-DDR base test------\n");
    status = sig_gen(vce_id, RSA_1024_LEN, HAL_EXT_MEM, n_1024, private_key_1024 + RSA_1024_LEN, msg_1024, sig_1024);
    status |= sig_gen(vce_id, RSA_2048_LEN, HAL_EXT_MEM, n_2048, private_key_2048, msg_2048, sig_2048);
    status |= sig_gen(vce_id, RSA_3072_LEN, HAL_EXT_MEM, n_3072, private_key_3072, msg_3072, sig_3072);
    status |= sig_gen(vce_id, RSA_4096_LEN, HAL_EXT_MEM, n_4096, private_key_4096, msg_4096, sig_4096);

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}

//signature verification
uint32_t sig_verify(uint32_t vce_id, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* n, uint8_t* public_expo, uint8_t* msg, uint8_t* except_s)
{
    rsa_pubkey_t pub_key_temp;
    void* crypto_handle;

    pub_key_temp.n = n;
    pub_key_temp.n_len = keysize;
    pub_key_temp.e = public_expo;
    pub_key_temp.e_len = keysize;

    uint64_t cur_time;

    LTRACEF("rsa verify enter\n");

    cur_time = current_time_hires();

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    uint32_t res = hal_rsa_verify(crypto_handle, SD_ALG_SHA256, msg, keysize,
                              except_s, keysize, &pub_key_temp, SD_RSA_PADDING_NONE, 0);
    hal_crypto_delete_handle(crypto_handle);
    total_time += current_time_hires() - cur_time;

    if (res) {
        LTRACEF("signature verify for %d fail, result: 0x%x\n\n", keysize, res);
        rsa_test_fail++;
    }
    else {
        LTRACEF("rsa verify pass \n");
    }

    return res;
}

uint32_t rsa_sig_verify_trav(uint32_t vce_id)
{
    uint32_t ret;
    uint32_t status;

    LTRACEF("-----rsa_sig_verify_trav test------\n");
    status = sig_verify(vce_id, RSA_1024_LEN, HAL_EXT_MEM, n_1024, public_expo_1024, msg_1024, sig_1024);
    status |= sig_verify(vce_id, RSA_2048_LEN, HAL_EXT_MEM, n_2048, public_expo_2048, msg_2048, sig_2048);
    status |= sig_verify(vce_id, RSA_3072_LEN, HAL_EXT_MEM, n_3072, public_expo_3072, msg_3072, sig_3072);
    status |= sig_verify(vce_id, RSA_4096_LEN, HAL_EXT_MEM, n_4096, public_expo_4096, msg_4096, sig_4096);

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}

//Private key generation
uint32_t priv_key_gen(uint32_t vce_id, uint32_t keysize, buff_addr_type_t addr_type, uint8_t* p, uint8_t* q, uint8_t* public_expo, uint8_t* except_n, uint8_t* except_pk)
{

    uint8_t __attribute__((aligned(CACHE_LINE))) n[RSA_4096_LEN];
    uint8_t __attribute__((aligned(CACHE_LINE))) private_key[RSA_4096_LEN * 2] = {0};
    uint64_t cur_time;
    void* crypto_handle;

    LTRACEF("rsa priv_key_gen enter keysize=%d \n", keysize);

    cur_time = current_time_hires();

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    uint32_t res = hal_rsa_keygen(crypto_handle, public_expo, keysize, keysize, n, private_key);
    hal_crypto_delete_handle(crypto_handle);
    total_time += current_time_hires() - cur_time;

    res = memcmp_reverse(except_n, n, keysize);

    if (res) {
        LTRACEF("priv_key_gen for %d except_n not right, res: %d\n\n", keysize, res);
        ce_printf_binary("key_gen n", n, keysize);
        rsa_test_fail++;
        return res;
    }

    res = memcmp_reverse(except_pk, private_key, keysize);

    if (res) {
        LTRACEF("rsa priv_key_gen fail\n");
        ce_printf_binary("rsa key_gen priv", private_key, keysize);
        rsa_test_fail++;
    }
    else {
        LTRACEF("rsa priv_key_gen pass \n");
    }

    return res;
}

uint32_t rsa_priv_key_gen_trav(uint32_t vce_id)
{
    uint32_t ret;
    uint32_t status;

    LTRACEF("----rsa priv key-DDR base test------\n");
    status = priv_key_gen(vce_id, RSA_1024_LEN, HAL_EXT_MEM, p_1024, q_1024, public_expo_1024, n_1024, private_key_1024 + RSA_1024_LEN);
    status |= priv_key_gen(vce_id, RSA_2048_LEN, HAL_EXT_MEM, p_2048, q_2048, public_expo_2048, n_2048, private_key_2048);
    status |= priv_key_gen(vce_id, RSA_3072_LEN, HAL_EXT_MEM, p_3072, q_3072, public_expo_3072, n_3072, private_key_3072);
    status |= priv_key_gen(vce_id, RSA_4096_LEN, HAL_EXT_MEM, p_4096, q_4096, public_expo_4096, n_4096, private_key_4096);

    if(status){
        ret = 1;
    }else{
        ret = 0;
    }

    return ret;
}

/* stress and performace test for key pair generation of RSA 2048*/
void kp_test(void)
{
   uint8_t __attribute__((aligned(CACHE_LINE))) keybuf[512*2] = {0};

    uint32_t status;
    void *crypto_handle = NULL;
    uint64_t cur_time;
    uint32_t h_times = 0;

    total_time=0;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);

    for (int i = 0; i <1000; i++) {
        cur_time = current_time_hires();
        status = hal_rsa_keygen_ex(crypto_handle, 256, public_expo_2048, keybuf);
        if (status) {
            LTRACEF(" RSA key generation FAILED %d\n", status);
        }

        total_time += (current_time_hires() - cur_time);
        if (total_time > 300000) {
            printf("total time : %llu, i: %d\n", total_time, i);
            h_times++;
        }
        total_time=0;
    }

    hal_crypto_delete_handle(crypto_handle);

    ce_printf_binary("key:n/lambda/pk/exponent", keybuf, 2*512);
    LTRACEF("total time : %llu, h_times: %d\n", total_time, h_times);
}

void private_verify_test(void)
{
    uint8_t * keybuf = NULL;
    uint8_t * keybuf1 = NULL;
    uint32_t result_len = 256;

    keybuf  = memalign(32, 512);
    if (!keybuf) {
        return;
    }

    uint32_t res = rsa_key_generation_blk(0, 256, block_t_convert(public_expo_2048, RSA_2048_LEN, EXT_MEM),
                        block_t_convert(keybuf, 512, EXT_MEM), false, false);
    if (res) {
        printf("rsa_key_generation_blk, status: %d\n", res);
        goto FREE_BUF;
    }

    ce_printf_binary("keybuf: %s\n", keybuf, 512);

    keybuf1 = memalign(32, 512);
    if (!keybuf1) {
        goto FREE_BUF;
    }

    //change n to big endian from little endian
    for (uint32_t i = 0; i < result_len; i++) {
        keybuf1[i] = keybuf[result_len - i - 1];
    }

    //change private_key to big endian from little endian
    for (uint32_t i = 0; i < result_len; i++) {
        uint8_t * temp1 = keybuf + result_len;
        uint8_t * temp2 = keybuf1 + result_len;
        temp2[i] = temp1[result_len - i - 1];
    }

    rsa_pubkey_t pub_key_temp;
    uint8_t __attribute__((aligned(CACHE_LINE))) result[256];
    uint8_t __attribute__((aligned(CACHE_LINE))) cipher[256];
    void* crypto_handle;

    pub_key_temp.n = keybuf1; //n_2048;
    pub_key_temp.n_len = result_len;
    pub_key_temp.e = public_expo_2048;
    pub_key_temp.e_len = result_len;

    hal_crypto_creat_handle(&crypto_handle, VCE_ID_GENERAL_SUPPORT_PKA);
    res = hal_rsa_encrypt(crypto_handle, &pub_key_temp, msg_2048, result_len,
                    result, result_len, SD_RSA_PADDING_NONE);
    if (res){
        printf("hal_rsa_encrypt, status: %d\n", res);
        hal_crypto_delete_handle(crypto_handle);
        goto FREE_BUF;
    }

    ce_printf_binary("cipher: %s\n", result, 256);

    //reverse byte order
    for (uint32_t i = 0; i < result_len; i++) {
        cipher[i] = result[result_len - i - 1];
    }

    rsa_keypair_t pri_key_temp;
    pri_key_temp.n = keybuf1;//n_2048;
    pri_key_temp.n_len = result_len;
    pri_key_temp.d = (keybuf1 + result_len);//private_key_2048;
    pri_key_temp.d_len = result_len;
    res = hal_rsa_decrypt(crypto_handle, &pri_key_temp, result_len, result, result_len,
                    cipher, result_len, SD_RSA_PADDING_NONE);
    if (res) {
        printf("hal_rsa_decrypt, status: %d\n", res);
        hal_crypto_delete_handle(crypto_handle);
        goto FREE_BUF;
    }

    for (uint32_t i = 0; i < result_len; i++) {
        cipher[i] = result[result_len - i - 1];
    }

    ce_printf_binary("dcrypt result: %s\n", cipher, result_len);
    ce_printf_binary("except result: %s\n", msg_2048, result_len);

    hal_crypto_delete_handle(crypto_handle);

FREE_BUF:
    if (!keybuf) {
        free(keybuf);
    }
    if (!keybuf1) {
        free(keybuf1);
    }
}

//one for a function: sha256, trng, skc, aes-cbc, rsa2048, ecdsa sig/verify
uint32_t rsa_test_slt(void* arg)
{
    uint32_t ret = 0;
    uint32_t result_value = 0;
    ce_test_t* ce_test_s;

    if(arg != NULL){
        ce_test_s = (ce_test_t*)arg;
    }else{
        ret |= (0x1 << CE_TEST_RESULT_OFFSET_UNKOWN_TEST);
        return ret;
    }

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_RSA_ENC_TEST;
    result_value = rsa_enc_trav(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_RSA_ENC_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_RSA_DEC_TEST;
    result_value = rsa_dec_trav(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_RSA_DEC_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_RSA_SIGN_GEN_TEST;
    result_value = rsa_sig_gen_trav(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_RSA_SIGN_GEN_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_RSA_SIGN_VERY_TEST;
    result_value = rsa_sig_verify_trav(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_RSA_SIGN_VERY_TEST);

    ce_test_s->current_index = CE_TEST_ITEM_INDEX_RSA_KEY_GEN_TEST;
    result_value = rsa_priv_key_gen_trav(0);
    ret |= (result_value << CE_TEST_RESULT_OFFSET_RSA_KEY_GEN_TEST);

    LTRACEF("finished rsa test rsa_test_fail =%d.\n", rsa_test_fail);

    return ret;
}

uint32_t rsa_test_uart(void)
{
    uint32_t ret = 0;

    ret = rsa_enc_trav(0);
    ret |= rsa_dec_trav(0);
    ret |= rsa_sig_gen_trav(0);
    ret |= rsa_sig_verify_trav(0);
    ret |= rsa_priv_key_gen_trav(0);

    return ret;
}
#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("rsa_test", "rsa basic test", (console_cmd)&rsa_test_uart)
STATIC_COMMAND("kp", "rsa key pair test", (console_cmd)&kp_test)
STATIC_COMMAND("kp_v", "rsa key pair verification test", (console_cmd)&private_verify_test)
STATIC_COMMAND_END(rsa_test);

#endif

APP_START(rsa_test)
.flags = 0
APP_END
