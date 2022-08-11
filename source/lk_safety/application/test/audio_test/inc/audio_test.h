#ifndef __AUDIO_TEST_H__
#define __AUDIO_TEST_H__

#include <audio_common.h>
#include <stdbool.h>

void sd_audio_test_usage(void);

void sd_audio_show_test_info(pcm_params_t pcm_info);

void sd_audio_show_test_info_by_casenum(u32 casenum);

pcm_params_t sd_audio_get_test_params(u32 casenum);

pcm_params_t sd_audio_get_symm_test_params(u32 casenum);

bool sd_audio_test_transfer_result(u8 *src, u8 *dst, int len);

#endif