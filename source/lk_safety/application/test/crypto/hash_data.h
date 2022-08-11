/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef _HASH_DATA_H
#define _HASH_DATA_H

#include <sys/types.h>
#include <string.h>

#include <sx_hash.h>

/* sram usage: 1st block(64 bytes) - iv
 *             2nd block(64 bytes) - dst
 *             3-5 block(64 bytes) - key
 *             6 block -         - src
 */
#define SRAM_IV_OFFSET     (0)
#define SRAM_DST_OFFSET    (64)
#define SRAM_KEY_OFFSET    (5 * 64)//(2 * 64)
#define SRAM_SRC_OFFSET    (8 * 64)//(5 * 64)

extern uint8_t md5_iv[MD5_INITSIZE];
extern uint8_t sha1_iv[SHA1_INITSIZE];
extern uint8_t sha224_iv[SHA224_INITSIZE];
extern uint8_t sha256_iv[SHA256_INITSIZE];
extern uint8_t sha384_iv[SHA384_INITSIZE];
extern uint8_t sha512_iv[SHA512_INITSIZE];
extern uint8_t sm3_iv[SM3_INITSIZE];

extern char input_data[64];
extern uint32_t inputdata_size;

extern char key[32];
extern uint32_t key_size;

extern uint8_t expected_digest_md5[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3[SM3_DIGESTSIZE];


extern uint8_t expected_digest_md5_2times[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1_2times[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224_2times[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256_2times[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384_2times[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512_2times[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3_2times[SM3_DIGESTSIZE];

extern uint8_t expected_digest_md5_h[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1_h[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224_h[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256_h[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384_h[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512_h[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3_h[SM3_DIGESTSIZE];

extern uint8_t expected_digest_md5_h_2times[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1_h_2times[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224_h_2times[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256_h_2times[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384_h_2times[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512_h_2times[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3_h_2times[SM3_DIGESTSIZE];

//data for aligned block with no padding
extern char block_align_sha256[SHA256_BLOCKSIZE];
extern uint8_t expected_digest_align_h[SHA256_BLOCKSIZE];

//data for long key test
extern char key_big[112];
extern uint8_t expected_digest_big_key_h[32];

//data for performance test
extern char key_perf[32];
extern char input_data_perf[96];

extern uint8_t expected_digest_md5_perf[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1_perf[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224_perf[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256_perf[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384_perf[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512_perf[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3_perf[SM3_DIGESTSIZE];

extern uint8_t expected_digest_md5_perf_h[MD5_DIGESTSIZE];
extern uint8_t expected_digest_sha1_perf_h[SHA1_DIGESTSIZE];
extern uint8_t expected_digest_sha224_perf_h[SHA224_DIGESTSIZE];
extern uint8_t expected_digest_sha256_perf_h[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_sha384_perf_h[SHA384_DIGESTSIZE];
extern uint8_t expected_digest_sha512_perf_h[SHA512_DIGESTSIZE];
extern uint8_t expected_digest_sm3_perf_h[SM3_DIGESTSIZE];

//data for multi-part test
extern const char input_data_multi[704];
extern uint32_t inputdata_multi_size;
extern uint8_t expected_digest_multi[SHA256_DIGESTSIZE];
extern uint8_t expected_digest_multi_part[10][SHA256_DIGESTSIZE];

void get_init_value(hash_alg_t alg, uint8_t* iv);

void ce_printf(hash_alg_t alg, bool iv_enable, ce_addr_type_t iv_addr_type, bool hmac,
               ce_addr_type_t key_addr_type, ce_addr_type_t src_addr_type, ce_addr_type_t dst_addr_type,
               bool pass, uint8_t* fail_result, uint32_t result_len);
void ce_printf_perf(hash_alg_t alg, bool key_en, uint32_t key_size,
                    ce_addr_type_t src_type, uint32_t src_size, ce_addr_type_t dst_type, uint64_t time_slice);

#endif
