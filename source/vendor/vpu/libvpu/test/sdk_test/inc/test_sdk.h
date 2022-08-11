/*
 * test_sdk.h
 *
 * Copyright (C) 2020 Semidrive Technology Co., Ltd.
 *
 * Description: Imple vpu sdk test function
 *
 * Revision Histrory:
 * -----------------
 * 1.0, 01/09/2020  chentianming <tianming.chen@semidrive.com> create this file
 *
 */
/*******************************************************************************
 *                       Include header files                                 *
 ******************************************************************************/
#ifndef __TEST_SDK_H__
#define __TEST_SDK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>

#include "main_helper.h"
#include "test_case.h"

/*******************************************************************************
 *                        Type definitions                                    *
 ******************************************************************************/


/*******************************************************************************
 *                        Global function declarations                        *
 ******************************************************************************/
/*
 * @brief sdk test for vpu device init
 * @param context  context info in test
 * @return 0  success
 *         !0 failure code
 * @note
 */
int sdk_test_vpu_init(struct test_context  *context);

/*
 * @brief sdk test for vpu device deinit
 * @param context  context info in test
 * @return 0  success
 *         !0 failure code
 * @note
 */
int sdk_test_vpu_deinit(struct test_context  *context);

/*
 * @brief sdk test for decoding and seek operation
 * @param enable_seek
 *        1: seek operation in decoding
          0: decoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @note
 */
int sdk_test_vpu_decode(struct test_context  *context, BOOL enable_seek);

/*
 * @brief sdk test for encoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @note
 */
int sdk_test_case_encode(struct test_context  *context);

/*
 * @brief sdk test for loop decoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @note
 */
int sdk_test_vpu_decode_loop(struct test_context *context);

/*
 * @brief sdk_test_vpu_encode - sdk test for encoding
 * @param context context info in test
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_vpu_encode(struct test_context  *context);

#ifdef __cplusplus
}
#endif

#endif  /* End __TEST_CASE_H__ */

