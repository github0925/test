/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <err.h>
#include <string.h>
#include <lib/slt_module_test.h>

#define PERF_TIMES         100
#define STRESS_TEST_TIMES  100

#define CE_TEST_TIMEOUT_FOR_SLT_DEFAULT_VALUE  1000*60 //must < SLT_DOMAIN_TEST_TIMEOUT_DEFAULT_VALUE 

typedef enum ecdsa_test_type {
    ECDSA_P192  = 0x0,
    ECDSA_P256  = 0x1,
    ECDSA_P384  = 0x2,
    ECDSA_P521  = 0x3,
    ECDSA_E521  = 0x4
} ecdsa_test_type_t;

typedef enum sm2_test_type {
    SM2_P192  = 0x0,
    SM2_P256  = 0x1,
    SM2_P384  = 0x2,
    SM2_P521  = 0x3,
    SM2_E521  = 0x4
} sm2_test_type_t;

typedef enum ce_test_item_index {
    CE_TEST_ITEM_INDEX_CIPHER_PATH_TEST  = 0x0,
    CE_TEST_ITEM_INDEX_CIPHER_CONTEXT_TEST,
    CE_TEST_ITEM_INDEX_DSA_KEY_GEN_TEST,
    CE_TEST_ITEM_INDEX_DSA_SIGN_GEN_TEST,
    CE_TEST_ITEM_INDEX_DSA_SIGN_VERY_TEST,
    CE_TEST_ITEM_INDEX_ECC_KEY_GEN_TEST,
    CE_TEST_ITEM_INDEX_ECDSA_SIG_GEN_TEST,
    CE_TEST_ITEM_INDEX_ECDSA_SIGN_VERY_TEST,
    CE_TEST_ITEM_INDEX_HASH_PATH_TEST,
    CE_TEST_ITEM_INDEX_HASH_MULT_TEST,
    CE_TEST_ITEM_INDEX_RSA_ENC_TEST,
    CE_TEST_ITEM_INDEX_RSA_DEC_TEST,
    CE_TEST_ITEM_INDEX_RSA_SIGN_GEN_TEST,
    CE_TEST_ITEM_INDEX_RSA_SIGN_VERY_TEST,
    CE_TEST_ITEM_INDEX_RSA_KEY_GEN_TEST,
    CE_TEST_ITEM_INDEX_SM2_SIGN_GEN_TEST,
    CE_TEST_ITEM_INDEX_SM2_SIGN_VERY_TEST,
} ce_test_item_index_t;

//ALL 32 BIT
typedef enum ce_test_result_offset {
    CE_TEST_RESULT_OFFSET_CIPHER_PATH_TEST  = 0x0,
    CE_TEST_RESULT_OFFSET_CIPHER_CONTEXT_TEST,
    CE_TEST_RESULT_OFFSET_DSA_KEY_GEN_TEST,
    CE_TEST_RESULT_OFFSET_DSA_SIGN_GEN_TEST,
    CE_TEST_RESULT_OFFSET_DSA_SIGN_VERY_TEST,
    CE_TEST_RESULT_OFFSET_ECC_KEY_GEN_TEST,
    CE_TEST_RESULT_OFFSET_ECDSA_SIG_GEN_TEST,
    CE_TEST_RESULT_OFFSET_ECDSA_SIGN_VERY_TEST,
    CE_TEST_RESULT_OFFSET_HASH_PATH_TEST,
    CE_TEST_RESULT_OFFSET_HASH_MULT_TEST,
    CE_TEST_RESULT_OFFSET_RSA_ENC_TEST,
    CE_TEST_RESULT_OFFSET_RSA_DEC_TEST,
    CE_TEST_RESULT_OFFSET_RSA_SIGN_GEN_TEST,
    CE_TEST_RESULT_OFFSET_RSA_SIGN_VERY_TEST,
    CE_TEST_RESULT_OFFSET_RSA_KEY_GEN_TEST,
    CE_TEST_RESULT_OFFSET_SM2_SIGN_GEN_TEST,
    CE_TEST_RESULT_OFFSET_SM2_SIGN_VERY_TEST,
    CE_TEST_RESULT_OFFSET_UNKOWN_TEST,
} ce_test_result_offset_t;

typedef struct ce_test {
    uint32_t ce_index;
    uint32_t current_index;
    uint32_t test_timer_init;
    timer_t test_timer;
    char* result_string;
} ce_test_t;


void ce_printf_binary(const char* info, const void* content, uint32_t content_len);

void enable_vce_key_interface(void);

int ce_readl_(addr_t reg);
int ce_writel_(uint32_t val,addr_t reg);
