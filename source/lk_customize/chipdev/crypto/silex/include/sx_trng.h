/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#ifndef SX_TRNG_H
#define SX_TRNG_H

#include <stdint.h>
#include <stdbool.h>

#include <ce_reg.h>
#include <ce.h>

typedef enum trng_test {
    TRNG_REP_TEST,      /**< Repetition count test. */
    TRNG_PROP_TEST,     /**< Adaptive proportion test. */
    TRNG_PREALM_TEST,   /**< AIS31 preliminary noise alarm */
    TRNG_ALM_TEST,      /**< AIS31 noise alarm. */
} trng_test_t;

typedef enum control_fsm_state {
    FSM_STATE_RESET = 0,
    FSM_STATE_STARTUP,
    FSM_STATE_IDLE_ON,
    FSM_STATE_IDLE_OFF,
    FSM_STATE_FILL_FIFO,
    FSM_STATE_ERROR
} control_fsm_state_t;

//RNG settings
#define RNG_CLKDIV            (0)
#define RNG_OFF_TIMER_VAL     (0)
#define RNG_FIFO_WAKEUP_LVL   (8)
#define RNG_INIT_WAIT_VAL     (512)
#define RNG_NB_128BIT_BLOCKS  (4)

/**
* @brief TRNG initialization
* @param cond_test_en When not zero, conditioning test is executed first
* @return CRYPTOLIB_SUCCESS or CRYPTOLIB_CRYPTO_ERR
*/
uint32_t trng_init(uint32_t cond_test_en);

/**
* @brief Indicate whether conditioning or startup test failed during init.
* Only valid during bootloader and diagnostic mode!
* If false is returned, true randomness can't be warranted and user must react
* (i.e. raising tamper)
* @return boolean
*/
bool trng_startup_failed(void);

/**
* @brief Sets a flag that tells the TRNG software module to wait for TRNG
* startup tests to be finished successfully before reading random data from the
* TRNG FIFO.This needs to be done after explicitly calling the ba431_softreset()
* in interrupt context and after TRNG initialization.
*
* The TRNG startup test will wait until the TRNG FIFO is filled with data and
* then check that the data is sufficiently random. If it is, the startup test is
* passed. The BA431 datasheet specifies that we shouldn't use data from the TRNG
* FIFO before it has been confirmed to be sufficiently random.
*/
void trng_set_startup_chk_flag(void);

/**
 * @brief Indicates whether a NIST-800-90B repetition count test fails,
 *  a adaptative proportion test fails or (if enabled) a AIS31 noise alarm
 *  is triggered.
 * @return true if one error described above is raised
 */
bool trng_critical_raised(void);

/**
* @brief Applies a soft reset on the random generator and sets a flag that tells
* the TRNG software module to wait for TRNG startup tests to be finished
* successfully before reading random data from the TRNG FIFO.
*
* The TRNG startup test will wait until the TRNG FIFO is filled with data and
* then check that the data is sufficiently random. If it is, the startup test is
* passed. The BA431 datasheet specifies that we shouldn't use data from the TRNG
* FIFO before it has been confirmed to be sufficiently random.
*/
void trng_soft_reset(void);

/**
* @brief Generate random data (block_t)
* @param dst Output location for the generated random data.
*/
uint32_t trng_get_rand_blk(uint32_t vce_id, block_t dst);

/**
* @brief Get an array of random values
* @param dst Pointer to the array where the random values are stored
* @param size Number of random bytes to be generate
*/
uint32_t trng_get_rand(uint32_t vce_id, uint8_t* dst, uint32_t size);

/**
* @brief Returns a random 32-bit word
* @return 32-bit random value
*/
uint32_t trng_get_word(uint32_t vce_id);

/**
* @brief Generate a random block of data by the TRNG. Defined to match the
* prototype of the struct sx_rng get_rand_blk()
* @param param Should be set to NULL
* @param result block_t for the generated values
*/
void trng_fill_blk(uint32_t vce_id, void* param, block_t result);

#ifdef UNITTESTS
static uint32_t trng_conditioning_test(void);
static uint32_t trng_wait_startup(void);
#endif

/**
 * @brief Return if the TRNG module is  facing an error state
 *
 * This error could be due to start-up tests (NIST-800-90B Start-up Test or
 * AIS31 Start-up Test) or due to online test (Adaptive Proportion Test or
 * AIS31 Online  Test)
 */
bool trng_is_error_detected(void);

uint32_t trng_get_hrng(uint32_t vce_id);

uint32_t rng_get_prng(uint32_t vce_id, uint8_t* dst, uint32_t size);
uint32_t rng_get_trng(uint32_t vce_id, uint8_t* dst, uint32_t size);

/**
* @brief Cause a TRNG to fail (on health tests)
* For test failure confirmation the TRNG interrupt needs to be checked.
*
* \note This test will wait until a ::BA431_STAT_MASK_PROP_FAIL even if other
*       kind of BA431 failure is generated before.
*/
void trng_trigger_failure(void);

/**
* @brief Cause a TRNG self test to fail for a specified test
* For test failure confirmation the TRNG status and/or interrupt needs to be
* checked.
* @param test Chosen TRNG self test
* @return CRYPTOLIB_SUCCESS if the TRNG was successfully configured for causing
*                          the test failure
*/
uint32_t trng_trigger_test_failure(trng_test_t test);

/**
* @brief get random number from fifo
* CE1 FIFO depth: 16 words, CE2 FIFO depth: 32
* @param dst random number buffer
* @param size random number count
* @return CRYPTOLIB_SUCCESS if the TRNG was successfully configured for causing
*                          the test failure
*/
uint32_t trng_get_rand_by_fifo(uint8_t* dst, uint32_t size);

/* @brief This function implements one of the methods approved by FIPS 186-4 to
 * generate a random number k with 1 <= k < n.
 *
 * Get a random value rnd of the appropriate length.
 * If rnd > n -2
 *    Try another value
 * Else
 *    k = rnd + 1
 * @param dst buffer of generated random
 * @param n buffer of refered random
 * @return CRYPTOLIB_SUCCESS if the TRNG was successfully or corresponding error
 */
uint32_t rng_get_rand_less_ref(uint32_t vce_id, block_t dst, block_t n);

#endif /* SX_TRNG_H */
