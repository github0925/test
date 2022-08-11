/*
 * test_case.c
 *
 * Copyright (C) 2020 Semidrive Technology Co., Ltd.
 *
 * Description: Imple vpu sdk test cases function
 *
 * Revision Histrory:
 * -----------------
 * 1.0, 28/08/2020  chentianming <tianming.chen@semidrive.com> create this file
 *
 */
/*******************************************************************************
 *                       Include header files                                 *
 ******************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "test_case.h"
#include "test_sdk.h"

/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/


/*******************************************************************************
 *                        static function declarations                        *
 ******************************************************************************/
static int sdk_test_device_init(struct test_context *context);
static int sdk_test_device_deinit(struct test_context *context);
static int sdk_test_device_init_deinit(struct test_context *context);
static void sdk_test_case_result(struct test_context *context);
static void *case_run(struct test_context *context);
static void sdk_test_cases_info_show(struct test_context *context);
static int sdk_test_case_run(struct test_context *context);
static int sdk_test_case_index_get(struct test_context *context);
static int sdk_test_param_res_alloc(struct test_context **context, struct test_config *cfg);
static void sdk_test_param_res_free(struct test_context *context);

static int test_case_sample(void *param);
static int test_case_coda_decode_init(void *param);
static int test_case_coda_decode_deinit(void *param);
static int test_case_coda_decode_init_deinit(void *param);
static int test_case_wave_decode_init(void *param);
static int test_case_wave_decode_deinit(void *param);
static int test_case_wave_decode_init_deinit(void *param);
static int test_case_coda_decode(void *param);
static int test_case_wave_decode(void *param);
static int test_case_coda_decode_seek(void *param);
static int test_case_wave_decode_seek(void *param);
static int test_case_coda_decode_exit(void *param);
static int test_case_wave_decode_exit(void *param);
static int test_case_coda_encode_init(void *param);
static int test_case_coda_encode_deinit(void *param);
static int test_case_coda_encode_init_deinit(void *param);
static int test_case_coda_avc_encode(void *param);
static int test_case_coda_mpeg4_encode(void *param);
static int test_case_coda_h263_encode(void *param);
static int test_case_wave_decode_vp9(void *param);
static int test_case_wave_decode_hevc(void *param);
static int test_case_coda_loop_decode_avc(void *param);
static int test_case_wave_loop_decode_vp9(void *param);
static int test_case_wave_loop_decode_hevc(void *param);

/* test case info */
const struct case_info test_cases[] = {
    {test_case_sample,                      "case sample", NULL},
    /*coda decode */
    {test_case_coda_decode_init,            "case coda decode device open", NULL},
    {test_case_coda_decode_deinit,          "case coda decode device close", NULL},
    {test_case_coda_decode_init_deinit,     "case coda decode device open&close times", NULL},
    {test_case_coda_decode,                 "case coda decode whole file, according input formats \
avc,mpeg4/2,rv,h263,vp8,avs,vc1", NULL},
    {test_case_coda_decode_seek,            "case coda seek in decoding, user can input seek info: start time, end time, offset \
time, seek interval(frames)", NULL},
    {test_case_coda_decode_exit,            "case code decode specified frames exit when in decoding", NULL},
    {test_case_coda_loop_decode_avc,        "case coda loop decode whole file, should assign avc formats \
formats", NULL},
    /*coda encode */
    {test_case_coda_encode_init,            "case coda encode device open", NULL},
    {test_case_coda_encode_deinit,          "case coda encode device close", NULL},
    {test_case_coda_encode_init_deinit,     "case coda encode device open&close times", NULL},
    {test_case_coda_avc_encode,             "case encode avc format various resolutions", NULL},
    {test_case_coda_mpeg4_encode,           "case encode mpeg4 format various resolutions", NULL},
    {test_case_coda_h263_encode,            "case encode h263 format various resolutions", NULL},
    /*wave decode */
    {test_case_wave_decode_init,            "case wave decode device device open", NULL},
    {test_case_wave_decode_deinit,          "case wave decode device device close", NULL},
    {test_case_wave_decode_init_deinit,     "case wave decode device device open&close times", NULL},
    {test_case_wave_decode_vp9,             "case wave decode whole file vp9 format", NULL},
    {test_case_wave_decode_hevc,            "case wave decode whole file hevc format", NULL},
    {test_case_wave_decode_seek,            "case wave seek in decoding, user can input seek info: start time, end time, offset  \
time, seek interval(frames)", NULL},
    {test_case_wave_decode_exit,            "case wave decode specified frames exit when in decoding", NULL},
    {test_case_wave_loop_decode_vp9,        "case wave loop decode whole file, formats vp9", NULL},
    {test_case_wave_loop_decode_hevc,       "case wave loop decode whole file, formats hevc", NULL},
    /* add case following */
};

