/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_SM2_H
#define SX_SM2_H

#include <stdint.h>
#include <sx_ecc_keygen.h>
#include <sx_hash.h>
#include <sx_trng.h>

/**
 * @brief Verify an SM2 signature from a \p formatted_digest
 *
 * @param vce_id      vce index
 * @param curve            ECC curve
 * @param formatted_digest digest to verify against \p signature.
 * @param key              ECC public key
 * @param signature        signature to verify against the \p formatted_digest
 * @return CRYPTOLIB_SUCCESS if successful
 *         CRYPTOLIB_INVALID_SIGN_ERR if verification failed
 *
 * \note the \p formatted_digest corresponds to the digest truncated if required
 *       to match the ecc curve element size and also right shifted if required
 *       (left padded with zeros)
 */
uint32_t sm2_verify_signature_digest(uint32_t vce_id,
                                     const sx_ecc_curve_t* curve,
                                     block_t formatted_digest,
                                     block_t key,
                                     block_t signature);

/**
 * @brief Configure hardware for signature generation.
 *
 * Load the curve parameters, the key and formatted_digest. Those are
 * needed once before attempting to generate signatures.
 * @param vce_id      vce index
 * @param curve            selects the ECC curve
 * @param formatted_digest digest to generate the signature.
 * @param key              ECC private key
 * @return CRYPTOLIB_SUCCESS if successful
 *
 * \note the \p formatted_digest corresponds to the digest truncated if required
 *       to match the ecc curve element size and also right shifted if required
 *       (left padded with zeros)
 */
uint32_t sm2_configure_signature(uint32_t vce_id,
                                 const sx_ecc_curve_t* curve,
                                 block_t formatted_digest,
                                 block_t key);

/**
 * @brief Attempt to compute an SM2 signature
 *
 * This function tries to generate a signature and gives up immediately in case
 * of error. It is under the caller responsibility to attempt again to generate
 * a signature.
 * @param vce_id      vce index
 * @param curve            selects the ECC curve
 * @param signature        output signature of the \p formatted_digest
 * @return CRYPTOLIB_SUCCESS if the signature is generated and valid,
 *         CRYPTOLIB_INVALID_SIGN_ERR if the generated signature is not valid
 *
 * \warning It is required to call ::sx_sm2_configure_signature \e before
 *          attempting to generate a signature as a few parameters have to be
 *          loaded.
 */
uint32_t sm2_attempt_signature(uint32_t vce_id,
                               const sx_ecc_curve_t* curve,
                               block_t signatur);

/**
 * @brief Verifies a given signature corresponds to a given message
 *
 * @param vce_id      vce index
 * @param curve     selects the ECC curve
 * @param message   data to hash and verify against \p signature
 * @param key       ECC public key
 * @param signature signature to check
 * @param hash_fct  select hash algorithm to use on \p message
 * @return CRYPTOLIB_SUCCESS if successful
 *         CRYPTOLIB_INVALID_SIGN_ERR if signature verification failed
 */
uint32_t sm2_verify_signature(uint32_t vce_id,
                              const void* curve_info,
                              block_t message,
                              block_t key,
                              block_t signature,
                              hash_alg_t hash_fct);

/**
 * @brief Verifies a given signature corresponds to a given digest
 *
 * @param vce_id      vce index
 * @param curve            selects the ECC curve
 * @param formatted_digest digest to use to generate \p signature.
 * @param key              ECC private key
 * @param signature        output signature of the \p formatted_digest
 * @return CRYPTOLIB_SUCCESS if successful
 *         CRYPTOLIB_INVALID_SIGN_ERR if signature verification failed
 *
 * \note the \p formatted_digest corresponds to the digest truncated if required
 *       to match the ecc curve element size and also right shifted if required
 *       (left padded with zeros)
 */

uint32_t sm2_generate_signature_digest(uint32_t vce_id,
                                       const sx_ecc_curve_t* curve,
                                       block_t formatted_digest,
                                       block_t key,
                                       block_t signature);

/**
 * @brief perform digest computation used for signing or verification
 * @param curve SM2 curve to use
 * @param message message to sign or verify
 * @param hash_fct algorithm to use for hash
 * @param digest_blk computed digest
 * @return CRYPTOLIB_INVALID_PARAM when there is not enough space in the
 *                                 digest_blk to compute the digest
 *         CRYPTOLIB_SUCCESS otherwise
 */
uint32_t sm2_generate_digest(uint32_t vce_id,
                             const sx_ecc_curve_t* curve,
                             block_t message,
                             hash_alg_t hash_fct,
                             block_t* digest_blk);

/**
 * @brief Generates an SM2 signature from a \p message
 *
 * @param vce_id      vce index
 * @param curve     selects the ECC curve
 * @param message   data to hash and sign
 * @param key ECC   private key
 * @param signature location to store the signature
 * @param hash_fct  select hash algorithm to use on \p message
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t sm2_generate_signature(uint32_t vce_id,
                                const void* curve_info,
                                block_t message,
                                block_t key,
                                block_t signature,
                                hash_alg_t hash_fct);

/**
 * @brief SM2 exchange
 *
 * @param vce_id      vce index
 * @param curve     selects the ECC curve
 * @param prv_key   private key
 * @param pub_key   public key
 * @param pointb    point b (RBx,RBy)
 * @param cofactor  cofactor
 * @param pointa_x  RAx of point a and 2_w
 * @param ex_pub_key exchange result key
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t sm2_key_exchange(uint32_t vce_id,
                          const void* curve_info,
                          block_t prv_key,
                          block_t pub_key,
                          block_t pointb,
                          block_t cofactor,
                          block_t pointa,
                          block_t two_w,
                          block_t ex_pub_key);

#endif
