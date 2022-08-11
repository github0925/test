/*
 * test_case.h
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
#ifndef __TEST_CASE_H__
#define __TEST_CASE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>

#include "main_helper.h"


/*******************************************************************************
 *                        Macro definitions                                   *
 ******************************************************************************/
#define TEST_CASE_DEBUG
#define log_err(...)        do {VLOG(ERR, "Error in function: %s line: %d ", __FUNCTION__,__LINE__);  VLOG(ERR, __VA_ARGS__); } while(0)

#ifdef  TEST_CASE_DEBUG
#define log_debug(...)      do {VLOG(INFO, __VA_ARGS__); /* printf(__VA_ARGS__) */; } while(0)
#else
#define log_debug(...)
#endif

#define CHECK_PARAM(X)    do {if(!(X))  { log_err("param error \n"); return RET_CODE_INVALID_PARAM; }} while(0)
#define CALL_FUNCTION(X)  do {int ret_t; ret_t = (X); if (ret_t != RET_CODE_SUCCESS) { log_err("get ret value %d\n", ret_t); return ret_t; } } while(0)

#define MAX_COUNT                       0x30
#define MAX_BUF_NUM                     0x2
#define ENCODE_SRC_BUF_NUM              0x2
#define CODA_CORE_ID                    0x0
#define WAVE_CORE_ID                    0x1
#define DEFAULT_TEST_TEIMS              0x5

/*******************************************************************************
 *                        Type definitions                                    *
 ******************************************************************************/
/* err code */
enum ret_code {
    RET_CODE_SUCCESS,
    RET_CODE_INVALID_PARAM,
    RET_CODE_FAILURE_THREAD,
    RET_CODE_FAILURE,
    RET_CODE_INVALID_CORE_ID,
    RET_CODE_FAILURE_PRODUCT_ID,
    RET_CODE_FAILURE_ALLOC,
    RET_CODE_FAILURE_LOAD_FW,
    RET_CODE_FAILURE_DEVICE_OPEN,
    RET_CODE_FAILURE_INIT_DEINIT,
    RET_CODE_FAILURE_FEEDER,
    RET_CODE_FAILURE_FRAMEBUFFER_REGISTER,
    RET_CODE_FAILURE_FEED_STREAM,
    RET_CODE_FAILURE_SEQINIT,
    RET_CODE_FAILURE_PPU,
    RET_CODE_FAILURE_TIME_OUT,
    RET_CODE_FAILURE_DEOCDE,
    RET_CODE_FAILURE_SQE_CHANGE,
    RET_CODE_FAILURE_START_FRAME,
    RET_CODE_FAILURE_OUTPUT_INFO,
    RET_CODE_FAILURE_FILE,
    RET_CODE_FAILURE_NO_FB,
    RET_CODE_FAILURE_SEEK,
    RET_CODE_FAILURE_SRMA_CFG,
    RET_CODE_FAILURE_ENC_OPEN_PARAM,
    RET_CODE_FAILURE_ENC_INIT,
    RET_CODE_FAILURE_READER,
    RET_CODE_FAILURE_ENCODE_SEQINIT,
    RET_CODE_FAILURE_ENCODE,
    RET_CODE_FAILURE_CASE_INDEX,
    RET_CODE_FAILURE_SEEK_PARAM,
    RET_CODE_FAILURE_LOOP,
    RET_CODE_DECODE_CONTINUE = 0xff,
    RET_CODE_DECODE_END,
};

/*
 * @brief  struct  case_info
 *         Description of test case information
 * @param  description  case description
 * @param  data  data
 */
struct case_info {
    int (*case_func)(void *param);
    const char *description;
    char *data;
};


/*
 * @brief  struct  vpu_performance
 *         Description of decode & encode times
 * @param  start_us  decoder configuration
 * @param  diff_us  encoder configuration
 * @param  end_time
 * @param  total_us
 * @param  frame_time
 * @param  fps
 */
struct vpu_performance {
    uint64_t start_us;
    uint64_t diff_us;
    uint64_t end_time;
    uint64_t total_us;
    uint64_t frame_time;
    uint64_t fps;
};

