/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_RSA_H
#define SX_RSA_H

#include <stdint.h>
#include <sx_hash.h>

/**
* @brief Enumeration of possible padding types
*/
typedef enum rsa_pad_types_e {
    ESEC_RSA_PADDING_NONE      = 0x0,/**< No padding */
    ESEC_RSA_PADDING_OAEP      = 0x1,/**< Optimal Asymmetric Encryption Padding */
    ESEC_RSA_PADDING_EME_PKCS  = 0x2,/**< EME-PKCS padding */
    ESEC_RSA_PADDING_EMSA_PKCS = 0x3,/**< EMSA-PKCS padding */
    ESEC_RSA_PADDING_PSS       = 0x4 /**< EMSA-PSS padding */
} rsa_pad_types_t;


/**
 * @brief Generate an asymmetric key for RSA
 * @param vce_id    vce index
 * @param nbr_of_bytes number of required bytes for the key
 * @param exponent exponent needed for RSA private key generation
 * @param key buffer containing the generated key: private followed by public
 * key (exponent) is followed by:
 * - modulus
 * - lambda(N), if lambda flag is set
 * - private key
 * - exponent if the public flag is set
 * @param public Flag means while public key should be also provided
 * @param lambda Flag means while lambda data is required
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t rsa_key_generation_blk(uint32_t vce_id,
                                uint32_t nbr_of_bytes,
                                block_t exponent,
                                block_t key,
                                bool public,
                                bool lambda);

/**
* @brief RSA encryption (block_t as parameters)
 * @param vce_id      vce index
* @param padding_type  Padding to use to encode message
* @param message       Message to be encrypted
* @param n             = p * q (multiplication of two random primes)
* @param public_expo   Public exponent
* @param result        Result buffer
* @param hashType      Hash type used in OAEP padding type
* @param rng           The random number generator to use
* @return CRYPTOLIB_SUCCESS if successful
*/
uint32_t rsa_encrypt_blk(uint32_t vce_id,
                         rsa_pad_types_t padding_type,
                         block_t message,
                         block_t n,
                         block_t public_expo,
                         block_t result,
                         hash_alg_t hashType);

/**
* @brief RSA decryption (block_t as parameters)
 * @param vce_id        vce index
* @param padding_type    Padding to use to encode message
* @param cipher          Cipher to be decrypted
* @param n               = p * q (multiplication of two random primes), the RSA
*                        modulus
* @param private_key     Private key
* @param result          Result buffer
* @param crt             1 if CRT decryption (in that case, private_key contains
*                        the private parameters)
* @param msg_len         Pointer to the message length (for padded messages)
* @param hashType        Hash type used in OAEP padding type
* @return CRYPTOLIB_SUCCESS if successful
*/
uint32_t rsa_decrypt_blk(uint32_t vce_id,
                         rsa_pad_types_t padding_type,
                         block_t cipher,
                         block_t n,
                         block_t private_key,
                         block_t result,
                         uint32_t crt,
                         uint32_t* msg_len,
                         hash_alg_t hashType);

/**
* @brief RSA signature generation (block_t as parameters)
* @param vce_id       vce index
* @param sha_type     Hash type to use for signature
* @param padding_type Padding type to use (see ::rsa_pad_types_t)
* @param message      Message to be signed
* @param result       Location where the signature will be stored
* @param n             = p * q (multiplication of two random primes), the RSA
*                     modulus
* @param private_key  Private key:
*                     - the pair (n, d) where n is the RSA modulus and d is the
*                     private exponent
*                     - the tuple (p, q, dP, dQ, qInv) where p is the first
*                     factor, q  is the second factor, dP is the first factor's
*                     CRT exponent, dQ is the second factor's CRT exponent, qInv
*                     is the (first) CRT coefficient
* @param crt          If equal to 1, uses CRT parameters
* @param salt_length  Expected length of the salt used in PSS padding_type
* @param rng          Random number generator to use for the OAEP and PKCS
*                     padding
* @return CRYPTOLIB_SUCCESS if successful
*/
uint32_t rsa_signature_generation_blk(uint32_t vce_id,
                                      hash_alg_t sha_type,
                                      rsa_pad_types_t padding_type,
                                      block_t message,
                                      block_t result,
                                      block_t n,
                                      block_t private_key,
                                      uint32_t salt_length);

/**
 * @brief RSA Signature verification (on block_t)
 * @param vce_id         vce index
 * @param sha_type       Hash function type used for verification
 * @param padding_type   Defines the padding to use
 * @param message        The message
 * @param n              = p * q (multiplication of two random primes), the RSA
 *                       modulus
 * @param public_expo    e, the RSA public exponent
 * @param signature      Pointer to signature
 * @param salt_length    Expected length of the salt used in PSS padding_type
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t rsa_signature_verification_blk(uint32_t vce_id,
                                        hash_alg_t sha_type,
                                        rsa_pad_types_t padding_type,
                                        block_t message,
                                        block_t n,
                                        block_t public_expo,
                                        block_t signature,
                                        uint32_t salt_length);


/**
* @brief RSA private key generation using block_t
* @param vce_id      vce index
* @param p           p, the first factor
* @param q           q, the second factor
* @param public_expo e, the RSA public exponent
* @param n           = p * q (multiplication of two random primes), the RSA
*                    modulus
* @param private_key The private key: n followed by lambda(n) if \p lambda is
*                    set, followed by d, the private exponent
* @param size        Number of bytes of the parameters
* @param lambda      1 if lambda(n) should be copied to the resulting private
*                    key (non-CRT only)
* @return CRYPTOLIB_SUCCESS if successful
*/
uint32_t rsa_private_key_generation_blk(uint32_t vce_id,
                                        block_t p,
                                        block_t q,
                                        block_t public_expo,
                                        block_t n,
                                        block_t private_key,
                                        uint32_t size,
                                        uint32_t lambda);

/**
* Generates CRT parameters based on \p p, \p q and \p d
* @param  vce_id vce index
* @param  p      p, the first factor
* @param  q      q, the second factor
* @param  d      the private exponent
* @param  dp     the first factor's CRT exponent, dP
* @param  dq     the second factor's CRT exponent, dQ
* @param  inv    the (first) CRT coefficient, qInv
* @param  size   the size (in bytes) of an element
* @return CRYPTOLIB_SUCCESS if no error
*/
uint32_t rsa_crt_key_generation_blk(uint32_t vce_id,
                                    block_t p,
                                    block_t q,
                                    block_t d,
                                    block_t dp,
                                    block_t dq,
                                    block_t inv,
                                    uint32_t size);

#endif