/*******************************************************************************
 *                        static function definitions                         *
 ******************************************************************************/
/*
 * brief test_case_sample - this sample case
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_sample(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);

    context = (struct test_context *)(param);
    log_debug("this is a sample case, case description (%s)\n", context->cases_info[0].description);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_decode_init
         test coda device open
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_coda_decode_init(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_decode_deinit
         test coda device close
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_coda_decode_deinit(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;

}

/*
 * brief test_case_coda_decode_init_deinit
         test coda device open and close
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_coda_decode_init_deinit(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init_deinit(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;

}

/*
 * brief test_case_wave_decode_init
         test wave device open
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_wave_decode_init(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;

}

/*
 * brief test_case_wave_decode_deinit
         test wave device open and close
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_wave_decode_deinit(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_deinit(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;

}

/*
 * brief test_case_wave_decode_init_deinit
         test wave device open and close
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_wave_decode_init_deinit(void *param)
{
    struct test_context *context = NULL;

    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init_deinit(context));
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_decode - test decode various formats
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should assign one format
 */
static int test_case_coda_decode(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, false));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode - test decode(vp9&hevc)
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_wave_decode(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, false));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_decode_seek - test seek operation
 *       According to the entered timestamp, Seek operation,
 *       start time, end time, seek offset, seek frames interval,
 *       in decoding with coda-ip
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_decode_seek(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, TRUE));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode_seek - test seek operation
 *       According to the entered timestamp, Seek operation,
 *       start time, end time, seek offset, seek frames interval,
 *       in decoding with wave-ip
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_wave_decode_seek(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, TRUE));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode_exit - test exit  random frames-num when in
 *       decoding coda-ip
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_decode_exit(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, false));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode_exit - test exit  random frames-num when in
 *       decoding wave-ip
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_wave_decode_exit(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode(context, false));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_encode_init - test vpu device init
 *       open device, feeder create
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_encode_init(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_encode_deinit - test vpu device deinit
 *       close device, feeder destrory
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_encode_deinit(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_deinit(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_encode_init_deinit - test vpu device init & deinit
 *       open and close device more times
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_encode_init_deinit(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_device_init_deinit(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_avc_encode - test avc(h264) encode with coda-ip,
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_avc_encode(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_encode(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_mpeg4_encode - test mpeg4 encode with coda-ip,
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_mpeg4_encode(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->enc_config->stdMode != STD_MPEG4) {
        log_err("test case parma is invalid, stdMode(%d)\n", context->test_cfg->enc_config->stdMode);
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_encode(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_h263_encode - test h263 encode with coda-ip,
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_coda_h263_encode(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_encode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->enc_config->stdMode != STD_H263) {
        log_err("test case parma is invalid, stdMode(%d)\n", context->test_cfg->enc_config->stdMode);
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_encode(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode_vp9 - test vp9 decode with wave-ip,
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_wave_decode_vp9(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->dec_config->bitFormat != STD_VP9) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(test_case_wave_decode(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_decode_hevc - test hevc decode with wave-ip,
 * param param  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int test_case_wave_decode_hevc(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->dec_config->bitFormat != STD_HEVC) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(test_case_wave_decode(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_coda_loop_decode_avc - loop decode test for avc
 * param param  test context configuration info
 * return void
 * notes
 */
static int test_case_coda_loop_decode_avc(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->dec_config->bitFormat != STD_AVC) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode_loop(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_loop_decode_vp9 - loop decode test for avc
 * param param  test context configuration info
 * return void
 * notes
 */
static int test_case_wave_loop_decode_vp9(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != WAVE_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->dec_config->bitFormat != STD_VP9) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode_loop(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief test_case_wave_loop_decode_hevc - loop decode test for avc
 * param param  test context configuration info
 * return void
 * notes
 */
static int test_case_wave_loop_decode_hevc(void *param)
{
    struct test_context *context = NULL;
    CHECK_PARAM(param);
    context = (struct test_context *) param;

    if (context->test_cfg->core_id != CODA_CORE_ID) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (!context->test_cfg->is_decode) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    if (context->test_cfg->dec_config->bitFormat != STD_HEVC) {
        log_err("test case parma is invalid \n");
        return RET_CODE_INVALID_PARAM;
    }

    CALL_FUNCTION(sdk_test_vpu_decode_loop(context));
    log_debug("%s : %d \n", __FUNCTION__, __LINE__);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_device_init - test device open,
         coda & wave deivce open, feeder create
 * param context  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_device_init(struct test_context *context)
{
    int ret = 0;

    CHECK_PARAM(context);

    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
        sdk_test_vpu_deinit(context);
        log_err("test case device init&deinit error ret(%d)\n", ret);
        return RET_CODE_FAILURE_INIT_DEINIT;
    }

    sdk_test_vpu_deinit(context);

    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;
}

static int sdk_test_device_deinit(struct test_context *context)
{
    int ret = 0;

    CHECK_PARAM(context);

    if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
        sdk_test_vpu_deinit(context);
        log_err("test case device init&deinit error ret(%d)\n", ret);
        return RET_CODE_FAILURE_INIT_DEINIT;
    }

    sdk_test_vpu_deinit(context);
    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_device_init_deinit - test device destruct,
         coda & wave deivce close, feeder destrory
 * param context  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_device_init_deinit(struct test_context *context)
{
    int i = 0;
    int ret = 0;
    uint32_t times = 0;

    CHECK_PARAM(context);

    if (context->test_cfg->case_test_times == 0)
        times = DEFAULT_TEST_TEIMS;  /* default times */
    else
        times = context->test_cfg->case_test_times;

    for (i = 0; i < times; i++) {
        if (RET_CODE_SUCCESS != (ret = sdk_test_vpu_init(context))) {
            CALL_FUNCTION(sdk_test_vpu_deinit(context));
            log_err("test case device init&deinit error ret(%d)\n", ret);
            return RET_CODE_FAILURE_INIT_DEINIT;
        }

        CALL_FUNCTION(sdk_test_vpu_deinit(context));
    }

    log_debug("test case finish index(%d)\n", context->current_index);
    return RET_CODE_SUCCESS;
}

/*
 * brief sdk_test_case_result - get and print test result
 * param context  test context configuration info
 * return void
 * notes
 */
static void sdk_test_case_result(struct test_context *context)
{
    int i = 0;

    log_debug("\ncase test end, pass: %d fail: %d \n", context->case_success_count, context->case_failure_count);

    for (i = 0; i < context->case_max_count; i++) {
        if (context->case_success_index[i]) {
            log_debug("index:%d pass\n", context->case_success_index[i]);
        }
        else if (context->case_failure_index[i]) {
            log_debug("index:%d fail\n", context->case_failure_index[i]);
        }
    }
}

/*
 * brief case_run - run case function
 * param context  test context configuration info
 * return void
 * notes
 */
static void *case_run(struct test_context *context)
{
    int i = 0;
    int ret = 0;
    int current_index = 0;
    BOOL first = TRUE;

    for (i = 0; i < context->case_max_count ; i++) {
        if ((context->case_index[i] < context->case_max_count)
                && (context->cases_info[context->case_index[i]].case_func)) {
            context->current_index = context->case_index[i];
            current_index = context->current_index;

            if (current_index == 0) {
                /* index 0 case is sample case */
                if (first) {
                    ret = context->cases_info[0].case_func(context);
                    first = FALSE;
                }
                continue;
            }

            log_debug("case index (%d) case description (%s) begin run ...\n", context->current_index, context->cases_info[current_index].description);
            ret = context->cases_info[current_index].case_func(context);

            if (ret != RETCODE_SUCCESS) {
                context->case_failure_count++;
                context->case_failure_index[i] = current_index;
                log_err("case index (%d) case description (%s) run failure\n",
                        context->current_index,
                        context->cases_info[current_index].description);
            }
            else {
                context->case_success_count++;
                context->case_success_index[i] = current_index;
                log_debug("case index (%d) case description: (%s) run success\n",
                          context->current_index,
                          context->cases_info[current_index].description);
            }
        }
    }

    sdk_test_case_result(context);
    return NULL;
}

/*
 * brief sdk_test_cases_info_show - print all case info(index, description) and
 *       user can select one
 * param context  test context configuration info
 * return void
 *
 * notes
 */
static void sdk_test_cases_info_show(struct test_context *context)
{
    int i = 0;

    log_debug("\n############# SDK TEST CASE INFO #############\n");

    for (i = 0; i < (sizeof(test_cases) / sizeof(struct case_info)); i++) {
        log_debug("case index (%d)  case description (%s)\n", i, context->cases_info[i].description);
    }

    context->case_max_count = i;
    log_debug("case test max count (%d)\n", context->case_max_count);
    log_debug("############# SDK TEST CASE INFO #############\n");
}

/*
 * brief sdk_test_case_run - create thread for run test case
 * param context  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes
 */
static int sdk_test_case_run(struct test_context *context)
{
    int ret = RET_CODE_SUCCESS;
    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, (void *)&case_run, (void *)context);

    if (ret != 0) {
        log_err("create thread err value %d\n", ret);
        return RET_CODE_FAILURE_THREAD;
    }

    pthread_join(thread_id, NULL);
    return ret;
}

/*
 * brief sdk_test_case_index_get - get test case index from terminal
 * param context  test context configuration info
 * return 0  success
 *        !0 failure code
 * notes user should input integer separate with ","
 */
static int sdk_test_case_index_get(struct test_context *context)
{
    int index = 0;
    char *arg = NULL;
    char *ptr = NULL;
    char string[64] = {0};

    log_debug("\nplease input run case index(integer variable) and separate with \",\"\n");
    scanf("%s", string);
    ptr = strtok_r(string, ",", &arg);

    while (ptr != NULL && (index < context->case_max_count)) {
        context->case_index[index]  = atoi(ptr);

        if (context->case_index[index] > context->case_max_count) {
            log_debug("input index(%d) is invalid \n", context->case_index[index]);
            context->case_index[index] = 0;
            return RET_CODE_FAILURE_CASE_INDEX;
        }

        log_debug("input test case index(%d)\n", context->case_index[index]);
        index++;
        ptr = strtok_r(NULL, ",", &arg);
    }

    return RET_CODE_SUCCESS;
}

/*
 * brief  sdk_test_param_res_alloc - alloc param memory's struct
 * param  context
 * param cfg user input config info
 * return 0  success
 *         !0 failure code
 * notes
 */
static int sdk_test_param_res_alloc(struct test_context **context, struct test_config *cfg)
{
    struct test_context *ctx = NULL;
    CHECK_PARAM(cfg);
    CHECK_PARAM(context);

    ctx = (struct test_context *) malloc(sizeof(struct test_context));

    if (!ctx) {
        log_err("sdk test alloc memory error\n");
        return RET_CODE_FAILURE_ALLOC;
    }

    memset(ctx, 0, sizeof(struct test_context));

    /* set config param */
    ctx->test_cfg = (struct test_config *)cfg;
    ctx->cases_info = (struct case_info *)&test_cases;
    ctx->case_max_count = MAX_COUNT;
    ctx->current_index = 0;
    ctx->test_cfg->case_test_times = DEFAULT_TEST_TEIMS; /*default*/

    if (cfg->is_decode) {
        ctx->vpu_dec_info = (struct vpu_decode_info *) malloc(sizeof(struct vpu_decode_info));

        if (!ctx->vpu_dec_info) {
            free(ctx);
            ctx = NULL;
            return RET_CODE_FAILURE_ALLOC;
        }

        memset(ctx->vpu_dec_info, 0, sizeof(struct vpu_decode_info));

        if (ctx->test_cfg->core_id == CODA_CORE_ID) {
            ctx->vpu_dec_info->bstream_num = 1;
            ctx->vpu_dec_info->core_id = CODA_CORE_ID;
        }
        else if (ctx->test_cfg->core_id == WAVE_CORE_ID) {
            ctx->vpu_dec_info->bstream_num = 2;
            ctx->vpu_dec_info->core_id = WAVE_CORE_ID;
        }
        else {
            log_err("core id %d is invalid\n", ctx->vpu_dec_info->core_id);
            free(ctx);
            return RET_CODE_FAILURE;
        }

    }
    else if (cfg->is_encode) {
        ctx->vpu_enc_info = (struct vpu_encode_info *) malloc(sizeof(struct vpu_encode_info));

        if (!ctx->vpu_enc_info) {
            free(ctx);
            ctx = NULL;
            return RET_CODE_FAILURE_ALLOC;
        }

        memset(ctx->vpu_enc_info, 0, sizeof(struct vpu_encode_info));
        ctx->vpu_enc_info->core_id = CODA_CORE_ID;
    }
    else {
        log_err("sdk test param error\n");
        free(ctx);
        return RET_CODE_FAILURE;
    }

    *context = ctx;

    return RET_CODE_SUCCESS;
}

/*
 * brief  sdk_test_param_res_free - test param resource free
 * param  context  test configuration struct
 * return void
 * notes
 */
static void sdk_test_param_res_free(struct test_context *context)
{
    if (context) {
        if (context->vpu_dec_info) {
            free(context->vpu_dec_info);
            context->vpu_dec_info = NULL;
        }

        if (context->vpu_enc_info) {
            free(context->vpu_enc_info);
            context->vpu_enc_info = NULL;
        }

        free(context);
        context = NULL;
    }
}

/*******************************************************************************
 *                        Global function definitions                         *
 ******************************************************************************/
/*
 * @brief  sdk test main entry
 * @param  test_cfg  test configuration
 * @return 0  success
 *         !0 failure code
 * @notes
 */
int sdk_test_entry(struct test_config *test_cfg)
{
    int ret = RET_CODE_SUCCESS;
    struct test_context  *context = NULL;

    CHECK_PARAM(test_cfg);
    CALL_FUNCTION(sdk_test_param_res_alloc(&context, test_cfg));
    /* cat test case info */
    sdk_test_cases_info_show(context);

    /* get case index to run */
    if (RET_CODE_SUCCESS != (ret = sdk_test_case_index_get(context))) {
        sdk_test_param_res_free(context);
        log_err("sdk test get case index error ret(%d)\n", ret);
        return ret;
    }

    /* run test assign case */
    ret = sdk_test_case_run(context);
    sdk_test_param_res_free(context);

    return ret;
}