/*
 * @brief  struct  vpu_decode_info
 *         Description of current decode information
 * @param  dis_frame_count
 * @param  dec_frame_count
 * @param  bstream_num
 * @param  bstream_index
 * @param  bit_size
 * @param  core_id
 * @param  ts_start
 * @param  ts_end
 * @param  ts_offset
 * @param  bit_code
 * @param  seek_frame_interval
 * @param  save_fp
 */
struct vpu_decode_info {
    uint32_t dis_frame_count;
    uint32_t dec_frame_count;
    uint32_t bstream_num;
    uint32_t bstream_index;
    uint32_t bit_size;
    uint32_t core_id;
    uint32_t loop_times;
    uint64_t ts_start;
    uint64_t ts_end;
    uint64_t ts_offset;
    uint64_t ts_seek;
    uint16_t *bit_code;
    uint32_t seek_frame_interval;
    FILE *save_fp[2];
    char * wave_init_data;
    char *feeder;
    Queue *ppu_queue;
    BOOL ppu_enable;
    BOOL need_stream;
    BOOL wait_post_processing;
    DecHandle handle;
    DecOpenParam  open_param;
    DecInitialInfo sequence_info;
    DecParam dec_param;
    vpu_buffer_t bstream_buf[MAX_BUF_NUM];
    vpu_buffer_t pfb_mem[MAX_REG_FRAME];
    vpu_buffer_t ppu_fb_mem[MAX_REG_FRAME];
    vpu_buffer_t user_data;
    struct vpu_performance performance;
};

/*
 * @brief  struct  vpu_encode_info
 *         Description of current encode information
 * @param  bit_size
 * @param  core_id
 * @param  frames_count
 * @param  bit_code
 * @param  bit_size
 * @param  regframe_buffer_count
 * @param  yuv_feeder
 * @param  bs_reader
 * @param  bstream_buf
 * @param  handle
 * @param  enc_open_param
 * @param  enc_param
 */
struct vpu_encode_info {
    uint32_t bit_size;
    uint32_t core_id;
    uint32_t frames_count;
    uint16_t *bit_code;
    uint32_t regframe_buffer_count;
    void *yuv_feeder;
    BitstreamReader bs_reader;
    vpu_buffer_t bstream_buf;
    EncHandle handle;
    EncOpenParam enc_open_param;
    EncParam enc_param;
    ENC_CFG cfg_param;
    EncInitialInfo enc_initinfo;
    EncOutputInfo enc_output_info;
    FrameBuffer enc_frame_src[ENCODE_SRC_BUF_NUM];
    vpu_buffer_t enc_src_frame_mem[ENCODE_SRC_BUF_NUM];
    vpu_buffer_t register_frame_buffer[MAX_REG_FRAME];
    TiledMapConfig map_config;
    struct vpu_performance performance;
};

/*
 * @brief  struct  test_config
 *         Description of user input info
 * @param  dec_config  decoder configuration
 * @param  enc_config  encoder configuration
 * @param  is_decode  1 decode
 * @param  is_encode  1 encode
 * @param  core_id  device id
 *                  0: coda
 *                  1: wave
 * @param  bstream_size streams buffer size
 * @param  case_times current test times
 */
struct test_config {
    TestDecConfig *dec_config;
    TestEncConfig *enc_config;
    int is_decode;
    int is_encode;
    int core_id;
    int bstream_size;
    int case_test_times;
};

/*
 * @brief  struct  test_context
 *         Description of sdk test context
 * @param  dec_config  decoder configuration
 * @param  enc_config  encoder configuration
 * @param  test_cfg
 * @param  cases_info
 * @param  case_max_count
 * @param  current_index
 */
struct test_context {
    struct test_config *test_cfg;
    struct case_info *cases_info;
    struct vpu_decode_info *vpu_dec_info;
    struct vpu_encode_info *vpu_enc_info;
    int case_max_count;
    int current_index;
    int case_index[MAX_COUNT];
    int case_success_count;
    int case_failure_count;
    int case_failure_index[MAX_COUNT];
    int case_success_index[MAX_COUNT];
};

/*******************************************************************************
 *                        Global function declarations                        *
 ******************************************************************************/
/*
 * @brief  sdk test main entry
 * @param  test_cfg  test configuration
 * @return 0  success
 *         !0 failure
 * @note
 */
int sdk_test_entry(struct test_config *test_cfg);

#ifdef __cplusplus
}
#endif

#endif  /* End __TEST_CASE_H__ */

