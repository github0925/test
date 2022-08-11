/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_PKE_FUNCS_H
#define SX_PKE_FUNCS_H

#include <stdint.h>

/**
 * @brief modular addation(C= A + B mod N).
 *
 * @param vce_id      vce index
 * @param ptra        input number1
 * @param ptrb        input number2
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_add(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc);

/**
 * @brief modular substraction(C= A - B mod N).
 *
 * @param vce_id      vce index
 * @param ptra        input number1
 * @param ptrb        input number2
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_sub(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc);

/**
 * @brief modular multiplication(C= A * B mod N).
 *
 * @param vce_id      vce index
 * @param ptra        input number1
 * @param ptrb        input number2
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_multi(uint32_t vce_id,
                       block_t ptra,
                       block_t ptrb,
                       block_t ptrn,
                       block_t ptrc);

/**
 * @brief modular reduction for odd number(C= B mod N).
 *
 * @param vce_id      vce index
 * @param ptrb        input number
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_reduce_odd(uint32_t vce_id,
                            block_t ptrb,
                            block_t ptrn,
                            block_t ptrc);

/**
 * @brief modular divsion(C= A / B mod N).
 *
 * @param vce_id      vce index
 * @param ptra        input number1
 * @param ptrb        input number2
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_div(uint32_t vce_id,
                     block_t ptra,
                     block_t ptrb,
                     block_t ptrn,
                     block_t ptrc);

/**
 * @brief modular inversion for odd number(C= 1 / B mod N).
 *
 * @param vce_id      vce index
 * @param ptrb        input number
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_inverse_odd(uint32_t vce_id,
                             block_t ptrb,
                             block_t ptrn,
                             block_t ptrc);

/**
 * @brief modular square root(C= sqrt(A) mod N).
 *
 * @param vce_id      vce index
 * @param ptra        input number
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_square_root(uint32_t vce_id,
                             block_t ptra,
                             block_t ptrn,
                             block_t ptrc);

/**
 * @brief multiplicaton(C= A * B).
 *
 * @param vce_id      vce index
 * @param ptra        input number1
 * @param ptrb        input number2
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
/*  */
uint32_t multiplicate(uint32_t vce_id,
                      block_t ptra,
                      block_t ptrb,
                      block_t ptrc);

/**
 * @brief modular inversion for even number(C= 1 / B mod N).
 *
 * @param vce_id      vce index
 * @param ptrb        input number
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_inverse_even(uint32_t vce_id,
                              block_t ptrb,
                              block_t ptrn,
                              block_t ptrc);

/**
 * @brief modular reduction for even number(C= B mod N).
 *
 * @param vce_id      vce index
 * @param ptrb        input number
 * @param ptrn        modular number
 * @param ptrc        result
 * @return CRYPTOLIB_SUCCESS if successful
 */
uint32_t modular_reduce_even(uint32_t vce_id,
                             block_t ptrb,
                             block_t ptrn,
                             block_t ptrc);

#endif
